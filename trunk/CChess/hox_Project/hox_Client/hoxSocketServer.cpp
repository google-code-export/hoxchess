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
// Name:            hoxSocketServer.cpp
// Created:         10/25/2007
//
// Description:     The main (only) server socket that handles 
//                  all incoming connection.
/////////////////////////////////////////////////////////////////////////////

#include "hoxSocketServer.h"
#include "hoxTypes.h"
#include "MyApp.h"    // To access wxGetApp()
#include "hoxServer.h"
#include "hoxNetworkAPI.h"
#include "hoxUtil.h"
#include "hoxRemotePlayer.h"
#include "hoxRemoteConnection.h"
#include "hoxPlayerMgr.h"

//
// hoxSocketServer
//

hoxSocketServer::hoxSocketServer( int        nPort,
                                  hoxServer* server,
                                  hoxSite*   site )
        : wxThreadHelper()
        , m_nPort( nPort )
        , m_pSServer( NULL )
        , m_site( site )
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
        wxLogError("%s: Failed to listen at port [%d].", FNAME, m_nPort);
        m_pSServer->Destroy();
        m_pSServer = NULL;
        return NULL;  // *** Terminate the thread.
    }

    m_pSServer->SetTimeout( hoxSOCKET_SERVER_ACCEPT_TIMEOUT );
    m_pSServer->Notify( false );  // Disable socket-events.

    wxLogDebug("%s: Server listening at port [%d] with timeout = [%d] seconds.", 
        FNAME, m_nPort, hoxSOCKET_SERVER_ACCEPT_TIMEOUT);
    ///////////////////////////////////

    hoxRequest*   request = NULL;
	wxSocketBase* newSock = NULL;

    while ( ! m_shutdownRequested )
    {
        newSock = m_pSServer->Accept( true /* wait */ );

        if ( newSock == NULL )
        {
            //wxLogDebug("%s: Timeout. No new connection.", FNAME);
            continue;  // *** Ignore the error
        }

		wxLogDebug("%s: New client connection accepted.", FNAME);

        /************************************************************
         * Create a handler to handle this socket.
         * Subscribe to 'input' and 'lost-connection' events
         *************************************************************/

        newSock->SetFlags( wxSOCKET_WAITALL );

        wxLogDebug("%s: Set a default time-out = [%d] seconds on new BLOCKING client-socket.", 
            FNAME, hoxSOCKET_CLIENT_SOCKET_TIMEOUT);
        newSock->SetTimeout( hoxSOCKET_CLIENT_SOCKET_TIMEOUT );

        /////////////////////////////////////////////////////
        // The following CONNECT is put here to handle the problem of
        // not being able to connect to the server.
        /////////////////////////////////////////////////////

		if ( hoxRC_OK != _HandleNewConnect( newSock ) )
		{
			wxLogDebug("%s: *** WARN *** Failed to handle new connection.", FNAME);
			continue;
		}

    } // while(...)

    /* Close the server-socket before exit */
    _DestroySocketServer();

    return NULL;
}

hoxResult 
hoxSocketServer::_HandleNewConnect( wxSocketBase* newSock )
{
	const char* FNAME = "hoxSocketServer::_HandleNewConnect";

    wxLogDebug("%s: Turn OFF the socket-events until we finish handling CONNECT...", FNAME);
    newSock->Notify( false );
    wxString playerId;  // Who is sending this request?

    /* Read the incoming command */
    hoxCommand command;
    hoxResult result = hoxNetworkAPI::ReadCommand( newSock, command );
    if ( result != hoxRC_OK )
    {
        wxLogDebug("%s: *** ERROR *** Failed to read incoming command.", FNAME);
        return hoxRC_ERR;
    }

    /* Process the command */
    if ( command.type != hoxREQUEST_LOGIN )
    {
        wxLogDebug("%s: *** ERROR *** Unsupported Request-Type [%s].", 
            FNAME, hoxUtil::RequestTypeToString(command.type).c_str());
        return hoxRC_ERR;
    }

    playerId = command.parameters["pid"];

    const int playerScore = 1999;   // FIXME hard-coded player's score

    /* Create a new player to represent this new remote player */
    wxLogDebug("%s: Creating a remote player [%s]...", FNAME, playerId.c_str());
    hoxRemotePlayer* newPlayer = 
        m_site->m_playerMgr.CreateRemotePlayer( playerId, playerScore );

    /* Create a connection for the new player. */
    hoxRemoteConnection* connection = new hoxRemoteConnection();
    connection->SetServer( m_server );
    connection->SetCBSocket( newSock );
    newPlayer->SetConnection( connection );

    /* Let the player handles all socket events. */
    wxLogDebug("%s: Let this Remote player [%s] handle all socket events", 
        FNAME, newPlayer->GetName().c_str());
    newSock->SetEventHandler(*newPlayer, CLIENT_SOCKET_ID);
    newSock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    newSock->Notify(true);

    // *** Save the connection so that later we can cleanup before closing.
    wxLogDebug("%s: Posting ACCEPT request to save the new client connection.", FNAME);
    hoxRequest* request = new hoxRequest( hoxREQUEST_ACCEPT );
    request->parameters["pid"] = playerId;
    request->socket = newSock;
    m_server->AddRequest( request );

   /* Simply send back an OK response. */

    wxUint32 nWrite;
    wxString response;

    response << "0\r\n"  // code
             << "OK - Accepted\r\n"  // message
             << "sessionId-" << playerId << "\r\n"  // Session-Id
             << playerScore << "\r\n"   // Player-Score
             ;

    nWrite = (wxUint32) response.size();
    newSock->WriteMsg( response, nWrite );
    if ( newSock->LastCount() != nWrite )
    {
        wxLogDebug("%s: *** ERROR *** Writing to socket failed. Error = [%s]", 
            FNAME, hoxNetworkAPI::SocketErrorToString(newSock->LastError()).c_str());
        return hoxRC_ERR;
    }

	return hoxRC_OK;
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
