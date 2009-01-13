/***************************************************************************
 *  Copyright 2007, 2008, 2009 Huy Phan  <huyphan@playxiangqi.com>         *
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

#include <wx/socket.h>
#include "hoxConnection.h"
#include "hoxTypes.h"

/* Forward declarations. */
class hoxSocketWriter;
class hoxSocketReader;

/* Typedef(s) */
typedef boost::shared_ptr<hoxSocketWriter> hoxSocketWriter_SPtr;
typedef boost::shared_ptr<hoxSocketReader> hoxSocketReader_SPtr;

// ----------------------------------------------------------------------------
// hoxSocketWriter
// ----------------------------------------------------------------------------

class hoxSocketWriter : public wxThread
{
public:
    hoxSocketWriter( wxEvtHandler*           player,
                     const hoxServerAddress& serverAddress );
    virtual ~hoxSocketWriter();

    bool AddRequest( hoxRequest_APtr apRequest );
    bool IsConnected() const { return m_bConnected; }

protected:
    // entry point for the thread
    virtual void *Entry();

    virtual void HandleRequest( hoxRequest_APtr apRequest );

private:
    void        _StartReader( wxSocketClient* socket );

    hoxRequest_APtr _GetRequest();

    hoxResult _Login( const hoxServerAddress& serverAddress,
                      const wxString&         sRequest,
                      wxString&               sResponse );

    void _Disconnect();

    hoxResult _WriteLine( wxSocketBase*   sock, 
                          const wxString& contentStr );

protected:
    wxEvtHandler*           m_player;

    const hoxServerAddress  m_serverAddress;

    wxSocketClient*         m_socket;
                /* The socket to handle network connections */

    hoxSocketReader_SPtr    m_reader;
                /* The Reader Thread. */

    // Storage to hold pending outgoing request.
    wxSemaphore             m_semRequests;
    hoxRequestQueue         m_requests;

    bool                    m_shutdownRequested;
                /* Has a shutdown-request been received? */

    bool                    m_bConnected;
                /* Has the connection been established with the server */

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
    hoxSocketReader( wxEvtHandler* player );
    virtual ~hoxSocketReader();

    void SetSocket( wxSocketClient* socket ) { m_socket = socket; }

protected:
    // entry point for the thread
    virtual void *Entry();

    virtual hoxResult ReadLine( wxSocketBase*   sock,
                                wxMemoryBuffer& data );

protected:
    wxEvtHandler*         m_player;
                /* The player that would receive notifications */

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
    hoxSocketConnection( const hoxServerAddress& serverAddress,
                         wxEvtHandler*           player );
    virtual ~hoxSocketConnection();

    // **** Override the parent's API ****
    virtual void Start();
    virtual void Shutdown();
    virtual bool AddRequest( hoxRequest_APtr apRequest );
    virtual bool IsConnected() const;

protected:
    virtual void StartWriter();

protected:
    const hoxServerAddress   m_serverAddress;

    hoxSocketWriter_SPtr     m_writer;
                /* The Write Thread. 
                 * This Thread also creates and manages the Reader Thread.
                 */

    DECLARE_DYNAMIC_CLASS(hoxSocketConnection)
};

#endif /* __INCLUDED_HOX_SOCKET_CONNECTION_H_ */
