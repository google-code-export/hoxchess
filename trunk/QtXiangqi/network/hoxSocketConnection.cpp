/***************************************************************************
 *  Copyright 2007-2009 Huy Phan  <huyphan@playxiangqi.com>                *
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
// Name:            hoxSocketConnection.cpp
// Created:         10/28/2007
//
// Description:     The Socket-Connection Thread to help MY player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxSocketConnection.h"
//#include "hoxPlayer.h"
//#include "hoxUtil.h"

#define wxLogDebug qDebug

#define HC_APP_INAME        "QOXChess"  /* The internal app-name     */
#define HC_APP_IVERSION     "1.0"       /* The internal app-version  */

namespace hox {
namespace network {

// ----------------------------------------------------------------------------
// hoxSocketWriter
// ----------------------------------------------------------------------------

hoxSocketWriter::hoxSocketWriter( DataHandler* dataHandler,
                                  const ServerAddress& serverAddress )
        : m_dataHandler( dataHandler )
        , m_serverAddress( serverAddress )
        , m_shutdownRequested( false )
        , m_bConnected( false )
        , m_pSocket( NULL )
        , m_io_service_thread( NULL )
{
}

hoxSocketWriter::~hoxSocketWriter()
{
}

void
hoxSocketWriter::start()
{
    m_thread = boost::thread(&hoxSocketWriter::entry, this);
}

void
hoxSocketWriter::join()
{
    m_thread.join();
}

bool
hoxSocketWriter::isRunning()
{
    return m_thread.joinable();
}

bool
hoxSocketWriter::addRequest( Request_SPtr request )
{
    if ( m_shutdownRequested )
    {
        wxLogDebug("%s: *WARN* Deny request [%d]. The thread is shutdowning.",
            __FUNCTION__, request->m_type);
        return false;
    }

    {
        boost::unique_lock<boost::mutex> lock(m_mutexRequests);
        m_requests.push_back( request );
    }
    m_condRequests.notify_one();

    return true;
}

Request_SPtr
hoxSocketWriter::_getRequest()
{
    Request_SPtr request = m_requests.front();
    m_requests.pop_front();

    //wxCHECK_MSG(apRequest.get() != NULL, apRequest, "At least one request must exist");

    /* Handle SHUTDOWN request here to avoid the possible memory leaks.
     * The reason is that others (timers, for example) may continue to 
     * send requests to this thread while this thread is shutdowning it self. 
     *
     * NOTE: The SHUTDOWN request is (purposely) handled here inside this function 
     *       because the "mutex-lock" is still being held.
     */

    if ( request->m_type == REQUEST_SHUTDOWN )
    {
        wxLogDebug("%s: A SHUTDOWN request just received.", __FUNCTION__);
        m_shutdownRequested = true;
    }

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * NOTE: The shutdown-request can come from:
     *       (1) The code segment ABOVE, or
     *       (2) It can be triggered from the outside callers
     *           who could invoke this->Shutdown() to this Thread.
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     */

    if ( m_shutdownRequested )
    {
        wxLogDebug("%s: Shutting down this thread...", __FUNCTION__);
        request.reset(); /* Release memory and signal "no more request" ...
                          * ... to the caller!
                          */
    }

    return request;
}

void
hoxSocketWriter::entry()
{
    Result      result = hoxRC_OK;
    std::string sError;

    wxLogDebug("%s: ENTER.", __FUNCTION__);

    while ( !m_shutdownRequested )
    {
        boost::unique_lock<boost::mutex> lock(m_mutexRequests);
        while (m_requests.empty())
        {
            m_condRequests.wait(lock);
        }

        Request_SPtr request = _getRequest();
        if ( ! request )
        {
            wxLogDebug("%s: *INFO* This thread must be shutdowning..", __FUNCTION__);
            break;  // Exit the thread.
        }
        const RequestType requestType = request->m_type;
        wxLogDebug("%s: Processing request = [%d]...", __FUNCTION__, requestType);

        result = _handleRequest( request, sError );
        if ( result != hoxRC_OK )
        {
            _postEventToHandler( result, sError );
        }

        if ( requestType == REQUEST_LOGOUT )
        {
            break; // !!! Force to close !!!
        }
    }

    _closeSocket();

    /* Notify the Player. */
    wxLogDebug("%s: Notify event-handler of connection CLOSED.", __FUNCTION__);
    _postEventToHandler( hoxRC_CLOSED, "Connection CLOSED" );
}

Result
hoxSocketWriter::_handleRequest( Request_SPtr request,
                                 std::string& sError )
{
    Result result = hoxRC_OK;

    sError = "";

    /* Make sure the connection is established. */
    if ( ! m_bConnected )
    {
        result = _connect( sError );
        if ( result != hoxRC_OK )
        {
            wxLogDebug("%s: *WARN* Failed to establish a connection.", __FUNCTION__);
            return result;
        }
    }

    /* Send the request. */
    const std::string sData = request->m_data;
    result = _writeLine( sData );

    return result;
}

Result
hoxSocketWriter::_connect( std::string& sError )
{
    try
    {
        tcp::resolver resolver( m_io_service );
        tcp::resolver::query query( m_serverAddress.m_host, m_serverAddress.m_port );
        tcp::resolver::iterator iterator = resolver.resolve(query);

        m_pSocket = new hoxAsyncSocket( m_io_service, iterator, m_dataHandler);
    
        // TODO: Set timeout = hoxSOCKET_CLIENT_SOCKET_TIMEOUT.

        m_io_service_thread =
            new asio::thread( boost::bind(&asio::io_service::run, &m_io_service) );

        // TODO: Not really have enough info to declare a 'success' connection!
        m_bConnected = true;
    }
    catch (std::exception& ex)
    {
        sError = std::string("Exception :[") + ex.what() + "] when connect to server.";
        wxLogDebug("%s: *WARN* %s.", __FUNCTION__, sError.c_str());
        return hoxRC_ERR;
    }

    return hoxRC_OK;
}

void
hoxSocketWriter::_closeSocket()
{
    if ( m_io_service_thread )
    {
        m_pSocket->close();
                /* Need to call since some server does NOT
                 * auto-close the connection upon receiving LOGOUT.
                 */

        wxLogDebug("%s: Waiting for IO-Service Thread to end...", __FUNCTION__);
        m_io_service_thread->join();   // ************ WAIT HERE
        wxLogDebug("%s: IO-Service Thread ended.", __FUNCTION__);
        delete m_pSocket;
        m_pSocket = NULL;
        delete m_io_service_thread;
        m_io_service_thread = NULL;
        m_bConnected = false;
    }
}

Result
hoxSocketWriter::_writeLine( const std::string& sData )
{
    if ( ! m_pSocket )
    {
        wxLogDebug("%s: *WARN* Socket Agent not available. END.", __FUNCTION__);
        return hoxRC_ERR;
    }

    // TODO: Need to confirm that this is UTF-8 string.
    const std::string sUtf8Msg = sData + "\n";
    m_pSocket->write( sUtf8Msg );
    return hoxRC_OK;
}

void
hoxSocketWriter::_postEventToHandler( const Result       result,
                                      const std::string& sEvent )
{
    wxLogDebug("%s: *INFO* Request [%s] return error-code = [%d].",
        __FUNCTION__, sEvent.c_str(), result);

    const DataPayload payload(TYPE_DATA, sEvent);
    m_dataHandler->onNewPayload(payload);

    /*hoxResponse_APtr apResponse( new hoxResponse(requestType) );
    apResponse->content = sEvent;
    apResponse->code = result;

    wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, requestType );
    event.SetEventObject( apResponse.release() );  // Caller will de-allocate.
    wxPostEvent( m_evtHandler, event );*/
}

//-----------------------------------------------------------------------------
// hoxSocketConnection
//-----------------------------------------------------------------------------

hoxSocketConnection::hoxSocketConnection( const ServerAddress& serverAddress,
                                          DataHandler*         dataHandler )
        : m_dataHandler( dataHandler )
        , m_serverAddress( serverAddress )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

hoxSocketConnection::~hoxSocketConnection()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

void
hoxSocketConnection::start()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    // Start a Writer.
    // This writer thread will also create and manage the Reader thread.

    if ( m_writer && m_writer->isRunning() )
    {
        wxLogDebug("%s: The Writer has already been started. END.", __FUNCTION__);
        return;
    }

    wxLogDebug("%s: Create the Writer Thread...", __FUNCTION__);
    m_writer.reset( new hoxSocketWriter( m_dataHandler,
                                         m_serverAddress ) );
    m_writer->start();
}

void
hoxSocketConnection::stop()
{
    if ( m_writer )
    {
        wxLogDebug("%s: Request the Writer thread to shutdown...", __FUNCTION__);
        m_writer->join();
        m_writer.reset();
    }
}

bool
hoxSocketConnection::addRequest( Request_SPtr request )
{
    if ( !m_writer)
    {
        wxLogDebug("%s: ERROR: The Writer thread not yet created.", __FUNCTION__);
        return false;
    }
    return m_writer->addRequest( request );
}

bool
hoxSocketConnection::isConnected() const
{ 
    return ( m_writer && m_writer->isConnected() );
}

void
hoxSocketConnection::send_LOGIN( const std::string& pid,
                                 const std::string& password )
{
    pid_ = pid;   // Needed when LOGOUT.
    password_ = password;

    const std::string sCmd = std::string("op=LOGIN")
                             + "&version=" + HC_APP_INAME + "-" + HC_APP_IVERSION
                             + "&password=" + password;
    _sendRequest(sCmd);
}

void
hoxSocketConnection::send_LOGOUT()
{
    const std::string sCmd = std::string("op=LOGOUT");
    _sendRequest(sCmd, REQUEST_LOGOUT);
               /* NOTE: Special flag to shutdown the connection
                * as well.
                */
}

void
hoxSocketConnection::send_LIST()
{
    const std::string sCmd = std::string("op=LIST");
    _sendRequest(sCmd, REQUEST_LOGOUT);
               /* NOTE: Special flag to shutdown the connection
                * as well.
                */
}

void
hoxSocketConnection::_sendRequest( const std::string& sCmd,
                                   RequestType        type /* = REQUEST_COMMAND */ )
{
    Request_SPtr request(new Request(type));
    request->m_data = sCmd + "&pid=" + pid_;
    m_writer->addRequest(request);
}

} // namespace network
} // namespace hox


/************************* END OF FILE ***************************************/
