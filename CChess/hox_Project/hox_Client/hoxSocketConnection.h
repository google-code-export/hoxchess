/***************************************************************************
 *  Copyright 2007-2009 Huy Phan  <huyphan@playxiangqi.com>                *
 *                      Bharatendra Boddu (bharathendra at yahoo dot com)  *
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

#ifndef __INCLUDED_HOX_SOCKET_CONNECTION_H__
#define __INCLUDED_HOX_SOCKET_CONNECTION_H__

#include <asio.hpp>
#include "hoxConnection.h"
#include "hoxTypes.h"
#include "hoxAsyncSocket.h"
#include <deque>
#include <boost/bind.hpp>

using asio::ip::tcp;

/* Forward declarations. */
class hoxSocketWriter;

/* Typedef(s) */
typedef boost::shared_ptr<hoxSocketWriter> hoxSocketWriter_SPtr;

// ----------------------------------------------------------------------------
// hoxSocketWriter
// ----------------------------------------------------------------------------

class hoxSocketWriter : public wxThread
{
public:
    hoxSocketWriter( wxEvtHandler*           evtHandler,
                     const hoxServerAddress& serverAddress );
    virtual ~hoxSocketWriter();

    bool AddRequest( hoxRequest_APtr apRequest );
    bool IsConnected() const { return m_bConnected; }

protected:
    virtual void* Entry(); // Entry point for the thread.

    virtual hoxResult HandleRequest( hoxRequest_APtr apRequest,
                                     wxString&       sError );
    virtual hoxResult Connect( wxString& sError );

    virtual hoxAsyncSocket* CreateSocketAgent( asio::io_service&       io_service,
                                               tcp::resolver::iterator endpoint_iterator,
                                               wxEvtHandler*           evtHandler );
    // ----
    void AskSocketAgentToWrite( const wxString& sRawMsg );
    void CloseSocketAgent();

private:
    hoxRequest_APtr _GetRequest();
    hoxResult       _WriteLine( const wxString& sContent );

    void _postEventToHandler( const hoxResult      result,
                              const wxString&      sEvent,
                              const hoxRequestType requestType );

protected:
    wxEvtHandler*           m_evtHandler;
    const hoxServerAddress  m_serverAddress;

    // Storage to hold pending outgoing requests.
    wxSemaphore             m_semRequests;
    hoxRequestQueue         m_requests;

    bool                    m_shutdownRequested;
                /* Has a shutdown-request been received? */

    bool                    m_bConnected;
                /* Has the connection been established with the server */

private:
    asio::io_service        m_io_service;
    hoxAsyncSocket*         m_pSocketAgent;
    asio::thread*           m_io_service_thread;
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
    hoxSocketConnection() {} // DUMMY default constructor required for RTTI.
    hoxSocketConnection( const hoxServerAddress& serverAddress,
                         wxEvtHandler*           evtHandler );
    virtual ~hoxSocketConnection();

    // **** Override the parent's API ****
    virtual void Start();
    virtual void Shutdown();
    virtual bool AddRequest( hoxRequest_APtr apRequest );
    virtual bool IsConnected() const;

protected:
    virtual hoxSocketWriter_SPtr CreateWriter( wxEvtHandler*           evtHandler,
                                               const hoxServerAddress& serverAddress );

private:
    void _StartWriter();

private:
    const hoxServerAddress   m_serverAddress;

    hoxSocketWriter_SPtr     m_writer;
                /* The Write Thread. 
                 * This Thread also creates and manages the Reader Thread.
                 */

    DECLARE_DYNAMIC_CLASS(hoxSocketConnection)
};

#endif /* __INCLUDED_HOX_SOCKET_CONNECTION_H__ */
