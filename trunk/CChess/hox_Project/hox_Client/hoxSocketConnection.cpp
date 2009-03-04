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
// hoxSocketWriter
// ----------------------------------------------------------------------------

hoxSocketWriter::hoxSocketWriter( wxEvtHandler*           evtHandler,
                                  const hoxServerAddress& serverAddress )
        : wxThread( wxTHREAD_JOINABLE )
        , m_evtHandler( evtHandler )
        , m_serverAddress( serverAddress )
        , m_socket( NULL )
        , m_shutdownRequested( false )
        , m_bConnected( false )
{
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * NOTE: We need to use wxSOCKET_BLOCK option to avoid the problem
     *       of high CPU usage in secondary threads.
     *       To find out why, look into the source code of wxSocket).
     *
     * Reference:
     *     http://www.wxwidgets.org/wiki/index.php/WxSocket
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     */
    const wxSocketFlags socketFlags = wxSOCKET_BLOCK; // *** See NOTE above!
    m_socket = new wxSocketClient( socketFlags );

    m_socket->Notify( false /* Disable socket-events */ );
}

hoxSocketWriter::~hoxSocketWriter()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
    this->Disconnect();
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

void
hoxSocketWriter::StartReader( wxSocketClient* socket )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    if ( m_reader && m_reader->IsRunning() )
    {
        wxLogDebug("%s: The Reader has already been started. END.", __FUNCTION__);
        return;
    }

    wxLogDebug("%s: Create the Reader Thread...", __FUNCTION__);
    m_reader = this->CreateReader( m_evtHandler, socket );

    if ( m_reader->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogDebug("%s: *WARN* Failed to create the Reader thread.", __FUNCTION__);
        m_reader.reset();
        return;
    }
    wxASSERT_MSG(!m_reader->IsDetached(), "The Reader thread must be joinable");

    m_reader->Run();
}

hoxSocketReader_SPtr
hoxSocketWriter::CreateReader( wxEvtHandler*   evtHandler,
                               wxSocketClient* socket )
{
    hoxSocketReader_SPtr reader( new hoxSocketReader( evtHandler, socket ) );
    return reader;
}

void
hoxSocketWriter::WaitUntilReaderExit()
{
    if ( m_reader )
    {
        wxLogDebug("%s: Request the Reader thread to be shutdowned...", __FUNCTION__);
        wxThread::ExitCode exitCode = 0;
        m_reader->Delete( &exitCode );
        wxLogDebug("%s: The Reader thread shutdowned with exit-code = [%d].", __FUNCTION__, exitCode);
        m_reader.reset();
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
        wxLogDebug("%s: Processing request Type = [%s]...", 
            __FUNCTION__, hoxUtil::RequestTypeToString(apRequest->type).c_str());

        const hoxRequestType requestType = apRequest->type;
        if ( requestType == hoxREQUEST_LOGOUT )
        {
            this->WaitUntilReaderExit();
            m_shutdownRequested = true; // !!! Force to close !!!
        }

        this->HandleRequest( apRequest );
    }

    /* Make sure that the Reader Thread to exit. */
    this->WaitUntilReaderExit();

    /* Close the socket-connection. */
    this->Disconnect();

    /* Notify the Player. */
    wxLogDebug("%s: Notify event-handler of connection CLOSED.", __FUNCTION__);
    const hoxRequestType type = hoxREQUEST_PLAYER_DATA;
    hoxResponse_APtr apResponse( new hoxResponse(type) );
    wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, type );
    apResponse->code = hoxRC_CLOSED;
    event.SetEventObject( apResponse.release() );  // Caller will de-allocate.
    wxPostEvent( m_evtHandler, event );

    return NULL;
}

void
hoxSocketWriter::HandleRequest( hoxRequest_APtr apRequest )
{
    hoxResult    result = hoxRC_OK;
    const hoxRequestType requestType = apRequest->type;
    hoxResponse_APtr apResponse( new hoxResponse(requestType, 
                                                 apRequest->sender) );

    /* Make sure the connection is established. */
    if ( ! m_bConnected )
    {
        wxString sError;
        result = this->Connect( sError );
        if ( result != hoxRC_OK )
        {
            wxLogDebug("%s: *WARN* Failed to establish a connection.", __FUNCTION__);
            apResponse->content = sError;
        }
    }

    /* Send the request. */
    if ( result == hoxRC_OK )
    {
        const wxString sRequest = apRequest->ToString();
        result = _WriteLine( m_socket, sRequest );
    }

    /* Return error. */
    if ( result != hoxRC_OK )
    {
        wxLogDebug("%s: *INFO* Request [%s]: return error-code = [%s]...", 
            __FUNCTION__, hoxUtil::RequestTypeToString(requestType).c_str(), 
            hoxUtil::ResultToStr(result));

        /* Notify the Player of this error. */
        wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, requestType );
        apResponse->code = result;
        event.SetEventObject( apResponse.release() );  // Caller will de-allocate.
        wxPostEvent( m_evtHandler, event );
    }
}

hoxResult
hoxSocketWriter::Connect( wxString& sError )
{
    if ( m_bConnected )
    {
        wxLogDebug("%s: *INFO* The connection already established. END.", __FUNCTION__);
        return hoxRC_OK; // Consider "success".
    }

    /* Get the server address. */
    wxIPV4address addr;
    addr.Hostname( m_serverAddress.name );
    addr.Service( m_serverAddress.port );

    wxLogDebug("%s: Trying to connect to [%s]...", __FUNCTION__, m_serverAddress.c_str());

    if ( ! m_socket->Connect( addr, true /* wait */ ) )
    {
        wxLogWarning("%s: *WARN* Failed to connect to the server [%s]. Error = [%s].",
            __FUNCTION__, m_serverAddress.c_str(), 
            hoxNetworkAPI::SocketErrorToString(m_socket->LastError()).c_str());
        sError = "Failed to connect to server";
        return hoxRC_ERR;
    }
    
    m_socket->SetTimeout( hoxSOCKET_CLIENT_SOCKET_TIMEOUT );

    this->StartReader( m_socket ); // Start the READER thread.

    wxLogDebug("%s: Succeeded! Connection established to the server.", __FUNCTION__);
    m_bConnected = true;

    return hoxRC_OK;
}

void
hoxSocketWriter::Disconnect()
{
    if ( m_socket != NULL )
    {
        wxLogDebug("%s: Closing the client socket...", __FUNCTION__);
        m_socket->Destroy();
        m_socket = NULL;
    }
    m_bConnected = false;
}

hoxResult
hoxSocketWriter::_WriteLine( wxSocketBase*   sock, 
                             const wxString& contentStr )
{
	wxString sRequest;
	sRequest.Printf("%s\n", contentStr.c_str());

    return hoxNetworkAPI::WriteLine( sock, sRequest );
}

// ----------------------------------------------------------------------------
// hoxSocketReader
// ----------------------------------------------------------------------------

hoxSocketReader::hoxSocketReader( wxEvtHandler*   evtHandler,
                                  wxSocketClient* socket )
        : wxThread( wxTHREAD_JOINABLE )
        , m_evtHandler( evtHandler )
        , m_socket( socket )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

void*
hoxSocketReader::Entry()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    wxCHECK_MSG(m_socket, NULL, "Socket must be set first");

    const hoxRequestType type = hoxREQUEST_PLAYER_DATA;
    hoxResult result = hoxRC_OK;

    while (   !TestDestroy()
            && result != hoxRC_CLOSED )
    {
        hoxResponse_APtr apResponse( new hoxResponse(type) );

        if ( !m_socket->WaitForRead( 0, 500 ) ) // timeout (0.5 sec)?
        {
            continue;
        }

        if ( hoxRC_OK != (result = this->ReadLine( m_socket, 
                                                   apResponse->data )) )
        {
            wxLogDebug("%s: *INFO* Failed to read incoming command.", __FUNCTION__);
            result = hoxRC_CLOSED;   // *** Shutdown the Thread.
        }

        /* Notify the Player. */
        wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, type );
        apResponse->code = result;
        event.SetEventObject( apResponse.release() );  // Caller will de-allocate.
        wxPostEvent( m_evtHandler, event );
    }

    return NULL;
}

hoxResult
hoxSocketReader::ReadLine( wxSocketBase*   sock, 
                           wxMemoryBuffer& data )
{
    data.SetDataLen( 0 );  // Clear old data.

    const size_t maxSize = hoxNETWORK_MAX_MSG_SIZE;

	/* Read a line until "\n\n" */

	bool   bSawOne = false; // just saw one '\n'?
    wxUint8 c;

    for (;;)
    {
        sock->Read( &c, 1 );
        if ( sock->LastCount() == 1 )
        {
			if ( !bSawOne && c == '\n' )
			{
				bSawOne = true;
                data.AppendByte( c );
			}
			else if ( bSawOne && c == '\n' )
			{
                data.SetDataLen( data.GetDataLen()-1 ); // Remove '\n'
				return hoxRC_OK;  // Done.
			}
            else
            {
                bSawOne = false;

                data.AppendByte( c );
                if ( data.GetDataLen() >= maxSize ) // Impose some limit.
                {
                    wxLogDebug("%s: *WARN* Max size [%d] reached.", __FUNCTION__, maxSize);
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
            if (  err == wxSOCKET_TIMEDOUT ) {
                wxLogDebug("%s: *INFO* Socket timeout (%d).", __FUNCTION__, data.GetDataLen());
            } else {
                wxLogDebug("%s: *WARN* Some socket error [%s] (%d).", __FUNCTION__,
                    hoxNetworkAPI::SocketErrorToString(err).c_str(), data.GetDataLen());
            }
            break;
        }
        else
        {
            wxLogDebug("%s: No more data. Len of data = [%d].", __FUNCTION__, data.GetDataLen());
            return hoxRC_NOT_FOUND;  // Done.
        }
    }

    return hoxRC_ERR;
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

    this->StartWriter();
    // *** This thread will also create and manage the Reader thread.
}

void
hoxSocketConnection::StartWriter()
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
    wxLogDebug("%s: Request the Writer thread to shutdown...", __FUNCTION__);
    if ( m_writer )
    {
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
