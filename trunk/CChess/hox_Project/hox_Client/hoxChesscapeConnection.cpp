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
// Name:            hoxChesscapeConnection.cpp
// Created:         12/12/2007
//
// Description:     The Socket-Connection Thread to help Chesscape player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxChesscapeConnection.h"
#include "hoxLocalPlayer.h"
#include "hoxUtil.h"
#include "hoxNetworkAPI.h"

IMPLEMENT_DYNAMIC_CLASS(hoxChesscapeConnection, hoxSocketConnection)

//-----------------------------------------------------------------------------
// hoxChesscapeWriter
//-----------------------------------------------------------------------------

hoxChesscapeWriter::hoxChesscapeWriter( hoxPlayer*              player,
                                        const hoxServerAddress& serverAddress )
            : hoxSocketWriter( player, serverAddress )
{
}
hoxChesscapeWriter::~hoxChesscapeWriter()
{
}

void 
hoxChesscapeWriter::HandleRequest( hoxRequest_APtr apRequest )
{
    const char* FNAME = "hoxChesscapeWriter::HandleRequest";
    hoxResult    result = hoxRC_ERR;
    hoxResponse_APtr response( new hoxResponse(apRequest->type, 
                                               apRequest->sender) );

    switch( apRequest->type )
    {
        case hoxREQUEST_LOGIN:
		{
			const wxString login = apRequest->parameters["pid"]; 
		    const wxString password = apRequest->parameters["password"];
            result = _Login(login, password, response->content);
            if ( result == hoxRC_HANDLED )
            {
                result = hoxRC_OK;  // Consider "success".
            }
			break;
		}

        case hoxREQUEST_LOGOUT:
		{
			const wxString login = apRequest->parameters["pid"]; 
            result = _Logout(login);
			break;
		}

        case hoxREQUEST_JOIN:
		{
		    const wxString tableId = apRequest->parameters["tid"];
			const bool hasRole = (apRequest->parameters["joined"] == "1");
			const hoxColor requestColor = 
				hoxUtil::StringToColor( apRequest->parameters["color"] );
            result = _Join(tableId, hasRole, requestColor);
			response->content = tableId;
            break;
		}

        case hoxREQUEST_PLAYER_STATUS:
		{
		    const wxString playerStatus = apRequest->parameters["status"];
            result = _UpdateStatus( playerStatus );
            break;
		}

        case hoxREQUEST_LEAVE:
		{
            result = _Leave();
            break;
		}

        case hoxREQUEST_MOVE:
		{
            result = _Move( apRequest );
            break;
		}

        case hoxREQUEST_NEW:
		{
            result = _New();
            break;
		}

        case hoxREQUEST_MSG:
		{
            result = _WallMessage( apRequest );
            break;
		}

        case hoxREQUEST_DRAW:
		{
			const wxString drawResponse = apRequest->parameters["draw_response"];
            result = _Draw( drawResponse );
            break;
		}

        default:
            wxLogDebug("%s: *** WARN *** Unsupported Request [%s].", 
                FNAME, hoxUtil::RequestTypeToString(apRequest->type).c_str());
            result = hoxRC_NOT_SUPPORTED;
            break;
    }

    if ( result != hoxRC_OK )
    {
        wxLogDebug("%s: * INFO * Request [%s]: return error-code = [%s]...", 
            FNAME, hoxUtil::RequestTypeToString(apRequest->type).c_str(), 
            hoxUtil::ResultToStr(result));
    }
}

void
hoxChesscapeWriter::_StartReader( wxSocketClient* socket )
{
    const char* FNAME = "hoxChesscapeWriter::_StartReader";

    wxLogDebug("%s: ENTER.", FNAME);

    if (    m_reader.get() != NULL
         && m_reader->IsRunning() )
    {
        wxLogDebug("%s: The connection has already been started. END.", FNAME);
        return;
    }

    /* Create Reader thread. */
    wxLogDebug("%s: Create the Reader Thread...", FNAME);
    m_reader.reset( new hoxChesscapeReader( m_player ) );

    /* Set the socket to READ from. */
    m_reader->SetSocket( socket );

    if ( m_reader->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogDebug("%s: *** WARN *** Failed to create the Reader thread.", FNAME);
        return;
    }
    wxASSERT_MSG( !m_reader->IsDetached(), 
                  "The Reader thread must be joinable." );

    m_reader->Run();
}

hoxResult
hoxChesscapeWriter::_Login( const wxString& login, 
		                    const wxString& password,
                            wxString&       responseStr )
{
    const char* FNAME = "hoxChesscapeWriter::_Login";

    if ( m_bConnected )
    {
        wxLogDebug("%s: The connection already established. END.", FNAME);
        return hoxRC_HANDLED;
    }

    /* Get the server address. */
    wxIPV4address addr;
    addr.Hostname( m_serverAddress.name );
    addr.Service( m_serverAddress.port );

    wxLogDebug("%s: Trying to connect to [%s]...", FNAME, m_serverAddress.c_str());

    if ( ! m_socket->Connect( addr, true /* wait */ ) )
    {
        wxLogError("%s: Failed to connect to the server [%s]. Error = [%s].",
            FNAME, m_serverAddress.c_str(), 
            hoxNetworkAPI::SocketErrorToString(m_socket->LastError()).c_str());
        return hoxRC_ERR;
    }

    wxLogDebug("%s: Succeeded! Connection established with the server.", FNAME);
    m_bConnected = true;

    //////////////////////////////////
    // Start the READER thread.
    _StartReader( m_socket );

	////////////////////////////
    // Send LOGIN request.
	wxLogDebug("%s: Sending LOGIN request over the network...", FNAME);
	wxString loginRequest;
	loginRequest.Printf("\x02\x10uLogin?0\x10%s\x10%s\x10\x03", login.c_str(), password.c_str());

	return hoxNetworkAPI::WriteLine( m_socket, loginRequest );
}

hoxResult
hoxChesscapeWriter::_Logout( const wxString& login )
{
    const char* FNAME = "hoxChesscapeWriter::_Logout";

    if ( ! this->IsConnected() )
    {
        // NOTE: The connection could have been closed if the server is down.
        wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
        return hoxRC_OK;   // *** Fine. Do nothing.
    }

    /* Send LOGOUT request. */

	wxLogDebug("%s: Sending LOGOUT request for login [%s]...", FNAME, login.c_str());
	wxString cmdRequest;
	cmdRequest.Printf("\x02\x10%s\x10\x03", "logout?");

	return hoxNetworkAPI::WriteLine( m_socket, cmdRequest );
}

hoxResult
hoxChesscapeWriter::_Join( const wxString& tableId,
							   const bool      hasRole,
							   hoxColor   requestColor )
{
    const char* FNAME = "hoxChesscapeWriter::_Join";

    if ( ! this->IsConnected() )
    {
        // NOTE: The connection could have been closed if the server is down.
        wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
        return hoxRC_ERR;
    }

    /* Send JOIN request if the player is NOT in the table. */

	if ( ! hasRole )
	{
		wxLogDebug("%s: Sending JOIN request with table-Id = [%s]...", FNAME, tableId.c_str());
		wxString cmdRequest;
		cmdRequest.Printf("\x02\x10join?%s\x10\x03", tableId.c_str());

		if ( hoxRC_OK != hoxNetworkAPI::WriteLine( m_socket, cmdRequest ) )
		{
			return hoxRC_ERR;
		}
	}

    /* Send REQUEST-SEAT request, if asked. */
	wxString requestSeat;
	if      ( requestColor == hoxCOLOR_RED )   requestSeat = "RedSeat";
	else if ( requestColor == hoxCOLOR_BLACK ) requestSeat = "BlkSeat";

	if ( ! requestSeat.empty() )
	{
		wxLogDebug("%s: Sending REQUEST-SEAT request with seat = [%s]...", FNAME, requestSeat.c_str());
		wxString cmdRequest;
		cmdRequest.Printf("\x02\x10tCmd?%s\x10\x03", requestSeat.c_str());

		if ( hoxRC_OK != hoxNetworkAPI::WriteLine( m_socket, cmdRequest ) )
		{
			return hoxRC_ERR;
		}
	}

    return hoxRC_OK;
}

hoxResult
hoxChesscapeWriter::_UpdateStatus( const wxString& playerStatus )
{
    const char* FNAME = "hoxChesscapeWriter::_UpdateStatus";

    if ( ! this->IsConnected() )
    {
        // NOTE: The connection could have been closed if the server is down.
        wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
        return hoxRC_ERR;
    }

    /* Send UPDATE-STATUS request. */

	wxLogDebug("%s: Sending UPDATE-STATUS request with status = [%s]...", 
		FNAME, playerStatus.c_str());
	wxString cmdRequest;
	cmdRequest.Printf("\x02\x10updateStatus?%s\x10\x03", playerStatus.c_str());

    return hoxNetworkAPI::WriteLine( m_socket, cmdRequest );
}

hoxResult
hoxChesscapeWriter::_Leave()
{
    const char* FNAME = "hoxChesscapeWriter::_Leave";

    if ( ! this->IsConnected() )
    {
        // NOTE: The connection could have been closed if the server is down.
        wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
        return hoxRC_ERR;
    }

    /* Send LEAVE (table) request. */

	wxLogDebug("%s: Sending LEAVE (the current table) request...", FNAME);
	wxString cmdRequest;
	cmdRequest.Printf("\x02\x10%s\x10\x03", "closeTable?");

    return hoxNetworkAPI::WriteLine( m_socket, cmdRequest );
}

hoxResult
hoxChesscapeWriter::_New()
{
    const char* FNAME = "hoxChesscapeWriter::_New";

    if ( ! this->IsConnected() )
    {
        // NOTE: The connection could have been closed if the server is down.
        wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
        return hoxRC_ERR;
    }

    /* Send NEW (table) request. */

	wxLogDebug("%s: Sending NEW (the current table) request...", FNAME);
	wxString cmdRequest;
	cmdRequest.Printf("\x02\x10%s\x10%d\x10\x03", 
		"create?com.chesscape.server.xiangqi.TableHandler",
		0 /* Rated Table */ );

    return hoxNetworkAPI::WriteLine( m_socket, cmdRequest );
}

hoxResult   
hoxChesscapeWriter::_Move( hoxRequest_APtr apRequest )
{
    const char* FNAME = "hoxChesscapeWriter::_Move";

    if ( ! this->IsConnected() )
    {
        // NOTE: The connection could have been closed if the server is down.
        wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
        return hoxRC_ERR;
    }

	/* Extract parameters. */
	const wxString moveStr     = apRequest->parameters["move"];
	const wxString statusStr   = apRequest->parameters["status"];
	const wxString gameTimeStr = apRequest->parameters["game_time"];
	int gameTime = ::atoi( gameTimeStr.c_str() ) * 1000;  // convert to miliseconds

    /* Send MOVE request. */

	wxLogDebug("%s: Sending MOVE [%s] request...", FNAME, moveStr.c_str());
	wxString cmdRequest;
	cmdRequest.Printf("\x02\x10tCmd?Move\x10%s\x10%d\x10\x03", 
		moveStr.c_str(), gameTime);

	if ( hoxRC_OK != hoxNetworkAPI::WriteLine( m_socket, cmdRequest ) )
	{
		return hoxRC_ERR;
	}

	/* Send GAME-STATUS request */
    const hoxGameStatus gameStatus = 
        hoxUtil::StringToGameStatus( statusStr );

	if (   gameStatus == hoxGAME_STATUS_RED_WIN 
        || gameStatus == hoxGAME_STATUS_BLACK_WIN )
	{
		wxLogDebug("%s: Sending GAME-STATUS [%s] request...", FNAME, statusStr.c_str());
		cmdRequest.Printf("\x02\x10tCmd?%s\x10\x03",
			"Winner" /* FIXME: Hard-code */);

		if ( hoxRC_OK != hoxNetworkAPI::WriteLine( m_socket, cmdRequest ) )
		{
			return hoxRC_ERR;
		}
	}

    return hoxRC_OK;
}

hoxResult   
hoxChesscapeWriter::_WallMessage( hoxRequest_APtr apRequest )
{
    const char* FNAME = "hoxChesscapeWriter::_WallMessage";

    if ( ! this->IsConnected() )
    {
        // NOTE: The connection could have been closed if the server is down.
        wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
        return hoxRC_ERR;
    }

	/* Extract parameters. */
	const wxString message = apRequest->parameters["msg"];

    /* Send MESSAGE request. */

	wxLogDebug("%s: Sending MESSAGE [%s] request...", FNAME, message.c_str());
	wxString cmdRequest;
	cmdRequest.Printf("\x02\x10tMsg?%s\x10\x03", message.c_str());

    return hoxNetworkAPI::WriteLine( m_socket, cmdRequest );
}

hoxResult   
hoxChesscapeWriter::_Draw( const wxString& drawResponse )
{
    const char* FNAME = "hoxChesscapeWriter::_Draw";

    if ( ! this->IsConnected() )
    {
        // NOTE: The connection could have been closed if the server is down.
        wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
        return hoxRC_ERR;
    }

	/* Send the response to a DRAW request, if asked.
	 * Otherwise, send DRAW request. 
	 */

	wxString drawCmd;

	if      ( drawResponse == "1" )   drawCmd = "AcceptDraw";
	else if ( drawResponse.empty() )  drawCmd = "OfferDraw";
	else /* ( drawResponse == "0" ) */
	{
		// Send nothing. Done.
		wxLogDebug("%s: DRAW request is denied. Do nothing. END.", FNAME);
		return hoxRC_OK;
	}

	wxLogDebug("%s: Sending DRAW command [%s]...", FNAME, drawCmd.c_str());
	wxString cmdRequest;
	cmdRequest.Printf("\x02\x10tCmd?%s\x10\x03", drawCmd.c_str());

    return hoxNetworkAPI::WriteLine( m_socket, cmdRequest );
}

//-----------------------------------------------------------------------------
// hoxChesscapeReader
//-----------------------------------------------------------------------------

hoxChesscapeReader::hoxChesscapeReader( hoxPlayer* player )
            : hoxSocketReader( player )
{
    const char* FNAME = "hoxChesscapeReader::hoxChesscapeReader";

    wxLogDebug("%s: ENTER.", FNAME);
}

hoxChesscapeReader::~hoxChesscapeReader()
{
}

hoxResult
hoxChesscapeReader::ReadLine( wxSocketBase* sock, 
                              wxString&     result )
{
    const char* FNAME = "hoxChesscapeReader::ReadLine";
    wxString commandStr;

	/* Read a line between '0x02' and '0x03' */

	const wxChar START_CHAR = 0x02;
	const wxChar END_CHAR   = 0x03;
	bool   bStart = false;
    wxChar c;

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
				result = commandStr;
				return hoxRC_OK;  // Done.
			}
            else
            {
                commandStr += c;

                // Impose some limit.
                if ( commandStr.size() >= hoxNETWORK_MAX_MSG_SIZE )
                {
                    wxLogDebug("%s: *** WARN *** Maximum message's size [%d] reached. Likely to be an error.", 
                        FNAME, hoxNETWORK_MAX_MSG_SIZE);
                    wxLogDebug("%s: *** WARN *** Partial read message (64 bytes) = [%s ...].", 
                        FNAME, commandStr.substr(0, 64).c_str());
                    break;
                }
            }
        }
        else if ( sock->Error() )
        {
            wxLogDebug("%s: *** WARN *** Fail to read 1 byte from the network. Error = [%s].", 
                FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
            wxLogDebug("%s: *** WARN *** Result message accumulated so far = [%s].", FNAME, commandStr.c_str());
            break;
        }
    }

    return hoxRC_ERR;
}

//-----------------------------------------------------------------------------
// hoxChesscapeConnection
//-----------------------------------------------------------------------------

hoxChesscapeConnection::hoxChesscapeConnection()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxChesscapeConnection::hoxChesscapeConnection( const hoxServerAddress& serverAddress,
                                                hoxPlayer*              player )
        : hoxSocketConnection( serverAddress, player )
{
    const char* FNAME = "hoxChesscapeConnection::hoxChesscapeConnection";

    wxLogDebug("%s: ENTER.", FNAME);
    wxLogDebug("%s: END.", FNAME);
}

hoxChesscapeConnection::~hoxChesscapeConnection()
{
    const char* FNAME = "hoxChesscapeConnection::~hoxChesscapeConnection";

    wxLogDebug("%s: ENTER.", FNAME);
    wxLogDebug("%s: END.", FNAME);
}

void
hoxChesscapeConnection::StartWriter()
{
    const char* FNAME = "hoxChesscapeConnection::StartWriter";

    wxLogDebug("%s: ENTER.", FNAME);

    if (    m_writer 
         && m_writer->IsRunning() )
    {
        wxLogDebug("%s: The connection has already been started. END.", FNAME);
        return;
    }

    /* Create Writer thread. */
    wxLogDebug("%s: Create the Writer Thread...", FNAME);
    m_writer.reset( new hoxChesscapeWriter( this->GetPlayer(), 
                                            m_serverAddress ) );
    if ( m_writer->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogDebug("%s: *** WARN *** Failed to create the Writer thread.", FNAME);
        return;
    }
    wxASSERT_MSG( !m_writer->IsDetached(), 
                  "The Writer thread must be joinable." );

    m_writer->Run();
}

/************************* END OF FILE ***************************************/
