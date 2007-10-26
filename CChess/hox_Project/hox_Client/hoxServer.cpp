/////////////////////////////////////////////////////////////////////////////
// Name:            hoxServer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/23/2007
//
// Description:     The Connection Thread to help a "network" player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxServer.h"
#include "hoxWWWPlayer.h"
#include "hoxEnums.h"
#include "MyApp.h"
#include "hoxTableMgr.h"
#include "hoxNetworkPlayer.h"
#include "hoxPlayerMgr.h"

#include <wx/sstream.h>
#include <wx/protocol/http.h>
#include <wx/tokenzr.h>
#include <algorithm>

//-----------------------------------------------------------------------------
// hoxServer
//-----------------------------------------------------------------------------


hoxServer::hoxServer( int nPort )
        : wxThreadHelper()
        , m_nPort( nPort )
        , m_shutdownRequested( false )
        , m_pSServer( NULL )
{
}

hoxServer::~hoxServer()
{
    const char* FNAME = "hoxServer::~hoxServer";

    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId(), wxThread::IsMain(), FNAME);

    _DestroyAllActiveSockets();
    _Disconnect();
}

void
hoxServer::_DestroyAllActiveSockets()
{
    const char* FNAME = "hoxServer::_DestroyAllActiveSockets";

    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId(), wxThread::IsMain(), FNAME);

    for ( SocketList::iterator it = m_activeSockets.begin(); it != m_activeSockets.end(); ++it )
    {
        (*it)->Destroy();
    }
}

void
hoxServer::_DestroyActiveSocket( wxSocketBase *sock )
{
    const char* FNAME = "hoxServer::_DestroyActiveSocket";

    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId(), wxThread::IsMain(), FNAME);

    if ( _DetachActiveSocket( sock ) )
    {
        sock->Destroy();
    }
}

bool
hoxServer::_DetachActiveSocket( wxSocketBase *sock )
{
    const char* FNAME = "hoxServer::_DetachActiveSocket";

    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId(), wxThread::IsMain(), FNAME);

    SocketList::iterator found = std::find( m_activeSockets.begin(), m_activeSockets.end(), sock );

    if ( found != m_activeSockets.end() )
    {
        m_activeSockets.erase( found );
        return true;
    }

    return false;
}

void*
hoxServer::Entry()
{
    const char* FNAME = "hoxServer::Entry";
    hoxRequest* request = NULL;

    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId(), wxThread::IsMain(), FNAME);

    ///////////////////////////////////
#if 0
    // Create the address - defaults to localhost:0 initially
    wxIPV4address addr;
    addr.Service( m_nPort );

    // Create the socket
    m_pSServer = new wxSocketServer(addr);

    // We use Ok() here to see if the server is really listening
    if ( ! m_pSServer->Ok() )
    {
        wxLogError(wxString::Format("%s: Could not listen at the specified port [%d]!", FNAME, m_nPort));
        delete m_pSServer;
        m_pSServer = NULL;
    }

    wxLogDebug(wxString::Format("%s: Server listening at port [%d].", FNAME, m_nPort));

    // Setup the event handler and subscribe to 'connection' events
    m_pSServer->SetEventHandler( wxGetApp(), SERVER_ID );
    m_pSServer->SetNotify( wxSOCKET_CONNECTION_FLAG );
    m_pSServer->Notify( true );
#endif
    ///////////////////////////////////

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
hoxServer::AddRequest( hoxRequest* request )
{
    const char* FNAME = "hoxServer::AddRequest";
    wxMutexLocker lock( m_mutexRequests );

    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId(), wxThread::IsMain(), FNAME);

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
hoxServer::_HandleRequest( hoxRequest* request )
{
    const char* FNAME = "hoxServer::_HandleRequest";
    hoxResult  result = hoxRESULT_ERR;
    hoxResponse* response = new hoxResponse( request->type );

    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId(), wxThread::IsMain(), FNAME);

    switch( request->type )
    {
        case hoxREQUEST_TYPE_ACCEPT:
            result = _HandleRequest_Accept( request );
            break;

        case hoxREQUEST_TYPE_LIST:     /* fall through */
        case hoxREQUEST_TYPE_JOIN:     /* fall through */
        case hoxREQUEST_TYPE_DATA:
            result = _SendRequest_Data( request, response->content );
            break;

        case hoxREQUEST_TYPE_CONNECT:
            result = _SendRequest_Connect( request->content, response->content );
            break;

        case hoxREQUEST_TYPE_POLL:     /* fall through */
        case hoxREQUEST_TYPE_MOVE:     /* fall through */
        //case hoxREQUEST_TYPE_LIST:     /* fall through */
        case hoxREQUEST_TYPE_NEW:      /* fall through */
        //case hoxREQUEST_TYPE_JOIN:     /* fall through */
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
        wxCommandEvent event( hoxEVT_SERVER_RESPONSE );
        event.SetInt( result );
        event.SetEventObject( response );
        wxPostEvent( request->sender, event );
    }
}

hoxResult 
hoxServer::_HandleRequest_Accept( hoxRequest* request ) 
{
    const char* FNAME = "hoxServer::_SendRequest_Accept";  // function's name
    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId, wxThread::IsMain(), FNAME);

    // *** Save the connection so that later we can cleanup before closing.
    wxLogDebug(wxString::Format("%s: Save active connection.", FNAME));
    m_activeSockets.push_back( request->socket );

#if 0
    // Accept new connection if there is one in the pending
    // connections queue, else exit. We use Accept(false) for
    // non-blocking accept (although if we got here, there
    // should ALWAYS be a pending connection).

    wxSocketBase* sock = m_pSServer->Accept(false);

    if (sock)
    {
        wxLogDebug(wxString::Format("%s: New client connection accepted.", FNAME));
    }
    else
    {
        wxLogError(wxString::Format("%s: Couldn't accept a new connection", FNAME));
        return hoxRESULT_OK;
    }

    // *** Save the connection so that later we can cleanup before closing.
    wxLogDebug(wxString::Format("%s: Save active connection.", FNAME));
    m_activeSockets.push_back( sock );

    /************************************************************
     * Create a handler to handle this socket.
     * Subscribe to 'input' and 'lost-connection' events
     *************************************************************/

    sock->SetEventHandler( wxGetApp(), SERVER_SOCKET_ID );
    sock->SetNotify( wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG );
    sock->Notify( true );
#endif
    return hoxRESULT_OK;
}

hoxResult 
hoxServer::_SendRequest_Data( const hoxRequest* request, 
                              wxString&         response )
{
    const char* FNAME = "hoxServer::_SendRequest_Data";  // function's name
    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId, wxThread::IsMain(), FNAME);
    wxString s;
    s.Printf("%s: ", FNAME);

    wxSocketBase *sock = request->socket;
    //wxASSERT_MSG( sock == m_pendingSock, _T("Sockets should match!") );
    
    // Now we process the event
    switch( request->socketEvent )
    {
        case wxSOCKET_INPUT:
        {
            // We disable input events, so that the test doesn't trigger
            // wxSocketEvent again.
            //sock->SetNotify(wxSOCKET_LOST_FLAG); // remove the wxSOCKET_INPUT_FLAG!!!

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
                    case hoxREQUEST_TYPE_CONNECT:
                        HandleCommand_Connect(sock); 
                        break;

                    case hoxREQUEST_TYPE_LIST:
                        HandleCommand_List(sock); 
                        break;

                    case hoxREQUEST_TYPE_JOIN:
                        HandleCommand_Join(sock, command); 
                        break;

                    //case hoxREQUEST_TYPE_LEAVE:
                    //    HandleCommand_Leave(sock, command); 
                    //    break;

                    default:
                        break;
                }
            }
#if 0
            switch (command)
            {
                case hoxNETWORK_CMD_QUERY_TABLE:
                    HandleQueryTable(sock); 
                    break;

                case hoxNETWORK_CMD_JOIN_TABLE:
                    HandleJoinTable(sock);
                    break;

                default:
                    wxLogDebug(_("Unknown command [%d] received from client"), command);
            }
#endif
            // Enable input events again.
            //sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
            break;
        }
        case wxSOCKET_LOST:
        {
            //m_numClients--;

            // Destroy() should be used instead of delete wherever possible,
            // due to the fact that wxSocket uses 'delayed events' (see the
            // documentation for wxPostEvent) and we don't want an event to
            // arrive to the event handler (the frame, here) after the socket
            // has been deleted. Also, we might be doing some other thing with
            // the socket at the same time; for example, we might be in the
            // middle of a test or something. Destroy() takes care of all
            // this for us.

            wxLogDebug(wxString::Format("%s: Deleting pending socket.", FNAME));
            _DestroyActiveSocket( sock );
            break;
        }
        default: 
            break;
    }

    return hoxRESULT_OK;
}

hoxResult        
hoxServer::_SendRequest_Connect( const wxString& request, 
                                     wxString&       response )
{
    return hoxRESULT_OK;
}

void 
hoxServer::HandleCommand_Connect( wxSocketBase* sock )
{
    const char* FNAME = "hoxServer::HandleCommand_Connect";

    /* NOTE: Accummulate the entire response and send it at once
     *       with the first number indicating the size of the message.
     */

    /* Simply send back an OK response. */

    wxUint32 nWrite;
    wxString response;

    response << "0\r\n"  // code
             << "OK - Accepted\r\n";  // message

    nWrite = (wxUint32) response.size();
    sock->WriteMsg( response, nWrite );
    if ( sock->LastCount() != nWrite )
    {
        wxLogWarning(wxString::Format("%s: Writing to  socket failed.", FNAME));
        return;
    }
}

void 
hoxServer::HandleCommand_List( wxSocketBase *sock )
{
    const char* FNAME = "hoxServer::HandleCommand_List";

    // Get the list of hosted tables.
    const hoxTableList& tables = hoxTableMgr::GetInstance()->GetTables();
    wxUint32 tableCount = (wxUint32) tables.size();

    // Write it back
    wxLogDebug(wxString::Format("%s: ... We have [%d] tables.", FNAME, tableCount));

    wxUint32 nWrite;
    wxString response;

    response << "0\r\n"  // code
             //<< "We have " << tableCount << " tables\r\n";  // message
             ;

    // Return the info of tables.
    for ( hoxTableList::const_iterator it = tables.begin(); 
                                       it != tables.end(); ++it )
    {
        hoxPlayer* redPlayer   = (*it)->GetRedPlayer();
        hoxPlayer* blackPlayer = (*it)->GetBlackPlayer();

        response << (*it)->GetId() << " "
                 << "1 "   // TODO: Hard-coded for table-status
                 << (redPlayer != NULL ? redPlayer->GetName() : "0") << " "
                 << (blackPlayer != NULL ? blackPlayer->GetName() : "0") << " "
                 << "\r\n";
    }

    nWrite = (wxUint32) response.size();
    sock->WriteMsg( response, nWrite );
    if ( sock->LastCount() != nWrite )
    {
        wxLogError(wxString::Format("%s: Writing to  socket failed.", FNAME));
        return;
    }
}

void 
hoxServer::HandleCommand_Join( wxSocketBase*      sock,
                               hoxCommand&  command )
{
    const char* FNAME = "hoxServer::HandleCommand_Join";

    wxLogDebug(wxString::Format("%s: ENTER.", FNAME));

    wxString tableId;
    wxString sRequesterName;
    int      nRequesterScore;
    wxString existingPlayerId;

    tableId = command.parameters["tid"];
    sRequesterName = command.parameters["pid"];
    nRequesterScore = 1999;   // FIXME hard-coded player's score

    wxLogDebug(wxString::Format("%s: The requester: name=[%s], score=[%d]", 
                    FNAME, sRequesterName, nRequesterScore));

    wxUint32 nWrite;
    wxString response;

    // Find the table hosted on this system using the specified table-Id.
    hoxTable* table = hoxTableMgr::GetInstance()->FindTable( tableId );

    if ( table == NULL )
    {
        wxLogError(wxString::Format("%s: Table [%s] not found.", FNAME, tableId));
        response << "1\r\n"  // code
                 << "Table " << tableId << " not found.\r\n";
        goto exit_label;
    }

    //bool     bTableOpen = true;
    //if (!pTable || pTable->GetStatus() != hoxTABLE_STATUS_OPEN) {
    //    bTableOpen = false;
    //}

    /* Get the ID of the existing player */

    if ( table->GetRedPlayer() != NULL )
    {
        existingPlayerId = table->GetRedPlayer()->GetName();
    }
    else if ( table->GetBlackPlayer() != NULL )
    {
        existingPlayerId = table->GetBlackPlayer()->GetName();
    }
    else
    {
        wxLogError(wxString::Format("%s: No one is at the table [%s] not found.", FNAME, tableId));
        response << "2\r\n"  // code
                 << "Not one is at the table " << tableId << ".\r\n";
        goto exit_label;
    }

    /***********************/
    /* Setup players       */
    /***********************/

    hoxResult result;

    wxLogDebug(wxString::Format("%s: Create a network player to the table...", FNAME));
    hoxNetworkPlayer* black_player = 
        hoxPlayerMgr::GetInstance()->CreateNetworkPlayer( sRequesterName,
                                                          nRequesterScore );
    black_player->AddRole( hoxRole( tableId, hoxPIECE_COLOR_BLACK ) );

    // Set the callback socket to communicate back to the 'real' network player.
    black_player->SetCBSocket(sock);
    if ( ! _DetachActiveSocket( sock ) ) // Let the player take care of this socket!
    {
        wxFAIL_MSG("Active socket not found.");
    }

    // Setup the event handler and let the player handles all socket events.
    wxLogDebug(wxString::Format(_("%s: Let this Network player handle all socket events"), FNAME));
    sock->SetEventHandler(*black_player, CLIENT_SOCKET_ID);
    sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    sock->Notify(true);

    result = black_player->JoinTable( table );
    wxASSERT( result == hoxRESULT_OK  );

	// Finally, return 'success'.
	response << "0\r\n"       // error-code = SUCCESS
	         << "INFO: (JOIN) Join Table [" << tableId << "] OK\r\n"
	         << tableId << " " << "1 " << existingPlayerId << " " << sRequesterName << "\r\n";

exit_label:
    // Send back response.
    nWrite = (wxUint32) response.size();
    sock->WriteMsg( response, nWrite );
    if ( sock->LastCount() != nWrite )
    {
        wxLogError(wxString::Format("%s: Writing to  socket failed.", FNAME));
        return;
    }

    wxLogDebug(_("%s: END"), FNAME);
}

hoxRequest*
hoxServer::_GetRequest()
{
    const char* FNAME = "hoxServer::_GetRequest";
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
hoxServer::_SendRequest( const wxString& request,
                             wxString&       response )
{
#if 0
    const char* FNAME = "hoxServer::_SendRequest";

    /* Currently, the caller needs to initiate the connection first. */

    if ( m_pSClient == NULL )
    {
        wxLogError(wxString::Format("%s: The connection is not yet established."));
        return hoxRESULT_ERR;
    }

    // Tell the server which 'command' we are sending
    wxUint32 nWrite;

    m_pSClient->WaitForWrite(3 /* seconds */);
    m_pSClient->Write( request.c_str(), (wxUint32) request.size() );
    nWrite = m_pSClient->LastCount();
    if ( nWrite != request.size() )
    {
        wxLogWarning(wxString::Format("%s: Failed to write request.", FNAME));
        return hoxRESULT_ERR;
    }

    // Wait until data available (will also return if the connection is lost)
    wxLogDebug(wxString::Format("%s: Waiting for response from the server (timeout = 3 sec)...", FNAME));
    m_pSClient->WaitForRead(3 /* seconds */);

    /***************************
     * Read the response
     ***************************/

    if ( ! m_pSClient->IsData() )
    {
        wxLogWarning(wxString::Format("%s: Timeout! Sending comand failed.", FNAME));
        return hoxRESULT_ERR;
    }

    wxUint32 nRead;
    wxChar* buf = new wxChar[hoxNETWORK_MAX_MSG_SIZE];

    m_pSClient->ReadMsg( buf, hoxNETWORK_MAX_MSG_SIZE );
    nRead = m_pSClient->LastCount();

    if ( nRead == 0 )
    {
        wxLogWarning(wxString::Format("%s: Failed to read response.", FNAME));
        return hoxRESULT_ERR;
    }

    response.assign( buf, nRead );
#endif
    return hoxRESULT_OK;
}

void
hoxServer::_Disconnect()
{    
    if ( m_pSServer != NULL )
    {
        m_pSServer->Destroy();
        m_pSServer = NULL;
    }
}

/* static */
hoxResult
hoxServer::read_line(wxSocketBase *sock, wxString& result)
{
    const char* FNAME = "hoxServer::read_line";
    // ***********************
    {
        //sock->SetFlags( wxSOCKET_WAITALL | wxSOCKET_BLOCK ); // Block socket + GUI
        wxString commandStr;
        //hoxResult result = hoxServer::read_line( sock, commandStr );

        //bool seenCR = false;
        //bool seenLF = false;
        for (;;)
        {
            wxChar c;
            sock->Read( &c, 1 );
            if ( sock->LastCount() == 1 )
            {
                //if ( c == '\r' )
                //{
                //    seenCR = true;
                //}
                /*else*/ if ( c == '\n' )
                {
                    if ( !commandStr.empty() && commandStr[ commandStr.size()-1 ] == '\r' )
                    {
                        //seenLF = true;
                        result = commandStr.substr(0, commandStr.size()-1);
                        return hoxRESULT_OK;
                    }
                }
                else
                {
                    commandStr << c;
                }
            }
        }
        
        wxLogDebug(wxString::Format("%s: Received command-string [%s] received from client", 
                        FNAME, commandStr));

    }
    return hoxRESULT_OK;
    // *************************
#if 0
    static const int LINE_BUF = 4095;

    result.clear();

    wxCharBuffer buf(LINE_BUF);
    char *pBuf = buf.data();
    //while ( sock->WaitForRead() )
    while ( 1 /* sock->IsData() */)
    {
        // peek at the socket to see if there is a CRLF
        sock->Peek(pBuf, LINE_BUF);

        size_t nRead = sock->LastCount();
        if ( !nRead && sock->Error() )
            return hoxRESULT_ERR;

        // look for "\r\n" paying attention to a special case: "\r\n" could
        // have been split by buffer boundary, so check also for \r at the end
        // of the last chunk and \n at the beginning of this one
        pBuf[nRead] = '\0';
        const char *eol = strchr(pBuf, '\n');

        // if we found '\n', is there a '\r' as well?
        if ( eol )
        {
            if ( eol == pBuf )
            {
                // check for case of "\r\n" being split
                if ( result.empty() || result.Last() != _T('\r') )
                {
                    // ignore the stray '\n'
                    eol = NULL;
                }
                //else: ok, got real EOL

                // read just this '\n' and restart
                nRead = 1;
            }
            else // '\n' in the middle of the buffer
            {
                // in any case, read everything up to and including '\n'
                nRead = eol - pBuf + 1;

                if ( eol[-1] != '\r' )
                {
                    // as above, simply ignore stray '\n'
                    eol = NULL;
                }
            }
        }

        sock->Read(pBuf, (wxUint32) nRead);
        if ( sock->LastCount() != nRead )
            return hoxRESULT_OK;

        pBuf[nRead] = '\0';
        result += wxString::FromAscii(pBuf);

        if ( eol )
        {
            // remove trailing "\r\n"
            result.RemoveLast(2);

            return hoxRESULT_OK;
        }
    }

    return hoxRESULT_ERR;
#endif
}

/* static */
hoxResult
hoxServer::parse_command( const wxString& commandStr, 
                          hoxCommand&     command )
{
    const char* FNAME = "hoxServer::parse_command";

    wxStringTokenizer tkz( commandStr, "&" );

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();

        size_t foundIndex = token.find( '=' );
        
        if ( foundIndex == wxNOT_FOUND )
            continue;  // ignore this 'error'.

        wxString paramName;
        wxString paramValue;

        paramName = token.substr( 0, foundIndex );
        paramValue = token.substr( foundIndex+1 );

        // Special case for "op" param-name.
        if ( paramName == "op" )
        {
            if ( paramValue == "HELLO" )
            {
                command.type = hoxREQUEST_TYPE_CONNECT;
            }
            else if ( paramValue == "LIST" )
            {
                command.type = hoxREQUEST_TYPE_LIST;
            }
            else if ( paramValue == "JOIN" )
            {
                command.type = hoxREQUEST_TYPE_JOIN;
            }
            else if ( paramValue == "LEAVE" )
            {
                command.type = hoxREQUEST_TYPE_LEAVE;
            }
            else if ( paramValue == "MOVE" )
            {
                command.type = hoxREQUEST_TYPE_MOVE;
            }
            else if ( paramValue == "TABLE_MOVE" )
            {
                command.type = hoxREQUEST_TYPE_TABLE_MOVE;
            }
            else
            {
                wxLogError(wxString::Format("%s: Unsupported command-type = [%s].", FNAME, paramValue));
            }
        }
        else
        {
            command.parameters[paramName] = paramValue;
        }
    }

    return hoxRESULT_OK;
}

/************************* END OF FILE ***************************************/
