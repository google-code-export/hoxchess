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
// Name:            hoxAsyncSocket.cpp
// Created:         06/27/2009
//
// Description:     The Asynchronous Socket based on Asio library.
/////////////////////////////////////////////////////////////////////////////

#include "hoxAsyncSocket.h"
#include "hoxPlayer.h" // TODO: required for hoxEVT_CONNECTION_RESPONSE
#include "hoxUtil.h"

// ----------------------------------------------------------------------------
//
//     hoxAsyncSocket
//
// ----------------------------------------------------------------------------

hoxAsyncSocket::hoxAsyncSocket( asio::io_service&       io_service,
                                tcp::resolver::iterator endpoint_iter,
                                wxEvtHandler*           evtHandler)
        : _io_service( io_service )
        , m_socket( io_service )
        , m_timer( io_service )
        , m_connectState( CONNECT_STATE_INIT )
        , m_evtHandler( evtHandler )
{
    m_connectState = CONNECT_STATE_CONNECTING;
    tcp::endpoint endpoint = *endpoint_iter;
    m_socket.async_connect( endpoint,
                            boost::bind(&hoxAsyncSocket::handleConnect, this,
                                        asio::placeholders::error, ++endpoint_iter));
}

void
hoxAsyncSocket::write( const std::string& msg )
{
    out_tries_ = 0;
    out_msg_   = msg;  // *** make a copy
    _io_service.post( boost::bind(&hoxAsyncSocket::_doWrite, this, out_msg_) );
}

void
hoxAsyncSocket::close()
{
    _io_service.post( boost::bind(&hoxAsyncSocket::closeSocket, this) );
}

void
hoxAsyncSocket::handleConnect( const asio::error_code& error,
                               tcp::resolver::iterator endpoint_iter)
{
    if ( !error )
    {
        m_connectState = CONNECT_STATE_CONNECTED;
        wxLogDebug("%s: Connection established.", __FUNCTION__);
        asio::async_read_until( m_socket, m_inBuffer, "\n\n",
                                boost::bind(&hoxAsyncSocket::handleIncomingData, this,
                                            asio::placeholders::error));
    }
    else if ( endpoint_iter != tcp::resolver::iterator() )
    {
        m_socket.close();
        tcp::endpoint endpoint = *endpoint_iter;
        m_socket.async_connect( endpoint,
                                boost::bind(&hoxAsyncSocket::handleConnect, this,
                                            asio::placeholders::error, ++endpoint_iter));
    }
    else  // Failed.
    {
        m_connectState = CONNECT_STATE_CLOSED;
        wxLogDebug("%s: *WARN* Fail to establish connection.", __FUNCTION__);
        this->postEvent( hoxRC_CLOSED, "Fail to establish connection", hoxREQUEST_LOGIN );
        m_socket.close();
        //_io_service.stop(); // FORCE to stop!
    }
}

void
hoxAsyncSocket::handleIncomingData( const asio::error_code& error )
{
    if ( error )
    {
        closeSocket();
        this->postEvent( hoxRC_CLOSED, "Connection closed while reading" );
        return;
    }

    this->consumeIncomingData();
}

void
hoxAsyncSocket::consumeIncomingData()
{
    std::istream response_stream( &m_inBuffer );
    wxUint8      b;
    bool         bSawOne = false; // just saw one '\n'?

	/* Read a line until "\n\n" */

    while ( response_stream.read( (char*) &b, 1 ) )
    {
        if ( !bSawOne && b == '\n' )
        {
	        bSawOne = true;
        }
        else if ( bSawOne && b == '\n' )
        {
            this->postEvent( hoxRC_OK, m_sCurrentEvent );
            m_sCurrentEvent = ""; // Clear old data (... TO BE SAFE!).
        }
        else
        {
            if ( bSawOne ) {
                m_sCurrentEvent.append( 1, '\n' );
                bSawOne = false;
            }
            m_sCurrentEvent.append( 1, b );
        }
    }

    // Read incoming data (AGAIN!).
    asio::async_read_until( m_socket, m_inBuffer, "\n\n",
                            boost::bind(&hoxAsyncSocket::handleIncomingData, this,
                                        asio::placeholders::error));
}

void
hoxAsyncSocket::_doWrite( const std::string msg )
{
    if ( m_connectState == CONNECT_STATE_CLOSED )
    {
        wxLogDebug("%s: Connection closed. Abort (tries = [%d]).", __FUNCTION__, out_tries_);
        return;
    }
    else if ( m_connectState != CONNECT_STATE_CONNECTED )
    {
        const int WAIT_INTERVAL = 100; // in milliseconds.
        const int MAX_TRIES     = 200; // a total of 20 seconds!!!
        if ( ++out_tries_ > MAX_TRIES )   
        {
            wxLogDebug("%s: Timeout after [%d] tries.", __FUNCTION__, out_tries_);
            return;
        }
        m_timer.expires_from_now(boost::posix_time::milliseconds(WAIT_INTERVAL));
        m_timer.async_wait(boost::bind(&hoxAsyncSocket::_doWrite, this, msg));
        return;
    }

    const bool write_in_progress = !m_writeQueue.empty();
    m_writeQueue.push_back(msg);
    if ( !write_in_progress )
    {
        asio::async_write( m_socket,
                           asio::buffer( m_writeQueue.front().data(),
                                         m_writeQueue.front().length()),
                           boost::bind( &hoxAsyncSocket::_handleWrite, this,
                                        asio::placeholders::error));
    }
}

void
hoxAsyncSocket::_handleWrite( const asio::error_code& error )
{
    if ( error )
    {
        closeSocket();
        this->postEvent( hoxRC_CLOSED, "Connection closed while writing" );
        return;
    }

    m_writeQueue.pop_front();
    if ( !m_writeQueue.empty() )
    {
        asio::async_write( m_socket,
                           asio::buffer( m_writeQueue.front().data(),
                                         m_writeQueue.front().length()),
                           boost::bind( &hoxAsyncSocket::_handleWrite, this,
                                        asio::placeholders::error));
    }
}

void
hoxAsyncSocket::closeSocket()
{
    m_socket.close();
}

void
hoxAsyncSocket::postEvent( const hoxResult      result,
                           const std::string&   sEvent,
                           const hoxRequestType type /* = hoxREQUEST_PLAYER_DATA */ )
{
    hoxResponse_APtr apResponse( new hoxResponse(type, result) );

    for ( std::string::const_iterator it = sEvent.begin(); it != sEvent.end(); ++it )
    {
        apResponse->data.AppendByte( *it );
    }
 
    /* Notify the event-handler. */
    wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, type );
    event.SetEventObject( apResponse.release() ); // Caller will de-allocate.
    wxPostEvent( m_evtHandler, event ); /* TODO: 2.9.0 wxWidgets says
                                         * we should use wxQueueEvent instead
                                         */
}

/************************* END OF FILE ***************************************/
