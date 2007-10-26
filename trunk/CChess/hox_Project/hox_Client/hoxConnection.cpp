/////////////////////////////////////////////////////////////////////////////
// Name:            hoxConnection.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/23/2007
//
// Description:     The Connection Thread to help a "network" player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxConnection.h"
#include "hoxWWWPlayer.h"
#include "hoxEnums.h"
#include "hoxServer.h"
#include "hoxTableMgr.h"
#include "hoxWWWThread.h"

#include <wx/sstream.h>
#include <wx/protocol/http.h>
#include <wx/tokenzr.h>


//-----------------------------------------------------------------------------
// hoxConnection
//-----------------------------------------------------------------------------


hoxConnection::hoxConnection( const wxString&  sHostname,
                              int              nPort,
                              hoxMyPlayer*     player )
        : wxThreadHelper()
        , m_sHostname( sHostname )
        , m_nPort( nPort )
        , m_shutdownRequested( false )
        , m_pSClient( NULL )
        , m_player( player )
{
}

hoxConnection::~hoxConnection()
{
    const char* FNAME = "hoxConnection::~hoxConnection";

    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId(), wxThread::IsMain(), FNAME);

    _Disconnect();
}

void*
hoxConnection::Entry()
{
    const char* FNAME = "hoxConnection::Entry";
    hoxRequest* request = NULL;

    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId(), wxThread::IsMain(), FNAME);

    while ( !m_shutdownRequested && m_semRequests.Wait() == wxSEMA_NO_ERROR )
    {
        request = _GetRequest();
        if ( request == NULL )
        {
            wxASSERT_MSG( m_shutdownRequested, "This thread must be shutdowning." );
            break;  // Exit the thread.
        }
        wxLogDebug("%s: Processing request Type [%d]...", FNAME, request->type);

         _HandleRequest( request );
        delete request;
    }

    return NULL;
}

void 
hoxConnection::AddRequest( hoxRequest* request )
{
    const char* FNAME = "hoxConnection::AddRequest";
    wxMutexLocker lock( m_mutexRequests );

    if ( m_shutdownRequested )
    {
        wxLogWarning(wxString::Format("%s: Deny request [%d]. The thread is shutdowning.", 
                        FNAME, request->type));
        delete request;
        return;
    }

    m_requests.push_back( request );
    m_semRequests.Post();
}

void 
hoxConnection::_HandleRequest( hoxRequest* request )
{
    const char* FNAME = "hoxConnection::_HandleRequest";
    hoxResult  result = hoxRESULT_ERR;
    hoxResponse* response = new hoxResponse( request->type );

    switch( request->type )
    {
        case hoxREQUEST_TYPE_CONNECT:
            result = _SendRequest_Connect( request->content, response->content );
            break;

        case hoxREQUEST_TYPE_LISTEN:
            result = _HandleRequest_Listen( request );
            break;

        case hoxREQUEST_TYPE_TABLE_MOVE:
            result = _HandleCommand_TableMove(request); 
            break;

        case hoxREQUEST_TYPE_DATA:
            result = _HandleRequest_Data( request );
            break;

        case hoxREQUEST_TYPE_POLL:     /* fall through */
        case hoxREQUEST_TYPE_MOVE:     /* fall through */
        case hoxREQUEST_TYPE_LIST:     /* fall through */
        case hoxREQUEST_TYPE_NEW:      /* fall through */
        case hoxREQUEST_TYPE_JOIN:     /* fall through */
        case hoxREQUEST_TYPE_LEAVE:
            result = _SendRequest( request->content, response->content );
            break;

        default:
            wxLogError("%s: Unsupported request Type [%d].", FNAME, request->type);
            result = hoxRESULT_NOT_SUPPORTED;
            response->content = "";
            break;
    }

    /* Keep-alive if requested. */
    if ( (request->flags & hoxREQUEST_FLAG_KEEP_ALIVE) != 0 )
    {
        _Disconnect();
    }

    /* NOTE: If there was error, just return it to the caller. */

    if ( request->sender != NULL )
    {
        wxCommandEvent event( hoxEVT_WWW_RESPONSE );
        event.SetInt( result );
        event.SetEventObject( response );  // Caller will de-allocate.
        wxPostEvent( request->sender, event );
    }
    else
    {
        delete response;
    }
}

hoxResult        
hoxConnection::_SendRequest_Connect( const wxString& request, 
                                     wxString&       response )
{
    const char* FNAME = "hoxConnection::_SendRequest_Connect";
    hoxResult result = hoxRESULT_ERR;  // Assume: failure.
    wxUint32 nWrite;
    wxUint32 nRead;
    wxChar* buf = NULL;

    /* Delete the old connection, if any. */
    _Disconnect();
    wxASSERT_MSG( m_pSClient == NULL, "The previous connection should have been closed." );

    /* Make a new connection */

    m_pSClient = new wxSocketClient( /* wxSOCKET_NONE */ wxSOCKET_WAITALL );
    m_pSClient->Notify( false /* Disable socket-events */ );

    // Get the server address.
    wxIPV4address addr;
    addr.Hostname( m_sHostname );
    addr.Service( m_nPort );

    wxLogDebug(wxString::Format(_("Trying to connect to [%s:%d] (timeout = 10 sec)..."), 
                        m_sHostname, m_nPort));
    m_pSClient->Connect( addr, false /* no-wait */ );
    m_pSClient->WaitOnConnect( 10 /* wait for 10 seconds */ );

    if ( ! m_pSClient->IsConnected() ) 
    {
        wxLogError(wxString::Format("%s: Failed to connect to the server [%s:%d]",
                                       FNAME, m_sHostname, m_nPort));
        goto exit_label;
    }
    wxLogDebug(wxString::Format("%s: Succeeded! Connection established with the server.", FNAME));

    // Tell the server which 'command' we are sending
    m_pSClient->Write( request.c_str(), (wxUint32) request.size() );
    nWrite = m_pSClient->LastCount();
    if ( nWrite < request.size() )
    {
        wxLogError(wxString::Format("%s: Failed to write request. [%d] < [%d]", FNAME, nWrite, request.size()));
        goto exit_label;
    }

    // Wait until data available (will also return if the connection is lost)
    wxLogDebug(wxString::Format("%s: Waiting for response from the server (timeout = 5 sec)...", FNAME));
    m_pSClient->WaitForRead(5 /* seconds */);

    /***************************
     * Read the response
     ***************************/

    if ( ! m_pSClient->IsData() )
    {
        wxLogError(wxString::Format("%s: Timeout! Sending comand failed.", FNAME));
        goto exit_label;
    }

    buf = new wxChar[hoxNETWORK_MAX_MSG_SIZE];

    m_pSClient->ReadMsg( buf, hoxNETWORK_MAX_MSG_SIZE );
    nRead = m_pSClient->LastCount();

    if ( nRead == 0 )
    {
        wxLogError(wxString::Format("%s: Failed to read response.", FNAME));
        goto exit_label;
    }

    response.assign( buf, nRead );
    result = hoxRESULT_OK;  // Finally, success.

exit_label:
    if ( result != hoxRESULT_OK )
    {
        _Disconnect();
    }
    delete[] buf;
    return result;
}

hoxResult   
hoxConnection::_HandleRequest_Listen( hoxRequest*  request )
{
    const char* FNAME = "hoxConnection::_HandleRequest_Listen";

    wxLogDebug(wxString::Format("%s: ENTER.", FNAME));

    if ( m_pSClient == NULL )
    {
        wxLogError(wxString::Format("%s: Connection is not yet established.", FNAME));
        return hoxRESULT_ERR;
    }

    wxASSERT_MSG( request->sender != NULL, "Sender must not be NULL." );

    // Setup the event handler and let the player handles all socket events.
    m_pSClient->SetEventHandler( *(request->sender), CLIENT_SOCKET_ID );
    m_pSClient->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    m_pSClient->Notify(true);

    // Clear the sender since there is no need to send back a response.
    request->sender = NULL;

    return hoxRESULT_OK;
}

hoxResult   
hoxConnection::_HandleRequest_Data( hoxRequest*  request )
{
    const char* FNAME = "hoxConnection::_HandleRequest_Data";

    wxLogDebug(wxString::Format("%s: ENTER.", FNAME));


    //wxSocketBase* sock = request->socket;
    wxSocketBase* sock = m_pSClient;
    //wxASSERT_MSG( sock == m_pendingSock, _T("Sockets should match!") );
    
    // Now we process the event
    //switch( request->socketEvent )
    //{
    //    case wxSOCKET_INPUT:
        {
            wxString commandStr;
            hoxResult result = hoxServer::read_line( sock, commandStr );
            
            wxLogDebug(wxString::Format("%s: Received command-string [%s] received from client", 
                            FNAME, commandStr));
            
            hoxCommand   command;
            result = hoxServer::parse_command( commandStr, command );
            if ( result != hoxRESULT_OK )
            {
                wxLogWarning(wxString::Format("%s: Failed to parse command-string [%s].", 
                            FNAME, commandStr));
            }
            else
            {
                switch ( command.type )
                {
                    case hoxREQUEST_TYPE_MOVE:
                    {
                        _HandleCommand_Move(request, command); 
                        break;
                    }

                    default:
                        break;
                }
            }
            //break;
        }
        //case wxSOCKET_LOST:
        //{
        //    //wxLogDebug(wxString::Format("%s: Deleting pending socket.", FNAME));
        //    //_DestroyActiveSocket( sock );
        //    break;
        //}
        //default: 
        //    break;
    //}

    return hoxRESULT_OK;
}

hoxResult   
hoxConnection::_HandleCommand_Move( hoxRequest*   request, 
                                    hoxCommand&   command )
{
    const char* FNAME = "hoxConnection::_HandleCommand_Move";

    wxUint32 nWrite;
    wxString response;
    hoxNetworkEvent networkEvent;

    wxSocketBase* sock = m_pSClient;

    wxString moveStr = command.parameters["move"];
    wxString tableId = command.parameters["tid"];
    wxString playerId = command.parameters["pid"];
    hoxPlayer* player = NULL;

    // Find the table hosted on this system using the specified table-Id.
    hoxTable* table = hoxTableMgr::GetInstance()->FindTable( tableId );

    if ( table == NULL )
    {
        wxLogError(wxString::Format("%s: Table [%s] not found.", FNAME, tableId));
        response << "1\r\n"  // code
                 << "Table " << tableId << " not found.\r\n";
        goto exit_label;
    }

    /* Look up player */

    if ( table->GetRedPlayer() != NULL && table->GetRedPlayer()->GetName() == playerId )
    {
        player = table->GetRedPlayer();
    }
    else if ( table->GetBlackPlayer() != NULL && table->GetBlackPlayer()->GetName() == playerId )
    {
        player = table->GetBlackPlayer();
    }
    else
    {
        wxLogError(wxString::Format("%s: Player [%s] not found at the table [%s].", 
                        FNAME, playerId, tableId));
        response << "2\r\n"  // code
                 << "Player " << playerId << " not found.\r\n";
        goto exit_label;
    }

    networkEvent.content = moveStr;
    networkEvent.type = hoxNETWORK_EVENT_TYPE_NEW_MOVE;

    // Inform our table...
    table->OnEvent_FromWWWNetwork( player, networkEvent );

    // Finally, return 'success'.
    response << "0\r\n"       // error-code = SUCCESS
             << "INFO: (MOVE) Move at Table [" << tableId << "] OK\r\n";

exit_label:
    // Send back response.
    nWrite = (wxUint32) response.size();
    sock->WriteMsg( response, nWrite );
    if ( sock->LastCount() != nWrite )
    {
        wxLogError(wxString::Format("%s: Writing to  socket failed.", FNAME));
        return hoxRESULT_ERR;
    }

    return hoxRESULT_OK;
}

hoxResult   
hoxConnection::_HandleCommand_TableMove( hoxRequest* requestCmd )
{
    const char* FNAME = "hoxConnection::_HandleCommand_TableMove";

    wxUint32 nWrite;
    wxString request;

    hoxResult result;
    wxSocketBase* sock = m_pSClient;

    wxString responseStr;
    int        returnCode = -1;
    wxString   returnMsg;
    wxString   commandStr;
    hoxCommand command;
    wxString  tableId;
    wxString  playerId;
    wxString  moveStr;
    wxUint32  nRead;
    wxChar*   buf = NULL;

    wxLogDebug(wxString::Format("%s: ENTER.", FNAME));    

    // We disable input events, so that the test doesn't trigger
    // wxSocketEvent again.
    sock->SetNotify(wxSOCKET_LOST_FLAG); // remove the wxSOCKET_INPUT_FLAG!!!

    // Remove new-line characters.
    commandStr = requestCmd->content;
    commandStr = commandStr.Trim();

    result = hoxServer::parse_command( commandStr, command );
    if ( result != hoxRESULT_OK )
    {
        wxLogError(wxString::Format("%s: Failed to parse command-string [%s].", 
                    FNAME, commandStr));
        result = hoxRESULT_ERR;
        goto exit_label;
    }

    tableId  = command.parameters["tid"];
    playerId = command.parameters["pid"];
    moveStr  = command.parameters["move"];

    request = wxString::Format("op=MOVE&tid=%s&pid=%s&move=%s\r\n", 
                        tableId, playerId, moveStr);

    // Send request.
    nWrite = (wxUint32) request.size();
    sock->Write( request, nWrite );
    if ( sock->LastCount() != nWrite )
    {
        wxLogError(wxString::Format("%s: Writing to  socket failed.", FNAME));
        result = hoxRESULT_ERR;
        goto exit_label;
    }

    // Wait until data available (will also return if the connection is lost)
    wxLogDebug(wxString::Format("%s: Waiting for response from the network (timeout = 2 sec)...", FNAME));
    sock->WaitForRead(2);

    // Read back the response.

    buf = new wxChar[hoxNETWORK_MAX_MSG_SIZE];

    sock->ReadMsg( buf, hoxNETWORK_MAX_MSG_SIZE );
    nRead = sock->LastCount();

    if ( nRead == 0 )
    {
        wxLogWarning(wxString::Format("%s: Failed to read response.", FNAME));
        result = hoxRESULT_ERR;
        goto exit_label;
    }

    responseStr.assign( buf, nRead );

    /* Parse the response */
    result = hoxWWWThread::parse_string_for_simple_response( responseStr,
                                                             returnCode,
                                                             returnMsg );
    if ( result != hoxRESULT_OK )
    {
        wxLogError(wxString::Format("%s: Failed to parse for SEND-MOVE's response.", FNAME));
        result = hoxRESULT_ERR;
        goto exit_label;
    }
    else if ( returnCode != 0 )
    {
        wxLogError(wxString::Format("%s: Send MOVE failed. [%s]", FNAME, returnMsg));
        result = hoxRESULT_ERR;
        goto exit_label;
    }

    result = hoxRESULT_OK;

exit_label:
    delete[] buf;
    // Enable the input flag again.
    sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);

    return result;
}

hoxRequest*
hoxConnection::_GetRequest()
{
    const char* FNAME = "hoxConnection::_GetRequest";
    wxMutexLocker lock( m_mutexRequests );

    hoxRequest* request = NULL;

    wxASSERT_MSG( !m_requests.empty(), "We must have at least one request.");
    request = m_requests.front();
    m_requests.pop_front();

    /* Handle SHUTDOWN request here to avoid the possible memory leaks.
     * The reason is that others (timers, for example) may continue to 
     * send requests to this thread while this thread is shutdowning it self. 
     *
     * NOTE: The SHUTDOWN request is (purposely) handled here inside this function 
     *       because the "mutex-lock" is still being held.
     */

    if ( request->type == hoxREQUEST_TYPE_SHUTDOWN )
    {
        wxLogDebug(wxString::Format("%s: Shutdowning this thread...", FNAME));
        m_shutdownRequested = true;
        delete request; // *** Signal "no more request" ...
        return NULL;    // ... to the caller!
    }

    return request;
}

hoxResult 
hoxConnection::_SendRequest( const wxString& request,
                             wxString&       response )
{
    const char* FNAME = "hoxConnection::_SendRequest";
    wxLogDebug(wxString::Format("%s: ENTER.", FNAME));

    hoxResult result;
    wxUint32 nRead;
    wxChar* buf = NULL;
    wxUint32 nWrite;

    /* Currently, the caller needs to initiate the connection first. */

    if ( m_pSClient == NULL )
    {
        wxLogError(wxString::Format("%s: The connection is not yet established."));
        return hoxRESULT_ERR;
    }

    // We disable input events, so that the test doesn't trigger
    // wxSocketEvent again.
    m_pSClient->SetNotify(wxSOCKET_LOST_FLAG); // remove the wxSOCKET_INPUT_FLAG!!!

    // Tell the server which 'command' we are sending

    m_pSClient->WaitForWrite(3 /* seconds */);
    m_pSClient->Write( request.c_str(), (wxUint32) request.size() );
    nWrite = m_pSClient->LastCount();
    if ( nWrite != request.size() )
    {
        wxLogError(wxString::Format("%s: Failed to write request.", FNAME));
        result = hoxRESULT_ERR;
        goto exit_label;
    }

    // Wait until data available (will also return if the connection is lost)
    wxLogDebug(wxString::Format("%s: Waiting for response from the server (timeout = 3 sec)...", FNAME));
    m_pSClient->WaitForRead(3 /* seconds */);

    /***************************
     * Read the response
     ***************************/

    if ( ! m_pSClient->IsData() )
    {
        wxLogError(wxString::Format("%s: Timeout! Sending comand failed.", FNAME));
        result = hoxRESULT_ERR;
        goto exit_label;
    }

    buf = new wxChar[hoxNETWORK_MAX_MSG_SIZE];

    m_pSClient->ReadMsg( buf, hoxNETWORK_MAX_MSG_SIZE );
    nRead = m_pSClient->LastCount();

    if ( nRead == 0 )
    {
        wxLogError(wxString::Format("%s: Failed to read response.", FNAME));
        result = hoxRESULT_ERR;
        goto exit_label;
    }

    response.assign( buf, nRead );
    result = hoxRESULT_OK;

exit_label:
    delete[] buf;
    // Enable the input flag again.
    m_pSClient->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);

    return result;
}

void
hoxConnection::_Disconnect()
{
    const char* FNAME = "hoxConnection::_Disconnect";

    if ( m_pSClient != NULL )
    {
        wxLogDebug("%s: Close the client socket.", FNAME);
        m_pSClient->Destroy();
        m_pSClient = NULL;
    }
}

/************************* END OF FILE ***************************************/
