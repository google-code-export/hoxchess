/***************************************************************************
 *  Copyright 2007, 2008 Huy Phan  <huyphan@playxiangqi.com>               *
 *                                                                         * 
 *  This file is part of HOXChess.                                         *
 *                                                                         *
 *  HOXChess is free software: you can redistribute it and/or modify       *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  HOXChess is distributed in the hope that it will be useful,            *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with HOXChess.  If not, see <http://www.gnu.org/licenses/>.      *
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// Name:            hoxMyPlayer.cpp
// Created:         10/28/2007
//
// Description:     The new "advanced" LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxMyPlayer.h"
#include "hoxNetworkAPI.h"
#include "MyApp.h"      // wxGetApp()
#include "MyFrame.h"
#include "hoxUtility.h"

#include <wx/tokenzr.h>

IMPLEMENT_DYNAMIC_CLASS(hoxMyPlayer, hoxLocalPlayer)

BEGIN_EVENT_TABLE(hoxMyPlayer, hoxLocalPlayer)
    EVT_SOCKET(CLIENT_SOCKET_ID,  hoxMyPlayer::OnIncomingNetworkData)
    EVT_COMMAND(hoxREQUEST_PLAYER_DATA, hoxEVT_CONNECTION_RESPONSE, hoxMyPlayer::OnConnectionResponse_PlayerData)
    EVT_COMMAND(wxID_ANY, hoxEVT_CONNECTION_RESPONSE, hoxMyPlayer::OnConnectionResponse)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxMyPlayer
//-----------------------------------------------------------------------------

hoxMyPlayer::hoxMyPlayer()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxMyPlayer::hoxMyPlayer( const wxString& name,
                          hoxPlayerType   type,
                          int             score )
            : hoxLocalPlayer( name, type, score )
{ 
    const char* FNAME = "hoxMyPlayer::hoxMyPlayer";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxMyPlayer::~hoxMyPlayer() 
{
    const char* FNAME = "hoxMyPlayer::~hoxMyPlayer";
    wxLogDebug("%s: ENTER.", FNAME);
}

void
hoxMyPlayer::OnIncomingNetworkData( wxSocketEvent& event )
{
    const char* FNAME = "hoxMyPlayer::OnIncomingNetworkData";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxRequest* request = new hoxRequest( hoxREQUEST_PLAYER_DATA, this );
    request->socket      = event.GetSocket();
    request->socketEvent = event.GetSocketEvent();
    this->AddRequestToConnection( request );
}

void 
hoxMyPlayer::OnConnectionResponse_PlayerData( wxCommandEvent& event )
{
    const char* FNAME = "hoxMyPlayer::OnConnectionResponse_PlayerData";
    hoxResult result = hoxRESULT_OK;

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    /* Make a note to 'self' that one request has been serviced. */
    DecrementOutstandingRequests();

    /* NOTE: Only handle the connection-lost event. */

    if ( (response->flags & hoxRESPONSE_FLAG_CONNECTION_LOST) !=  0 )
    {
        wxLogDebug("%s: Connection has been lost.", FNAME);
        /* Currently, we support one connection per player.
         * Since this ONLY connection is closed, the player must leave
         * all tables.
         */
        this->LeaveAllTables();
        wxLogDebug("%s: END (lost).", FNAME);
        return;
    }

    /* Handle other type of data. */

    const wxString commandStr = response->content;
    hoxCommand  command;

    result = hoxNetworkAPI::ParseCommand( commandStr, command );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse command-string [%s].", FNAME, commandStr.c_str());
        return;
    }
    wxLogDebug("%s: Received a command [%s].", FNAME, 
        hoxUtility::RequestTypeToString(command.type).c_str());

    const wxString sType    = hoxUtility::RequestTypeToString(command.type);
    const wxString sCode    = command.parameters["code"];
    const wxString sContent = command.parameters["content"];

    switch ( command.type )
    {
        case hoxREQUEST_LOGOUT:
        {
            // TODO: Ignore this event for now.
		    wxLogDebug("%s: *** TODO *** Ignore this LOGOUT's event [%s] for now.", 
			    FNAME, sContent.c_str());
            break;
        }
        case hoxREQUEST_LIST:
        {
		    hoxNetworkTableInfoList* pTableList = new hoxNetworkTableInfoList;
		    result = _ParseNetworkTables( sContent,
					                      *pTableList );
		    if ( result != hoxRESULT_OK )
		    {
			    wxLogDebug("%s: *** WARN *** Failed to parse LIST's response [%s].", 
				    FNAME, sContent.c_str());
			    response->code = result;
		    }

	        // Inform the site.
            std::auto_ptr<hoxNetworkTableInfoList> autoPtr_tablelist( pTableList );  // prevent memory leak!
	        hoxRemoteSite* remoteSite = static_cast<hoxRemoteSite*>( this->GetSite() );
	        remoteSite->DisplayListOfTables( *pTableList );
            break;
        }
        case hoxREQUEST_NEW:
        {
		    std::auto_ptr<hoxNetworkTableInfo> pTableInfo( new hoxNetworkTableInfo() );
		    result = hoxNetworkAPI::ParseOneNetworkTable( sContent,
													      *pTableInfo );
		    if ( result != hoxRESULT_OK )
		    {
			    wxLogDebug("%s: *** WARN *** Failed to parse NEW's event [%s].", 
				    FNAME, sContent.c_str());
                break;
		    }
		    hoxRemoteSite* remoteSite = static_cast<hoxRemoteSite*>( this->GetSite() );
		    remoteSite->JoinNewTable( *pTableInfo );
		    break;
        }
        case hoxREQUEST_LEAVE:
        {
            hoxTable*  table       = NULL;
            hoxPlayer* leavePlayer = NULL;

		    result = _ParsePlayerLeaveEvent( sContent,
										     table, leavePlayer );
		    if ( result != hoxRESULT_OK )
		    {
			    wxLogDebug("%s: Table/Player not found. LEAVE's event [%s] ignored.", 
				    FNAME, sContent.c_str());
                break;
		    }
            wxLogDebug("%s: Player [%s] left Table [%s].", FNAME, 
                leavePlayer->GetName().c_str(), table->GetId().c_str());
            table->OnLeave_FromNetwork( leavePlayer, this );
            break;
        }
        case hoxREQUEST_JOIN:
        {
		    std::auto_ptr<hoxNetworkTableInfo> pTableInfo( new hoxNetworkTableInfo() );
		    result = hoxNetworkAPI::ParseOneNetworkTable( sContent,
													      *pTableInfo );
		    if ( result != hoxRESULT_OK )
		    {
			    wxLogDebug("%s: *** WARN *** Failed to parse JOIN's event [%s].", 
				    FNAME, sContent.c_str());
                break;
		    }
		    hoxRemoteSite* remoteSite = static_cast<hoxRemoteSite*>( this->GetSite() );
		    remoteSite->JoinNewTable( *pTableInfo );
		    break;
        }
        case hoxREQUEST_E_JOIN:
        {
            wxString      tableId;
            wxString      playerId;
            int           nPlayerScore = 0;
            hoxColor joinColor  = hoxCOLOR_NONE; // Default = observer.

		    result = _ParsePlayerJoinEvent( sContent,
									        tableId, playerId, nPlayerScore, joinColor );
		    if ( result != hoxRESULT_OK )
		    {
			    wxLogDebug("%s: Failed to parse E_JOIN's event [%s].",
                    FNAME, sContent.c_str());
                break;
		    }
            wxLogDebug("%s: Player [%s] joined Table [%s] as [%d].", FNAME, 
                playerId.c_str(), tableId.c_str(), joinColor);
            hoxRemoteSite* remoteSite = static_cast<hoxRemoteSite*>( this->GetSite() );
            result = remoteSite->OnPlayerJoined( tableId, 
                                                 playerId, 
                                                 nPlayerScore,
                                                 joinColor );
            if ( result != hoxRESULT_OK )
            {
                wxLogDebug("%s: *** ERROR *** Failed to ask table to join as color [%d].", 
                    FNAME, joinColor);
                break;
            }
            break;
        }
        case hoxREQUEST_MSG:
        {
            hoxTable*     table = NULL;
            wxString      playerId;  // Who sent the message?
            wxString      message;

		    result = _ParsePlayerMsgEvent( sContent,
									       table, playerId, message );
		    if ( result != hoxRESULT_OK )
		    {
			    wxLogDebug("%s: Failed to parse MSG's event [%s].",
                    FNAME, sContent.c_str());
                break;
		    }
            wxLogDebug("%s: Player [%s] sent msg [%s] in Table [%s].", FNAME, 
                playerId.c_str(), message.c_str(), table->GetId().c_str());
            table->OnMessage_FromNetwork( playerId, message );
            break;
        }
        case hoxREQUEST_MOVE:
        {
            hoxTable*     table = NULL;
            hoxPlayer*    movePlayer = NULL;
            wxString      sMove;

		    result = _ParsePlayerMoveEvent( sContent,
									        table, movePlayer, sMove );
		    if ( result != hoxRESULT_OK )
		    {
			    wxLogDebug("%s: Failed to parse MOVE's event [%s].",
                    FNAME, sContent.c_str());
                break;
		    }
            wxLogDebug("%s: Player [%s] sent move [%s] in Table [%s].", FNAME, 
                movePlayer->GetName().c_str(), sMove.c_str(), table->GetId().c_str());
            table->OnMove_FromNetwork( movePlayer, sMove );
            break;
        }
        case hoxREQUEST_DRAW:
        {
            hoxTable*     table = NULL;
            hoxPlayer*    offerPlayer = NULL;

		    result = _ParsePlayerDrawEvent( sContent,
									        table, offerPlayer );
		    if ( result != hoxRESULT_OK )
		    {
			    wxLogDebug("%s: Failed to parse DRAW's event [%s].",
                    FNAME, sContent.c_str());
                break;
		    }

            if ( sCode != "0" ) // failed?
            {
                wxLogDebug("%s: Request [%s] failed with error [%s].", 
                    FNAME, sType.c_str(), sCode.c_str());

                // Post the error on the Board.
                const wxString sMessage = "Draw Request failed with code = " + sCode;
                table->PostSystemMessage( sMessage );
            }
            else // Offer to Draw?
            {
                wxLogDebug("%s: Inform table of player [%s] offering Draw-Request.", 
	                FNAME, offerPlayer->GetName().c_str());
                table->OnDrawRequest_FromNetwork( offerPlayer );
            }
            break;
        }
        case hoxREQUEST_E_END:
        {
            hoxTable*     table = NULL;
            hoxGameStatus gameStatus = hoxGAME_STATUS_UNKNOWN;
            wxString      sReason;

		    result = _ParsePlayerEndEvent( sContent,
									       table, gameStatus, sReason );
		    if ( result != hoxRESULT_OK )
		    {
			    wxLogDebug("%s: Failed to parse E_END's event [%s].",
                    FNAME, sContent.c_str());
                break;
		    }

            wxLogDebug("%s: The game has ended. Status = [%s]. Reason = [%s]",
                FNAME, hoxUtility::GameStatusToString( gameStatus ).c_str(), sReason.c_str());
            table->OnGameOver_FromNetwork( this, gameStatus );
            break;
        }
        default:
        {
		    wxLogDebug("%s: *** WARN *** Unsupported command-type [%s].", 
			    FNAME, sType.c_str());
        }
    } // switch()

    wxLogDebug("%s: END.", FNAME);
}

void 
hoxMyPlayer::OnConnectionResponse( wxCommandEvent& event )
{
    const char* FNAME = "hoxMyPlayer::OnConnectionResponse";
    hoxResult   result;

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    /* Make a note to 'self' that one request has been serviced. */
    DecrementOutstandingRequests();

    const wxString sType = hoxUtility::RequestTypeToString(response->type);

	switch ( response->type )
	{
        case hoxREQUEST_LOGIN:
		{
            result = this->HandleResponseEvent_Connect(event);
			if ( result != hoxRESULT_OK )
			{
				wxLogDebug("%s: *** WARN *** Failed to handle [%s] 's response [%s].", 
                    FNAME, sType.c_str(), response->content.c_str());
				response->code = result;
			}
            break;
        }
        case hoxREQUEST_LOGOUT:
        {
            wxLogDebug("%s: Informing the sender about [%s] 's event.", FNAME, sType.c_str());
            break;
        }
        case hoxREQUEST_MSG:    /* fall-through */
        case hoxREQUEST_MOVE:   /* fall-through */
        case hoxREQUEST_DRAW:
        {
            wxLogDebug("%s: Received [%s] 's event. Do nothing.", FNAME, sType.c_str());
            break;
        }
        case hoxREQUEST_LIST:    /* fall-through */
        case hoxREQUEST_NEW:     /* fall-through */
        case hoxREQUEST_LEAVE:   /* fall-through */
        case hoxREQUEST_JOIN:
		{
			/* NOTE: This command is not done yet. 
			 * We still need to wait for server's response...
			 */
			if ( response->sender && response->sender != this )
			{
				wxLogDebug("%s: Delay informing sender of [%s] 's response.", FNAME, sType.c_str());
				response->sender = NULL;  // TODO: Temporarily clear out sender to skip sending...
			}
			break;
        }
		default:
			wxLogDebug("%s: *** WARN *** Unsupported request-type [%s].", 
				FNAME, hoxUtility::RequestTypeToString(response->type));
			break;
	} // switch


	/* Post event to the sender if it is not THIS player */

    if ( response->sender && response->sender != this )
    {
        wxEvtHandler* sender = response->sender;
        response.release();
        wxPostEvent( sender, event );
    }

    wxLogDebug("%s: The response is OK.", FNAME);
}

hoxResult
hoxMyPlayer::_ParseNetworkTables( const wxString&          responseStr,
                                   hoxNetworkTableInfoList& tableList )
{
    const char* FNAME = "hoxMyPlayer::_ParseNetworkTables";
    hoxResult  result = hoxRESULT_ERR;

    wxLogDebug("%s: ENTER.", FNAME);

	wxStringTokenizer tkz( responseStr, "\n" );
    hoxNetworkTableInfo tableInfo;

    tableList.clear();
   
	while ( tkz.HasMoreTokens() )
	{
        wxString token = tkz.GetNextToken();

        hoxNetworkAPI::ParseOneNetworkTable(token, tableInfo);
		tableList.push_back( tableInfo );
    }

	return hoxRESULT_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerLeaveEvent( const wxString& sContent,
                                     hoxTable*&      table,
                                     hoxPlayer*&     player )
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerLeaveEvent";
    const wxString tableId = sContent.BeforeFirst(';');
    const wxString playerId = sContent.AfterFirst(';');

    table = NULL;
    player = NULL;

    /* Lookup Table. */
    table = this->GetSite()->FindTable( tableId );
    if ( table == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRESULT_NOT_FOUND;
    }

    /* Lookup Player. */
    player = this->GetSite()->FindPlayer( playerId );
    if ( player == NULL ) 
    {
        wxLogDebug("%s: Player [%s] not found.", FNAME, playerId.c_str());
        return hoxRESULT_NOT_FOUND;
    }

	return hoxRESULT_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerJoinEvent( const wxString& sContent,
                                    wxString&       tableId,
                                    wxString&       playerId,
                                    int&            nPlayerScore,
                                    hoxColor&  color)
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerJoinEvent";

    nPlayerScore = 0;
    color        = hoxCOLOR_NONE; // Default = observer.

    /* Parse the input string. */

	// ... Do not return empty tokens
	wxStringTokenizer tkz( sContent, ";", wxTOKEN_STRTOK );
	int tokenPosition = 0;
	wxString token;

	while ( tkz.HasMoreTokens() )
	{
		token = tkz.GetNextToken();
		switch ( tokenPosition++ )
		{
			case 0: tableId = token;  break;
			case 1: playerId = token;  break;
            case 2: nPlayerScore = ::atoi( token.c_str() ); break; 
            case 3: color = hoxUtility::StringToColor( token ); break;
			default: /* Ignore the rest. */ break;
		}
	}		

	return hoxRESULT_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerMsgEvent( const wxString& sContent,
                                   hoxTable*&      table,
                                   wxString&       playerId,
                                   wxString&       message )
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerMsgEvent";
    wxString tableId;

    table    = NULL;
    playerId = "";
    message  = "";

    /* Parse the input string. */

	// ... Do not return empty tokens
	wxStringTokenizer tkz( sContent, ";", wxTOKEN_STRTOK );
	int tokenPosition = 0;
	wxString token;

	while ( tkz.HasMoreTokens() )
	{
		token = tkz.GetNextToken();
		switch ( tokenPosition++ )
		{
			case 0: tableId  = token;  break;
			case 1: playerId = token;  break;
            case 2: message  = token;  break; 
			default: /* Ignore the rest. */ break;
		}
	}		

    /* Lookup Table. */
    table = this->GetSite()->FindTable( tableId );
    if ( table == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRESULT_NOT_FOUND;
    }

	return hoxRESULT_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerMoveEvent( const wxString& sContent,
                                    hoxTable*&      table,
                                    hoxPlayer*&     player,
                                    wxString&       sMove )
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerMoveEvent";
    wxString tableId;
    wxString playerId;

    table   = NULL;
    player  = NULL;
    sMove   = "";

    /* Parse the input string. */

	// ... Do not return empty tokens
	wxStringTokenizer tkz( sContent, ";", wxTOKEN_STRTOK );
	int tokenPosition = 0;
	wxString token;

	while ( tkz.HasMoreTokens() )
	{
		token = tkz.GetNextToken();
		switch ( tokenPosition++ )
		{
			case 0: tableId  = token;  break;
			case 1: playerId = token;  break;
            case 2: sMove    = token;  break; 
			default: /* Ignore the rest. */ break;
		}
	}		

    /* Lookup Table. */
    table = this->GetSite()->FindTable( tableId );
    if ( table == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRESULT_NOT_FOUND;
    }

    /* Lookup Player. */
    player = this->GetSite()->FindPlayer( playerId );
    if ( player == NULL ) 
    {
        wxLogDebug("%s: Player [%s] not found.", FNAME, playerId.c_str());
        return hoxRESULT_NOT_FOUND;
    }

	return hoxRESULT_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerDrawEvent( const wxString& sContent,
                                    hoxTable*&      table,
                                    hoxPlayer*&     player )
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerDrawEvent";
    wxString tableId;
    wxString playerId;

    table   = NULL;
    player  = NULL;

    /* Parse the input string. */

	// ... Do not return empty tokens
	wxStringTokenizer tkz( sContent, ";", wxTOKEN_STRTOK );
	int tokenPosition = 0;
	wxString token;

	while ( tkz.HasMoreTokens() )
	{
		token = tkz.GetNextToken();
		switch ( tokenPosition++ )
		{
			case 0: tableId    = token;  break;
            case 1: playerId   = token;  break;
			default: /* Ignore the rest. */ break;
		}
	}		

    /* Lookup Table. */
    table = this->GetSite()->FindTable( tableId );
    if ( table == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRESULT_NOT_FOUND;
    }

    /* Lookup Player. */
    player = this->GetSite()->FindPlayer( playerId );
    if ( player == NULL ) 
    {
        wxLogDebug("%s: Player [%s] not found.", FNAME, playerId.c_str());
        return hoxRESULT_NOT_FOUND;
    }

	return hoxRESULT_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerEndEvent( const wxString& sContent,
                                   hoxTable*&      table,
                                   hoxGameStatus&  gameStatus,
                                   wxString&       sReason )
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerEndEvent";
    wxString tableId;
    wxString sStatus;  // Game-status.

    table      = NULL;
    gameStatus = hoxGAME_STATUS_UNKNOWN;
    sReason    = "";

    /* Parse the input string. */

	// ... Do not return empty tokens
	wxStringTokenizer tkz( sContent, ";", wxTOKEN_STRTOK );
	int tokenPosition = 0;
	wxString token;

	while ( tkz.HasMoreTokens() )
	{
		token = tkz.GetNextToken();
		switch ( tokenPosition++ )
		{
			case 0: tableId  = token;  break;
            case 1: sStatus  = token;  break;
            case 2: sReason  = token;  break; 
			default: /* Ignore the rest. */ break;
		}
	}		

    /* Lookup Table. */
    table = this->GetSite()->FindTable( tableId );
    if ( table == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRESULT_NOT_FOUND;
    }

    /* Convert game-status from the string ... */
    gameStatus = hoxUtility::StringToGameStatus( sStatus );

	return hoxRESULT_OK;
}

/************************* END OF FILE ***************************************/
