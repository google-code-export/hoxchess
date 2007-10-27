/////////////////////////////////////////////////////////////////////////////
// Name:            hoxSocketServer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/25/2007
//
// Description:     The main (only) server socket that handles 
//                  all incoming connection.
/////////////////////////////////////////////////////////////////////////////

#include "hoxSocketServer.h"
#include "hoxTypes.h"
#include "MyApp.h"    // To access wxGetApp()
#include "hoxServer.h"

//
// hoxSocketServer
//

hoxSocketServer::hoxSocketServer( int        nPort,
                                  hoxServer* server )
        : wxThreadHelper()
        , m_nPort( nPort )
        , m_pSServer( NULL )
        , m_server( server )
        , m_shutdownRequested( false )
{
    const char* FNAME = "hoxSocketServer::hoxSocketServer";
    wxLogDebug("%s: ENTER.", FNAME);

    wxASSERT_MSG(server != NULL, "The server must be set.");
}

hoxSocketServer::~hoxSocketServer()
{
    _DestroySocketServer();
}

void*
hoxSocketServer::Entry()
{
    const char* FNAME = "hoxSocketServer::Entry";

    wxLogDebug("%s: ENTER.", FNAME);

    ///////////////////////////////////
    // Create the address - defaults to localhost:0 initially
    wxIPV4address addr;
    addr.Service( m_nPort );

    // Create the socket
    m_pSServer = new wxSocketServer( addr );

    // We use Ok() here to see if the server is really listening
    if ( ! m_pSServer->Ok() )
    {
        wxLogError("%s: Failed to listen at the port [%d].", FNAME, m_nPort);
        m_pSServer->Destroy();
        m_pSServer = NULL;
    }

    m_pSServer->SetTimeout( hoxSOCKET_SERVER_ACCEPT_TIMEOUT );
    m_pSServer->Notify( false );  // Disable socket-events.

    wxLogDebug("%s: Server listening at port [%d] with timeout = [%d] seconds.", 
        FNAME, m_nPort, hoxSOCKET_SERVER_ACCEPT_TIMEOUT);
    ///////////////////////////////////

    hoxRequest* request = NULL;

    while ( ! m_shutdownRequested )
    {
        wxSocketBase* newSock = m_pSServer->Accept( true /* wait */ );

        if ( newSock != NULL )
        {
            wxLogDebug("%s: New client connection accepted.", FNAME);
        }
        else
        {
            //wxLogDebug("%s: Timeout. No new connection.", FNAME);
            continue;  // *** Ignore the error
        }

        /************************************************************
         * Create a handler to handle this socket.
         * Subscribe to 'input' and 'lost-connection' events
         *************************************************************/

        newSock->SetFlags( wxSOCKET_WAITALL );

        wxLogDebug("%s: Set a default time-out = [%d] seconds on new BLOCKING client-socket.", 
            FNAME, hoxSOCKET_CLIENT_SOCKET_TIMEOUT);
        newSock->SetTimeout( hoxSOCKET_CLIENT_SOCKET_TIMEOUT );

        newSock->SetEventHandler( wxGetApp(), SERVER_SOCKET_ID );
        newSock->SetNotify( wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG );
        newSock->Notify( true );

        // *** Save the connection so that later we can cleanup before closing.
        wxLogDebug("%s: Posting ACCEPT request to save the new client connection.", FNAME);
        request = new hoxRequest( hoxREQUEST_TYPE_ACCEPT );
        request->socket = newSock;
        m_server->AddRequest( request );
    }

    /* Close the server-socket before exit */
    _DestroySocketServer();

    return NULL;
}

void 
hoxSocketServer::_DestroySocketServer()
{
    if ( m_pSServer != NULL )
    {
        m_pSServer->Destroy();    
        m_pSServer = NULL;
    }
}

/************************* END OF FILE ***************************************/
