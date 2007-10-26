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
    wxString msg;
    msg.Printf("%s: ", FNAME);

    switch(event.GetSocketEvent())
    {
        case wxSOCKET_INPUT      : msg.Append("wxSOCKET_INPUT"); break;
        case wxSOCKET_LOST       : msg.Append("wxSOCKET_LOST"); break;
        case wxSOCKET_CONNECTION : msg.Append("wxSOCKET_CONNECTION"); break;
        default                  : msg.Append(_("Unexpected event !")); break;
    }
    wxLogDebug(msg);

    wxSocketBase* sock = event.GetSocket();
  
    // Now we process the event
    switch( event.GetSocketEvent() )
    {
        case wxSOCKET_INPUT:
        {
            // Receive data from socket and send it back. We will first
            // get a byte with the buffer size, so we can specify the
            // exact size and use the wxSOCKET_WAITALL flag. Also, we
            // disabled input events so we won't have unwanted reentrance.
            // This way we can avoid the infamous wxSOCKET_BLOCK flag.


            wxString commandStr;
            hoxResult result = hoxServer::read_line( sock, commandStr );
            
            wxLogDebug(wxString::Format("%s: Received command-string [%s] received from client", 
                            FNAME, commandStr));
            
            hoxCommand   command;
            result = hoxServer::parse_command( commandStr, command );
            if ( result != hoxRESULT_OK )
            {
                wxLogError(wxString::Format("%s: Failed to parse command-string [%s].", 
                                FNAME, commandStr));
            }
            else
            {
                switch ( command.type )
                {
                    case hoxREQUEST_TYPE_LEAVE:
                        HandleCommand_Leave(sock, command); 
                        break;

                    case hoxREQUEST_TYPE_MOVE:
                        HandleCommand_Move(sock, command); 
                        break;

                    default:
                        wxLogError(wxString::Format("%s: Unknown command [%d] received from client", FNAME, command));
                        break;
                }
            }

#if 0
            // Which command are we going to run?
            unsigned char command;
            sock->Read( &command, 1 );

            switch( command )
            {
                case hoxNETWORK_CMD_NEW_MOVE: 
                    HandleNewMove_FromNetwork( sock );
                    break;

                default:
                    wxLogWarning(_("%s: Unknown command [%d] received from client"), FNAME, command);
            }
#endif
            break;
        }

        case wxSOCKET_LOST:
        {
            // Destroy() should be used instead of delete wherever possible,
            // due to the fact that wxSocket uses 'delayed events' (see the
            // documentation for wxPostEvent) and we don't want an event to
            // arrive to the event handler (the frame, here) after the socket
            // has been deleted. Also, we might be doing some other thing with
            // the socket at the same time; for example, we might be in the
            // middle of a test or something. Destroy() takes care of all
            // this for us.

            wxLogDebug(wxString::Format("%s: Invoke socket-lost event handler...", FNAME));
            HandleSocketLostEvent( sock );
            break;
        }
  
        default: 
            break;
    } // switch

    wxLogDebug(wxString::Format(_("%s: END"), FNAME));
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
    static const char* CMD_STR = "New-Move";
    hoxResult result = hoxRESULT_ERR;
    wxUint32  nRead;
    wxChar*   buf = NULL;
    wxString  responseStr;
    int       returnCode = -1;
    wxString  returnMsg;


    wxLogDebug(wxString::Format("%s: Send a [%s] command to the server...", FNAME, CMD_STR));

    wxSocketBase* pSock = m_pCBSock;

    if ( !pSock || !pSock->IsConnected()) 
    {
        wxLogError(wxString::Format("%s: Connection is NOT yet opened!", FNAME));
        return;
    }

    ////////////// Disable INPUT //////
    pSock->SetNotify(wxSOCKET_LOST_FLAG);
    //////////////

    wxUint32 nWrite;
    wxString request;

    wxString     tableId     = event.GetTableId();
    hoxPosition  moveFromPos = event.GetOldPosition();
    hoxPosition  moveToPos   = event.GetPosition();

    wxString moveStr = wxString::Format("%d%d%d%d", 
                            moveFromPos.x, moveFromPos.y, moveToPos.x, moveToPos.y);

    request = wxString::Format("op=MOVE&tid=%s&pid=%s&move=%s\r\n", 
                        tableId, this->GetName(), moveStr);

    // Send request.
    nWrite = (wxUint32) request.size();
    pSock->Write( request, nWrite );
    if ( pSock->LastCount() != nWrite )
    {
        wxLogError(wxString::Format("%s: Writing to  socket failed.", FNAME));
        goto exit_label;
    }

    // Wait until data available (will also return if the connection is lost)
    wxLogDebug(wxString::Format("%s: Waiting for an event (timeout = 2 sec)...", FNAME));
    pSock->WaitForRead(2);

    // Read back the response.

    buf = new wxChar[hoxNETWORK_MAX_MSG_SIZE];

    pSock->ReadMsg( buf, hoxNETWORK_MAX_MSG_SIZE );
    nRead = pSock->LastCount();

    if ( nRead == 0 )
    {
        wxLogError(wxString::Format("%s: Failed to read response.", FNAME));
        goto exit_label;
    }

    responseStr.assign( buf, nRead );

    /* Parse the response */
    result = hoxWWWThread::parse_string_for_simple_response( responseStr,
                                                             returnCode,
                                                             returnMsg );
    if ( result != hoxRESULT_OK )
    {
        wxLogError(wxString::Format("%s: Failed to parse for SEND-CONNECT's response.", FNAME));
        goto exit_label;
    }
    else if ( returnCode != 0 )
    {
        wxLogError(wxString::Format("%s: Send MOVE failed. [%s]", FNAME, returnMsg));
        goto exit_label;
    }

exit_label:
    delete[] buf;
    ////////////// Enable INPUT //////
    pSock->SetNotify( wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG );
    //////////////

    wxLogDebug(wxString::Format("[%s]: END", FNAME));
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
