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
// Name:            hoxSocketConnection.cpp
// Created:         10/28/2007
//
// Description:     The Socket-Connection Thread to help MY player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxSocketConnection.h"
#include "hoxPlayer.h"
#include "hoxUtil.h"
#include "hoxNetworkAPI.h"

IMPLEMENT_DYNAMIC_CLASS(hoxSocketConnection, hoxConnection)

// ----------------------------------------------------------------------------
//
//     hoxSocketAgent
//
// ----------------------------------------------------------------------------

hoxSocketAgent::hoxSocketAgent( asio::io_service&       io_service,
                                tcp::resolver::iterator endpoint_iterator,
                                wxEvtHandler*           evtHandler)
        : _io_service( io_service )
        , m_socket( io_service )
        , m_timer( io_service )
        , m_connectState( CONNECT_STATE_INIT )
        , m_evtHandler( evtHandler )
{
    m_connectState = CONNECT_STATE_CONNECTING;
    tcp::endpoint endpoint = *endpoint_iterator;
    m_socket.async_connect( endpoint,
                            boost::bind(&hoxSocketAgent::handleConnect, this,
                                        asio::placeholders::error, ++endpoint_iterator));
}

void
hoxSocketAgent::write( const std::string& msg )
{
    out_tries_ = 0;
    out_msg_   = msg;  // *** make a copy
    _io_service.post( boost::bind(&hoxSocketAgent::_doWrite, this, out_msg_) );
}

void
hoxSocketAgent::close()
{
    _io_service.post( boost::bind(&hoxSocketAgent::closeSocket, this) );
}

void
hoxSocketAgent::handleConnect( const asio::error_code& error,
                               tcp::resolver::iterator endpoint_iterator)
{
    if ( !error )
    {
        m_connectState = CONNECT_STATE_CONNECTED;
        wxLogDebug("%s: Connection established to the server.", __FUNCTION__);
        asio::async_read_until( m_socket, m_inBuffer, "\n\n",
                                boost::bind(&hoxSocketAgent::handleIncomingData, this,
                                            asio::placeholders::error));
    }
    else if ( endpoint_iterator != tcp::resolver::iterator() )
    {
        m_socket.close();
        tcp::endpoint endpoint = *endpoint_iterator;
        m_socket.async_connect( endpoint,
                                boost::bind(&hoxSocketAgent::handleConnect, this,
                                            asio::placeholders::error, ++endpoint_iterator));
    }
    else  // Failed.
    {
        m_connectState = CONNECT_STATE_CLOSED;
        wxLogDebug("%s: *WARN* Fail to connect to server.", __FUNCTION__);
        this->postEvent( hoxRC_CLOSED, "Fail to connect to server", hoxREQUEST_LOGIN );
        m_socket.close();
        //_io_service.stop(); // FORCE to stop!
    }
}

void
hoxSocketAgent::handleIncomingData( const asio::error_code& error )
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
hoxSocketAgent::consumeIncomingData()
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

    // Read incomgin data (AGAIN!).
    asio::async_read_until( m_socket, m_inBuffer, "\n\n",
                            boost::bind(&hoxSocketAgent::handleIncomingData, this,
                                        asio::placeholders::error));
}

void
hoxSocketAgent::_doWrite( const std::string msg )
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
        m_timer.async_wait(boost::bind(&hoxSocketAgent::_doWrite, this, msg));
        return;
    }

    bool write_in_progress = !m_writeQueue.empty();
    m_writeQueue.push_back(msg);
    if (!write_in_progress)
    {
        asio::async_write( m_socket,
                           asio::buffer( m_writeQueue.front().data(),
                                         m_writeQueue.front().length()),
                           boost::bind( &hoxSocketAgent::_handleWrite, this,
                                        asio::placeholders::error));
    }
}

void
hoxSocketAgent::_handleWrite( const asio::error_code& error )
{
    if ( error )
    {
        closeSocket();
        this->postEvent( hoxRC_CLOSED, "Connection closed while writing" );
        return;
    }

    m_writeQueue.pop_front();
    if (!m_writeQueue.empty())
    {
        asio::async_write( m_socket,
                           asio::buffer( m_writeQueue.front().data(),
                                         m_writeQueue.front().length()),
                           boost::bind( &hoxSocketAgent::_handleWrite, this,
                                        asio::placeholders::error));
    }
}

void
hoxSocketAgent::closeSocket()
{
    m_socket.close();
}

void
hoxSocketAgent::postEvent( const hoxResult      result,
                           const std::string&   sEvent,
                           const hoxRequestType type /* = hoxREQUEST_PLAYER_DATA */ )
{
    hoxResponse_APtr apResponse( new hoxResponse(type) );

    for ( std::string::const_iterator it = sEvent.begin(); it != sEvent.end(); ++it )
    {
        apResponse->data.AppendByte( *it );
    }
 
    /* Notify the Player. */
    wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, type );
    apResponse->code = result;
    event.SetEventObject( apResponse.release() );  // Caller will de-allocate.
    wxPostEvent( m_evtHandler, event );
}


// ----------------------------------------------------------------------------
// hoxSocketWriter
// ----------------------------------------------------------------------------

hoxSocketWriter::hoxSocketWriter( wxEvtHandler*           evtHandler,
                                  const hoxServerAddress& serverAddress )
        : wxThread( wxTHREAD_JOINABLE )
        , m_evtHandler( evtHandler )
        , m_serverAddress( serverAddress )
        , m_shutdownRequested( false )
        , m_bConnected( false )
        , m_pSocketAgent( NULL )
        , m_io_service_thread( NULL )
{
}

hoxSocketWriter::~hoxSocketWriter()
{
}

bool
hoxSocketWriter::AddRequest( hoxRequest_APtr apRequest )
{
    if ( m_shutdownRequested )
    {
        wxLogDebug("%s: *WARN* Deny request [%s]. The thread is shutdowning.", 
            __FUNCTION__, hoxUtil::RequestTypeToString(apRequest->type).c_str());
        return false;
    }

    m_requests.PushBack( apRequest );
    m_semRequests.Post();  // Notify...
	return true;
}

hoxSocketAgent*
hoxSocketWriter::CreateSocketAgent( asio::io_service&       io_service,
                                    tcp::resolver::iterator endpoint_iterator,
                                    wxEvtHandler*           evtHandler)
{
    return new hoxSocketAgent( io_service, endpoint_iterator, evtHandler);
}

void
hoxSocketWriter::AskSocketAgentToWrite( const wxString& sRawMsg )
{
    if ( ! m_pSocketAgent )
    {
        wxLogDebug("%s: *WARN* Socket Agent not available. END.", __FUNCTION__);
        return;
    }

    const std::string sOutMsg( sRawMsg.ToUTF8() ); // Convert to UTF8.
    m_pSocketAgent->write( sOutMsg );
}

void
hoxSocketWriter::CloseSocketAgent()
{
    if ( m_io_service_thread )
    {
        m_pSocketAgent->close();
                /* Need to call since some server does NOT
                 * auto-close the connection upon receiving LOGOUT.
                 */

        wxLogDebug("%s: Waiting for IO-Service Thread to end...", __FUNCTION__);
        m_io_service_thread->join();   // ************ WAIT HERE
        wxLogDebug("%s: IO-Service Thread ended.", __FUNCTION__);
        delete m_pSocketAgent;
        m_pSocketAgent = NULL;
        delete m_io_service_thread;
        m_io_service_thread = NULL;
        //m_bConnected = false;
    }
}

hoxRequest_APtr
hoxSocketWriter::_GetRequest()
{
    hoxRequest_APtr apRequest = m_requests.PopFront();
    wxCHECK_MSG(apRequest.get() != NULL, apRequest, "At least one request must exist");

    /* Handle SHUTDOWN request here to avoid the possible memory leaks.
     * The reason is that others (timers, for example) may continue to 
     * send requests to this thread while this thread is shutdowning it self. 
     *
     * NOTE: The SHUTDOWN request is (purposely) handled here inside this function 
     *       because the "mutex-lock" is still being held.
     */

    if ( apRequest->type == hoxREQUEST_SHUTDOWN )
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
        apRequest.reset(); /* Release memory and signal "no more request" ...
                            * ... to the caller!
                            */
    }

    return apRequest;
}

void*
hoxSocketWriter::Entry()
{
    hoxRequest_APtr apRequest;
    hoxResult       result = hoxRC_OK;
    wxString        sError;

    wxLogDebug("%s: ENTER.", __FUNCTION__);

    while (   !m_shutdownRequested
            && m_semRequests.Wait() == wxSEMA_NO_ERROR )
    {
        apRequest = _GetRequest();
        if ( apRequest.get() == NULL )
        {
            wxASSERT_MSG( m_shutdownRequested, "This thread must be shutdowning." );
            break;  // Exit the thread.
        }
        const hoxRequestType requestType = apRequest->type;
        wxLogDebug("%s: Processing request = [%s]...", 
            __FUNCTION__, hoxUtil::RequestTypeToString(requestType).c_str());

        result = this->HandleRequest( apRequest, sError );
        if ( result != hoxRC_OK )
        {
            _postEventToHandler( result, sError, requestType );
        }

        if ( requestType == hoxREQUEST_LOGOUT )
        {
            break; // !!! Force to close !!!
        }
    }

    /* Close the socket-connection. */
    this->CloseSocketAgent();

    /* Notify the Player. */
    wxLogDebug("%s: Notify event-handler of connection CLOSED.", __FUNCTION__);
    _postEventToHandler( hoxRC_CLOSED, "Connection CLOSED", hoxREQUEST_PLAYER_DATA );

    return NULL;
}

hoxResult
hoxSocketWriter::HandleRequest( hoxRequest_APtr apRequest,
                                wxString&       sError )
{
    hoxResult    result = hoxRC_OK;
    const hoxRequestType requestType = apRequest->type;

    sError = "";

    /* Make sure the connection is established. */
    if ( ! m_bConnected )
    {
        result = this->Connect( sError );
        if ( result != hoxRC_OK )
        {
            wxLogDebug("%s: *WARN* Failed to establish a connection.", __FUNCTION__);
            return result;
        }
    }

    /* Send the request. */
    const wxString sRequest = apRequest->ToString();
    result = _WriteLine( sRequest );

    return result;
}

hoxResult
hoxSocketWriter::Connect( wxString& sError )
{
    if ( m_bConnected )
    {
        wxLogDebug("%s: *INFO* The connection already established. END.", __FUNCTION__);
        return hoxRC_OK; // Consider "success".
    }

    try
    {
        const std::string sHost = hoxUtil::wx2std( m_serverAddress.name );
        const wxString sPort = wxString::Format("%d", m_serverAddress.port ); 
        const std::string sService = hoxUtil::wx2std( sPort );

        tcp::resolver resolver( m_io_service );
        tcp::resolver::query query( sHost, sService );
        tcp::resolver::iterator iterator = resolver.resolve(query);

        m_pSocketAgent = this->CreateSocketAgent( m_io_service, iterator, m_evtHandler);
    
        // TODO: Set timeout = hoxSOCKET_CLIENT_SOCKET_TIMEOUT.

        m_io_service_thread =
            new asio::thread( boost::bind(&asio::io_service::run, &m_io_service) );

        // TODO: Not really have enough info to declare a 'success' connection!
        m_bConnected = true;
    }
    catch (std::exception& e)
    {
        sError.Printf("Exception [%s] when connect to [%s]", e.what(), m_serverAddress.c_str());
        wxLogWarning("%s: %s.", __FUNCTION__, sError.c_str());
        return hoxRC_ERR;
    }

    return hoxRC_OK;
}

hoxResult
hoxSocketWriter::_WriteLine( const wxString& sContent )
{
    wxString sRawMsg;
	sRawMsg.Printf("%s\n", sContent.c_str());

    this->AskSocketAgentToWrite( sRawMsg );
    return hoxRC_OK;
}

void
hoxSocketWriter::_postEventToHandler( const hoxResult      result,
                                      const wxString&      sEvent,
                                      const hoxRequestType requestType )
{
    wxLogDebug("%s: *INFO* Request [%s] return error-code = [%s].", 
        __FUNCTION__, hoxUtil::RequestTypeToString(requestType).c_str(), 
        hoxUtil::ResultToStr(result));

    hoxResponse_APtr apResponse( new hoxResponse(requestType) );
    apResponse->content = sEvent;
    apResponse->code = result;

    wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, requestType );
    event.SetEventObject( apResponse.release() );  // Caller will de-allocate.
    wxPostEvent( m_evtHandler, event );
}

//-----------------------------------------------------------------------------
// hoxSocketConnection
//-----------------------------------------------------------------------------

hoxSocketConnection::hoxSocketConnection( const hoxServerAddress& serverAddress,
                                          wxEvtHandler*           evtHandler )
        : hoxConnection( evtHandler )
        , m_serverAddress( serverAddress )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

hoxSocketConnection::~hoxSocketConnection()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

void
hoxSocketConnection::Start()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    _StartWriter();
    // *** This thread will also create and manage the Reader thread.
}

void
hoxSocketConnection::_StartWriter()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    if ( m_writer && m_writer->IsRunning() )
    {
        wxLogDebug("%s: The Writer has already been started. END.", __FUNCTION__);
        return;
    }

    wxLogDebug("%s: Create the Writer Thread...", __FUNCTION__);
    m_writer = this->CreateWriter( this->GetPlayer(), 
                                   m_serverAddress );
    if ( m_writer->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogDebug("%s: *WARN* Failed to create the Writer thread.", __FUNCTION__);
        m_writer.reset();
        return;
    }
    wxASSERT_MSG(!m_writer->IsDetached(), "The Writer thread must be joinable");

    m_writer->Run();
}

hoxSocketWriter_SPtr
hoxSocketConnection::CreateWriter( wxEvtHandler*           evtHandler,
                                   const hoxServerAddress& serverAddress )
{
    hoxSocketWriter_SPtr writer( new hoxSocketWriter( evtHandler, 
                                                      serverAddress ) );
    return writer;
}

void
hoxSocketConnection::Shutdown()
{
    if ( m_writer )
    {
        wxLogDebug("%s: Request the Writer thread to shutdown...", __FUNCTION__);
        wxThread::ExitCode exitCode = m_writer->Wait();
        wxLogDebug("%s: The Writer thread shutdowned with exit-code = [%d].", __FUNCTION__, exitCode);
        m_writer.reset();
    }
}

bool
hoxSocketConnection::AddRequest( hoxRequest_APtr apRequest )
{
    wxCHECK_MSG(m_writer, false, "The Writer thread not yet created");
    return m_writer->AddRequest( apRequest );
}

bool
hoxSocketConnection::IsConnected() const
{ 
    return ( m_writer && m_writer->IsConnected() );
}

/************************* END OF FILE ***************************************/
