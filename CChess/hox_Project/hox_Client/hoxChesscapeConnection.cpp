/***************************************************************************
 *  Copyright 2007, 2008, 2009 Huy Phan  <huyphan@playxiangqi.com>         *
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
// Name:            hoxChesscapeConnection.cpp
// Created:         12/12/2007
//
// Description:     The Socket-Connection Thread to help Chesscape player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxChesscapeConnection.h"
#include "hoxPlayer.h"
#include "hoxUtil.h"
#include "hoxNetworkAPI.h"

IMPLEMENT_DYNAMIC_CLASS(hoxChesscapeConnection, hoxSocketConnection)

//-----------------------------------------------------------------------------
// hoxChesscapeWriter
//-----------------------------------------------------------------------------

hoxChesscapeWriter::hoxChesscapeWriter( wxEvtHandler*           player,
                                        const hoxServerAddress& serverAddress )
            : hoxSocketWriter( player, serverAddress )
{
}

void 
hoxChesscapeWriter::HandleRequest( hoxRequest_APtr apRequest )
{
    hoxResult    result = hoxRC_ERR;
    const hoxRequestType requestType = apRequest->type;
    hoxResponse_APtr apResponse( new hoxResponse(requestType, 
                                                 apRequest->sender) );

    /* Make sure the connection is established. */
    if ( ! m_bConnected )
    {
        wxString sError;
        result = this->Connect( sError );
        if ( result != hoxRC_OK )
        {
            wxLogDebug("%s: *WARN* Failed to establish a connection.", __FUNCTION__);
            apResponse->content = sError;
        }
    }

    /* Process the request. */

    switch( requestType )
    {
        case hoxREQUEST_LOGIN:  result = _Login( apRequest ); break;
        case hoxREQUEST_LOGOUT: result = _Logout( apRequest ); break;
        case hoxREQUEST_JOIN:   result = _Join( apRequest ); break;
        case hoxREQUEST_INVITE: result = _Invite( apRequest ); break;
        case hoxREQUEST_PLAYER_INFO:  result = _GetPlayerInfo( apRequest ); break;
        case hoxREQUEST_PLAYER_STATUS: result = _UpdateStatus( apRequest ); break;
        case hoxREQUEST_LEAVE:  result = _Leave( apRequest ); break;
        case hoxREQUEST_MOVE:   result = _Move( apRequest ); break;
        case hoxREQUEST_NEW:    result = _New( apRequest ); break;
        case hoxREQUEST_MSG:    result = _WallMessage( apRequest ); break;
        case hoxREQUEST_UPDATE: result = _Update( apRequest ); break;
        case hoxREQUEST_RESIGN: result = _Resign( apRequest ); break;
        case hoxREQUEST_DRAW:   result = _Draw( apRequest ); break;
        default:
            wxLogDebug("%s: *WARN* Unsupported Request [%s].", 
                __FUNCTION__, hoxUtil::RequestTypeToString(requestType).c_str());
            result = hoxRC_NOT_SUPPORTED;
            break;
    }

    if ( result != hoxRC_OK )
    {
        wxLogDebug("%s: *INFO* Request [%s]: return error-code = [%s]...", 
            __FUNCTION__, hoxUtil::RequestTypeToString(requestType).c_str(), 
            hoxUtil::ResultToStr(result));

        /* Notify the Player of this error. */
        wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, requestType );
        apResponse->code = result;
        event.SetEventObject( apResponse.release() );  // Caller will de-allocate.
        wxPostEvent( m_evtHandler, event );
    }
}

hoxSocketReader_SPtr
hoxChesscapeWriter::CreateReader( wxEvtHandler*   evtHandler,
                                  wxSocketClient* socket )
{
    hoxSocketReader_SPtr reader( new hoxChesscapeReader( evtHandler, socket ) );
    return reader;
}

hoxResult
hoxChesscapeWriter::_Login( hoxRequest_APtr apRequest )
{
    const wxString login = apRequest->parameters["pid"]; 
    const wxString password = apRequest->parameters["password"];

    wxLogDebug("%s: Send LOGIN over the network...", __FUNCTION__);
    wxString loginRequest;
    if ( login.StartsWith( hoxGUEST_PREFIX ) )  // Guest login?
    {
        loginRequest.Printf("gLogin?0\x10%s", login.c_str());
    }
    else
    {
        loginRequest.Printf("uLogin?0\x10%s\x10%s", login.c_str(), password.c_str());
    }

    return _WriteLine( loginRequest );
}

hoxResult
hoxChesscapeWriter::_Logout( hoxRequest_APtr apRequest )
{
	wxString cmdRequest("logout?");
	return _WriteLine( cmdRequest );
}

hoxResult
hoxChesscapeWriter::_Join( hoxRequest_APtr apRequest )
{
    const wxString tableId = apRequest->parameters["tid"];
	const bool bJoined = (apRequest->parameters["joined"] == "1");
	const hoxColor requestColor = 
		hoxUtil::StringToColor( apRequest->parameters["color"] );

    /* Send JOIN request if the player is NOT in the table. */
	if ( ! bJoined )
	{
		wxLogDebug("%s: Send JOIN request with table-Id = [%s]...", __FUNCTION__, tableId.c_str());
		wxString cmdRequest;
		cmdRequest.Printf("join?%s", tableId.c_str());

		if ( hoxRC_OK != _WriteLine( cmdRequest ) )
		{
			return hoxRC_ERR;
		}
	}

    /* Send REQUEST-SEAT request, if asked. */
	wxString requestSeat;
	if      ( requestColor == hoxCOLOR_RED )   requestSeat = "RedSeat";
	else if ( requestColor == hoxCOLOR_BLACK ) requestSeat = "BlkSeat";
    else if (    bJoined  /* NOTE: Assuming having RED or BLACK role!!! */
              && requestColor == hoxCOLOR_NONE )  requestSeat = "StandUp";

	if ( ! requestSeat.empty() )
	{
		wxLogDebug("%s: Send REQUEST-SEAT with seat = [%s]...", __FUNCTION__, requestSeat.c_str());
		wxString cmdRequest;
		cmdRequest.Printf("tCmd?%s", requestSeat.c_str());

		if ( hoxRC_OK != _WriteLine( cmdRequest ) )
		{
			return hoxRC_ERR;
		}
	}

    return hoxRC_OK;
}

hoxResult
hoxChesscapeWriter::_Invite( hoxRequest_APtr apRequest )
{
    const wxString sPlayerId = apRequest->parameters["invitee"];

	wxLogDebug("%s: Send INVITE for player = [%s]...", __FUNCTION__, sPlayerId.c_str());
	wxString cmdRequest;
	cmdRequest.Printf("tCmd?Invite\x10%s", sPlayerId.c_str());

    return _WriteLine( cmdRequest );
}

hoxResult
hoxChesscapeWriter::_GetPlayerInfo( hoxRequest_APtr apRequest )
{
    const wxString sPlayerId = apRequest->parameters["info_pid"];

	wxLogDebug("%s: Send PLAYER-INFO for player = [%s]...", __FUNCTION__, sPlayerId.c_str());
	wxString cmdRequest;
	cmdRequest.Printf("playerInfo?%s", sPlayerId.c_str());

    return _WriteLine( cmdRequest );
}

hoxResult
hoxChesscapeWriter::_UpdateStatus( hoxRequest_APtr apRequest )
{
    const wxString playerStatus = apRequest->parameters["status"];

	wxLogDebug("%s: Send UPDATE-STATUS with status = [%s]...", __FUNCTION__, playerStatus.c_str());
	wxString cmdRequest;
	cmdRequest.Printf("updateStatus?%s", playerStatus.c_str());

    return _WriteLine( cmdRequest );
}

hoxResult
hoxChesscapeWriter::_Leave( hoxRequest_APtr apRequest )
{
	wxLogDebug("%s: Send LEAVE (the current table)...", __FUNCTION__);
	wxString cmdRequest("closeTable?");
    return _WriteLine( cmdRequest );
}

hoxResult
hoxChesscapeWriter::_New( hoxRequest_APtr apRequest )
{
	wxLogDebug("%s: Send NEW table...", __FUNCTION__);
	wxString cmdRequest;
	cmdRequest.Printf("%s\x10%d", 
		"create?com.chesscape.server.xiangqi.TableHandler",
		0 /* Rated Table */ );

    return _WriteLine( cmdRequest );
}

hoxResult   
hoxChesscapeWriter::_Move( hoxRequest_APtr apRequest )
{
	const wxString moveStr     = apRequest->parameters["move"];
	const wxString statusStr   = apRequest->parameters["status"];
	const wxString gameTimeStr = apRequest->parameters["game_time"];
	const int gameTime = ::atoi( gameTimeStr.c_str() ) * 1000;  // convert to miliseconds

    /* Send MOVE request. */
	wxLogDebug("%s: Send MOVE [%s]...", __FUNCTION__, moveStr.c_str());
	wxString cmdRequest;
	cmdRequest.Printf("tCmd?Move\x10%s\x10%d", moveStr.c_str(), gameTime);

	if ( hoxRC_OK != _WriteLine( cmdRequest ) )
	{
		return hoxRC_ERR;
	}

	/* Send GAME-STATUS request */
    const hoxGameStatus gameStatus = hoxUtil::StringToGameStatus( statusStr );

	if (   gameStatus == hoxGAME_STATUS_RED_WIN 
        || gameStatus == hoxGAME_STATUS_BLACK_WIN )
	{
		wxLogDebug("%s: Send GAME-STATUS [%s]...", __FUNCTION__, statusStr.c_str());
		cmdRequest.Printf("tCmd?Winner");  /* NOTE: The "last" player wins */

		if ( hoxRC_OK != _WriteLine( cmdRequest ) )
		{
			return hoxRC_ERR;
		}
	}

    return hoxRC_OK;
}

hoxResult   
hoxChesscapeWriter::_WallMessage( hoxRequest_APtr apRequest )
{
	const wxString message = apRequest->parameters["msg"];

	wxLogDebug("%s: Send MESSAGE [%s]...", __FUNCTION__, message.c_str());
	wxString cmdRequest;
    cmdRequest.Printf("tMsg?%s", message.c_str());

    return _WriteLine( cmdRequest );
}

hoxResult   
hoxChesscapeWriter::_Update( hoxRequest_APtr apRequest )
{
    const wxString sTimes = apRequest->parameters["itimes"];
    hoxTimeInfo timeInfo = hoxUtil::StringToTimeInfo( sTimes );

    int nType     = 0;   // TODO (Private vs. Public)
    int nNotRated = (apRequest->parameters["rated"] != "1");
    int nGameTime = timeInfo.nGame * 1000;
    int nFreeTime = timeInfo.nFree * 1000;

    /* Send UPDATE request. */
    wxLogDebug("%s: Send UPDATE-GAME: itimes = [%s]...", __FUNCTION__, sTimes.c_str());
	wxString cmdRequest;
	cmdRequest.Printf("tCmd?UpdateGame\x10%d\x10%d\x10T\x10%d\x10%d\x10%d\x10%d",
        nType, nNotRated, nGameTime, nFreeTime, nGameTime, nGameTime);

    if ( hoxRC_OK != _WriteLine( cmdRequest ) )
	{
		return hoxRC_ERR;
	}

    /* Also, send the current Role. */
	const hoxColor currentColor = 
		hoxUtil::StringToColor( apRequest->parameters["color"] );

    wxString currentSeat;
	if      ( currentColor == hoxCOLOR_RED )    currentSeat = "RedSeat";
	else if ( currentColor == hoxCOLOR_BLACK )  currentSeat = "BlkSeat";

    cmdRequest.Printf("tCmd?%s", currentSeat.c_str());
    return _WriteLine( cmdRequest );
}

hoxResult
hoxChesscapeWriter::_Resign( hoxRequest_APtr apRequest )
{
	wxLogDebug("%s: Send RESIGN (the current table)...", __FUNCTION__);
	wxString cmdRequest("tCmd?Resign");
    return _WriteLine( cmdRequest );
}

hoxResult   
hoxChesscapeWriter::_Draw( hoxRequest_APtr apRequest )
{
    const wxString drawResponse = apRequest->parameters["draw_response"];

	/* Send the response to a DRAW request, if asked.
	 * Otherwise, send DRAW request. 
	 */

	wxString drawCmd;

	if      ( drawResponse == "1" )   drawCmd = "AcceptDraw";
	else if ( drawResponse.empty() )  drawCmd = "OfferDraw";
	else /* ( drawResponse == "0" ) */
	{
		// Send nothing. Done.
		wxLogDebug("%s: DRAW request is denied. Do nothing. END.", __FUNCTION__);
		return hoxRC_OK;
	}

	wxLogDebug("%s: Send DRAW command [%s]...", __FUNCTION__, drawCmd.c_str());
	wxString cmdRequest;
	cmdRequest.Printf("tCmd?%s", drawCmd.c_str());

    return _WriteLine( cmdRequest );
}

hoxResult
hoxChesscapeWriter::_WriteLine( const wxString& cmdRequest )
{
	wxString sFormattedCmd;

	sFormattedCmd.Printf("\x02\x10%s\x10\x03", cmdRequest.c_str());
    return hoxNetworkAPI::WriteLine( m_socket, sFormattedCmd );
}

//-----------------------------------------------------------------------------
// hoxChesscapeReader
//-----------------------------------------------------------------------------

hoxChesscapeReader::hoxChesscapeReader( wxEvtHandler*   player,
                                        wxSocketClient* socket )
            : hoxSocketReader( player, socket )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

hoxResult
hoxChesscapeReader::ReadLine( wxSocketBase*   sock, 
                              wxMemoryBuffer& data )
{
    data.SetDataLen( 0 );  // Clear old data.

    const size_t maxSize = hoxNETWORK_MAX_MSG_SIZE;

	/* Read a line between '0x02' and '0x03' */

	const wxUint8 START_CHAR = 0x02;
	const wxUint8 END_CHAR   = 0x03;
	bool   bStart = false;
    wxUint8 c;

    for (;;)
    {
        sock->Read( &c, 1 );
        if ( sock->LastCount() == 1 )
        {
			if ( !bStart && c == START_CHAR )
			{
				bStart = true;
			}
			else if ( bStart && c == END_CHAR )
			{
                return hoxRC_OK;  // Done.
			}
            else
            {
                data.AppendByte( c );
                if ( data.GetDataLen() >= maxSize ) // Impose some limit.
                {
                    wxLogDebug("%s: *WARN* Max size [%d] reached.", __FUNCTION__, maxSize);
                    break;
                }
            }
        }
        else if ( sock->Error() )
        {
            wxSocketError err = sock->LastError();
            /* NOTE: This checking is now working given that
             *       we have used wxSOCKET_BLOCK as the socket option.
             */
            if (  err == wxSOCKET_TIMEDOUT )
                wxLogDebug("%s: *INFO* Socket timeout.", __FUNCTION__);
            else
                wxLogDebug("%s: *WARN* Some socket error [%s].", __FUNCTION__,
                    hoxNetworkAPI::SocketErrorToString(err).c_str());
            break;
        }
        else
        {
            wxLogDebug("%s: No more data. Len of data = [%d].", __FUNCTION__, data.GetDataLen());
            return hoxRC_NOT_FOUND;  // Done.
        }
    }

    return hoxRC_ERR;
}

//-----------------------------------------------------------------------------
// hoxChesscapeConnection
//-----------------------------------------------------------------------------

hoxChesscapeConnection::hoxChesscapeConnection( const hoxServerAddress& serverAddress,
                                                wxEvtHandler*           player )
        : hoxSocketConnection( serverAddress, player )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

hoxChesscapeConnection::~hoxChesscapeConnection()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

hoxSocketWriter_SPtr
hoxChesscapeConnection::CreateWriter( wxEvtHandler*           evtHandler,
                                      const hoxServerAddress& serverAddress )
{
    hoxSocketWriter_SPtr writer( new hoxChesscapeWriter( evtHandler, 
                                                         serverAddress ) );
    return writer;
}

/************************* END OF FILE ***************************************/
