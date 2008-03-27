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
// Name:            hoxSocketConnection.cpp
// Created:         10/28/2007
//
// Description:     The Socket-Connection Thread to help MY player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxSocketConnection.h"
#include "hoxMyPlayer.h"
#include "hoxUtil.h"
#include "hoxNetworkAPI.h"

IMPLEMENT_DYNAMIC_CLASS(hoxSocketConnection, hoxConnection)

static const wxString
_RequestToString( const hoxRequest& request );

static hoxResult
_WriteLine( wxSocketBase*   sock, 
            const wxString& contentStr );

static hoxResult
_ReadLine( wxSocketBase* sock, 
           wxString&     result );

// ----------------------------------------------------------------------------
// hoxRequestQueue
// ----------------------------------------------------------------------------

hoxRequestQueue::hoxRequestQueue()
{
}

hoxRequestQueue::~hoxRequestQueue()
{
    const char* FNAME = "hoxRequestQueue::~hoxRequestQueue";

    while ( ! m_list.empty() )
    {
        hoxRequest_APtr apRequest( m_list.front() );
        m_list.pop_front();
        wxLogDebug("%s: Deleting request [%s]...", FNAME, 
            hoxUtil::RequestTypeToString(apRequest->type).c_str());
    }
}

void
hoxRequestQueue::PushBack( hoxRequest_APtr apRequest )
{
    wxMutexLocker lock( m_mutex ); // Gain exclusive access.

    m_list.push_back( apRequest.release() );
}

hoxRequest_APtr
hoxRequestQueue::PopFront()
{
    hoxRequest_APtr apRequest;   // Empty pointer.

    wxMutexLocker lock( m_mutex ); // Gain exclusive access.

    if ( ! m_list.empty() )
    {
        apRequest.reset( m_list.front() );
        m_list.pop_front();
    }

    return apRequest;
}

// ----------------------------------------------------------------------------
// hoxSocketWriter
// ----------------------------------------------------------------------------

hoxSocketWriter::hoxSocketWriter( hoxSocketConnection& owner )
        : wxThread( wxTHREAD_JOINABLE )
        , m_owner( owner )
        , m_socket( NULL )
        , m_shutdownRequested( false )
{
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * NOTE: We need to use wxSOCKET_BLOCK option to avoid the problem
     *       of high CPU usage in secondary threads.
     *       To find out why, lookup into the source code of wxSocket).
     *
     * Reference:
     *     http://www.wxwidgets.org/wiki/index.php/WxSocket
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     */
    const wxSocketFlags socketFlags = wxSOCKET_BLOCK; // *** See NOTE above!
    m_socket = new wxSocketClient( socketFlags );

    m_socket->Notify( false /* Disable socket-events */ );
    m_socket->SetTimeout( 10*60 /* 10 minutes */ /*hoxSOCKET_CLIENT_SOCKET_TIMEOUT*/ );

    /* Create Reader thread. */
    m_reader.reset( new hoxSocketReader( owner ) );
}

hoxSocketWriter::~hoxSocketWriter()
{
    const char* FNAME = "hoxSocketWriter::~hoxSocketWriter";

    wxLogDebug("%s: ENTER.", FNAME);

    _Disconnect();

    wxLogDebug("%s: END.", FNAME);
}

bool
hoxSocketWriter::AddRequest( hoxRequest_APtr apRequest )
{
    const char* FNAME = "hoxSocketWriter::AddRequest";

    if ( m_shutdownRequested )
    {
        wxLogDebug("%s: *** WARN*** Deny request [%s]. The thread is shutdowning.", 
            FNAME, hoxUtil::RequestTypeToString(apRequest->type).c_str());
        return false;
    }

    m_requests.PushBack( apRequest );
    m_semRequests.Post();  // Notify...
	return true;
}

void
hoxSocketWriter::_StartReader( wxSocketClient* socket )
{
    const char* FNAME = "hoxSocketWriter::_StartReader";

    wxLogDebug("%s: ENTER.", FNAME);

    if (    m_reader 
         && m_reader->IsRunning() )
    {
        wxLogDebug("%s: The connection has already been started. END.", FNAME);
        return;
    }

    m_reader->SetSocket( socket );

    if ( m_reader->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogDebug("%s: *** WARN *** Failed to create the Reader thread.", FNAME);
        return;
    }
    wxASSERT_MSG( !m_reader->IsDetached(), 
                  "The Reader thread must be joinable." );

    m_reader->Run();
}

hoxRequest_APtr
hoxSocketWriter::_GetRequest()
{
    const char* FNAME = "hoxSocketWriter::_GetRequest";

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
        wxLogDebug("%s: A SHUTDOWN requested just received.", FNAME);
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
        wxLogDebug("%s: Shutting down this thread...", FNAME);
        apRequest.reset(); /* Release memory and signal "no more request" ...
                            * ... to the caller!
                            */
    }

    return apRequest;
}

void*
hoxSocketWriter::Entry()
{
    const char* FNAME = "hoxSocketWriter::Entry";
    hoxRequest_APtr apRequest;

    wxLogDebug("%s: ENTER.", FNAME);

    while (   !m_shutdownRequested
            && m_semRequests.Wait() == wxSEMA_NO_ERROR )
    {
        apRequest = _GetRequest();
        if ( apRequest.get() == NULL )
        {
            wxASSERT_MSG( m_shutdownRequested, "This thread must be shutdowning." );
            break;  // Exit the thread.
        }
        wxLogDebug("%s: Processing request Type = [%s]...", 
            FNAME, hoxUtil::RequestTypeToString(apRequest->type).c_str());

        _HandleRequest( apRequest );
    }

    /* Wait for the Reader Thread to exit. */

    wxLogDebug("%s: Request the Reader thread to be shutdowned...", FNAME);
    if ( m_reader /*&& m_reader->IsRunning()*/ )  // TODO: Is it a good way to test?
    {
        wxThread::ExitCode exitCode = m_reader->Wait();
        wxLogDebug("%s: The Reader thread shutdowned with exit-code = [%d].", FNAME, exitCode);
    }

    return NULL;
}

void
hoxSocketWriter::_HandleRequest( hoxRequest_APtr apRequest )
{
    const char* FNAME = "hoxSocketWriter::_HandleRequest";
    hoxResult    result = hoxRC_ERR;
    hoxResponse_APtr response( new hoxResponse(apRequest->type, 
                                               apRequest->sender) );

    switch( apRequest->type )
    {
        case hoxREQUEST_LOGIN:
        {
            result = _Login( m_owner.GetHostname(),
                             m_owner.GetPort(),
                             _RequestToString( *apRequest ),
                             response->content );
            if ( result == hoxRC_HANDLED )
            {
                result = hoxRC_OK;  // Consider "success".
                break;
            }
            else if ( result != hoxRC_OK )
            {
                wxLogDebug("%s: *** ERROR *** Failed to connect to server.", FNAME);
                break;
            }
            break;
        }
		case hoxREQUEST_LOGOUT:   /* fall through */
        case hoxREQUEST_OUT_DATA: /* fall through */
        case hoxREQUEST_MOVE:     /* fall through */
        case hoxREQUEST_LIST:     /* fall through */
        case hoxREQUEST_NEW:      /* fall through */
        case hoxREQUEST_JOIN:     /* fall through */
        case hoxREQUEST_LEAVE:    /* fall through */
        case hoxREQUEST_RESIGN:   /* fall through */
        case hoxREQUEST_DRAW:     /* fall through */
        case hoxREQUEST_RESET:    /* fall through */
        case hoxREQUEST_MSG:
        {
            if ( ! m_owner.IsConnected() )
            {
                // NOTE: The connection could have been closed if the server is down.
                wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
                result = hoxRC_OK;  // Consider "success".
                break;
            }
            result = _WriteLine( m_socket, 
                                 _RequestToString( *apRequest ) );
            break;
        }
        default:
        {
            wxLogDebug("%s: *** WARN *** Unsupported Request [%s].", 
                FNAME, hoxUtil::RequestTypeToString(apRequest->type).c_str());
            result = hoxRC_NOT_SUPPORTED;
        }
    }

    if ( result != hoxRC_OK )
    {
        wxLogDebug("%s: * INFO * Request [%s]: return error-code = [%s]...", 
            FNAME, hoxUtil::RequestTypeToString(apRequest->type).c_str(), 
            hoxUtil::ResultToStr(result));
    }

#if 0
    /* If there are data, just return it to the caller. */
    if ( result != hoxRC_NOT_FOUND )
    {
        wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, request->type );
        response->code = result;
        event.SetEventObject( response.release() );  // Caller will de-allocate.
        wxPostEvent( m_owner.GetPlayer(), event );
    }
#endif
}

hoxResult
hoxSocketWriter::_Login( const wxString& sHostname,
                         const int       nPort,
                         const wxString& request,
                         wxString&       response )
{
    const char* FNAME = "hoxSocketWriter::_Login";
    hoxResult result;

    if ( m_owner.IsConnected() )
    {
        wxLogDebug("%s: The connection already established. END.", FNAME);
        return hoxRC_HANDLED;
    }

    /* Get the server address. */
    wxIPV4address addr;
    addr.Hostname( sHostname );
    addr.Service( nPort );

    wxLogDebug("%s: Trying to connect to [%s:%d]...", 
        FNAME, addr.Hostname().c_str(), addr.Service());

    if ( ! m_socket->Connect( addr, true /* wait */ ) )
    {
        wxLogError("%s: Failed to connect to the server [%s:%d]. Error = [%s].",
            FNAME, addr.Hostname().c_str(), addr.Service(), 
            hoxNetworkAPI::SocketErrorToString(m_socket->LastError()).c_str());
        return hoxRC_ERR;
    }

    wxLogDebug("%s: Succeeded! Connection established with the server.", FNAME);
    m_owner.SetConnected( true );

    //////////////////////////////////
    // Start the READER thread.
    _StartReader( m_socket );


	////////////////////////////
    // Send LOGIN request.
	wxLogDebug("%s: Sending LOGIN request over the network...", FNAME);
    result = _WriteLine( m_socket, request );
    if ( result != hoxRC_OK )
        return result;

    return hoxRC_OK;
}

void
hoxSocketWriter::_Disconnect()
{
    const char* FNAME = "hoxSocketWriter::_Disconnect";

    if ( m_socket != NULL )
    {
        wxLogDebug("%s: Closing the client socket...", FNAME);
        m_socket->Destroy();
        m_socket = NULL;
    }
    m_owner.SetConnected( false );
}

// ----------------------------------------------------------------------------
// hoxSocketReader
// ----------------------------------------------------------------------------

hoxSocketReader::hoxSocketReader( hoxSocketConnection& owner )
        : wxThread( wxTHREAD_JOINABLE )
        , m_owner( owner )
        , m_socket( NULL )
        , m_shutdownRequested( false )
{
}

hoxSocketReader::~hoxSocketReader()
{
    const char* FNAME = "hoxSocketReader::~hoxSocketReader";

    wxLogDebug("%s: ENTER.", FNAME);

    // *** The Writer Thread will take care of closing the socket.

    wxLogDebug("%s: END.", FNAME);
}

void*
hoxSocketReader::Entry()
{
    const char* FNAME = "hoxSocketReader::Entry";

    wxLogDebug("%s: ENTER.", FNAME);

    wxCHECK_MSG(m_socket, NULL, "Socket must be set first");

    const hoxRequestType type = hoxREQUEST_PLAYER_DATA;
    hoxResult result = hoxRC_OK;

    while (   !m_shutdownRequested
            && result != hoxRC_CLOSED )
    {
        hoxResponse_APtr apResponse( new hoxResponse(type) );

        if ( hoxRC_OK != (result = _ReadLine( m_socket, apResponse->content )) )
        {
            wxLogDebug("%s: *** INFO *** Failed to read incoming command.", FNAME);
            result = hoxRC_CLOSED;   // *** Shutdown the Thread.
        }
        wxLogDebug("%s: Received data [%s].", FNAME, apResponse->content.c_str());

        wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, type );
        apResponse->code = result;
        event.SetEventObject( apResponse.release() );  // Caller will de-allocate.
        wxPostEvent( m_owner.GetPlayer(), event );
    }

    return NULL;
}

//-----------------------------------------------------------------------------
// hoxSocketConnection
//-----------------------------------------------------------------------------

hoxSocketConnection::hoxSocketConnection()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxSocketConnection::hoxSocketConnection( const wxString&  sHostname,
                                          int              nPort )
        : hoxConnection()
        , m_sHostname( sHostname )
        , m_nPort( nPort )
        , m_bConnected( false )
        , m_player( NULL )
{
    const char* FNAME = "hoxSocketConnection::hoxSocketConnection";

    /* Create Writer thread. */
    wxLogDebug("%s: Create the Writer Thread...", FNAME);
    m_writer.reset( new hoxSocketWriter( *this ) );
}

hoxSocketConnection::~hoxSocketConnection()
{
    const char* FNAME = "hoxSocketConnection::~hoxSocketConnection";

    wxLogDebug("%s: ENTER.", FNAME);
    wxLogDebug("%s: END.", FNAME);
}

void
hoxSocketConnection::Start()
{
    const char* FNAME = "hoxSocketConnection::Start";

    wxLogDebug("%s: ENTER.", FNAME);

    this->StartWriter();
    // *** The Reader thread is managned by the Writer thread.
}

void
hoxSocketConnection::StartWriter()
{
    const char* FNAME = "hoxSocketConnection::StartWriter";

    wxLogDebug("%s: ENTER.", FNAME);

    if (    m_writer 
         && m_writer->IsRunning() )
    {
        wxLogDebug("%s: The connection has already been started. END.", FNAME);
        return;
    }

    if ( m_writer->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogDebug("%s: *** WARN *** Failed to create the Writer thread.", FNAME);
        return;
    }
    wxASSERT_MSG( !m_writer->IsDetached(), 
                  "The Writer thread must be joinable." );

    m_writer->Run();
}

void
hoxSocketConnection::Shutdown()
{
    const char* FNAME = "hoxSocketConnection::Shutdown";

    m_shutdownRequested = true;

    wxLogDebug("%s: Request the Writer thread to be shutdowned...", FNAME);
    if ( m_writer && m_writer->IsRunning() )  // TODO: Is it a good way to test?
    {
        wxThread::ExitCode exitCode = m_writer->Wait();
        wxLogDebug("%s: The Writer thread shutdowned with exit-code = [%d].", FNAME, exitCode);
    }
}

bool
hoxSocketConnection::AddRequest( hoxRequest* request )
{
    hoxRequest_APtr apRequest( request );
    return m_writer->AddRequest( apRequest );
}

const wxString 
_RequestToString( const hoxRequest& request )
{
	wxString result;

	result += "op=" + hoxUtil::RequestTypeToString( request.type );

	hoxCommand::Parameters::const_iterator it;
	for ( it = request.parameters.begin();
		  it != request.parameters.end(); ++it )
	{
		result += "&" + it->first + "=" + it->second;
	}
	
	return result;
}

hoxResult
_ReadLine( wxSocketBase* sock, 
           wxString&     result )
{
    const char* FNAME = __FUNCTION__;
    wxString commandStr;

    result = "";

	/* Read a line until "\n\n" */

	bool   bSawOne = false;
    wxChar c;

    for (;;)
    {
        sock->Read( &c, 1 );
#if 0
        if ( sock->LastCount() == 0 )  // Connection closed by the remote peer?
        {
            wxLogDebug("%s: Connection closed by the remote peer. Result message = [%s].",
                FNAME, commandStr.c_str());
            return hoxRC_CLOSED;  // Done.
        }
#endif
        if ( sock->LastCount() == 1 )
        {
			if ( !bSawOne && c == '\n' )
			{
				bSawOne = true;
			}
			else if ( bSawOne && c == '\n' )
			{
				result = commandStr;
				return hoxRC_OK;  // Done.
			}
            else
            {
                bSawOne = false;
                commandStr += c;

                // Impose some limit.
                if ( commandStr.size() >= hoxNETWORK_MAX_MSG_SIZE )
                {
                    wxLogDebug("%s: *** WARN *** Maximum message's size [%d] reached. Likely to be an error.", 
                        FNAME, hoxNETWORK_MAX_MSG_SIZE);
                    wxLogDebug("%s: *** WARN *** Partial read message (64 bytes) = [%s ...].", 
                        FNAME, commandStr.substr(0, 64).c_str());
                    break;
                }
            }
        }
        else if ( sock->Error() )
        {
            wxSocketError err = sock->LastError();
            // FIXME: This checking is not working. The error is always "OK".
            if (  err == wxSOCKET_TIMEDOUT )
                wxLogDebug("%s: Timeout.", FNAME);
            else
                wxLogDebug("%s: Some socket error [%d].", FNAME, err);
            break;
        }
        else
        {
            wxLogDebug("%s: No more data. Result message = [%s].", FNAME, commandStr.c_str());
            return hoxRC_NOT_FOUND;  // Done.
        }
    }

    return hoxRC_ERR;
}

hoxResult
_WriteLine( wxSocketBase*   sock, 
            const wxString& contentStr )
{
    const char* FNAME = __FUNCTION__;

	wxLogDebug("%s: Sending a request over the network...", FNAME);
	wxString sRequest;
	sRequest.Printf("%s\n", contentStr.c_str());

	wxUint32 requestSize = (wxUint32) sRequest.size();
	sock->Write( sRequest, requestSize );
	wxUint32 nWrite = sock->LastCount();
	if ( nWrite < requestSize )
	{
		wxLogDebug("%s: *** WARN *** Failed to send request [%s] ( %d < %d ). Error = [%s].", 
			FNAME, sRequest.c_str(), nWrite, requestSize, 
			hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
		return hoxRC_ERR;
	}

    return hoxRC_OK;
}

/************************* END OF FILE ***************************************/
