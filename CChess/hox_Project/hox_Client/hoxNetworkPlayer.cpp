/////////////////////////////////////////////////////////////////////////////
// Name:            hoxNetworkPlayer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/09/2007
//
// Description:     The NETWORK Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxNetworkPlayer.h"
#include "hoxEnums.h"
#include "hoxTable.h"
#include "hoxTableMgr.h"
#include "hoxServer.h"
#include "hoxWWWThread.h"
#include "hoxUtility.h"
#include <algorithm>

// user code intercepting the event
IMPLEMENT_DYNAMIC_CLASS( hoxNetworkPlayer, hoxPlayer )

BEGIN_EVENT_TABLE(hoxNetworkPlayer, hoxPlayer)
    // Socket-event handler
    EVT_SOCKET(CLIENT_SOCKET_ID,  hoxNetworkPlayer::OnClientSocketEvent)
END_EVENT_TABLE()


//-----------------------------------------------------------------------------
// hoxNetworkPlayer
//-----------------------------------------------------------------------------

hoxNetworkPlayer::hoxNetworkPlayer()
            : hoxPlayer( _("Unknown"), 
                         hoxPLAYER_TYPE_NETWORK, 
                         1500 )
            , m_pCBSock( NULL )
{ 
    wxFAIL_MSG("This is not meant to be called.");
}

hoxNetworkPlayer::hoxNetworkPlayer( const wxString& name,
                                    hoxPlayerType   type,
                                    int             score /* = 1500 */)
            : hoxPlayer( name, type, score )
            , m_pCBSock( NULL )
{ 
}

hoxNetworkPlayer::~hoxNetworkPlayer() 
{
    this->DisconnectFromNetwork();
}

//
// Socket-event's handler.
//
void 
hoxNetworkPlayer::OnClientSocketEvent(wxSocketEvent& event)
{
    const char* FNAME = "hoxNetworkPlayer::OnClientSocketEvent";  // function's name

    wxLogDebug("%s: ENTER. socket-event = [%s].", FNAME, 
        hoxUtility::SocketEventToString(event.GetSocketEvent()) );

    wxSocketBase* sock = event.GetSocket();
    wxASSERT_MSG( sock != NULL, "Socket cannot be NULL." );
  
    // We disable input events until we are done processing the current command.
    sock->SetNotify(wxSOCKET_LOST_FLAG); // remove the wxSOCKET_INPUT_FLAG!!!

    // Now we process the event
    switch( event.GetSocketEvent() )
    {
        case wxSOCKET_INPUT:
        {
            wxString     commandStr;
            hoxCommand   command;
            hoxResult    result = hoxRESULT_ERR;  // Default = "error"

            wxLogDebug("%s: Reading incoming command from the network...", FNAME);
            result = hoxServer::read_line( sock, commandStr );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to parse command [%s].", FNAME, commandStr);
                goto exit_label;
            }
            wxLogDebug("%s: Received command [%s].", FNAME, commandStr);

            result = hoxServer::parse_command( commandStr, command );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to parse command-string [%s].", FNAME, commandStr);
                goto exit_label;
            }

            switch ( command.type )
            {
                case hoxREQUEST_TYPE_LEAVE:
                    HandleCommand_Leave(sock, command); 
                    break;

                case hoxREQUEST_TYPE_MOVE:
                    HandleCommand_Move(sock, command); 
                    break;

                default:
                    wxLogError("%s: Unknown command [%s] received from the network.", 
                        FNAME, hoxUtility::RequestTypeToString(command.type));
                    break;
            }
            break;
        }

        case wxSOCKET_LOST:
        {
            wxLogDebug("%s: Invoke socket-lost event handler...", FNAME);
            HandleSocketLostEvent( sock );
            break;
        }
  
        default: 
            wxLogError("%s: Unexpected socket-event event handler...", FNAME);
                FNAME, hoxUtility::SocketEventToString(event.GetSocketEvent());
            break;
    } // switch

exit_label:
    // Enable the input flag again.
    sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);

    wxLogDebug("%s: END.", FNAME);
}

void 
hoxNetworkPlayer::HandleCommand_Leave( wxSocketBase*  sock,
                                       hoxCommand&    command )
{
    const char* FNAME = "hoxNetworkPlayer::HandleCommand_Leave";

    wxLogDebug(wxString::Format("%s: ENTER.", FNAME));

    wxUint32 nWrite;
    wxString response;

    wxString tableId;
    wxString requesterId;
    hoxNetworkEvent networkEvent;

    tableId = command.parameters["tid"];
    requesterId = command.parameters["pid"];

    // Find the table hosted on this system using the specified table-Id.
    hoxTable* table = hoxTableMgr::GetInstance()->FindTable( tableId );

    if ( table == NULL )
    {
        wxLogError(wxString::Format("%s: Table [%s] not found.", FNAME, tableId));
        response << "1\r\n"  // code
                 << "Table " << tableId << " not found.\r\n";
        goto exit_label;
    }

    /* Create a LEAVE-event and send it to the table. */

    if ( table->GetRedPlayer() != NULL && table->GetRedPlayer()->GetName() == requesterId )
    {
        networkEvent.type = hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_RED;
    }
    else if ( table->GetBlackPlayer() != NULL && table->GetBlackPlayer()->GetName() == requesterId )
    {
        networkEvent.type = hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_BLACK;
    }
    else
    {
        wxLogError(wxString::Format("%s: Player [%s] not found at the table [%s].", FNAME, requesterId, tableId));
        response << "2\r\n"  // code
                 << "Player " << requesterId << " not found at the table " << tableId << ".\r\n";
        goto exit_label;
    }

    networkEvent.content = requesterId;

    // Inform our table...
    table->OnEvent_FromWWWNetwork( this, networkEvent );

	// Finally, return 'success'.
	response << "0\r\n"       // error-code = SUCCESS
	         << "INFO: (LEAVE) Leave Table [" << tableId << "] OK\r\n";

	//echo "INFO: (LEAVE) [$playerId] LEAVE Table [$tableId] at seat [$leaveSeat] OK\n";

exit_label:
    // Send back response.
    nWrite = (wxUint32) response.size();
    sock->WriteMsg( response, nWrite );
    if ( sock->LastCount() != nWrite )
    {
        wxLogError(wxString::Format("%s: Writing to  socket failed.", FNAME));
        return;
    }

    wxLogDebug(wxString::Format("%s: END", FNAME));
}

void 
hoxNetworkPlayer::HandleCommand_Move( wxSocketBase*  sock,
                                      hoxCommand&    command )
{
    const char* FNAME = "hoxNetworkPlayer::HandleCommand_Move";

    wxLogDebug(wxString::Format("%s: ENTER.", FNAME));

    wxUint32 nWrite;
    wxString response;

    // We disable input events, so that the test doesn't trigger
    // wxSocketEvent again.
    sock->SetNotify(wxSOCKET_LOST_FLAG); // remove the wxSOCKET_INPUT_FLAG!!!

    hoxNetworkEvent networkEvent;

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
        return;
    }

    // Enable the input flag again.
    sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);

    wxLogDebug(wxString::Format("%s: END", FNAME));
}

//
// Handle New-Move command.
//
void 
hoxNetworkPlayer::HandleNewMove_FromNetwork( wxSocketBase *sock )
{
    const char* FNAME = "hoxNetworkPlayer::HandleNewMove_FromNetwork";
    wxLogDebug(wxString::Format(_("%s: START"), FNAME));

    wxString    tableId;
    hoxPosition newMoveFromPos;
    hoxPosition newMoveToPos;

    // Read the table-Id.
    wxUint32 len = hoxNETWORK_MAX_MSG_SIZE;
    wxChar *msg2 = new wxChar[len];

    sock->ReadMsg(msg2, len);
    if (sock->Error()) 
    {
        wxLogWarning(_("Read table-Id failed !"));
        delete[] msg2;
        return;
    }
    tableId = msg2;
    wxLogDebug(wxString::Format(_("... table-Id [%s]"), tableId));
    delete[] msg2;

    // Read the new move.
    wxLogDebug(wxString::Format(_("Reading the new move..."))); 
    sock->Read(&newMoveFromPos.x, 1);
    sock->Read(&newMoveFromPos.y, 1);
    sock->Read(&newMoveToPos.x, 1);
    sock->Read(&newMoveToPos.y, 1);
    wxLogDebug(wxString::Format(_("New move: [%d,%d] to [%d,%d]."),
        newMoveFromPos.x, newMoveFromPos.y, newMoveToPos.x, newMoveToPos.y));

    // Write the return-code
    unsigned char nRetCode = 1;  // *** TODO Always OK for now
    wxLogDebug(_T("Sending the return-code back..."));
    sock->Write(&nRetCode, 1);

    // Find the table hosted on this system using the specified table-Id.
    hoxTable* table = hoxTableMgr::GetInstance()->FindTable( tableId );
    if ( table == NULL )
    {
        wxLogError(wxString::Format(_("Failed to find table with ID = [%s]."), tableId));
        return;
    }

    // Update the new move on our table.
    //SaveLastMove(newMoveFromPos, newMoveToPos);
    table->OnMove_FromNetwork( this, newMoveFromPos, newMoveToPos );

    wxLogDebug(wxString::Format(_("%s: END"), FNAME));
}

void 
hoxNetworkPlayer::OnNewMove_FromTable( hoxPlayerEvent&  event )
{
    const char* FNAME = "hoxNetworkPlayer::OnNewMove_FromTable";
    hoxResult    result = hoxRESULT_ERR;
    wxUint32     nRead;
    wxChar*      buf = NULL;
    wxString     responseStr;
    int          returnCode = -1;
    wxString     returnMsg;
    wxUint32     nWrite;
    wxString     request;
    wxString     tableId;
    hoxPosition  moveFromPos;
    hoxPosition  moveToPos;
    wxString     moveStr;

    wxLogDebug("%s: ENTER.", FNAME);

    wxSocketBase* pSock = m_pCBSock;

    if ( !pSock || !pSock->IsConnected()) 
    {
        wxLogError("%s: Connection is NOT yet opened.", FNAME);
        return;
    }

    ////////////// Disable INPUT //////
    pSock->SetNotify(wxSOCKET_LOST_FLAG);
    //////////////

    tableId     = event.GetTableId();
    moveFromPos = event.GetOldPosition();
    moveToPos   = event.GetPosition();

    moveStr = wxString::Format("%d%d%d%d", 
                    moveFromPos.x, moveFromPos.y, moveToPos.x, moveToPos.y);

    request = wxString::Format("op=MOVE&tid=%s&pid=%s&move=%s\r\n", 
                        tableId, this->GetName(), moveStr);

    // Send request.
    wxLogDebug("%s: Sending request [%s] over the network...", FNAME, request);
    nWrite = (wxUint32) request.size();
    pSock->Write( request, nWrite );
    if ( pSock->LastCount() != nWrite )
    {
        wxLogError(wxString::Format("%s: Writing to  socket failed.", FNAME));
        goto exit_label;
    }

    // Wait until data available (will also return if the connection is lost)
    wxLogDebug("%s: Waiting for response from the network (timeout = 5 sec)...", FNAME);
    pSock->WaitForRead(5);

    // Read back the response.
    wxLogDebug("%s: Starting reading response...", FNAME);
    buf = new wxChar[hoxNETWORK_MAX_MSG_SIZE];

    pSock->ReadMsg( buf, hoxNETWORK_MAX_MSG_SIZE );
    nRead = pSock->LastCount();

    if ( nRead == 0 )
    {
        wxLogError("%s: Failed to read response from the network.", FNAME);
        goto exit_label;
    }

    responseStr.assign( buf, nRead );

    /* Parse the response */
    wxLogDebug("%s: Check (parse) the response...", FNAME);
    result = hoxWWWThread::parse_string_for_simple_response( responseStr,
                                                             returnCode,
                                                             returnMsg );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse for SEND-CONNECT's response.", FNAME);
        goto exit_label;
    }
    else if ( returnCode != 0 )
    {
        wxLogError("%s: Parsed response done: Return-code failed [%d] [%s].", 
            FNAME, returnCode, returnMsg);
        goto exit_label;
    }

exit_label:
    delete[] buf;
    ////////////// Enable INPUT //////
    pSock->SetNotify( wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG );
    //////////////

    wxLogDebug("[%s]: END.", FNAME);
    return;
}

hoxResult 
hoxNetworkPlayer::SetCBSocket( wxSocketBase* socket )
{
    const char* FNAME = "hoxNetworkPlayer::SetCBSocket";

    if ( m_pCBSock != NULL )
    {
        wxLogError( "%s: Callback socket already exists.", FNAME );
        return hoxRESULT_ERR;
    }

    wxLogDebug(wxString::Format("%s: Assign callback socket to this user [%s]", 
                    FNAME, GetName()));
    m_pCBSock = socket;
    return hoxRESULT_OK;
}

hoxResult 
hoxNetworkPlayer::DisconnectFromNetwork()
{
    if ( m_pCBSock != NULL )
    {
        m_pCBSock->Destroy();
        m_pCBSock = NULL;
    }

    return hoxRESULT_OK;
}

//
// Handle SOCKET_LOST event.
//
void 
hoxNetworkPlayer::HandleSocketLostEvent(wxSocketBase *sock)
{
    const char* FNAME = "hoxNetworkPlayer::HandleSocketLostEvent";
    wxLogDebug(wxString::Format("%s: ENTER", FNAME));

    wxASSERT( sock != NULL );
    wxASSERT_MSG( m_pCBSock == sock, "The socket should be the same." );

    m_pCBSock->Destroy();
    m_pCBSock = NULL;
}

/************************* END OF FILE ***************************************/
