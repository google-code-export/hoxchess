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
// Name:            hoxRemoteConnection.cpp
// Created:         11/05/2007
//
// Description:     The Remote Connection for Remote players.
/////////////////////////////////////////////////////////////////////////////

#include "hoxRemoteConnection.h"
#include "hoxServer.h"

IMPLEMENT_DYNAMIC_CLASS(hoxRemoteConnection, hoxConnection)

//-----------------------------------------------------------------------------
// hoxRemoteConnection
//-----------------------------------------------------------------------------

hoxRemoteConnection::hoxRemoteConnection()
        : hoxConnection()
        , m_pCBSock( NULL )
        , m_server( NULL )
{
    const char* FNAME = "hoxRemoteConnection::hoxRemoteConnection";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxRemoteConnection::~hoxRemoteConnection()
{
    const char* FNAME = "hoxRemoteConnection::~hoxRemoteConnection";
    wxLogDebug("%s: ENTER.", FNAME);

	/* Disable event-handler for the callback socket because THIS object
	 * is being deleted and can no longer handle socket-events.
	 */
	if ( m_pCBSock != NULL )
	{
		m_pCBSock->SetNotify( false );
	}
}

void 
hoxRemoteConnection::Start()
{
}

void 
hoxRemoteConnection::Shutdown()
{
}

bool 
hoxRemoteConnection::AddRequest( hoxRequest_APtr apRequest )
{
    const char* FNAME = "hoxRemoteConnection::AddRequest";
    wxLogDebug("%s: ENTER.", FNAME);

    wxCHECK_MSG( m_server, false, "The Server component must have been set." );

    /* This type of connection does not need to handle SHUTDOWN. */
    if ( apRequest->type == hoxREQUEST_SHUTDOWN )
    {
        wxLogDebug("%s: Ignore this shutdown request.", FNAME);
        return false;
    }

	if ( m_pCBSock == NULL )
	{
        wxLogDebug("%s: *** INFO *** The Callback socket is not set. Ignore this request.", FNAME);
        return false;
	}

    // Perform sanity check if possible.
    if ( apRequest->socket != NULL )
    {
        wxCHECK_MSG(m_pCBSock == apRequest->socket, false, "The sockets must match.");
    }

    // *************************
    // Simply forward to the server to handle...
    // *************************

    apRequest->socket = m_pCBSock;
    return m_server->AddRequest( apRequest );
}

hoxResult 
hoxRemoteConnection::SetCBSocket( wxSocketBase* socket )
{
	/* Note: This API can also be used to clear Callback-socket. 
	 *       That is, the input socket can be NULL.
	 */

    m_pCBSock = socket;

    return hoxRC_OK;
}

void 
hoxRemoteConnection::SetServer( hoxServer* server) 
{ 
    m_server = server; 
}

/************************* END OF FILE ***************************************/
