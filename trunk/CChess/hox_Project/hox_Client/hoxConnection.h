/////////////////////////////////////////////////////////////////////////////
// Name:            hoxConnection.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/23/2007
//
// Description:     The Connection Thread to help a "network" player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_CONNECTION_H_
#define __INCLUDED_HOX_CONNECTION_H_

#include "wx/wx.h"
#include "wx/socket.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

class hoxMyPlayer;

// ----------------------------------------------------------------------------
// Connection
// ----------------------------------------------------------------------------

/** !!!!!!!!!!!!!!!!!!!!!!!
 * NOTE: If deriving from wxThread, acquiring mutex (using lock) would fail.
 *       Thus, I have to derive from wxThreadHelper.
 * !!!!!!!!!!!!!!!!!!!!!!! */
class hoxConnection : public wxThreadHelper
{
public:
    hoxConnection( const wxString& sHostname,
                   int             nPort,
                   hoxMyPlayer*    player );
    ~hoxConnection();

    // Thread execution starts here
    virtual void* Entry();

public:
    // **** My own public API ****

    void AddRequest( hoxRequest* request );

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
    wxString              m_sHostname; 
    int                   m_nPort;

    bool                  m_shutdownRequested;
                /* Has a shutdown-request been received? */

    wxSocketClient*       m_pSClient;
                /* The socket to handle network connections */

    wxSemaphore           m_semRequests;
    wxMutex               m_mutexRequests;
    hoxRequestList        m_requests;
};


#endif /* __INCLUDED_HOX_CONNECTION_H_ */
