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
#include "hoxSite.h"
#include "hoxUtil.h"

#include <wx/tokenzr.h>

IMPLEMENT_DYNAMIC_CLASS(hoxMyPlayer, hoxLocalPlayer)

BEGIN_EVENT_TABLE(hoxMyPlayer, hoxLocalPlayer)
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
            , m_bLoginSuccess( false )
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
hoxMyPlayer::OnConnectionResponse_PlayerData( wxCommandEvent& event )
{
    const char* FNAME = "hoxMyPlayer::OnConnectionResponse_PlayerData";
    hoxResult result = hoxRC_OK;

    wxLogDebug("%s: ENTER.", FNAME);

    const hoxResponse_APtr response( wxDynamicCast(event.GetEventObject(), hoxResponse) );

    hoxSite* site = this->GetSite();
    hoxTable_SPtr  pTable;

    /* Handle error-code. */

    if ( response->code != hoxRC_OK )
    {
        wxLogDebug("%s: *** WARN *** Received error-code [%s].", 
            FNAME, hoxUtil::ResultToStr(response->code));

        /* Close the connection and logout.
         */
        if ( m_bLoginSuccess )
        {
            this->LeaveAllTables();
            this->DisconnectFromNetworkServer();
            site->Handle_ShutdownReadyFromPlayer();
            wxLogDebug("%s: END (exception).", FNAME);
            return;  // *** Exit immediately.
        }
    }

    /* Handle other type of data. */

    const wxString commandStr = response->content;
    hoxCommand  command;

    result = hoxNetworkAPI::ParseCommand( commandStr, command );
    if ( result != hoxRC_OK )
    {
        wxLogError("%s: Failed to parse command-string [%s].", FNAME, commandStr.c_str());
        return;
    }

    const wxString sType    = hoxUtil::RequestTypeToString(command.type);
    const wxString sCode    = command.parameters["code"];
    const wxString sTableId = command.parameters["tid"];
    const wxString sContent = command.parameters["content"];

    wxLogDebug("%s: Received a command [%s].", FNAME, sType.c_str());

    /* Handle the error-code. */
    if ( sCode != "0" ) // failed?
    {
        const wxString sMessage = "Request " + sType + " failed with code = " + sCode;
        wxLogDebug("%s: %s.", FNAME, sMessage.c_str());

        pTable = site->FindTable( sTableId );
        if ( pTable.get() != NULL )
        {
            // Post the error on the Board.
            pTable->PostSystemMessage( sMessage );
        }
        // *** Allow to continue
    }

    switch ( command.type )
    {
        case hoxREQUEST_LOGIN:
		{
            if ( sCode != "0" )  // error?
            {
                wxLogDebug("%s: *** WARN *** Failed to login. Error = [%s: %s].", 
                    FNAME, sCode.c_str(), sContent.c_str());
                response->code = hoxRC_ERR;
                wxLogDebug("%s: *** INFO *** Shutdown connection due to LOGIN failure...", FNAME);
            }
            else
            {
                result = _HandleResponseEvent_LOGIN( sContent );
			    if ( result != hoxRC_OK )
			    {
				    wxLogDebug("%s: *** WARN *** Failed to handle [%s] 's response [%s].", 
                        FNAME, sType.c_str(), sContent.c_str());
				    response->code = result;
                } else {
                    m_bLoginSuccess = true;
                }
            }
            site->OnResponse_LOGIN( response );
            break;
        }
        case hoxREQUEST_LOGOUT:
        {
            site->OnResponse_LOGOUT( response );
            break;
        }
        case hoxREQUEST_LIST:
        {
            std::auto_ptr<hoxNetworkTableInfoList> pTableList( new hoxNetworkTableInfoList );
		    result = _ParseNetworkTables( sContent,
					                      *pTableList );
		    if ( result != hoxRC_OK )
		    {
			    wxLogDebug("%s: *** WARN *** Failed to parse LIST's response [%s].", 
				    FNAME, sContent.c_str());
			    response->code = result;
		    }
	        site->DisplayListOfTables( *pTableList );
            break;
        }
        case hoxREQUEST_I_TABLE:
        {
		    std::auto_ptr<hoxNetworkTableInfo> pTableInfo( new hoxNetworkTableInfo() );
		    result = hoxNetworkAPI::ParseOneNetworkTable( sContent,
													      *pTableInfo );
		    if ( result != hoxRC_OK )
		    {
			    wxLogDebug("%s: *** WARN *** Failed to parse [%s]'s event [%s].", 
				    FNAME, sType.c_str(), sContent.c_str());
                break;
		    }
		    site->JoinNewTable( *pTableInfo );
		    break;
        }
        case hoxREQUEST_LEAVE:
        {
            hoxPlayer* leavePlayer = NULL;

		    result = _ParsePlayerLeaveEvent( sContent,
										     pTable, leavePlayer );
		    if ( result != hoxRC_OK )
		    {
			    wxLogDebug("%s: Table/Player not found. LEAVE's event [%s] ignored.", 
				    FNAME, sContent.c_str());
                break;
		    }
            wxLogDebug("%s: Player [%s] left Table [%s].", FNAME, 
                leavePlayer->GetName().c_str(), pTable->GetId().c_str());
            pTable->OnLeave_FromNetwork( leavePlayer, this );
            break;
        }
        case hoxREQUEST_UPDATE:
        {
            hoxPlayer*    player = NULL;
            hoxTimeInfo   newTimeInfo;

		    result = _ParseTableUpdateEvent( sContent,
									         pTable, player, newTimeInfo );
		    if ( result != hoxRC_OK )
		    {
			    wxLogDebug("%s: Failed to parse [%s]'s event [%s].",
                    FNAME, sType.c_str(), sContent.c_str());
                break;
		    }
            wxLogDebug("%s: Player [%s] updated Timers to [%s] in Table [%s].", FNAME, 
                player->GetName().c_str(), hoxUtil::TimeInfoToString(newTimeInfo).c_str(),
                pTable->GetId().c_str());
            pTable->OnUpdate_FromPlayer( player, newTimeInfo );
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
		    if ( result != hoxRC_OK )
		    {
			    wxLogDebug("%s: Failed to parse E_JOIN's event [%s].",
                    FNAME, sContent.c_str());
                break;
		    }
            wxLogDebug("%s: Player [%s] joined Table [%s] as [%d].", FNAME, 
                playerId.c_str(), tableId.c_str(), joinColor);
            result = site->OnPlayerJoined( tableId, 
                                           playerId, 
                                           nPlayerScore,
                                           joinColor );
            if ( result != hoxRC_OK )
            {
                wxLogDebug("%s: *** ERROR *** Failed to ask table to join as color [%d].", 
                    FNAME, joinColor);
                break;
            }
            break;
        }
        case hoxREQUEST_MSG:
        {
            wxString      playerId;  // Who sent the message?
            wxString      message;

		    result = _ParsePlayerMsgEvent( sContent,
									       pTable, playerId, message );
		    if ( result != hoxRC_OK )
		    {
			    wxLogDebug("%s: Failed to parse MSG's event [%s].",
                    FNAME, sContent.c_str());
                break;
		    }
            wxLogDebug("%s: Player [%s] sent msg [%s] in Table [%s].", FNAME, 
                playerId.c_str(), message.c_str(), pTable->GetId().c_str());
            pTable->OnMessage_FromNetwork( playerId, message );
            break;
        }
        case hoxREQUEST_MOVE:
        {
            hoxPlayer*    movePlayer = NULL;
            wxString      sMove;

		    result = _ParsePlayerMoveEvent( sContent,
									        pTable, movePlayer, sMove );
		    if ( result != hoxRC_OK )
		    {
			    wxLogDebug("%s: Failed to parse MOVE's event [%s].",
                    FNAME, sContent.c_str());
                break;
		    }
            wxLogDebug("%s: Player [%s] sent move [%s] in Table [%s].", FNAME, 
                movePlayer->GetName().c_str(), sMove.c_str(), pTable->GetId().c_str());
            pTable->OnMove_FromNetwork( movePlayer, sMove );
            break;
        }
        case hoxREQUEST_DRAW:
        {
            hoxPlayer*    offerPlayer = NULL;

		    result = _ParsePlayerDrawEvent( sContent,
									        pTable, offerPlayer );
		    if ( result != hoxRC_OK )
		    {
			    wxLogDebug("%s: Failed to parse DRAW's event [%s].",
                    FNAME, sContent.c_str());
                break;
		    }
            wxLogDebug("%s: Inform table of player [%s] offering Draw-Request.", 
                FNAME, offerPlayer->GetName().c_str());
            pTable->OnDrawRequest_FromNetwork( offerPlayer );
            break;
        }
        case hoxREQUEST_RESET:
        {
		    result = _ParsePlayerResetEvent( sContent,
									         pTable );
		    if ( result != hoxRC_OK )
		    {
			    wxLogDebug("%s: Table not found. RESET's event [%s] ignored.", 
				    FNAME, sContent.c_str());
                break;
		    }

		    wxLogDebug("%s: Received RESET's event [%s].", FNAME, sContent.c_str());
            pTable->OnGameReset_FromNetwork();
            break;
        }
        case hoxREQUEST_E_END:
        {
            hoxGameStatus gameStatus = hoxGAME_STATUS_UNKNOWN;
            wxString      sReason;

		    result = _ParsePlayerEndEvent( sContent,
									       pTable, gameStatus, sReason );
		    if ( result != hoxRC_OK )
		    {
			    wxLogDebug("%s: Table not found. E_END's event [%s] ignored.", 
				    FNAME, sContent.c_str());
                break;
		    }

            wxLogDebug("%s: The game has ended. Status = [%s]. Reason = [%s]",
                FNAME, hoxUtil::GameStatusToString( gameStatus ).c_str(), sReason.c_str());
            pTable->OnGameOver_FromNetwork( this, gameStatus );
            break;
        }
        case hoxREQUEST_E_SCORE:
        {
            hoxPlayer*    player = NULL;
            int           nScore = 0;

		    result = _ParsePlayerScoreEvent( sContent,
									         pTable, player, nScore );
		    if ( result != hoxRC_OK )
		    {
			    wxLogDebug("%s: Failed to parse E_SCORE's event [%s].",
                    FNAME, sContent.c_str());
                break;
		    }
            wxLogDebug("%s: Inform table [%s] of player [%s] new Score [%d].", 
                FNAME, pTable->GetId().c_str(), player->GetName().c_str(), nScore);
            player->SetScore( nScore );
            pTable->OnScore_FromNetwork( player );
            break;
        }
        case hoxREQUEST_I_MOVES:
        {
            hoxStringList moves;

		    result = _ParsePastMovesEvent( sContent,
									       pTable, moves );
		    if ( result != hoxRC_OK )
		    {
			    wxLogDebug("%s: Failed to parse I_MOVES's event [%s].",
                    FNAME, sContent.c_str());
                break;
		    }
            wxLogDebug("%s: Inform Table [%s] of past Moves [%s].", FNAME, 
                pTable->GetId().c_str(), sContent.c_str());
            pTable->OnPastMoves_FromNetwork( this, moves );
            break;
        }
        default:
        {
		    wxLogDebug("%s: *** WARN *** Unexpected command [%s].", FNAME, sType.c_str());
        }
    } // switch()

    wxLogDebug("%s: END.", FNAME);
}

void 
hoxMyPlayer::OnConnectionResponse( wxCommandEvent& event )
{
    const char* FNAME = "hoxMyPlayer::OnConnectionResponse";
    wxLogDebug("%s: ENTER.", FNAME);

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     *
     * NOTE: Currently, this function exists to handle errors only.
     *
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    const hoxResponse_APtr apResponse( wxDynamicCast(event.GetEventObject(), hoxResponse) );

    hoxSite* site = this->GetSite();
    
    const wxString sType = hoxUtil::RequestTypeToString(apResponse->type);
    const wxString sContent = apResponse->content;

    switch ( apResponse->type )
    {
        case hoxREQUEST_LOGIN:
		{
            if ( apResponse->code != hoxRC_OK )  // error?
            {
                wxLogDebug("%s: *** WARN *** Failed to login. Error = [%s].", 
                    FNAME, sContent.c_str());
                site->OnResponse_LOGIN( apResponse );
            }
            break;
        }
        default:
        {
		    wxLogWarning("%s: Failed to handle Request [%s]. Error = [%s].", 
                FNAME, sType.c_str(), sContent.c_str());
        }
    }

    wxLogDebug("%s: END.", FNAME);
}

hoxResult 
hoxMyPlayer::_HandleResponseEvent_LOGIN( const wxString& sContent )
{
    const char* FNAME = "hoxMyPlayer::_HandleResponseEvent_LOGIN";
    hoxResult   result;
    wxString    playerId;
	int         nScore = 0;

    wxLogDebug("%s: ENTER.", FNAME);

    result = _ParsePlayerLoginEvent( sContent,
									 playerId,
								     nScore );
    if ( result != hoxRC_OK )
    {
        wxLogDebug("%s: *** WARN *** Failed to parse LOGIN event [%s]. (%d)", 
            FNAME, sContent.c_str(), result);
        return hoxRC_ERR;
    }

    wxLogDebug("%s: LOGIN event: Player Id = [%s], Score = [%d].",
        FNAME, playerId.c_str(), nScore);

    if ( playerId == this->GetName() )
    {
        wxLogDebug("%s: Set my score = [%d].", FNAME, nScore);
        this->SetScore( nScore );
    }
    else
    {
        wxLogDebug("%s: Got LOGIN event from other player [%s].", FNAME, playerId.c_str());
        // NOTE: Do nothing.
    }

    return hoxRC_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerLoginEvent( const wxString& sContent,
                                     wxString&       playerId,
                                     int&            nPlayerScore )
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerLoginEvent";

    playerId     = "";
    nPlayerScore = 0;

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
			case 0: playerId     = token;  break;
            case 1: nPlayerScore = ::atoi( token.c_str() ); break; 
			default: /* Ignore the rest. */ break;
		}
	}		

	return hoxRC_OK;
}

hoxResult
hoxMyPlayer::_ParseNetworkTables( const wxString&          responseStr,
                                   hoxNetworkTableInfoList& tableList )
{
    const char* FNAME = "hoxMyPlayer::_ParseNetworkTables";
    hoxResult  result = hoxRC_ERR;

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

	return hoxRC_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerLeaveEvent( const wxString& sContent,
                                     hoxTable_SPtr&  pTable,
                                     hoxPlayer*&     player )
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerLeaveEvent";
    const wxString tableId = sContent.BeforeFirst(';');
    const wxString playerId = sContent.AfterFirst(';');

    pTable.reset();
    player = NULL;

    /* Lookup Table. */
    pTable = this->GetSite()->FindTable( tableId );
    if ( pTable.get() == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRC_NOT_FOUND;
    }

    /* Lookup Player. */
    player = this->GetSite()->FindPlayer( playerId );
    if ( player == NULL ) 
    {
        wxLogDebug("%s: Player [%s] not found.", FNAME, playerId.c_str());
        return hoxRC_NOT_FOUND;
    }

	return hoxRC_OK;
}

hoxResult
hoxMyPlayer::_ParseTableUpdateEvent( const wxString& sContent,
                                     hoxTable_SPtr&  pTable,
                                     hoxPlayer*&     player,
                                     hoxTimeInfo&    newTimeInfo )
{
    const char* FNAME = "hoxMyPlayer::_ParseTableUpdateEvent";
    wxString tableId;
    wxString playerId;

    pTable.reset();
    player = NULL;

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
            case 2: newTimeInfo = hoxUtil::StringToTimeInfo(token); break; 
			default: /* Ignore the rest. */ break;
		}
	}		

    /* Lookup Table. */
    pTable = this->GetSite()->FindTable( tableId );
    if ( pTable.get() == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRC_NOT_FOUND;
    }

    /* Lookup Player. */
    player = this->GetSite()->FindPlayer( playerId );
    if ( player == NULL ) 
    {
        wxLogDebug("%s: Player [%s] not found.", FNAME, playerId.c_str());
        return hoxRC_NOT_FOUND;
    }

	return hoxRC_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerJoinEvent( const wxString& sContent,
                                    wxString&       tableId,
                                    wxString&       playerId,
                                    int&            nPlayerScore,
                                    hoxColor&       color)
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
            case 3: color = hoxUtil::StringToColor( token ); break;
			default: /* Ignore the rest. */ break;
		}
	}		

	return hoxRC_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerMsgEvent( const wxString& sContent,
                                   hoxTable_SPtr&  pTable,
                                   wxString&       playerId,
                                   wxString&       message )
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerMsgEvent";
    wxString tableId;

    pTable.reset();
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
    pTable = this->GetSite()->FindTable( tableId );
    if ( pTable.get() == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRC_NOT_FOUND;
    }

	return hoxRC_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerMoveEvent( const wxString& sContent,
                                    hoxTable_SPtr&  pTable,
                                    hoxPlayer*&     player,
                                    wxString&       sMove )
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerMoveEvent";
    wxString tableId;
    wxString playerId;

    pTable.reset();
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
    pTable = this->GetSite()->FindTable( tableId );
    if ( pTable.get() == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRC_NOT_FOUND;
    }

    /* Lookup Player. */
    player = this->GetSite()->FindPlayer( playerId );
    if ( player == NULL ) 
    {
        wxLogDebug("%s: Player [%s] not found.", FNAME, playerId.c_str());
        return hoxRC_NOT_FOUND;
    }

	return hoxRC_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerDrawEvent( const wxString& sContent,
                                    hoxTable_SPtr&  pTable,
                                    hoxPlayer*&     player )
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerDrawEvent";
    wxString tableId;
    wxString playerId;

    pTable.reset();
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
    pTable = this->GetSite()->FindTable( tableId );
    if ( pTable.get() == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRC_NOT_FOUND;
    }

    /* Lookup Player. */
    player = this->GetSite()->FindPlayer( playerId );
    if ( player == NULL ) 
    {
        wxLogDebug("%s: Player [%s] not found.", FNAME, playerId.c_str());
        return hoxRC_NOT_FOUND;
    }

	return hoxRC_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerEndEvent( const wxString& sContent,
                                   hoxTable_SPtr&  pTable,
                                   hoxGameStatus&  gameStatus,
                                   wxString&       sReason )
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerEndEvent";
    wxString tableId;
    wxString sStatus;  // Game-status.

    pTable.reset();
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
    pTable = this->GetSite()->FindTable( tableId );
    if ( pTable.get() == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRC_NOT_FOUND;
    }

    /* Convert game-status from the string ... */
    gameStatus = hoxUtil::StringToGameStatus( sStatus );

	return hoxRC_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerResetEvent( const wxString& sContent,
                                     hoxTable_SPtr&  pTable )
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerResetEvent";
    wxString tableId;

    pTable.reset();

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
			default: /* Ignore the rest. */ break;
		}
	}		

    /* Lookup Table. */
    pTable = this->GetSite()->FindTable( tableId );
    if ( pTable.get() == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRC_NOT_FOUND;
    }

	return hoxRC_OK;
}

hoxResult
hoxMyPlayer::_ParsePlayerScoreEvent( const wxString& sContent,
                                     hoxTable_SPtr&  pTable,
                                     hoxPlayer*&     player,
                                     int&            nScore )
{
    const char* FNAME = "hoxMyPlayer::_ParsePlayerScoreEvent";
    wxString tableId;
    wxString playerId;

    pTable.reset();
    player  = NULL;
    nScore  = 0;

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
            case 2: nScore     = ::atoi(token.c_str());  break;
			default: /* Ignore the rest. */ break;
		}
	}		

    /* Lookup Table. */
    pTable = this->GetSite()->FindTable( tableId );
    if ( pTable.get() == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRC_NOT_FOUND;
    }

    /* Lookup Player. */
    player = this->GetSite()->FindPlayer( playerId );
    if ( player == NULL ) 
    {
        wxLogDebug("%s: Player [%s] not found.", FNAME, playerId.c_str());
        return hoxRC_NOT_FOUND;
    }

	return hoxRC_OK;
}

hoxResult
hoxMyPlayer::_ParsePastMovesEvent( const wxString& sContent,
                                   hoxTable_SPtr&  pTable,
                                   hoxStringList&  moves )
{
    const char* FNAME = "hoxMyPlayer::_ParsePastMovesEvent";
    wxString tableId;
    wxString sMoves;

    pTable.reset();
    moves.clear();

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
            case 1: sMoves     = token;  break;
			default: /* Ignore the rest. */ break;
		}
	}		

    /* Lookup Table. */
    pTable = this->GetSite()->FindTable( tableId );
    if ( pTable.get() == NULL )
    {
        wxLogDebug("%s: Table [%s] not found.", FNAME, tableId.c_str());
        return hoxRC_NOT_FOUND;
    }

    /* Parse for the list of Moves. */
    _ParseMovesString( sMoves, moves );

	return hoxRC_OK;
}

hoxResult
hoxMyPlayer::_ParseMovesString( const wxString& sMoves,
                                hoxStringList&  moves )
{
	// ... Do not return empty tokens
	wxStringTokenizer tkz( sMoves, "/", wxTOKEN_STRTOK );
	wxString token;

	while ( tkz.HasMoreTokens() )
	{
		token = tkz.GetNextToken();
        moves.push_back( token );
	}		

	return hoxRC_OK;
}

/************************* END OF FILE ***************************************/
