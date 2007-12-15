/***************************************************************************
 *  Copyright 2007 Huy Phan  <huyphan@playxiangqi.com>                     *
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
#include "hoxMyPlayer.h"
#include "hoxUtility.h"
#include "hoxNetworkAPI.h"

IMPLEMENT_DYNAMIC_CLASS(hoxChesscapeConnection, hoxThreadConnection)

//-----------------------------------------------------------------------------
// hoxChesscapeConnection
//-----------------------------------------------------------------------------

hoxChesscapeConnection::hoxChesscapeConnection()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxChesscapeConnection::hoxChesscapeConnection( const wxString&  sHostname,
                                          int              nPort )
        : hoxThreadConnection( sHostname, nPort )
        , m_pSClient( NULL )
{
    const char* FNAME = "hoxChesscapeConnection::hoxChesscapeConnection";

    wxLogDebug("%s: Create a client-socket with default time-out = [%d] seconds.", 
        FNAME, hoxSOCKET_CLIENT_SOCKET_TIMEOUT);

    m_pSClient = new wxSocketClient( wxSOCKET_WAITALL );
    m_pSClient->Notify( false /* Disable socket-events */ );
    m_pSClient->SetTimeout( hoxSOCKET_CLIENT_SOCKET_TIMEOUT );
}

hoxChesscapeConnection::~hoxChesscapeConnection()
{
    const char* FNAME = "hoxChesscapeConnection::~hoxChesscapeConnection";

    wxLogDebug("%s: ENTER.", FNAME);

    _Disconnect();
}

void 
hoxChesscapeConnection::HandleRequest( hoxRequest* request )
{
    const char* FNAME = "hoxChesscapeConnection::HandleRequest";
    hoxResult    result = hoxRESULT_ERR;
    std::auto_ptr<hoxResponse> response( new hoxResponse(request->type, 
                                                         request->sender) );

    /* 
     * SPECIAL CASE: 
     *     Handle the "special" request: Socket-Lost event,
     *     which is applicable to some requests.
     */
    if ( request->type == hoxREQUEST_TYPE_PLAYER_DATA )
    {
        result = _CheckAndHandleSocketLostEvent( request, response->content );
        if ( result == hoxRESULT_HANDLED )
        {
            response->flags |= hoxRESPONSE_FLAG_CONNECTION_LOST;
            result = hoxRESULT_OK;  // Consider "success".
            goto exit_label;
        }
    }

    /*
     * NORMAL CASE: 
     *    Handle "normal" request.
     */
    switch( request->type )
    {
        case hoxREQUEST_TYPE_PLAYER_DATA: // Incoming data from remote player.
        {
            wxASSERT_MSG( request->socket == m_pSClient, "Sockets should match." );
            // We disable input events until we are done processing the current command.
            hoxNetworkAPI::SocketInputLock socketLock( m_pSClient );
            result = this->_ReadLine( m_pSClient, response->content );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to read incoming command.", FNAME);
                goto exit_label;
            }
            break;
        }

        //case hoxREQUEST_TYPE_OUT_DATA:
        //    result = hoxNetworkAPI::SendOutData( m_pSClient, 
        //                                         request->content );
        //    break;

        case hoxREQUEST_TYPE_CONNECT:
		{
			const wxString login = request->parameters["pid"]; 
		    const wxString password = request->parameters["password"];
            result = _Connect(login, password);
            if ( result == hoxRESULT_HANDLED )
            {
                result = hoxRESULT_OK;  // Consider "success".
            }
			break;
		}

        case hoxREQUEST_TYPE_JOIN:
		{
		    const wxString tableId = request->parameters["tid"];
            result = _Join(tableId);
			response->content = tableId;
            break;
		}

        case hoxREQUEST_TYPE_LEAVE:
		{
            result = _Leave();
            break;
		}

        //case hoxREQUEST_TYPE_MOVE:     /* fall through */
        //case hoxREQUEST_TYPE_LIST:     /* fall through */
        //case hoxREQUEST_TYPE_NEW:      /* fall through */
        //case hoxREQUEST_TYPE_WALL_MSG:
        //    if ( ! this->IsConnected() )
        //    {
        //        // NOTE: The connection could have been closed if the server is down.
        //        wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
        //        result = hoxRESULT_OK;  // Consider "success".
        //        break;
        //    }
        //    result = hoxNetworkAPI::SendRequest( m_pSClient, 
        //                                         request->content, 
        //                                         response->content );
            break;

        default:
            wxLogError("%s: Unsupported request Type [%s].", 
                FNAME, hoxUtility::RequestTypeToString(request->type).c_str());
            result = hoxRESULT_NOT_SUPPORTED;
            break;
    }

exit_label:
    if ( result != hoxRESULT_OK )
    {
        wxLogDebug("%s: *** WARN *** Error occurred while handling request [%s].", 
            FNAME, hoxUtility::RequestTypeToString(request->type).c_str());
        response->content = "!Error_Result!";
    }

    /* NOTE: If there was error, just return it to the caller. */

    wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, request->type );
    response->code = result;
    event.SetEventObject( response.release() );  // Caller will de-allocate.
    wxPostEvent( m_player, event );
}

hoxResult 
hoxChesscapeConnection::_CheckAndHandleSocketLostEvent( 
                                const hoxRequest* request, 
                                wxString&         response )
{
    const char* FNAME = "hoxChesscapeConnection::_CheckAndHandleSocketLostEvent";
    hoxResult result = hoxRESULT_OK;

    //wxLogDebug("%s: ENTER.", FNAME);

    wxASSERT_MSG( request->socket == m_pSClient, "Sockets should match." );

    if ( request->socketEvent == wxSOCKET_LOST )
    {
        wxLogDebug("%s: Received socket-lost event. Deleting client socket.", FNAME);
        _Disconnect();
        result = hoxRESULT_HANDLED;
    }

    return result;
}

hoxResult
hoxChesscapeConnection::_Connect( const wxString& login, 
		                          const wxString& password )
{
    const char* FNAME = "hoxChesscapeConnection::_Connect";

    if ( this->IsConnected() )
    {
        wxLogDebug("%s: The connection already established. END.", FNAME);
        return hoxRESULT_HANDLED;
    }

    /* Get the server address. */
    wxIPV4address addr;
    addr.Hostname( m_sHostname );
    addr.Service( m_nPort );

    wxLogDebug("%s: Trying to connect to [%s:%d]...", 
        FNAME, addr.Hostname().c_str(), addr.Service());

    //m_pSClient->Connect( addr, false /* no-wait */ );
    //m_pSClient->WaitOnConnect( 10 /* wait for 10 seconds */ );

    if ( ! m_pSClient->Connect( addr, true /* wait */ ) )
    {
        wxLogError("%s: Failed to connect to the server [%s:%d]. Error = [%s].",
            FNAME, addr.Hostname().c_str(), addr.Service(), 
            hoxNetworkAPI::SocketErrorToString(m_pSClient->LastError()).c_str());
        return hoxRESULT_ERR;
    }

    wxLogDebug("%s: Succeeded! Connection established with the server.", FNAME);
    this->SetConnected( true );

	////////////////////////////
    // Send LOGIN request.
	{
		wxLogDebug("%s: Sending LOGIN request over the network...", FNAME);
		wxString loginRequest;
		loginRequest.Printf("\x02\x10uLogin?0\x10%s\x10%s\x10\x03", login.c_str(), password.c_str());

		wxUint32 requestSize = (wxUint32) loginRequest.size();
		m_pSClient->Write( loginRequest, requestSize );
		wxUint32 nWrite = m_pSClient->LastCount();
		if ( nWrite < requestSize )
		{
			wxLogDebug("%s: *** WARN *** Failed to send request [%s] ( %d < %d ). Error = [%s].", 
				FNAME, loginRequest.c_str(), nWrite, requestSize, 
				hoxNetworkAPI::SocketErrorToString(m_pSClient->LastError()).c_str());
			return hoxRESULT_ERR;
		}
	}
	////////////////////////////

    wxCHECK_MSG(m_player, hoxRESULT_ERR, "The player is NULL.");
    wxLogDebug("%s: Let the connection's Player [%s] handle all socket events.", 
        FNAME, m_player->GetName());
    m_pSClient->SetEventHandler( *m_player, CLIENT_SOCKET_ID );
    m_pSClient->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    m_pSClient->Notify(true);

    return hoxRESULT_OK;
}

hoxResult
hoxChesscapeConnection::_Join( const wxString& tableId )
{
    const char* FNAME = "hoxChesscapeConnection::_Join";

    if ( ! this->IsConnected() )
    {
        // NOTE: The connection could have been closed if the server is down.
        wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
        return hoxRESULT_ERR;
    }

    /* Send JOIN request. */

	wxLogDebug("%s: Sending JOIN request with table-Id = [%s]...", FNAME, tableId.c_str());
	wxString cmdRequest;
	cmdRequest.Printf("\x02\x10join?%s\x10\x03", tableId.c_str());

	wxUint32 requestSize = (wxUint32) cmdRequest.size();
	m_pSClient->Write( cmdRequest, requestSize );
	wxUint32 nWrite = m_pSClient->LastCount();
	if ( nWrite < requestSize )
	{
		wxLogDebug("%s: *** WARN *** Failed to send request [%s] ( %d < %d ). Error = [%s].", 
			FNAME, cmdRequest.c_str(), nWrite, requestSize, 
			hoxNetworkAPI::SocketErrorToString(m_pSClient->LastError()).c_str());
		return hoxRESULT_ERR;
	}

    return hoxRESULT_OK;
}

hoxResult
hoxChesscapeConnection::_Leave()
{
    const char* FNAME = "hoxChesscapeConnection::_Leave";

    if ( ! this->IsConnected() )
    {
        // NOTE: The connection could have been closed if the server is down.
        wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
        return hoxRESULT_ERR;
    }

    /* Send LEAVE (table) request. */

	wxLogDebug("%s: Sending LEAVE (the current table) request...", FNAME);
	wxString cmdRequest;
	cmdRequest.Printf("\x02\x10%s\x10\x03", "closeTable?");

	wxUint32 requestSize = (wxUint32) cmdRequest.size();
	m_pSClient->Write( cmdRequest, requestSize );
	wxUint32 nWrite = m_pSClient->LastCount();
	if ( nWrite < requestSize )
	{
		wxLogDebug("%s: *** WARN *** Failed to send request [%s] ( %d < %d ). Error = [%s].", 
			FNAME, cmdRequest.c_str(), nWrite, requestSize, 
			hoxNetworkAPI::SocketErrorToString(m_pSClient->LastError()).c_str());
		return hoxRESULT_ERR;
	}

    return hoxRESULT_OK;
}

void
hoxChesscapeConnection::_Disconnect()
{
    const char* FNAME = "hoxChesscapeConnection::_Disconnect";

    if ( m_pSClient != NULL )
    {
        wxLogDebug("%s: Closing the client socket...", FNAME);
        m_pSClient->Destroy();
        m_pSClient = NULL;
    }
    this->SetConnected( false );
}

hoxResult
hoxChesscapeConnection::_ReadLine( wxSocketBase* sock, 
                                   wxString&     result )
{
    const char* FNAME = "hoxChesscapeConnection::_ReadLine";
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
				return hoxRESULT_OK;  // Done.
			}
            else
            {
                commandStr += c;

                // Impose some limit.
                if ( commandStr.size() >= hoxNETWORK_MAX_MSG_SIZE )
                {
                    wxLogError("%s: Maximum message's size [%d] reached. Likely to be an error.", 
                        FNAME, hoxNETWORK_MAX_MSG_SIZE);
                    wxLogError("%s: Partial read message (64 bytes) = [%s ...].", 
                        FNAME, commandStr.substr(0, 64).c_str());
                    break;
                }
            }
        }
        else if ( sock->Error() )
        {
            wxLogWarning("%s: Fail to read 1 byte from the network. Error = [%s].", 
                FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
            wxLogWarning("%s: Result message accumulated so far = [%s].", FNAME, commandStr.c_str());
            break;
        }
    }

    return hoxRESULT_ERR;
}

/************************* END OF FILE ***************************************/
