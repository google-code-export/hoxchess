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

// ----------------------------------------------------------------------------
// hoxSocketWriter
// ----------------------------------------------------------------------------

hoxSocketWriter::hoxSocketWriter( wxEvtHandler*           player,
                                  const hoxServerAddress& serverAddress )
        : wxThread( wxTHREAD_JOINABLE )
        , m_player( player )
        , m_serverAddress( serverAddress )
        , m_socket( NULL )
        , m_shutdownRequested( false )
        , m_bConnected( false )
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
    m_socket->SetTimeout( hoxSOCKET_CLIENT_SOCKET_TIMEOUT );
}

hoxSocketWriter::~hoxSocketWriter()
{
    const char* FNAME = __FUNCTION__;

    wxLogDebug("%s: ENTER.", FNAME);

    _Disconnect();

    wxLogDebug("%s: END.", FNAME);
}

bool
hoxSocketWriter::AddRequest( hoxRequest_APtr apRequest )
{
    const char* FNAME = __FUNCTION__;

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
    const char* FNAME = __FUNCTION__;

    wxLogDebug("%s: ENTER.", FNAME);

    if (    m_reader 
         && m_reader->IsRunning() )
    {
        wxLogDebug("%s: The connection has already been started. END.", FNAME);
        return;
    }

    /* Create Reader thread. */
    wxLogDebug("%s: Create the Reader Thread...", FNAME);
    m_reader.reset( new hoxSocketReader( m_player ) );

    /* Set the socket to READ from. */
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
    const char* FNAME = __FUNCTION__;

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
    const char* FNAME = __FUNCTION__;
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

        this->HandleRequest( apRequest );
    }

    /* Wait for the Reader Thread to exit. */

    wxLogDebug("%s: Request the Reader thread to be shutdowned...", FNAME);
    if ( m_reader != NULL )
    {
        wxThread::ExitCode exitCode = m_reader->Wait();
        wxLogDebug("%s: The Reader thread shutdowned with exit-code = [%d].", FNAME, exitCode);
    }

    return NULL;
}

void
hoxSocketWriter::HandleRequest( hoxRequest_APtr apRequest )
{
    const char* FNAME = __FUNCTION__;
    hoxResult    result = hoxRC_ERR;
    const hoxRequestType requestType = apRequest->type;
    hoxResponse_APtr apResponse( new hoxResponse(requestType, 
                                                 apRequest->sender) );

    const wxString sRequest = apRequest->ToString();

    switch( requestType )
    {
        case hoxREQUEST_LOGIN:
        {
            result = _Login( m_serverAddress,
                             sRequest,
                             apResponse->content );
            break;
        }
		case hoxREQUEST_LOGOUT:       /* fall through */
        case hoxREQUEST_MOVE:         /* fall through */
        case hoxREQUEST_LIST:         /* fall through */
        case hoxREQUEST_NEW:          /* fall through */
        case hoxREQUEST_JOIN:         /* fall through */
        case hoxREQUEST_LEAVE:        /* fall through */
        case hoxREQUEST_UPDATE:       /* fall through */
        case hoxREQUEST_RESIGN:       /* fall through */
        case hoxREQUEST_DRAW:         /* fall through */
        case hoxREQUEST_RESET:        /* fall through */
        case hoxREQUEST_INVITE:       /* fall through */
        case hoxREQUEST_PLAYER_INFO:  /* fall through */
        case hoxREQUEST_MSG:
        {
            if ( ! m_bConnected )
            {
                // NOTE: The connection could have been closed if the server is down.
                wxLogDebug("%s: Connection not yet established or has been closed.", FNAME);
                result = hoxRC_OK;  // Consider "success".
                break;
            }
            result = _WriteLine( m_socket, 
                                 sRequest );
            break;
        }
        default:
        {
            wxLogDebug("%s: *** WARN *** Unsupported Request [%s].", 
                FNAME, hoxUtil::RequestTypeToString(requestType).c_str());
            result = hoxRC_NOT_SUPPORTED;
        }
    }

    if ( result != hoxRC_OK )
    {
        wxLogDebug("%s: * INFO * Request [%s]: return error-code = [%s]...", 
            FNAME, hoxUtil::RequestTypeToString(requestType).c_str(), 
            hoxUtil::ResultToStr(result));

        /* Notify the Player of this error. */
        wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, requestType );
        apResponse->code = result;
        event.SetEventObject( apResponse.release() );  // Caller will de-allocate.
        wxPostEvent( m_player, event );
    }
}

hoxResult
hoxSocketWriter::_Login( const hoxServerAddress& serverAddress,
                         const wxString&         sRequest,
                         wxString&               sResponse )
{
    const char* FNAME = __FUNCTION__;

    if ( m_bConnected )
    {
        wxLogDebug("%s: * INFO *The connection already established. END.", FNAME);
        return hoxRC_OK; // Consider "success".
    }

    /* Get the server address. */
    wxIPV4address addr;
    addr.Hostname( serverAddress.name );
    addr.Service( serverAddress.port );

    wxLogDebug("%s: Trying to connect to [%s]...", FNAME, serverAddress.c_str());

    if ( ! m_socket->Connect( addr, true /* wait */ ) )
    {
        wxLogDebug("%s: *** WARN *** Failed to connect to the server [%s]. Error = [%s].",
            FNAME, serverAddress.c_str(), 
            hoxNetworkAPI::SocketErrorToString(m_socket->LastError()).c_str());
        sResponse = "Failed to connect to server";
        return hoxRC_ERR;
    }

    wxLogDebug("%s: Succeeded! Connection established with the server.", FNAME);
    m_bConnected = true;

    //////////////////////////////////
    // Start the READER thread.
    _StartReader( m_socket );

	////////////////////////////
    // Send LOGIN request.
	wxLogDebug("%s: Sending LOGIN request over the network...", FNAME);
    return _WriteLine( m_socket, sRequest );
}

void
hoxSocketWriter::_Disconnect()
{
    const char* FNAME = __FUNCTION__;

    if ( m_socket != NULL )
    {
        wxLogDebug("%s: Closing the client socket...", FNAME);
        m_socket->Destroy();
        m_socket = NULL;
    }
    m_bConnected = false;
}

hoxResult
hoxSocketWriter::_WriteLine( wxSocketBase*   sock, 
                             const wxString& contentStr )
{
    const char* FNAME = __FUNCTION__;

	wxLogDebug("%s: Sending a request over the network...", FNAME);
	wxString sRequest;
	sRequest.Printf("%s\n", contentStr.c_str());

    return hoxNetworkAPI::WriteLine( sock, sRequest );
}

// ----------------------------------------------------------------------------
// hoxSocketReader
// ----------------------------------------------------------------------------

hoxSocketReader::hoxSocketReader( wxEvtHandler* player )
        : wxThread( wxTHREAD_JOINABLE )
        , m_player( player )
        , m_socket( NULL )
        , m_shutdownRequested( false )
{
    const char* FNAME = __FUNCTION__;

    wxLogDebug("%s: ENTER.", FNAME);
}

hoxSocketReader::~hoxSocketReader()
{
    const char* FNAME = __FUNCTION__;

    wxLogDebug("%s: ENTER.", FNAME);

    // *** The Writer Thread will take care of closing the socket.

    wxLogDebug("%s: END.", FNAME);
}

void*
hoxSocketReader::Entry()
{
    const char* FNAME = __FUNCTION__;

    wxLogDebug("%s: ENTER.", FNAME);

    wxCHECK_MSG(m_socket, NULL, "Socket must be set first");

    const hoxRequestType type = hoxREQUEST_PLAYER_DATA;
    hoxResult result = hoxRC_OK;

    while (   !m_shutdownRequested
            && result != hoxRC_CLOSED )
    {
        hoxResponse_APtr apResponse( new hoxResponse(type) );

        if ( hoxRC_OK != (result = this->ReadLine( m_socket, 
                                                   apResponse->content )) )
        {
            wxLogDebug("%s: *** INFO *** Failed to read incoming command.", FNAME);
            result = hoxRC_CLOSED;   // *** Shutdown the Thread.
        }
        //wxLogDebug("%s: Received data [%s].", FNAME, apResponse->content.c_str());

        /* Notify the Player. */
        wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, type );
        apResponse->code = result;
        event.SetEventObject( apResponse.release() );  // Caller will de-allocate.
        wxPostEvent( m_player, event );
    }

    return NULL;
}

hoxResult
hoxSocketReader::ReadLine( wxSocketBase* sock, 
                           wxString&     result )
{
    wxString commandStr;

    result = "";

	/* Read a line until "\n\n" */

	bool   bSawOne = false;
    wxUint8 c;

    for (;;)
    {
        sock->Read( &c, 1 );
        if ( sock->LastCount() == 1 )
        {
			if ( !bSawOne && c == '\n' )
			{
				bSawOne = true;
                commandStr += c;
			}
			else if ( bSawOne && c == '\n' )
			{
                result = commandStr.substr(0, commandStr.size()-1);
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
                        __FUNCTION__, hoxNETWORK_MAX_MSG_SIZE);
                    wxLogDebug("%s: *** WARN *** Partial read message (64 bytes) = [%s ...].", 
                        __FUNCTION__, commandStr.substr(0, 64).c_str());
                    break;
                }
            }
        }
        else if ( sock->Error() )
        {
            wxSocketError err = sock->LastError();
            /* NOTE: This checking is now working given that
             *       we have used wxSOCKET_BLOCK as the socket option.
             */
            if (  err == wxSOCKET_TIMEDOUT )
                wxLogDebug("%s: * INFO * Socket timeout.", __FUNCTION__);
            else
                wxLogDebug("%s: * INFO * Some socket error [%d].", __FUNCTION__, err);
            break;
        }
        else
        {
            wxLogDebug("%s: No more data. Result message = [%s].", __FUNCTION__, commandStr.c_str());
            return hoxRC_NOT_FOUND;  // Done.
        }
    }

    return hoxRC_ERR;
}

//-----------------------------------------------------------------------------
// hoxSocketConnection
//-----------------------------------------------------------------------------

hoxSocketConnection::hoxSocketConnection()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxSocketConnection::hoxSocketConnection( const hoxServerAddress& serverAddress,
                                          wxEvtHandler*           player )
        : hoxConnection( player )
        , m_serverAddress( serverAddress )
{
    const char* FNAME = __FUNCTION__;

    wxLogDebug("%s: ENTER.", FNAME);
    wxLogDebug("%s: END.", FNAME);
}

hoxSocketConnection::~hoxSocketConnection()
{
    const char* FNAME = __FUNCTION__;

    wxLogDebug("%s: ENTER.", FNAME);
    wxLogDebug("%s: END.", FNAME);
}

void
hoxSocketConnection::Start()
{
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER.", FNAME);

    this->StartWriter();
    // *** This thread will also create and manage the Reader thread.
}

void
hoxSocketConnection::StartWriter()
{
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER.", FNAME);

    if (    m_writer.get() != NULL 
         && m_writer->IsRunning() )
    {
        wxLogDebug("%s: The connection has already been started. END.", FNAME);
        return;
    }

    /* Create Writer thread. */
    wxLogDebug("%s: Create the Writer Thread...", FNAME);
    m_writer.reset( new hoxSocketWriter( this->GetPlayer(), 
                                         m_serverAddress ) );
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
    const char* FNAME = __FUNCTION__;

    wxLogDebug("%s: Request the Writer thread to be shutdowned...", FNAME);
    if ( m_writer.get() != NULL )
    {
        wxThread::ExitCode exitCode = m_writer->Wait();
        wxLogDebug("%s: The Writer thread shutdowned with exit-code = [%d].", FNAME, exitCode);
    }
}

bool
hoxSocketConnection::AddRequest( hoxRequest_APtr apRequest )
{
    wxCHECK_MSG(m_writer.get() != NULL, false, "Writer is not yet created");
    return m_writer->AddRequest( apRequest );
}

bool
hoxSocketConnection::IsConnected() const
{ 
    return (    m_writer.get() != NULL 
             && m_writer->IsConnected() );
}

/************************* END OF FILE ***************************************/
