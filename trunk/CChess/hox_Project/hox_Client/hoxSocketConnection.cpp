/////////////////////////////////////////////////////////////////////////////
// Name:            hoxSocketConnection.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/28/2007
//
// Description:     The Socket-Connection Thread to help MY player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxSocketConnection.h"
#include "hoxMYPlayer.h"
#include "hoxEnums.h"
#include "hoxTableMgr.h"
#include "hoxUtility.h"
#include "hoxNetworkAPI.h"
#include "MyApp.h"    // To access wxGetApp()

//-----------------------------------------------------------------------------
// hoxSocketConnection
//-----------------------------------------------------------------------------


hoxSocketConnection::hoxSocketConnection( const wxString&  sHostname,
                                          int              nPort )
        : hoxConnection( sHostname, nPort )
        , m_pSClient( NULL )
{
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
    const char* FNAME = "hoxSocketConnection::_HandleRequest";
    hoxResult    result = hoxRESULT_ERR;
    std::auto_ptr<hoxResponse> response( new hoxResponse(request->type) );

    switch( request->type )
    {
        case hoxREQUEST_TYPE_CONNECT:
            result = _SendRequest_Connect( request->content, response->content );
            break;

        case hoxREQUEST_TYPE_LISTEN:
            result = _HandleRequest_Listen( request );
            break;

        case hoxREQUEST_TYPE_PLAYER_DATA:
            wxASSERT_MSG( request->socket == m_pSClient, "Sockets should match." );
            result = hoxNetworkAPI::HandlePlayerData( m_pSClient ); 
            break;

        case hoxREQUEST_TYPE_TABLE_MOVE: /* fall through */
        case hoxREQUEST_TYPE_LIST:     /* fall through */
        case hoxREQUEST_TYPE_NEW:      /* fall through */
        case hoxREQUEST_TYPE_JOIN:     /* fall through */
        case hoxREQUEST_TYPE_LEAVE:
            wxASSERT_MSG( m_pSClient, "Connection is not yet established" );
            result = hoxNetworkAPI::SendRequest( m_pSClient, 
                                                 request->content, 
                                                 response->content );
            break;

        default:
            wxLogError("%s: Unsupported request Type [%s].", 
                FNAME, hoxUtility::RequestTypeToString(request->type));
            result = hoxRESULT_NOT_SUPPORTED;
            break;
    }

    /* Log error */
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Error occurred while handling request [%s].", 
            FNAME, hoxUtility::RequestTypeToString(request->type));
        response->content = "!Error_Result!";
    }

    /* NOTE: If there was error, just return it to the caller. */

    if ( request->sender != NULL )
    {
        wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE );
        event.SetInt( result );
        event.SetEventObject( response.release() );  // Caller will de-allocate.
        wxPostEvent( request->sender, event );
    }
}

hoxResult        
hoxSocketConnection::_SendRequest_Connect( const wxString& request, 
                                           wxString&       response )
{
    const char* FNAME = "hoxSocketConnection::_SendRequest_Connect";
    hoxResult result = hoxRESULT_ERR;  // Assume: failure.

    /* Delete the old connection, if any. */
    _Disconnect(); 
    wxCHECK_MSG( !m_pSClient, hoxRESULT_ERR, "Connection should have been closed." );

    /* Make a new connection */

    wxLogDebug("%s: Create a client-socket with default time-out = [%d] seconds.", 
        FNAME, hoxSOCKET_CLIENT_SOCKET_TIMEOUT);

    m_pSClient = new wxSocketClient( /* wxSOCKET_NONE */ wxSOCKET_WAITALL );
    m_pSClient->Notify( false /* Disable socket-events */ );
    m_pSClient->SetTimeout( hoxSOCKET_CLIENT_SOCKET_TIMEOUT );

    // Get the server address.
    wxIPV4address addr;
    addr.Hostname( m_sHostname );
    addr.Service( m_nPort );

#if 0
    wxLogDebug("%s: Trying to connect to [%s:%d] (timeout = 10 sec)...", FNAME, m_sHostname, m_nPort);
    m_pSClient->Connect( addr, false /* no-wait */ );
    m_pSClient->WaitOnConnect( 10 /* wait for 10 seconds */ );

    if ( ! m_pSClient->IsConnected() ) 
    {
        wxLogError("%s: Failed to connect to the server [%s:%d]. Error = [%s].",
                   FNAME, m_sHostname, m_nPort,
                   hoxNetworkAPI::SocketErrorToString(m_pSClient->LastError()));
        goto exit_label;
    }
#endif
    wxLogDebug("%s: Trying to establish a connection to [%s:%d]...", FNAME, m_sHostname, m_nPort);
    if ( ! m_pSClient->Connect( addr, true /* wait */ ) )
    {
        wxLogError("%s: Failed to connect to the server [%s:%d]. Error = [%s].",
            FNAME, m_sHostname, m_nPort, 
            hoxNetworkAPI::SocketErrorToString(m_pSClient->LastError()));
        goto exit_label;
    }

    wxLogDebug("%s: Succeeded! Connection established with the server.", FNAME);

    // Send the request...
    result = hoxNetworkAPI::SendRequest( m_pSClient, 
                                         request,
                                         response );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to send the CONNECT request to the server.", FNAME);
        goto exit_label;
    }

    result = hoxRESULT_OK;  // Finally, success.

exit_label:
    if ( result != hoxRESULT_OK )
    {
        _Disconnect();
    }

    return result;
}

hoxResult   
hoxSocketConnection::_HandleRequest_Listen( hoxRequest*  request )
{
    const char* FNAME = "hoxSocketConnection::_HandleRequest_Listen";

    wxLogDebug("%s: ENTER.", FNAME);

    wxCHECK_MSG( m_pSClient, hoxRESULT_ERR, "Connection is not yet established" );
    wxCHECK_MSG( request->sender, hoxRESULT_ERR, "Sender must not be NULL." );

    // Setup the event handler and let the player handles all socket events.
    m_pSClient->SetEventHandler( *(request->sender), CLIENT_SOCKET_ID );
    m_pSClient->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    m_pSClient->Notify(true);

    // NOTE: Clear the sender since there is no need to send back a response.
    request->sender = NULL;

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
}

/************************* END OF FILE ***************************************/
