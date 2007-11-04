/////////////////////////////////////////////////////////////////////////////
// Name:            hoxServer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/23/2007
//
// Description:     The Connection Thread to help a "network" player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxServer.h"
#include "hoxEnums.h"
#include "MyApp.h"
#include "hoxTableMgr.h"
#include "hoxRemotePlayer.h"
#include "hoxPlayerMgr.h"
#include "hoxUtility.h"
#include "hoxNetworkAPI.h"

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
    const char* FNAME = "hoxServer::hoxServer";
    wxLogDebug("%s: ENTER.", FNAME);
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

    for ( SocketList::iterator it = m_activeSockets.begin(); 
                               it != m_activeSockets.end(); ++it )
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

    SocketList::iterator found = std::find( m_activeSockets.begin(), 
                                            m_activeSockets.end(), 
                                            sock );
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
    std::auto_ptr<hoxResponse> response( new hoxResponse(request->type) );

    wxLogDebug("%s: ENTER.", FNAME);

    /* 
     * SPECIAL CASE: 
     *     Handle the "special" request: Socket-Lost event,
     *     which is applicable to any request.
     */
    if (    request->type == hoxREQUEST_TYPE_PLAYER_DATA
         || request->type == hoxREQUEST_TYPE_DATA 
       )
    {
        result = _CheckAndHandleSocketLostEvent( request, response->content );
        if ( result == hoxRESULT_HANDLED )
        {
            result = hoxRESULT_OK;  // Consider "success".
            goto exit_label;
        }
    }

    /*
     * NORMAL CASE: 
     *    Handle "normal" request.
     */
    switch( request->type )
    {
        case hoxREQUEST_TYPE_ACCEPT:
            result = _HandleRequest_Accept( request );
            break;

        case hoxREQUEST_TYPE_TABLE_MOVE:
            /* fall through */
        case hoxREQUEST_TYPE_WALL_MSG:
            result = hoxNetworkAPI::SendRequest( request->socket, 
                                                 request->content,
                                                 response->content );
            break;

        case hoxREQUEST_TYPE_PLAYER_DATA: // incoming data from remote player.
            result = hoxNetworkAPI::HandlePlayerData( request->socket ); 
            break;

        case hoxREQUEST_TYPE_DATA:
            result = _SendRequest_Data( request, response->content );
            break;

        default:
            wxLogError("%s: Unsupported Request-Type [%s].", 
                FNAME, hoxUtility::RequestTypeToString(request->type));
            result = hoxRESULT_NOT_SUPPORTED;
            break;
    }

exit_label:
    /* Log error */
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Error occurred while handling request [%s].", 
            FNAME, hoxUtility::RequestTypeToString(request->type));
        response->content = "!Error_Result!";
    }

    /* NOTE: If there was error, just return it to the caller. */

    if ( request->sender != NULL )
    {
        wxCommandEvent event( hoxEVT_SERVER_RESPONSE );
        response->code = result;
        event.SetEventObject( response.release() );
        wxPostEvent( request->sender, event );
    }
}

hoxResult 
hoxServer::_CheckAndHandleSocketLostEvent( const hoxRequest* request, 
                                           wxString&         response )
{
    const char* FNAME = "hoxServer::_CheckAndHandleSocketLostEvent";
    hoxResult result = hoxRESULT_OK;

    wxLogDebug("%s: ENTER.", FNAME);

    wxSocketBase* sock = request->socket;
    //wxASSERT_MSG( sock == m_pendingSock, _T("Sockets should match!") );
    
    if ( request->socketEvent == wxSOCKET_LOST )
    {
        wxLogDebug("%s: Received socket-lost event. Deleting client socket.", FNAME);
        _DestroyActiveSocket( sock );
        result = hoxRESULT_HANDLED;
    }

    wxLogDebug("%s: Not a socket-lost event. Fine - Do nothing. END.", FNAME);
    return result;
}

hoxResult 
hoxServer::_HandleRequest_Accept( hoxRequest* request ) 
{
    const char* FNAME = "hoxServer::_HandleRequest_Accept";  // function's name

    wxLogDebug("%s: Saving an active (socket) connection.", FNAME);
    m_activeSockets.push_back( request->socket );

    return hoxRESULT_OK;
}

hoxResult 
hoxServer::_SendRequest_Data( const hoxRequest* request, 
                              wxString&         response )
{
    const char* FNAME = "hoxServer::_SendRequest_Data";
    hoxResult      result = hoxRESULT_OK;
    hoxCommand     command;
    wxSocketBase*  sock = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

    sock = request->socket;
    //wxASSERT_MSG( sock == m_pendingSock, _T("Sockets should match!") );
    
    /* We can ONLY able to handle socket-input event. */
    if ( request->socketEvent != wxSOCKET_INPUT )
    {
        wxLogError("%s: Unexpected socket-event [%s].", 
            FNAME, hoxNetworkAPI::SocketEventToString(request->socketEvent));
        return hoxRESULT_NOT_SUPPORTED;
    }

    /* Read the incoming command */
    result = hoxNetworkAPI::ReadCommand( sock, command );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to read incoming command.", FNAME);
        return hoxRESULT_ERR;
    }

    /* Process the command */
    switch ( command.type )
    {
        // **** TEMPORARY commented due to moving to hoxSocketServer
        //case hoxREQUEST_TYPE_CONNECT:
        //    _HandleCommand_Connect(sock); 
        //    break;

        case hoxREQUEST_TYPE_LIST:
            _HandleCommand_List(sock); 
            break;

        case hoxREQUEST_TYPE_JOIN:
            _HandleCommand_Join(sock, command); 
            break;

        case hoxREQUEST_TYPE_NEW:
            _HandleCommand_New(sock, command); 
            break;

        default:
            wxLogError("%s: Unsupported Request-Type [%s].", 
                FNAME, hoxUtility::RequestTypeToString(command.type));
            result = hoxRESULT_NOT_SUPPORTED;
            break;
    }

    return result;
}

void 
hoxServer::_HandleCommand_Connect( wxSocketBase* sock )
{
    const char* FNAME = "hoxServer::_HandleCommand_Connect";

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
        wxLogError("%s: Writing to socket failed. Error = [%s]", 
            FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()));
        return;
    }
}

void 
hoxServer::_HandleCommand_List( wxSocketBase *sock )
{
    const char* FNAME = "hoxServer::_HandleCommand_List";

    // Get the list of hosted tables.
    const hoxTableList& tables = hoxTableMgr::GetInstance()->GetTables();
    wxUint32 tableCount = (wxUint32) tables.size();

    // Write it back
    wxLogDebug("%s: ... We have [%d] tables.", FNAME, tableCount);

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
        wxLogError("%s: Writing to socket failed. Error = [%s]", 
            FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()));
        return;
    }
}

void 
hoxServer::_HandleCommand_Join( wxSocketBase*   sock,
                                hoxCommand&     command )
{
    const char* FNAME = "hoxServer::_HandleCommand_Join";

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

    wxLogDebug("%s: Create a remote player to the table...", FNAME);
    hoxRemotePlayer* black_player = 
        hoxPlayerMgr::GetInstance()->CreateRemotePlayer( sRequesterName,
                                                         nRequesterScore );

    // Setup the network-server which will handle all network communications.
    wxLogDebug("%s: Let this server manage the communication for this Network player [%s].", 
        FNAME, black_player->GetName());
    black_player->SetServer( this );

    // Set the callback socket to communicate back to the 'real' network player.
    black_player->SetCBSocket(sock);
    //if ( ! _DetachActiveSocket( sock ) ) // Let the player take care of this socket!
    //{
    //    wxFAIL_MSG("Active socket not found.");
    //}

    // Setup the event handler and let the player handles all socket events.
    wxLogDebug("%s: Let this Network player [%s] handle all socket events", FNAME, black_player->GetName());
    sock->SetEventHandler(*black_player, CLIENT_SOCKET_ID);
    sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    sock->Notify(true);

    result = black_player->JoinTable( table );
    wxASSERT( result == hoxRESULT_OK  );
    wxASSERT_MSG( black_player->HasRole( hoxRole(table->GetId(), 
                                                 hoxPIECE_COLOR_BLACK) ),
                  "Player must play BLACK");

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
            FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()));
        //return;
    }

    wxLogDebug("%s: END.", FNAME);
}

void 
hoxServer::_HandleCommand_New( wxSocketBase*  sock,
                              hoxCommand&    command )
{
    const char* FNAME = "hoxServer::_HandleCommand_New";

    wxLogDebug("%s: ENTER.", FNAME);

    wxLogWarning("%s: **** NOT YET IMPLEMENTED **** ", FNAME);
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

void
hoxServer::_Disconnect()
{    
    if ( m_pSServer != NULL )
    {
        m_pSServer->Destroy();
        m_pSServer = NULL;
    }
}

/************************* END OF FILE ***************************************/
