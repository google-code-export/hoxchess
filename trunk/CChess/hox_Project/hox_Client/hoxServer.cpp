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
#include "hoxRemoteConnection.h"
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
        it->socket->Destroy();
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

    for ( SocketList::iterator it = m_activeSockets.begin(); 
                               it != m_activeSockets.end(); ++it )
    {
        if ( it->socket == sock )
        {
            m_activeSockets.erase( it );
            return true;
        }
    }
    wxLogDebug("%s: Could NOT find the specified socket to detach.", FNAME);
    return false;
}

bool
hoxServer::_FindSocketInfo( const wxString& playerId,
                            SocketInfo&     socketInfo )
{
    for ( SocketList::iterator it = m_activeSockets.begin(); 
                               it != m_activeSockets.end(); ++it )
    {
        if ( it->playerId == playerId )
        {
            socketInfo = (*it);
            return true;
        }
    }

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
       )
    {
        result = _CheckAndHandleSocketLostEvent( request, response->content );
        if ( result == hoxRESULT_HANDLED )
        {
            response->flags |= hoxRESPONSE_FLAG_CONNECTION_LOST;
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

        case hoxREQUEST_TYPE_MOVE: /* fall through */
        case hoxREQUEST_TYPE_WALL_MSG:
            result = hoxNetworkAPI::SendRequest( request->socket, 
                                                 request->content,
                                                 response->content );
            break;

        case hoxREQUEST_TYPE_PLAYER_DATA: // incoming data from remote player.
        {
            wxSocketBase* sock = request->socket;
            // We disable input events until we are done processing the current command.
            wxString commandStr;
            hoxNetworkAPI::SocketInputLock socketLock( sock );
            result = hoxNetworkAPI::ReadLine( sock, commandStr );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to read incoming command.", FNAME);
                goto exit_label;
            }
            response->content = commandStr;
            break;
        }

        case hoxREQUEST_TYPE_OUT_DATA:
            result = hoxNetworkAPI::SendOutData( request->socket, 
                                                 request->content );
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
        wxCommandEvent event( hoxEVT_SERVER_RESPONSE, request->type );
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
    const wxString playerId = request->content;
    wxSocketBase* socket = request->socket;

    wxLogDebug("%s: Saving an active (socket) connection.", FNAME);
    wxCHECK_MSG(socket, hoxRESULT_ERR, "The socket cannot be NULL.");

    /* TODO: Check for existing player before create a new player/info. */

    /* Save the player + socket in our list */
    SocketInfo socketInfo(playerId, socket);
    m_activeSockets.push_back( socketInfo );

    return hoxRESULT_OK;
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
        wxLogDebug("%s: Shutting down this thread...", FNAME);
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
