/***************************************************************************
 *  Copyright 2007 Huy Phan  <huyphan@playxiangqi.com>                     *
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
// Name:            hoxServer.h
// Created:         10/24/2007
//
// Description:     The Server Thread to help this server dealing with
//                  network connections.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_SERVER_H_
#define __INCLUDED_HOX_SERVER_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include "hoxEnums.h"
#include "hoxTypes.h"
#include "hoxSite.h"

/* Server (response) event-type. */
DECLARE_EVENT_TYPE(hoxEVT_SERVER_RESPONSE, wxID_ANY)

class hoxSocketServer;

// ----------------------------------------------------------------------------
// hoxServer
// ----------------------------------------------------------------------------

/**
 * The server-component managed all remote connections.
 * New connections are arriving from the other server-component,
 * namely hoxSocketServer.
 * 
 * @note If deriving from wxThread, acquiring mutex (using lock) would fail.
 *       Thus, I have to derive from wxThreadHelper.
 *
 * @see hoxSocketServer
 */
class hoxServer : public wxThreadHelper
{
public:
    hoxServer(hoxSite* site);
    ~hoxServer();

    // Thread execution starts here
    virtual void* Entry();

public:
    // **** My own public API ****
	
	hoxResult StartServer(int nPort);
	void      CloseServer();
    
	bool AddRequest( hoxRequest* request );

    hoxSite* GetSite() const { return m_site; }

private:
    hoxRequest* _GetRequest();         
    void        _HandleRequest( hoxRequest* request );
    hoxResult   _CheckAndHandleSocketLostEvent( const hoxRequest* request, 
                                                wxString&         response );
    hoxResult   _HandleRequest_Accept( hoxRequest* request );

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
    };
    typedef std::list<SocketInfo> SocketList;

	hoxSocketServer*      m_socketServer;

    bool                  m_shutdownRequested;
                /* Has a shutdown-request been received? */

    hoxSite*              m_site;

    SocketList            m_activeSockets;

    wxSemaphore           m_semRequests;
    wxMutex               m_mutexRequests;
    hoxRequestList        m_requests;
};


#endif /* __INCLUDED_HOX_SERVER_H_ */
