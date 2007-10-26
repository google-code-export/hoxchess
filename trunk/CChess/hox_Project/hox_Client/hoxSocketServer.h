/////////////////////////////////////////////////////////////////////////////
// Name:            hoxSocketServer.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/25/2007
//
// Description:     The main (only) server socket that handles 
//                  all incoming connection.
/////////////////////////////////////////////////////////////////////////////

#ifndef __HOX_SOCKET_SERVER_H_
#define __HOX_SOCKET_SERVER_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include "hoxEnums.h"
#include "hoxTypes.h"

/* Forward declarations */
class hoxServer;

//
// hoxSocketServer
//
// Perhaps we should rename it to something without the word "socket"
// in it (may be used pipe for local connections...)

class hoxSocketServer : public wxThreadHelper
{
public:
    hoxSocketServer( int        nPort,
                     hoxServer* server );
    ~hoxSocketServer();

    // Thread execution starts here
    virtual void* Entry();

public:
    /* My own API */
    void RequestShutdown() { m_shutdownRequested = true; }

private:
    int               m_nPort;       // The main server's port.
    wxSocketServer*   m_pSServer;    // The main server's socket

    hoxServer*        m_server;

    bool              m_shutdownRequested;
                /* Has a shutdown-request been received? */

};

#endif /* __HOX_SOCKET_SERVER_H_ */
