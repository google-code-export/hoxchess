/////////////////////////////////////////////////////////////////////////////
// Name:            hoxRemoteConnection.h
// Program's Name:  Huy's Open Xiangqi
// Created:         11/05/2007
//
// Description:     The Remote Connection for Remote players.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_REMOTE_CONNECTION_H_
#define __INCLUDED_HOX_REMOTE_CONNECTION_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include "hoxConnection.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/* Forward declarations */
class hoxServer;

// ----------------------------------------------------------------------------
// hoxRemoteConnection
// ----------------------------------------------------------------------------

class hoxRemoteConnection : public hoxConnection
{
public:
    hoxRemoteConnection();
    virtual ~hoxRemoteConnection();

    // **** Override the parent's API ****
    virtual void Start();
    virtual void Shutdown();
    virtual void AddRequest( hoxRequest* request );

    /**
     * Set the Callback socket. 
     */
    hoxResult SetCBSocket( wxSocketBase* socket );

    /**
     * Set the server component that will manage this connection.
     */
    void SetServer( hoxServer* server);

private:
    wxSocketBase*     m_pCBSock; 
        /* Callback socket to help the server.  */

    hoxServer*        m_server;
        /* Set the server component that will manage this connection. */

    DECLARE_DYNAMIC_CLASS(hoxRemoteConnection)
};


#endif /* __INCLUDED_HOX_REMOTE_CONNECTION_H_ */
