/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPlayer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/06/2007
//
// Description:     The Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxPlayer.h"
#include "hoxEnums.h"
#include "hoxTable.h"
#include "hoxConnection.h"
#include "hoxTableMgr.h"
#include "hoxUtility.h"
#include "hoxNetworkAPI.h"

#include <algorithm>  // std::find

IMPLEMENT_CLASS(hoxPlayer, wxEvtHandler)

//----------------------------------------------------------------------------
// event types
//----------------------------------------------------------------------------
DEFINE_EVENT_TYPE(hoxEVT_PLAYER_TABLE_CLOSED)
DEFINE_EVENT_TYPE(hoxEVT_PLAYER_NEW_MOVE)

/* Define player-event based on wxCommandEvent */
DEFINE_EVENT_TYPE(hoxEVT_PLAYER_WALL_MSG)

BEGIN_EVENT_TABLE(hoxPlayer, wxEvtHandler)
    EVT_PLAYER_TABLE_CLOSED (wxID_ANY,  hoxPlayer::OnClose_FromTable)
    EVT_PLAYER_NEW_MOVE (wxID_ANY,  hoxPlayer::OnNewMove_FromTable)

    EVT_COMMAND(wxID_ANY, hoxEVT_PLAYER_WALL_MSG, hoxPlayer::OnWallMsg_FromTable)
END_EVENT_TABLE()


//-----------------------------------------------------------------------------
// hoxPlayer
//-----------------------------------------------------------------------------

hoxPlayer::hoxPlayer()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxPlayer::hoxPlayer( const wxString& name,
                      hoxPlayerType   type,
                      int             score /* = 1500 */)
            : m_name( name )
            , m_type( type )
            , m_score( score )
            , m_connection( NULL )
{ 
}

hoxPlayer::~hoxPlayer() 
{
    const char* FNAME = "hoxPlayer::~hoxPlayer";
    wxLogDebug("%s: ENTER.", FNAME);

    delete m_connection;
}

void               
hoxPlayer::AddRole( hoxRole role )
{
    // TODO: Check for duplicate!!! (join same table twice)
    m_roles.push_back( role );
}

void               
hoxPlayer::RemoveRole( hoxRole role )
{
    wxASSERT( this->HasRole( role ) );
    m_roles.remove( role );
}

bool
hoxPlayer::RemoveRoleAtTable( const wxString& tableId )
{
    for ( hoxRoleList::iterator it = m_roles.begin();
                                it != m_roles.end();
                              ++it )
    {
        if ( it->tableId == tableId )
        {
            m_roles.remove( *it );
            return true; // role found.
        }
    }

    return false;  // role not found.
}

bool               
hoxPlayer::HasRole( hoxRole role )
{
    hoxRoleList::iterator found 
        = std::find( m_roles.begin(), m_roles.end(), role );
    return ( found != m_roles.end() );
}

// We can check the player's color afterwards to 
// see which role the player has.
hoxResult 
hoxPlayer::JoinTable( hoxTable* table )
{
    wxCHECK_MSG( table != NULL, hoxRESULT_ERR, "The table is NULL." );
    // TODO: Check for duplicate!!! (join same table twice)
    hoxPieceColor assignedColor;
    hoxResult result = table->AssignPlayer( this, assignedColor );
    if ( result == hoxRESULT_OK )
    {
        this->AddRole( hoxRole( table->GetId(), assignedColor ) );
    }
    return result;
}

hoxResult 
hoxPlayer::LeaveTable( hoxTable* table )
{
    wxCHECK_MSG(table != NULL, hoxRESULT_ERR, "The table is NULL." );

    table->OnLeave_FromPlayer( this );
    this->RemoveRoleAtTable( table->GetId() );
    return hoxRESULT_OK;
}

hoxResult 
hoxPlayer::LeaveAllTables()
{
    const char* FNAME = "hoxPlayer::LeaveAllTables";
    bool bErrorFound = false;

    wxLogDebug("%s: ENTER.", FNAME);

    while ( ! m_roles.empty() )
    {
        const wxString tableId = m_roles.front().tableId;
        m_roles.pop_front();

        // Find the table hosted on this system using the specified table-Id.
        hoxTable* table = hoxTableMgr::GetInstance()->FindTable( tableId );
        if ( table == NULL )
        {
            wxLogError("%s: Failed to find table with ID = [%s].", FNAME, tableId.c_str());
            bErrorFound = true;
            continue;
        }

        // Inform the table that this player is leaving...
        wxLogDebug("%s: Player [%s] leaving table [%s]...", FNAME, this->GetName().c_str(), tableId.c_str());
        table->OnLeave_FromPlayer( this );
    }

    return bErrorFound ? hoxRESULT_ERR : hoxRESULT_OK;
}

void 
hoxPlayer::ResetConnection()
{ 
    const char* FNAME = "hoxPlayer::ResetConnection";

    wxLogDebug("%s: ENTER.", FNAME);
    delete m_connection; 
    m_connection = NULL; 
}

void 
hoxPlayer::OnClose_FromTable( hoxPlayerEvent&  event )
{
    const char* FNAME = "hoxPlayer::OnClose_FromTable";

    const wxString tableId  = event.GetTableId();

    this->RemoveRoleAtTable( tableId );
}

void 
hoxPlayer::OnNewMove_FromTable( hoxPlayerEvent&  event )
{
    const char* FNAME = "hoxPlayer::OnNewMove_FromTable";

    if ( m_connection == NULL )
    {
        wxLogDebug("%s: No connection. Fine. Ignore this Move.", FNAME);
    }
    else
    {
        wxString     tableId     = event.GetTableId();
        hoxPosition  moveFromPos = event.GetOldPosition();
        hoxPosition  moveToPos   = event.GetPosition();

        wxString moveStr = wxString::Format("%d%d%d%d", 
                                moveFromPos.x, moveFromPos.y, moveToPos.x, moveToPos.y);

        wxLogDebug("%s: ENTER. Move = [%s].", FNAME, moveStr.c_str());

        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_MOVE );
        request->content =
                wxString::Format("op=MOVE&tid=%s&pid=%s&move=%s\r\n", 
                            tableId.c_str(), this->GetName().c_str(), moveStr.c_str());
        m_connection->AddRequest( request );
    }
}

void 
hoxPlayer::OnWallMsg_FromTable( wxCommandEvent&  event )
{
    const char* FNAME = "hoxPlayer::OnWallMsg_FromTable";
    
    if ( m_connection == NULL )
    {
        wxLogDebug("%s: No connection. Fine. Ignore this Message.", FNAME);
    }
    else
    {
        const wxString commandStr = event.GetString();
        wxLogDebug("%s: ENTER. commandStr = [%s].", FNAME, commandStr.c_str());

        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_WALL_MSG );
        request->content = 
            wxString::Format("op=WALL_MSG&%s\r\n", commandStr.c_str());
        m_connection->AddRequest( request );
    }
}

bool 
hoxPlayer::SetConnection( hoxConnection* connection )
{
    const char* FNAME = "hoxPlayer::SetConnection";

    if ( m_connection != NULL )
    {
        wxLogDebug("%s: Connection already set to this user [%s]. Fine. END.", 
            FNAME, GetName().c_str());
        return false;
    }

    wxLogDebug("%s: Assign the connection to this user [%s]", FNAME, GetName().c_str());
    m_connection = connection;
    return true;
}

hoxResult 
hoxPlayer::HandleIncomingData( const wxString& commandStr )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData";
    hoxResult   result = hoxRESULT_OK;
    hoxCommand  command;

    wxLogDebug("%s: ENTER.", FNAME);

    result = hoxNetworkAPI::ParseCommand( commandStr, command );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse command-string [%s].", FNAME, commandStr.c_str());
        return hoxRESULT_ERR;
    }
    wxLogDebug("%s: Received a command [%s].", FNAME, 
        hoxUtility::RequestTypeToString(command.type).c_str());

    switch ( command.type )
    {
        case hoxREQUEST_TYPE_MOVE:
            result = this->HandleIncomingData_Move( /*sock,*/ command );
            break;

        case hoxREQUEST_TYPE_LEAVE:
            result = this->HandleIncomingData_Leave( /*sock,*/ command );
            break;

        case hoxREQUEST_TYPE_WALL_MSG:
            result = this->HandleIncomingData_WallMsg( /*sock,*/ command );
            break;

        case hoxREQUEST_TYPE_LIST:
            result = this->HandleIncomingData_List( /*sock,*/ command ); 
            break;

        case hoxREQUEST_TYPE_JOIN:
            result = this->HandleIncomingData_Join( /*sock,*/ command ); 
            break;

        default:
            wxLogError("%s: Unsupported Request-Type [%s].", 
                FNAME, hoxUtility::RequestTypeToString(command.type).c_str());
            result = hoxRESULT_NOT_SUPPORTED;
            break;
    }

//exit_label:
    wxLogDebug("%s: END.", FNAME);

    return result;
}

hoxResult   
hoxPlayer::HandleIncomingData_Move( /* wxSocketBase* sock, */
                                    hoxCommand&   command )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData_Move";
    hoxResult       result = hoxRESULT_ERR;   // Assume: failure.
    //wxUint32        nWrite;
    wxString        response;

    wxString moveStr = command.parameters["move"];
    wxString tableId = command.parameters["tid"];
    wxString playerId = command.parameters["pid"];
    hoxPlayer* player = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

    // Find the table hosted on this system using the specified table-Id.
    hoxTable* table = hoxTableMgr::GetInstance()->FindTable( tableId );

    if ( table == NULL )
    {
        wxLogError("%s: Table [%s] not found.", FNAME, tableId.c_str());
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
        wxLogError("%s: Player [%s] not found at the table [%s].", 
            FNAME, playerId.c_str(), tableId.c_str());
        response << "2\r\n"  // code
                 << "Player " << playerId << " not found.\r\n";
        goto exit_label;
    }

    // Inform our table...
    table->OnMove_FromNetwork( player, moveStr );

    // Finally, return 'success'.
    response << "0\r\n"       // error-code = SUCCESS
             << "INFO: (MOVE) Move at Table [" << tableId << "] OK\r\n";

    result = hoxRESULT_OK;

exit_label:
    //// Send back response.
    //nWrite = (wxUint32) response.size();
    //sock->WriteMsg( response, nWrite );
    //if ( sock->LastCount() != nWrite )
    //{
    //    wxLogError("%s: Writing to socket failed. Error = [%s]", 
    //        FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
    //    result = hoxRESULT_ERR;
    //}

    /* Send response back to the remote player */
    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_OUT_DATA );
    request->content = response;
    m_connection->AddRequest( request );

    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult 
hoxPlayer::HandleIncomingData_Leave( /* wxSocketBase*  sock,*/
                                     hoxCommand&    command )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData_Leave";
    hoxResult       result = hoxRESULT_ERR;   // Assume: failure.
    //wxUint32        nWrite;
    wxString        response;
    wxString        tableId;
    wxString        requesterId;
    hoxPlayer*      player = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

    tableId = command.parameters["tid"];
    requesterId = command.parameters["pid"];

    // Find the table hosted on this system using the specified table-Id.
    hoxTable* table = hoxTableMgr::GetInstance()->FindTable( tableId );

    if ( table == NULL )
    {
        wxLogError("%s: Table [%s] not found.", FNAME, tableId.c_str());
        response << "1\r\n"  // code
                 << "Table " << tableId << " not found.\r\n";
        goto exit_label;
    }

    /* Create a LEAVE-event and send it to the table. */

    if ( table->GetRedPlayer() != NULL && table->GetRedPlayer()->GetName() == requesterId )
    {
        player = table->GetRedPlayer();
    }
    else if ( table->GetBlackPlayer() != NULL && table->GetBlackPlayer()->GetName() == requesterId )
    {
        player = table->GetBlackPlayer();
    }
    else
    {
        wxLogError("%s: Player [%s] not found at the table [%s].", FNAME, requesterId.c_str(), tableId.c_str());
        response << "2\r\n"  // code
                 << "Player " << requesterId << " not found at the table " << tableId << ".\r\n";
        goto exit_label;
    }


    // Inform our table...
    table->OnLeave_FromPlayer( player );

	// Finally, return 'success'.
	response << "0\r\n"       // error-code = SUCCESS
	         << "INFO: (LEAVE) Leave Table [" << tableId << "] OK\r\n";

    result = hoxRESULT_OK;

exit_label:
    //// Send back response.
    //nWrite = (wxUint32) response.size();
    //sock->WriteMsg( response, nWrite );
    //if ( sock->LastCount() != nWrite )
    //{
    //    wxLogError("%s: Writing to socket failed. Error = [%s]", 
    //        FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
    //    result = hoxRESULT_ERR;
    //}

    /* Send response back to the remote player */
    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_OUT_DATA );
    request->content = response;
    m_connection->AddRequest( request );

    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult   
hoxPlayer::HandleIncomingData_WallMsg( /* wxSocketBase* sock, */
                                       hoxCommand&   command )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData_WallMsg";
    hoxResult       result = hoxRESULT_ERR;   // Assume: failure.
    //wxUint32        nWrite;
    wxString        response;

    wxString message = command.parameters["msg"];
    wxString tableId = command.parameters["tid"];
    wxString playerId = command.parameters["pid"];

    wxLogDebug("%s: ENTER.", FNAME);

    // Find the table hosted on this system using the specified table-Id.
    hoxTable* table = hoxTableMgr::GetInstance()->FindTable( tableId );

    if ( table == NULL )
    {
        wxLogError("%s: Table [%s] not found.", FNAME, tableId.c_str());
        response << "1\r\n"  // code
                 << "Table " << tableId << " not found.\r\n";
        goto exit_label;
    }

    /* Look up player */
    // TODO: Ignore looking up the player for now!!!

    // Inform our table...
    table->OnMessage_FromNetwork( playerId, message );

    // Finally, return 'success'.
    response << "0\r\n"       // error-code = SUCCESS
             << "INFO: (MESSAGE) Message at Table [" << tableId << "] OK\r\n";

    result = hoxRESULT_OK;

exit_label:
    //// Send back response.
    //nWrite = (wxUint32) response.size();
    //sock->WriteMsg( response, nWrite );
    //if ( sock->LastCount() != nWrite )
    //{
    //    wxLogError("%s: Writing to socket failed. Error = [%s]", 
    //        FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
    //    result = hoxRESULT_ERR;
    //}

    /* Send response back to the remote player */
    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_OUT_DATA );
    request->content = response;
    m_connection->AddRequest( request );

    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult 
hoxPlayer::HandleIncomingData_List( /* wxSocketBase *sock, */
                                    hoxCommand&   command )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData_List";

    // Get the list of hosted tables.
    const hoxTableList& tables = hoxTableMgr::GetInstance()->GetTables();
    wxUint32 tableCount = (wxUint32) tables.size();

    // Write it back
    wxLogDebug("%s: ... We have [%d] tables.", FNAME, tableCount);

    //wxUint32 nWrite;
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

    /* Send response back to the remote player */
    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_OUT_DATA );
    request->content = response;
    m_connection->AddRequest( request );

    //nWrite = (wxUint32) response.size();
    //sock->WriteMsg( response, nWrite );
    //if ( sock->LastCount() != nWrite )
    //{
    //    wxLogError("%s: Writing to socket failed. Error = [%s]", 
    //        FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
    //    return;
    //}
    return hoxRESULT_OK;
}

hoxResult 
hoxPlayer::HandleIncomingData_Join( /* wxSocketBase *sock, */
                                    hoxCommand&   command )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData_Join";
    hoxResult result = hoxRESULT_ERR;   // Assume: failure.
    hoxTable* table = NULL;
    wxString  existingPlayerId;
    //wxUint32  nWrite;
    wxString  response;

    wxLogDebug("%s: ENTER.", FNAME);

    const wxString tableId = command.parameters["tid"];

    // Find the table hosted on this system using the specified table-Id.
    table = hoxTableMgr::GetInstance()->FindTable( tableId );
    if ( table == NULL )
    {
        wxLogError("%s: Table [%s] not found.", FNAME, tableId.c_str());
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
        wxLogError("%s: No one is at the table [%s] not found.", FNAME, tableId.c_str());
        response << "2\r\n"  // code
                 << "Not one is at the table " << tableId << ".\r\n";
        goto exit_label;
    }

    /***********************/
    /* Setup players       */
    /***********************/

    //// Create a connection for the new player.
    //hoxRemoteConnection* connection = new hoxRemoteConnection();
    //connection->SetServer( this );
    //connection->SetCBSocket( sock );

    //player->SetConnection( connection );

    //// Setup the event handler and let the player handles all socket events.
    //wxLogDebug("%s: Let this Remote player [%s] handle all socket events", 
    //    FNAME, player->GetName().c_str());
    //sock->SetEventHandler(*player, CLIENT_SOCKET_ID);
    //sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    //sock->Notify(true);

    result = this->JoinTable( table );
    wxASSERT( result == hoxRESULT_OK  );
    wxASSERT_MSG( this->HasRole( hoxRole(table->GetId(), 
                                         hoxPIECE_COLOR_BLACK) ),
                  "Player must play BLACK");

	// Finally, return 'success'.
	response << "0\r\n"       // error-code = SUCCESS
	         << "INFO: (JOIN) Join Table [" << tableId << "] OK\r\n"
	         << tableId << " " << "1 " << existingPlayerId << " " << this->GetName() << "\r\n";

    result = hoxRESULT_OK;

exit_label:
    //// Send back response.
    //nWrite = (wxUint32) response.size();
    //sock->WriteMsg( response, nWrite );
    //if ( sock->LastCount() != nWrite )
    //{
    //    wxLogError("%s: Failed to send back response over the network.", 
    //        FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
    //    //return;
    //}

    /* Send response back to the remote player */
    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_OUT_DATA );
    request->content = response;
    m_connection->AddRequest( request );

    wxLogDebug("%s: END.", FNAME);
    return result;
}

/************************* END OF FILE ***************************************/
