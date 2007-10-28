/////////////////////////////////////////////////////////////////////////////
// Name:            hoxSocketConnection.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/28/2007
//
// Description:     The Socket-Connection Thread to help MY player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_SOCKET_CONNECTION_H_
#define __INCLUDED_HOX_SOCKET_CONNECTION_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include "hoxConnection.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// hoxSocketConnection
// ----------------------------------------------------------------------------

class hoxSocketConnection : public hoxConnection
{
public:
    hoxSocketConnection( const wxString& sHostname,
                         int             nPort );
    ~hoxSocketConnection();

    // Thread execution starts here
    virtual void* Entry();

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void AddRequest( hoxRequest* request );

private:
    hoxRequest* _GetRequest();         
    void        _HandleRequest( hoxRequest* request );
    hoxResult   _SendRequest( const wxString& request, wxString& response );
    hoxResult   _SendRequest_Connect( const wxString& request, wxString& response );
    
    hoxResult   _HandleRequest_Listen( hoxRequest* request );
    hoxResult   _HandleRequest_PlayerData( hoxRequest* request );

    hoxResult   _HandleCommand_Move( hoxRequest*       request, 
                                     hoxCommand& command );
    hoxResult   _HandleCommand_TableMove( hoxRequest* request );

    void        _Disconnect();

private:
    bool                  m_shutdownRequested;
                /* Has a shutdown-request been received? */

    wxSocketClient*       m_pSClient;
                /* The socket to handle network connections */

    wxSemaphore           m_semRequests;
    wxMutex               m_mutexRequests;
    hoxRequestList        m_requests;
};


#endif /* __INCLUDED_HOX_SOCKET_CONNECTION_H_ */
