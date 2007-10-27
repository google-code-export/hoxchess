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
#include "hoxUtility.h"

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

    wxLogDebug("%s: ENTER.", FNAME);

    _DestroyAllActiveSockets();
    _Disconnect();
}

void
hoxServer::_DestroyAllActiveSockets()
{
    const char* FNAME = "hoxServer::_DestroyAllActiveSockets";

    wxLogDebug("%s: ENTER.", FNAME);

    for ( SocketList::iterator it = m_activeSockets.begin(); it != m_activeSockets.end(); ++it )
    {
        (*it)->Destroy();
    }
}

void
hoxServer::_DestroyActiveSocket( wxSocketBase *sock )
{
    const char* FNAME = "hoxServer::_DestroyActiveSocket";

    wxLogDebug("%s: ENTER.", FNAME);

    if ( _DetachActiveSocket( sock ) )
    {
        sock->Destroy();
    }
}

bool
hoxServer::_DetachActiveSocket( wxSocketBase *sock )
{
    const char* FNAME = "hoxServer::_DetachActiveSocket";

    wxLogDebug("%s: ENTER.", FNAME);

    SocketList::iterator found = std::find( m_activeSockets.begin(), m_activeSockets.end(), sock );

    if ( found != m_activeSockets.end() )
    {
        m_activeSockets.erase( found );
        return true;
    }

    wxLogDebug("%s: Could NOT find the specified socket to detach.", FNAME);
    return false;
}

void*
hoxServer::Entry()
{
    const char* FNAME = "hoxServer::Entry";
    hoxRequest* request = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

    while ( !m_shutdownRequested && m_semRequests.Wait() == wxSEMA_NO_ERROR )
    {
        request = _GetRequest();
        if ( request == NULL )
        {
            wxASSERT_MSG( m_shutdownRequested, "This thread must be shutdowning." );
            break;  // Exit the thread.
        }
        wxLogDebug("%s: Processing request [%s]...", 
            FNAME, hoxUtility::RequestTypeToString(request->type));

         _HandleRequest( request );
        delete request;
    }

    return NULL;
}

void 
hoxServer::AddRequest( hoxRequest* request )
{
    const char* FNAME = "hoxServer::AddRequest";
    wxLogDebug("%s: ENTER. Trying to obtain the lock...", FNAME);
    wxMutexLocker lock( m_mutexRequests );

    if ( m_shutdownRequested )
    {
        wxLogWarning("%s: Deny request [%s]. The thread is shutdowning.", 
            FNAME, hoxUtility::RequestTypeToString(request->type));
        delete request;
        return;
    }

    m_requests.push_back( request );
    m_semRequests.Post();
    wxLogDebug("%s: END.", FNAME);
}

void 
hoxServer::_HandleRequest( hoxRequest* request )
{
    const char* FNAME = "hoxServer::_HandleRequest";
    hoxResult    result = hoxRESULT_ERR;
    hoxResponse* response = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

    response = new hoxResponse( request->type );

    switch( request->type )
    {
        case hoxREQUEST_TYPE_ACCEPT:
            result = _HandleRequest_Accept( request );
            break;

        //case hoxREQUEST_TYPE_LIST:     /* fall through */
        //case hoxREQUEST_TYPE_JOIN:     /* fall through */
        case hoxREQUEST_TYPE_DATA:
            result = _SendRequest_Data( request, response->content );
            break;

        //case hoxREQUEST_TYPE_CONNECT:
        //    result = _SendRequest_Connect( request->content, response->content );
        //    break;

        //case hoxREQUEST_TYPE_POLL:     /* fall through */
        //case hoxREQUEST_TYPE_MOVE:     /* fall through */
        //case hoxREQUEST_TYPE_NEW:      /* fall through */
        //case hoxREQUEST_TYPE_LEAVE:
        //    result = _SendRequest( request->content, response->content );
        //    break;

        default:
            wxLogError("%s: Unsupported request Type [%s].", 
                FNAME, hoxUtility::RequestTypeToString(request->type));
            result = hoxRESULT_NOT_SUPPORTED;
            response->content = "";
            break;
    }

    /* Log error */
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Error occurred while handling request [%s].", 
            FNAME, hoxUtility::RequestTypeToString(request->type));
        response->content = "!Error_Result!";
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
    const char* FNAME = "hoxServer::_HandleRequest_Accept";  // function's name
    wxLogDebug("%s: ENTER.", FNAME);

    // *** Save the connection so that later we can cleanup before closing.
    wxLogDebug("%s: Save an active (socket) connection.", FNAME);
    m_activeSockets.push_back( request->socket );

    return hoxRESULT_OK;
}

hoxResult 
hoxServer::_SendRequest_Data( const hoxRequest* request, 
                              wxString&         response )
{
    const char* FNAME = "hoxServer::_SendRequest_Data";  // function's name
    wxLogDebug("%s: ENTER.", FNAME);

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

            wxString     commandStr;
            hoxCommand   command;
            hoxResult    result = hoxRESULT_ERR;  // Default = "error"

            wxLogDebug("%s: Reading incoming command from the network...", FNAME);
            result = hoxServer::read_line( sock, commandStr );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to read incoming command.", FNAME);
                return hoxRESULT_ERR;
            }
            wxLogDebug("%s: Received command [%s].", FNAME, commandStr);

            result = hoxServer::parse_command( commandStr, command );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to parse command-string [%s].", FNAME, commandStr);
                return hoxRESULT_ERR;
            }

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

                case hoxREQUEST_TYPE_NEW:
                    HandleCommand_New(sock, command); 
                    break;

                //case hoxREQUEST_TYPE_LEAVE:
                //    HandleCommand_Leave(sock, command); 
                //    break;

                default:
                    break;
            }

            // Enable input events again.
            //sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
            break;
        }
        case wxSOCKET_LOST:
        {
            wxLogDebug("%s: Deleting pending socket.", FNAME);
            _DestroyActiveSocket( sock );
            break;
        }
        default: 
            wxLogError("%s: Unexpected socket-event [%s].", 
                FNAME, hoxUtility::SocketEventToString(request->socketEvent));
            break;
    }

    return hoxRESULT_OK;
}

//hoxResult        
//hoxServer::_SendRequest_Connect( const wxString& request, 
//                                     wxString&       response )
//{
//    const char* FNAME = "hoxServer::_SendRequest_Connect";
//    wxLogError("%s: Are we using this function.", FNAME );
//    return hoxRESULT_OK;
//}

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

    wxLogDebug("%s: ENTER.", FNAME);

    wxString tableId;
    wxString sRequesterName;
    int      nRequesterScore;
    wxString existingPlayerId;

    tableId = command.parameters["tid"];
    sRequesterName = command.parameters["pid"];
    nRequesterScore = 1999;   // FIXME hard-coded player's score

    wxLogDebug("%s: The requester: name=[%s], score=[%d]", 
        FNAME, sRequesterName, nRequesterScore);

    wxUint32 nWrite;
    wxString response;

    // Find the table hosted on this system using the specified table-Id.
    hoxTable* table = hoxTableMgr::GetInstance()->FindTable( tableId );

    if ( table == NULL )
    {
        wxLogError("%s: Table [%s] not found.", FNAME, tableId);
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
        wxLogError("%s: No one is at the table [%s] not found.", FNAME, tableId);
        response << "2\r\n"  // code
                 << "Not one is at the table " << tableId << ".\r\n";
        goto exit_label;
    }

    /***********************/
    /* Setup players       */
    /***********************/

    hoxResult result;

    wxLogDebug("%s: Create a network player to the table...", FNAME);
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
    wxLogDebug("%s: Let this Network player [%s] handle all socket events", FNAME, black_player->GetName());
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
        wxLogError("%s: Failed to send back response over the network.", 
            FNAME, hoxUtility::SocketErrorToString(sock->LastError()));
        //return;
    }

    wxLogDebug("%s: END.", FNAME);
}

void 
hoxServer::HandleCommand_New( wxSocketBase*  sock,
                              hoxCommand&    command )
{
    const char* FNAME = "hoxServer::HandleCommand_New";

    wxLogDebug("%s: ENTER.", FNAME);

    wxLogDebug("%s: **** NOT YET IMPLEMENTED **** ", FNAME);
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

//hoxResult 
//hoxServer::_SendRequest( const wxString& request,
//                             wxString&       response )
//{
//    return hoxRESULT_OK;
//}

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
hoxServer::read_line( wxSocketBase* sock, 
                      wxString&     result )
{
    const char* FNAME = "hoxServer::read_line";
    wxString commandStr;

    for (;;)
    {
        wxChar c;

        sock->Read( &c, 1 );
        if ( sock->LastCount() == 1 )
        {
            if ( c == '\n' )
            {
                if ( !commandStr.empty() && commandStr[ commandStr.size()-1 ] == '\r' )
                {
                    result = commandStr.substr(0, commandStr.size()-1);
                    return hoxRESULT_OK;
                }
            }
            else
            {
                commandStr << c;

                // Impose some limit.
                if ( commandStr.size() >= hoxNETWORK_MAX_MSG_SIZE )
                {
                    wxLogError("%s: Maximum message's size [%d] reached. Likely to be an error.", 
                        FNAME, hoxNETWORK_MAX_MSG_SIZE);
                    wxLogError("%s: Partial read message (64 bytes) = [%s ...].", 
                        FNAME, commandStr.substr(0, 64));
                    break;
                }
            }
        }
        else if ( sock->Error() )
        {
            wxLogWarning("%s: Fail to read 1 byte from the network. Error = [%s].", 
                FNAME, hoxUtility::SocketErrorToString(sock->LastError()));
            wxLogWarning("%s: Result message accumulated so far = [%s].", FNAME, commandStr);
            break;
        }
    }

    return hoxRESULT_ERR;
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
            else if ( paramValue == "NEW" )
            {
                command.type = hoxREQUEST_TYPE_NEW;
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
                wxLogError("%s: Unsupported command-type = [%s].", FNAME, paramValue);
                return hoxRESULT_NOT_SUPPORTED;
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
