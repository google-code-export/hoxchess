/***************************************************************************
 *  Copyright 2010-2011 Huy Phan  <huyphan@playxiangqi.com>                *
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
// Created:         12/11/2010
//
// Description:     The Socket-Connection Thread.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_SOCKET_CONNECTION_H__
#define __INCLUDED_HOX_SOCKET_CONNECTION_H__

#include <string>
#include <asio.hpp>
#include <boost/thread.hpp>
#include "hoxAsyncSocket.h"

using asio::ip::tcp;

enum Result
{
    hoxRC_UNKNOWN = -1,

    hoxRC_OK = 0,
    hoxRC_ERR,   // A generic error.

    hoxRC_CLOSED    // Something (socket,...) has been closed.
};

enum RequestType
{
    REQUEST_LOGOUT,
    REQUEST_SHUTDOWN,
    REQUEST_COMMAND
};

struct Request
{
    RequestType m_type;
    std::string m_data;
    Request(RequestType type) : m_type(type) {}
};
typedef boost::shared_ptr<Request> Request_SPtr;

namespace hox {
namespace network {

struct ServerAddress
{
    std::string m_host;
    std::string m_port;
    ServerAddress(const std::string& host, const std::string& port)
        : m_host(host), m_port(port) {}
};

/* Forward declarations. */
class SocketWriter;

/* Typedef(s) */
typedef boost::shared_ptr<SocketWriter> SocketWriter_SPtr;
typedef std::list<Request_SPtr> hoxRequestQueue;

// ----------------------------------------------------------------------------
// SocketWriter
// ----------------------------------------------------------------------------

class SocketWriter
{
public:
    SocketWriter( DataHandler*         dataHandler,
                  const ServerAddress& serverAddress );
    virtual ~SocketWriter() {}

    bool addRequest( Request_SPtr request );
    bool isConnected() const { return m_bConnected; }

    //*** Thread-like API.
    void start();
    void join();
    bool isRunning();

protected:
    virtual void entry(); // Entry point for the thread.

private:
    Result _connect( std::string& sError );
    void _closeSocket();
    Result _handleRequest( Request_SPtr request,
                           std::string& sError );

private:
    Request_SPtr _getRequest();
    Result       _writeLine( const std::string& sData );

    void _postEventToHandler( const Result       result,
                              const std::string& sEvent );

private:
    boost::thread             m_thread;

    DataHandler*              m_dataHandler;
    const ServerAddress       m_serverAddress;

    // Storage to hold pending outgoing requests.
    boost::mutex              m_mutexRequests;
    boost::condition_variable m_condRequests;
    hoxRequestQueue           m_requests;

    bool                      m_shutdownRequested;
                /* Has a shutdown-request been received? */

    bool                      m_bConnected;
                /* Has the connection been established with the server */

    asio::io_service          m_io_service;
    hoxAsyncSocket*           m_pSocket;
    asio::thread*             m_io_service_thread;
};


// ----------------------------------------------------------------------------
// SocketConnection
// ----------------------------------------------------------------------------

/**
 * A Connection based on a network Socket.
 */
class SocketConnection
{
public:
    SocketConnection( const ServerAddress&  serverAddress,
                      DataHandler*          dataHandler );
    virtual ~SocketConnection();

    std::string getPid() const { return pid_; }
    std::string getPassword() const { return password_; }

    // **** Override the parent's API ****
    virtual void start();
    virtual void stop();
    virtual bool addRequest( Request_SPtr request );
    virtual bool isConnected() const;

    // *** Client requests.
    void send_LOGIN(const std::string& pid, const std::string& password);
    void send_LOGOUT();
    void send_LIST();
    void send_JOIN(const std::string& tid, const std::string& joinColor);

private:
    void sendRequest_( const std::string& sCmd,
                       RequestType        type = REQUEST_COMMAND );

private:
    DataHandler*         m_dataHandler;
    const ServerAddress  m_serverAddress;

    SocketWriter_SPtr    m_writer;
                /* The Writer Thread.
                 * This Thread also creates and manages the Reader Thread.
                 */

    std::string          pid_;      // My player-Id (pid).
    std::string          password_; // My password.
};


} // namespace network
} // namespace hox

#endif /* __INCLUDED_HOX_SOCKET_CONNECTION_H__ */
