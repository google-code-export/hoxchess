/////////////////////////////////////////////////////////////////////////////
// Name:            hoxRemoteConnection.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         11/05/2007
//
// Description:     The Remote Connection for Remote players.
/////////////////////////////////////////////////////////////////////////////

#include "hoxRemoteConnection.h"
#include "hoxEnums.h"
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
}

void 
hoxRemoteConnection::Start()
{
}

void 
hoxRemoteConnection::Shutdown()
{
}

void 
hoxRemoteConnection::AddRequest( hoxRequest* request )
{
    const char* FNAME = "hoxRemoteConnection::AddRequest";
    wxLogDebug("%s: ENTER.", FNAME);

    // *** Simply forward to the server to handle...

    wxASSERT( m_server != NULL );
    {
        // Perform sanity check if possible.
        if ( request->socket != NULL )
        {
            wxCHECK_RET(m_pCBSock == request->socket, "The sockets must match.");
        }
        request->socket = m_pCBSock;
        m_server->AddRequest( request );
    }
}

hoxResult 
hoxRemoteConnection::SetCBSocket( wxSocketBase* socket )
{
    const char* FNAME = "hoxRemoteConnection::SetCBSocket";

    wxCHECK_MSG(m_pCBSock == NULL, hoxRESULT_ERR, "Callback socket already exists.");
    wxCHECK_MSG(socket != NULL, hoxRESULT_ERR, "The socket is NULL.");

    wxLogDebug("%s: Assign callback socket to this connection.", FNAME);
    m_pCBSock = socket;

    return hoxRESULT_OK;
}

void 
hoxRemoteConnection::SetServer( hoxServer* server) 
{ 
    m_server = server; 
}

/************************* END OF FILE ***************************************/
