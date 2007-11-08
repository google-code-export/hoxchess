/////////////////////////////////////////////////////////////////////////////
// Name:            hoxServer.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/24/2007
//
// Description:     The Server Thread to help this server dealing with
//                  network connections.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_SERVER_H_
#define __INCLUDED_HOX_SERVER_H_

#include "wx/wx.h"
#include "wx/socket.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// hoxServer
// ----------------------------------------------------------------------------

/** !!!!!!!!!!!!!!!!!!!!!!!
 * NOTE: If deriving from wxThread, acquiring mutex (using lock) would fail.
 *       Thus, I have to derive from wxThreadHelper.
 * !!!!!!!!!!!!!!!!!!!!!!! */
class hoxServer : public wxThreadHelper
{
public:
    hoxServer( int nPort );
    ~hoxServer();

    // Thread execution starts here
    virtual void* Entry();

public:
    // **** My own public API ****

    void AddRequest( hoxRequest* request );

private:
    hoxRequest* _GetRequest();         
    void        _HandleRequest( hoxRequest* request );
    hoxResult   _CheckAndHandleSocketLostEvent( const hoxRequest* request, 
                                                wxString&         response );
    hoxResult   _HandleRequest_Accept( hoxRequest* request );

    void        _Disconnect();
    void        _DestroyAllActiveSockets();
    void        _DestroyActiveSocket( wxSocketBase *sock );
    bool        _DetachActiveSocket( wxSocketBase *sock );

    class SocketInfo; // TODO: ...
    bool        _FindSocketInfo( const wxString& playerId, SocketInfo& socketInfo );

private:

    class SocketInfo {
    public:
        wxString       playerId;
        wxSocketBase*  socket;
        SocketInfo() : socket(NULL) {}
        SocketInfo(const wxString& p, wxSocketBase* s) 
                    : playerId(p), socket(s) {}
        SocketInfo& operator=(const SocketInfo& other)
        {
            playerId = other.playerId;
            socket = other.socket;
            return *this;
        }
        //bool operator==(const SocketInfo& other)
        //{
        //    return ( (playerId == other.playerId) && (socket == other.socket) );
        //}
    };
    typedef std::list<SocketInfo> SocketList;

    int                   m_nPort;

    bool                  m_shutdownRequested;
                /* Has a shutdown-request been received? */

    wxSocketServer*       m_pSServer;    // The main server's socket

    SocketList            m_activeSockets;

    wxSemaphore           m_semRequests;
    wxMutex               m_mutexRequests;
    hoxRequestList        m_requests;
};


#endif /* __INCLUDED_HOX_SERVER_H_ */
