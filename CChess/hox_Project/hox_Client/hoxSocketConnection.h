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
// Name:            hoxSocketConnection.h
// Created:         10/28/2007
//
// Description:     The Socket-Connection Thread to help MY player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_SOCKET_CONNECTION_H_
#define __INCLUDED_HOX_SOCKET_CONNECTION_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include <boost/shared_ptr.hpp>
#include "hoxThreadConnection.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/* Forward declarations. */
class hoxSocketConnection;
class hoxSocketWriter;
class hoxSocketReader;

/* Typedef(s) */
typedef boost::shared_ptr<hoxSocketWriter> hoxSocketWriter_SPtr;
typedef boost::shared_ptr<hoxSocketReader> hoxSocketReader_SPtr;

// ----------------------------------------------------------------------------
// hoxRequestQueue
// ----------------------------------------------------------------------------

class hoxRequestQueue
{
public:
    hoxRequestQueue();
    ~hoxRequestQueue();

    void            PushBack( hoxRequest_APtr apRequest );
    hoxRequest_APtr PopFront();

private:
    hoxRequestList    m_list;   // The list of requests.
    wxMutex           m_mutex;  // Lock
};

// ----------------------------------------------------------------------------
// hoxSocketWriter
// ----------------------------------------------------------------------------

class hoxSocketWriter : public wxThread
{
public:
    hoxSocketWriter( hoxSocketConnection& owner );
    ~hoxSocketWriter();

    bool AddRequest( hoxRequest_APtr apRequest );

protected:
    // entry point for the thread
    virtual void *Entry();

private:
    void        _StartReader( wxSocketClient* socket );

    hoxRequest_APtr _GetRequest();
    void            _HandleRequest( hoxRequest_APtr apRequest );

    hoxResult _Login( const wxString& sHostname,
                      const int       nPort,
                      const wxString& request,
                      wxString&       response );

    void _Disconnect();

private:
    // the owner of the thread
    hoxSocketConnection&  m_owner;

    wxSocketClient*       m_socket;
                /* The socket to handle network connections */

    hoxSocketReader_SPtr  m_reader;
                /* The Reader Thread. */

    // Storage to hold pending outgoing request.
    wxSemaphore           m_semRequests;
    hoxRequestQueue       m_requests;

    bool                  m_shutdownRequested;
                /* Has a shutdown-request been received? */

    // no copy ctor/assignment operator
    hoxSocketWriter(const hoxSocketWriter&);
    hoxSocketWriter& operator=(const hoxSocketWriter&);
};

// ----------------------------------------------------------------------------
// hoxSocketReader
// ----------------------------------------------------------------------------

class hoxSocketReader : public wxThread
{
public:
    hoxSocketReader( hoxSocketConnection& owner );
    ~hoxSocketReader();

    void SetSocket( wxSocketClient* socket ) { m_socket = socket; }

protected:
    // entry point for the thread
    virtual void *Entry();

private:
    // the owner of the thread
    hoxSocketConnection&  m_owner;

    wxSocketClient*       m_socket;
                /* The socket to handle network connections */

    bool                  m_shutdownRequested;
                /* Has a shutdown-request been received? */

    // no copy ctor/assignment operator
    hoxSocketReader(const hoxSocketReader&);
    hoxSocketReader& operator=(const hoxSocketReader&);
};

// ----------------------------------------------------------------------------
// hoxSocketConnection
// ----------------------------------------------------------------------------

/**
 * A Connection based on a network Socket.
 */
class hoxSocketConnection : public hoxConnection
{
public:
    hoxSocketConnection(); // DUMMY default constructor required for RTTI info.
    hoxSocketConnection( const wxString& sHostname,
                         int             nPort );
    virtual ~hoxSocketConnection();

    // **** Override the parent's API ****
    virtual void Start();
    virtual void Shutdown();
    virtual bool AddRequest( hoxRequest* request );
    virtual bool IsConnected() { return m_bConnected; }

    virtual void       SetPlayer(hoxPlayer* player) { m_player = player; }
    virtual hoxPlayer* GetPlayer()                  { return m_player; }

    // **** API for Socket-Writer and -Reader ****
    wxString       GetHostname() const { return m_sHostname; }
    int            GetPort() const { return m_nPort; }

    void StartWriter();

    virtual void SetConnected(bool connected) { m_bConnected = connected; }

    /**
     * Is this Thread being shutdowned by the System.
     */
    bool IsBeingShutdowned() const { return m_shutdownRequested; } 

private:
    wxString              m_sHostname; 
    int                   m_nPort;

    bool                  m_bConnected;
                /* Has the connection been established with the server */

    hoxPlayer*            m_player;
                /* The player that owns this connection */

    bool                  m_shutdownRequested;
                /* Has a shutdown-request been received? */

    hoxSocketWriter_SPtr  m_writer;
                /* The Reader Thread. 
                 * This Thread also creates and manages the Writer Thread.
                 */

    DECLARE_DYNAMIC_CLASS(hoxSocketConnection)
};

#endif /* __INCLUDED_HOX_SOCKET_CONNECTION_H_ */
