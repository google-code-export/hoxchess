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
#include "hoxPlayer.h"
#include "hoxTable.h"
#include "hoxTableMgr.h"
#include "hoxPlayerMgr.h"
#include "hoxServer.h"
#include <wx/tokenzr.h>

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
    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId, wxThread::IsMain(), FNAME);

    wxASSERT_MSG(server != NULL, "The server must be set.");
}

hoxSocketServer::~hoxSocketServer()
{
    if ( m_pSServer != NULL )
    {
        m_pSServer->Destroy();    
        m_pSServer = NULL;
    }
}

void*
hoxSocketServer::Entry()
{
    const char* FNAME = "hoxSocketServer::Entry";

    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId(), wxThread::IsMain(), FNAME);

    ///////////////////////////////////
    // Create the address - defaults to localhost:0 initially
    wxIPV4address addr;
    addr.Service( m_nPort );

    // Create the socket
    m_pSServer = new wxSocketServer( addr );

    // We use Ok() here to see if the server is really listening
    if ( ! m_pSServer->Ok() )
    {
        wxLogError(wxString::Format("%s: Could not listen at port [%d]!", FNAME, m_nPort));
        m_pSServer->Destroy();
        m_pSServer = NULL;
    }

    const int SERVER_TIME_OUT = 5; /* seconds */
    m_pSServer->SetTimeout( SERVER_TIME_OUT );
    wxLogDebug(wxString::Format("%s: Server listening at port [%d] with timeout = [%d] seconds.", 
                    FNAME, m_nPort, SERVER_TIME_OUT));

    m_pSServer->Notify( false );  // Disable socket-events.
    ///////////////////////////////////

    hoxRequest* request = NULL;

    while ( ! m_shutdownRequested )
    {
        wxSocketBase* newSock = m_pSServer->Accept( true /* wait */ );

        if ( newSock != NULL )
        {
            wxLogDebug(wxString::Format("%s: New client connection accepted.", FNAME));
        }
        else
        {
            //wxLogDebug(wxString::Format("%s: Timeout. No a new connection.", FNAME));
            continue;  // *** Ignore the error
        }

        /************************************************************
         * Create a handler to handle this socket.
         * Subscribe to 'input' and 'lost-connection' events
         *************************************************************/

        newSock->SetEventHandler( wxGetApp(), SERVER_SOCKET_ID );
        newSock->SetNotify( wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG );
        newSock->Notify( true );

        // *** Save the connection so that later we can cleanup before closing.
        wxLogDebug(wxString::Format("%s: Save the new active connection.", FNAME));
        request = new hoxRequest( hoxREQUEST_TYPE_ACCEPT, NULL );
        request->socket = newSock;
        request->content = "";
        m_server->AddRequest( request );
    }

    return NULL;
}


/************************* END OF FILE ***************************************/
