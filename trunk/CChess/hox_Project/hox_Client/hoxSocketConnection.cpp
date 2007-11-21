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
// Name:            hoxSocketConnection.cpp
// Created:         10/28/2007
//
// Description:     The Socket-Connection Thread to help MY player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxSocketConnection.h"
#include "hoxMyPlayer.h"
#include "hoxEnums.h"
#include "hoxUtility.h"
#include "hoxNetworkAPI.h"

IMPLEMENT_DYNAMIC_CLASS(hoxSocketConnection, hoxThreadConnection)

//-----------------------------------------------------------------------------
// hoxSocketConnection
//-----------------------------------------------------------------------------

hoxSocketConnection::hoxSocketConnection()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxSocketConnection::hoxSocketConnection( const wxString&  sHostname,
                                          int              nPort )
        : hoxThreadConnection( sHostname, nPort )
        , m_pSClient( NULL )
{
    const char* FNAME = "hoxSocketConnection::hoxSocketConnection";

    wxLogDebug("%s: Create a client-socket with default time-out = [%d] seconds.", 
        FNAME, hoxSOCKET_CLIENT_SOCKET_TIMEOUT);

    m_pSClient = new wxSocketClient( wxSOCKET_WAITALL );
    m_pSClient->Notify( false /* Disable socket-events */ );
    m_pSClient->SetTimeout( hoxSOCKET_CLIENT_SOCKET_TIMEOUT );
}

hoxSocketConnection::~hoxSocketConnection()
{
    const char* FNAME = "hoxSocketConnection::~hoxSocketConnection";

    wxLogDebug("%s: ENTER.", FNAME);

    _Disconnect();
}

void 
hoxSocketConnection::HandleRequest( hoxRequest* request )
{
    const char* FNAME = "hoxSocketConnection::HandleRequest";
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
            result = hoxNetworkAPI::ReadLine( m_pSClient, response->content );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to read incoming command.", FNAME);
                goto exit_label;
            }
            break;
        }

        case hoxREQUEST_TYPE_OUT_DATA:
            result = hoxNetworkAPI::SendOutData( m_pSClient, 
                                                 request->content );
            break;

        case hoxREQUEST_TYPE_CONNECT:
            result = _Connect();
            if ( result == hoxRESULT_HANDLED )
            {
                result = hoxRESULT_OK;  // Consider "success".
                break;
            }
            else if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to connect to server.", FNAME);
                break;
            }
            /* fall through */

        case hoxREQUEST_TYPE_MOVE:     /* fall through */
        case hoxREQUEST_TYPE_LIST:     /* fall through */
        case hoxREQUEST_TYPE_NEW:      /* fall through */
        case hoxREQUEST_TYPE_JOIN:     /* fall through */
        case hoxREQUEST_TYPE_LEAVE:    /* fall through */
        case hoxREQUEST_TYPE_WALL_MSG:
            if ( ! this->IsConnected() )
            {
                // NOTE: The connection could have been closed if the server is down.
                wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
                result = hoxRESULT_OK;  // Consider "success".
                break;
            }
            result = hoxNetworkAPI::SendRequest( m_pSClient, 
                                                 request->content, 
                                                 response->content );
            break;

        default:
            wxLogError("%s: Unsupported request Type [%s].", 
                FNAME, hoxUtility::RequestTypeToString(request->type).c_str());
            result = hoxRESULT_NOT_SUPPORTED;
            break;
    }

exit_label:
    /* Log error */
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Error occurred while handling request [%s].", 
            FNAME, hoxUtility::RequestTypeToString(request->type).c_str());
        response->content = "!Error_Result!";
    }

    /* NOTE: If there was error, just return it to the caller. */

    //if ( request->sender != NULL )
    {
        wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, request->type );
        response->code = result;
        event.SetEventObject( response.release() );  // Caller will de-allocate.
        wxPostEvent( m_player /*request->sender*/, event );
    }
}

hoxResult 
hoxSocketConnection::_CheckAndHandleSocketLostEvent( 
                                const hoxRequest* request, 
                                wxString&         response )
{
    const char* FNAME = "hoxSocketConnection::_CheckAndHandleSocketLostEvent";
    hoxResult result = hoxRESULT_OK;

    wxLogDebug("%s: ENTER.", FNAME);

    wxASSERT_MSG( request->socket == m_pSClient, "Sockets should match." );

    if ( request->socketEvent == wxSOCKET_LOST )
    {
        wxLogDebug("%s: Received socket-lost event. Deleting client socket.", FNAME);
        _Disconnect();
        result = hoxRESULT_HANDLED;
    }

    wxLogDebug("%s: Not a socket-lost event. Fine - Do nothing. END.", FNAME);
    return result;
}

hoxResult
hoxSocketConnection::_Connect()
{
    const char* FNAME = "hoxSocketConnection::_Connect";

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

    wxCHECK_MSG(m_player, hoxRESULT_ERR, "The player is NULL.");
    wxLogDebug("%s: Let the connection's Player [%s] handle all socket events.", 
        FNAME, m_player->GetName());
    m_pSClient->SetEventHandler( *m_player, CLIENT_SOCKET_ID );
    m_pSClient->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    m_pSClient->Notify(true);

    return hoxRESULT_OK;
}

void
hoxSocketConnection::_Disconnect()
{
    const char* FNAME = "hoxSocketConnection::_Disconnect";

    if ( m_pSClient != NULL )
    {
        wxLogDebug("%s: Close the client socket.", FNAME);
        m_pSClient->Destroy();
        m_pSClient = NULL;
    }
    m_bConnected = false;
}

/************************* END OF FILE ***************************************/
