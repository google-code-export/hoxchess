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
// Name:            hoxAsyncSocket.h
// Created:         12/11/2010
//
// Description:     The Asynchronous Socket based on Asio library.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_ASYNC_SOCKET_H__
#define __INCLUDED_HOX_ASYNC_SOCKET_H__

#include <asio.hpp>
#include <deque>
#include <boost/bind.hpp>

#include <QtDebug>

using asio::ip::tcp;

namespace hox {
    namespace network {

        enum DataType
        {
            TYPE_ERROR,
            TYPE_DATA
        };

        class DataPayload
        {
        public:
            DataPayload(DataType type, const std::string& data)
                    : type_(type)
                    , data_(data) {}
            const std::string& data() const { return data_; }
            DataType type() const { return type_; }

        private:
            DataType    type_;
            std::string data_;
        };

        class DataHandler
        {
        public:
            virtual ~DataHandler() {}
            virtual void onNewPayload(const DataPayload& /*payload*/) = 0;
        };

// ----------------------------------------------------------------------------
// hoxAsyncSocket
// ----------------------------------------------------------------------------

class hoxAsyncSocket
{
protected:
    enum ConnectState
    {
        CONNECT_STATE_INIT,
        CONNECT_STATE_CONNECTING,
        CONNECT_STATE_CONNECTED,
        CONNECT_STATE_CLOSED
    };

public:
    hoxAsyncSocket( asio::io_service&       io_service,
                    tcp::resolver::iterator endpoint_iter,
                    DataHandler*            dataHandler );
    virtual ~hoxAsyncSocket() {}

    virtual void handleIncomingData( const asio::error_code& error );

    void write( const std::string& msg );
    void close();

protected:
    virtual void handleConnect( const asio::error_code& error,
                                tcp::resolver::iterator endpoint_iter );
    // ----
    bool checkAndCloseSocketIfError( const asio::error_code& error );
    void closeSocket();

private:
    void _postEvent( const DataType     type,
                     const std::string& sData );
    void _doWrite( const std::string msg );
    void _handleWrite( const asio::error_code& error );

protected:
    asio::io_service&    _io_service; // A reference only!
    tcp::socket          m_socket;
    asio::deadline_timer m_timer;
 
    int                  out_tries_; // FIXME: Concurrency problem!
    std::string          out_msg_;   // FIXME: Concurrency problem!

    typedef std::deque<std::string> MessageQueue;
    MessageQueue         m_writeQueue;

    std::string          m_sCurrentEvent;
                /* The incoming event (being accumulated so far). */

    asio::streambuf      m_inBuffer; // The buffer of incoming data.
    ConnectState         m_connectState;
    DataHandler*         m_dataHandler;
};


} // namespace network
} // namespace hox

#endif /* __INCLUDED_HOX_ASYNC_SOCKET_H__ */
