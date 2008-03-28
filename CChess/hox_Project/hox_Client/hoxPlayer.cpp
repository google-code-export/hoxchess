/***************************************************************************
 *  Copyright 2007, 2008 Huy Phan  <huyphan@playxiangqi.com>               *
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
// Name:            hoxPlayer.cpp
// Created:         10/06/2007
//
// Description:     The Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxPlayer.h"
#include "hoxTable.h"
#include "hoxConnection.h"
#include "hoxTableMgr.h"
#include "hoxPlayerMgr.h"
#include "hoxUtil.h"
#include "hoxNetworkAPI.h"
#include "MyApp.h"

#include <algorithm>  // std::find

IMPLEMENT_DYNAMIC_CLASS(hoxPlayer, wxEvtHandler)

//----------------------------------------------------------------------------
// Event types
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(hoxPlayer, wxEvtHandler)
	// Empty table
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
            , m_nOutstandingRequests( 0 )
			, m_siteClosing( false )
{ 
}

hoxPlayer::~hoxPlayer() 
{
    const char* FNAME = "hoxPlayer::~hoxPlayer";
    wxLogDebug("%s: ENTER. (%s)", FNAME, this->GetName().c_str());

    if ( m_connection == NULL )
        return;

    if ( m_nOutstandingRequests > 0 )
    {
        wxLogDebug("%s: *** WARN *** There are still [%d] outstanding requests waiting to be serviced.", 
            FNAME, m_nOutstandingRequests);
    }

    this->ShutdownConnection();

    /* Deleting the connection itself.
     */
    wxLogDebug("%s: Deleting connection...", FNAME);
    delete m_connection;
}

void               
hoxPlayer::AddRole( hoxRole role )
{
    for ( hoxRoleList::iterator it = m_roles.begin();
                                it != m_roles.end();
                              ++it )
    {
        if ( it->tableId == role.tableId )
        {
			it->color = role.color;
			return;  // *** DONE.
        }
    }

    m_roles.push_back( role );
}

void               
hoxPlayer::RemoveRole( hoxRole role )
{
    wxASSERT( this->HasRole( role ) );
    m_roles.remove( role );

    if ( m_siteClosing && /*m_nOutstandingRequests == 0 &&*/ m_roles.empty() )
    {
		_PostSite_ShutdownReady();
    }
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
			this->RemoveRole( *it );
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

bool
hoxPlayer::HasRoleAtTable( const wxString& tableId ) const
{
	hoxColor assignedColor;
	return this->FindRoleAtTable( tableId, assignedColor );
}

bool
hoxPlayer::FindRoleAtTable( const wxString& tableId, 
	                        hoxColor&  assignedColor ) const
{
    for ( hoxRoleList::const_iterator it = m_roles.begin();
                                      it != m_roles.end();
                                    ++it )
    {
        if ( it->tableId == tableId )
        {
			assignedColor = it->color;
            return true; // role found.
        }
    }

	return false; // role not found.
}

/**
 * @note We can check the player's color afterwards to 
 *       see which role the player has.
 */
hoxResult 
hoxPlayer::JoinTable( hoxTable* table )
{
    const char* FNAME = "hoxPlayer::JoinTable";

    wxCHECK_MSG( table != NULL, hoxRC_ERR, "The table is NULL." );
    // TODO: Check for duplicate!!! (join same table twice)
    hoxColor assignedColor;
    bool          informOthers = true;

    /* NOTE: Except for dummy players, this player will inform other
     *       about his presence.
     */
    if ( this->GetType() == hoxPLAYER_TYPE_DUMMY )
    {
        wxLogDebug("%s: Dummy player [%s] will not inform others about his JOIN.", 
            FNAME, this->GetName().c_str());
        informOthers = false;
    }

    hoxResult result = table->AssignPlayer( this, 
                                            assignedColor, 
                                            informOthers );
    if ( result == hoxRC_OK )
    {
        this->AddRole( hoxRole( table->GetId(), assignedColor ) );
    }

    return result;
}

hoxResult 
hoxPlayer::JoinTableAs( hoxTable*     table,
                        hoxColor requestColor )
{
    wxCHECK_MSG( table != NULL, hoxRC_ERR, "The table is NULL." );

    hoxResult result = table->AssignPlayerAs( this, requestColor );
    if ( result == hoxRC_OK )
    {
        this->AddRole( hoxRole( table->GetId(), requestColor ) );
    }
    return result;
}

hoxResult 
hoxPlayer::LeaveTable( hoxTable* table )
{
    const char* FNAME = "hoxPlayer::LeaveTable";

    wxCHECK_MSG(table != NULL, hoxRC_ERR, "The table is NULL." );

    wxLogDebug("%s: Player [%s] is leaving table [%s]...", 
        FNAME, this->GetName().c_str(), table->GetId().c_str());

    table->OnLeave_FromPlayer( this );
    this->RemoveRoleAtTable( table->GetId() );

    return hoxRC_OK;
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
        hoxTable* table = m_site->FindTable( tableId );
        if ( table == NULL )
        {
            wxLogError("%s: Failed to find table with ID = [%s].", FNAME, tableId.c_str());
            bErrorFound = true;
            continue;
        }

        // Inform the table that this player is leaving...
        this->LeaveTable( table );
    }

    return bErrorFound ? hoxRC_ERR : hoxRC_OK;
}

bool 
hoxPlayer::ResetConnection()
{ 
    const char* FNAME = "hoxPlayer::ResetConnection";

    wxLogDebug("%s: ENTER.", FNAME);

    if ( m_connection != NULL )
    {
        wxLogDebug("%s: Request the Connection thread to be shutdowned...", FNAME);
        hoxRequest* request = new hoxRequest( hoxREQUEST_SHUTDOWN, NULL );
        m_connection->AddRequest( request );

        m_connection->Shutdown();

        delete m_connection; 
        m_connection = NULL; 
    }

	return true;  // everything is fine.
}

void 
hoxPlayer::OnClose_FromTable( const wxString& tableId )
{
    this->RemoveRoleAtTable( tableId );
}

void 
hoxPlayer::OnRequest_FromTable( hoxRequest* request )
{
    const char* FNAME = "hoxPlayer::OnRequest_FromTable";

	std::auto_ptr<hoxRequest> apRequest( request );

    if ( m_connection == NULL )
    {
        wxLogDebug("%s: No connection. Fine. Ignore this Request.", FNAME);
        return;
    }

    apRequest->sender = this;
    this->AddRequestToConnection( apRequest.release() );
}

void 
hoxPlayer::OnClosing_FromSite()
{
    const char* FNAME = "hoxPlayer::OnClosing_FromSite";

    wxLogDebug("%s: ENTER. (player = [%s])", FNAME, this->GetName().c_str());

    m_siteClosing = true; // *** Turn it ON!

    if ( /*m_nOutstandingRequests == 0 &&*/ m_roles.empty() )
    {
		_PostSite_ShutdownReady();
    }
}

bool 
hoxPlayer::SetConnection( hoxConnection* connection )
{
    const char* FNAME = "hoxPlayer::SetConnection";

    if ( m_connection != NULL )
    {
        wxLogDebug("%s: Connection already set to this Player [%s]. Fine. END.", 
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
    hoxResult   result = hoxRC_OK;
    hoxCommand  command;
    wxString    response;

    wxLogDebug("%s: ENTER.", FNAME);

    result = hoxNetworkAPI::ParseCommand( commandStr, command );
    if ( result != hoxRC_OK )
    {
        wxLogError("%s: Failed to parse command-string [%s].", FNAME, commandStr.c_str());
        return hoxRC_ERR;
    }
    wxLogDebug("%s: Received a command [%s].", FNAME, 
        hoxUtil::RequestTypeToString(command.type).c_str());

    switch ( command.type )
    {
        case hoxREQUEST_LOGOUT:
            result = this->HandleIncomingData_Disconnect( command );
            break;

        case hoxREQUEST_MOVE:
            result = this->HandleIncomingData_Move( command, response );
            break;

        case hoxREQUEST_LEAVE:
            result = this->HandleIncomingData_Leave( command, response );
            break;

        case hoxREQUEST_MSG:
            result = this->HandleIncomingData_WallMsg( command, response );
            break;

        case hoxREQUEST_LIST:
            result = this->HandleIncomingData_List( command, response ); 
            break;

        case hoxREQUEST_JOIN:
            result = this->HandleIncomingData_Join( command, response ); 
            break;

        case hoxREQUEST_E_JOIN:
            result = this->HandleIncomingData_NewJoin( command, response ); 
            break;

        case hoxREQUEST_NEW:
            result = this->HandleIncomingData_New( command, response ); 
            break;

        default:
            wxLogDebug("%s: *** WARN *** Unsupported Request-Type [%s]. Command-Str = [%s].", 
                FNAME, hoxUtil::RequestTypeToString(command.type).c_str(), commandStr.c_str());
            result = hoxRC_NOT_SUPPORTED;
            //response = "Not supported.";
            break;
    }

    /* Send response back to the remote player 
     */
	if ( ! response.empty() )
	{
		hoxRequest* request = new hoxRequest( hoxREQUEST_OUT_DATA, this );
        request->parameters["data"] = response;
		this->AddRequestToConnection( request );
	}

    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult 
hoxPlayer::HandleIncomingData_Disconnect( hoxCommand& command )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData_Disconnect";
    hoxResult result = hoxRC_ERR;   // Assume: failure.

    wxLogDebug("%s: ENTER.", FNAME);

    const wxString content = command.parameters["content"];
    const wxString playerId = content;

    /* Check the player-Id. */
    if ( playerId != this->GetName() )
    {
        wxLogError("%s: No player-Id. (%s vs. %s)", FNAME, playerId.c_str(), this->GetName().c_str());
        goto exit_label;
    }

	/* Shutdown the connection. */
	wxLogDebug("%s: Shutdown the connection for player [%s]...", FNAME, playerId.c_str());
	this->ShutdownConnection();

    /* Inform the site about this event. */
    m_site->Handle_DisconnectFromPlayer( this );

	/* Finally, return 'success'. */
    result = hoxRC_OK;

exit_label:
    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult   
hoxPlayer::HandleIncomingData_Move( hoxCommand& command,
                                    wxString&   response )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData_Move";
    hoxResult       result = hoxRC_ERR;   // Assume: failure.

    wxString moveStr = command.parameters["move"];
    wxString tableId = command.parameters["tid"];
    wxString playerId = command.parameters["pid"];
    hoxPlayer* player = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

    // Find the table hosted on this system using the specified table-Id.
    hoxTable* table = m_site->FindTable( tableId );

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

    result = hoxRC_OK;

exit_label:
    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult 
hoxPlayer::HandleIncomingData_Leave( hoxCommand& command,
                                     wxString&   response )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData_Leave";
    hoxResult       result = hoxRC_ERR;   // Assume: failure.
    wxString        tableId;
    wxString        requesterId;
    hoxPlayer*      player = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

    tableId = command.parameters["tid"];
    requesterId = command.parameters["pid"];

    // Find the table hosted on this system using the specified table-Id.
    hoxTable* table = m_site->FindTable( tableId );

    if ( table == NULL )
    {
        wxLogDebug("%s: *** WARN *** Table [%s] not found.", FNAME, tableId.c_str());
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
    table->OnLeave_FromNetwork( player, this );
    if ( this == player )
    {
        wxLogDebug("%s: Remove myself as Player [%s] from table [%s].", 
            FNAME, this->GetName().c_str(), table->GetId().c_str());
        this->RemoveRoleAtTable( table->GetId() );
    }

	// Finally, return 'success'.
	response << "0\r\n"       // error-code = SUCCESS
	         << "INFO: (LEAVE) Leave Table [" << tableId << "] OK\r\n";

    result = hoxRC_OK;

exit_label:
    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult   
hoxPlayer::HandleIncomingData_WallMsg( hoxCommand& command,
                                       wxString&   response )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData_WallMsg";
    hoxResult   result = hoxRC_ERR;
    hoxTable*   table = NULL;
    hoxPlayer*  player = NULL;

    wxString message = command.parameters["msg"];
    wxString tableId = command.parameters["tid"];
    wxString playerId = command.parameters["pid"];

    wxLogDebug("%s: ENTER.", FNAME);

    /* Lookup table */

    table = m_site->FindTable( tableId );
    if ( table == NULL )
    {
        wxLogError("%s: Table [%s] not found.", FNAME, tableId.c_str());
        response << "1\r\n"  // code
                 << "Table " << tableId << " not found.\r\n";
        goto exit_label;
    }

    /* Look up player */

    player = m_site->FindPlayer( playerId );
    if ( player == NULL )
    {
        wxLogError("%s: Player [%s] not found.", FNAME, playerId.c_str());
        response << "2\r\n"  // code
                 << "Player " << playerId << " not found.\r\n";
        goto exit_label;
    }

    // Inform our table...
    table->OnMessage_FromNetwork( playerId, message );

    // Finally, return 'success'.
    response << "0\r\n"       // error-code = SUCCESS
             << "INFO: (MESSAGE) Message at Table [" << tableId << "] OK\r\n";

    result = hoxRC_OK;

exit_label:
    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult 
hoxPlayer::HandleIncomingData_List( hoxCommand& command,
                                    wxString&   response )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData_List";

    wxLogDebug("%s: ENTER.", FNAME);

    // Get the list of hosted tables.
    const hoxTableList& tables = m_site->GetTables();
    wxUint32 tableCount = (wxUint32) tables.size();

    // Write it back
    wxLogDebug("%s: ... We have [%d] tables.", FNAME, tableCount);

    response << "0\r\n"  // code
		     << "INFO: We have " << tableCount << " tables\r\n";  // message

    // Return the info of tables.
    wxString iTimes, rTimes, bTimes;
    wxString rid, rscore;
    wxString bid, bscore;
    for ( hoxTableList::const_iterator it = tables.begin(); 
                                       it != tables.end(); ++it )
    {
        hoxPlayer* redPlayer   = (*it)->GetRedPlayer();
        hoxPlayer* blackPlayer = (*it)->GetBlackPlayer();
		iTimes = hoxUtil::TimeInfoToString( (*it)->GetInitialTime() );
		rTimes = hoxUtil::TimeInfoToString( (*it)->GetRedTime() );
		bTimes = hoxUtil::TimeInfoToString( (*it)->GetBlackTime() );
        if ( redPlayer != NULL ) {
            rid = redPlayer->GetName();
            rscore = wxString::Format("%d", redPlayer->GetScore());
        } else {
            rid = "";
            rscore = "";
        }
        if ( blackPlayer != NULL ) {
            bid = blackPlayer->GetName();
            bscore = wxString::Format("%d", blackPlayer->GetScore());
        } else {
            bid = "";
            bscore = "";
        }

        response << (*it)->GetId() << ";"
                 << "0" << ";"  // TODO: Hard-coded for Group
				 << "0" << ";"  // TODO: Hard-coded for Type
				 << iTimes << ";"
				 << rTimes << ";"
				 << bTimes << ";"
                 << rid << ";"
                 << rscore << ";"
                 << bid << ";"
                 << bscore << ";"
                 << "\r\n";
    }

    wxLogDebug("%s: END.", FNAME);
    return hoxRC_OK;
}

hoxResult 
hoxPlayer::HandleIncomingData_Join( hoxCommand& command,
                                    wxString&   response )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData_Join";
    hoxResult  result = hoxRC_ERR;
    hoxTable*  table = NULL;
    hoxPlayer* redPlayer = NULL;
    hoxPlayer* blackPlayer = NULL;
    wxString   iTimes, rTimes, bTimes;
    wxString   rid, rscore;
    wxString   bid, bscore;

    wxLogDebug("%s: ENTER.", FNAME);

    const wxString tableId = command.parameters["tid"];

    // Find the table hosted on this system using the specified table-Id.
    table = m_site->FindTable( tableId );
    if ( table == NULL )
    {
        wxLogError("%s: Table [%s] not found.", FNAME, tableId.c_str());
        response << "1\r\n"  // code
                 << "Table " << tableId << " not found.\r\n";
        goto exit_label;
    }

    /***********************/
    /* Setup players       */
    /***********************/

    result = this->JoinTable( table );
    if ( result != hoxRC_OK  )
    {
        wxLogError("%s: Failed to ask Table [%s] to join.", FNAME, tableId.c_str());
        response << "2\r\n"  // code
                 << "JOIN failed at table " << tableId << ".\r\n";
        goto exit_label;
    }

	// Finally, return 'success'.

    redPlayer = table->GetRedPlayer();
    blackPlayer = table->GetBlackPlayer();
	iTimes = hoxUtil::TimeInfoToString( table->GetInitialTime() );
	rTimes = hoxUtil::TimeInfoToString( table->GetRedTime() );
	bTimes = hoxUtil::TimeInfoToString( table->GetBlackTime() );
    if ( redPlayer != NULL ) {
        rid = redPlayer->GetName();
        rscore = wxString::Format("%d", redPlayer->GetScore());
    } else {
        rid = "";
        rscore = "";
    }
    if ( blackPlayer != NULL ) {
        bid = blackPlayer->GetName();
        bscore = wxString::Format("%d", blackPlayer->GetScore());
    } else {
        bid = "";
        bscore = "";
    }

	response << "0\r\n"       // error-code = SUCCESS
	         << "INFO: (JOIN) Join Table [" << tableId << "] OK\r\n"
             << tableId << ";"
             << "0" << ";"  // TODO: Hard-coded for Group
			 << "0" << ";"  // TODO: Hard-coded for Type
			 << iTimes << ";"
			 << rTimes << ";"
			 << bTimes << ";"
             << rid << ";"
             << rscore << ";"
             << bid << ";"
             << bscore << ";"
             << "\r\n";

    result = hoxRC_OK;

exit_label:
    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult 
hoxPlayer::HandleIncomingData_NewJoin( hoxCommand& command,
                                       wxString&   response )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData_NewJoin";
    hoxResult result = hoxRC_ERR;
    hoxTable*  table = NULL;
    hoxPlayer* player = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

    const wxString tableId = command.parameters["tid"];
    const wxString playerId = command.parameters["pid"];
    const hoxColor requestColor = 
        (hoxColor) ::atoi(command.parameters["color"]); // FIXME: Force it!!!

    /* Lookup Table. */

    table = m_site->FindTable( tableId );
    if ( table == NULL )
    {
        wxLogError("%s: Table [%s] not found.", FNAME, tableId.c_str());
        response << "1\r\n"  // code
                 << "Table " << tableId << " not found.\r\n";
        goto exit_label;
    }

    /* Lookup Player. */

    player = m_site->FindPlayer( playerId );
    if ( player == NULL )
    {
        /* The site that THIS Player belongs must be remote. */
        wxASSERT_MSG( m_site->GetType() != hoxSITE_TYPE_LOCAL, "The site must be remote.");
        player = m_site->CreateDummyPlayer( playerId );
    }

    /* Request to join the Table as the specified color. 
     * NOTE: The player in this case can be different from THIS player.
     */

    result = player->JoinTableAs( table, requestColor );
    if ( result != hoxRC_OK )
    {
        wxLogError("%s: Table denied the JOIN request as color [%d] from player [%s].", 
            FNAME, requestColor, playerId.c_str());
        response << "3\r\n"  // code
                 << "Table " << tableId << " denied the JOIN request.\r\n";
        goto exit_label;
    }
    wxASSERT_MSG( player->HasRole( hoxRole(table->GetId(), requestColor) ),
                  "Player must play the specified color");

	// Finally, return 'success'.
	response << "0\r\n"       // error-code = SUCCESS
	         << "INFO: (NEW-JOIN) New-Join Table [" << tableId << "] OK\r\n";

    result = hoxRC_OK;

exit_label:
    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult 
hoxPlayer::HandleIncomingData_New( hoxCommand& command,
                                   wxString&   response )
{
    const char* FNAME = "hoxPlayer::HandleIncomingData_New";
    hoxResult result = hoxRC_ERR;   // Assume: failure.
    hoxTable* table = NULL;
    wxString newTableId;

    wxLogDebug("%s: ENTER.", FNAME);

    const wxString playerId = command.parameters["pid"];
	const wxString itimes = command.parameters["itimes"];
	const hoxTimeInfo initialTime = hoxUtil::StringToTimeInfo( itimes );

    /* Check the player-Id. */
    if ( playerId != this->GetName() )
    {
        wxLogDebug("%s: *** ERROR *** No player-Id. (%s vs. %s)", 
            FNAME, playerId.c_str(), this->GetName().c_str());
        response << "1\r\n"  // code
                 << "Wrong player-Id. " << playerId << " vs. " << this->GetName() << ".\r\n";
        goto exit_label;
    }

    /* Create a new Table. */
    result = m_site->CreateNewTableAsPlayer( newTableId, this, initialTime );
    if ( result != hoxRC_OK )
    {
        wxLogError("%s: Failed to create a new table.", FNAME);
        response << "2\r\n"  // code
                 << "Failed to create a new table." << "\r\n";
        goto exit_label;
    }

	/* Finally, return 'success'. */
	response << "0\r\n"       // error-code = SUCCESS
	         << "INFO: (NEW) The new table-Id = [" << newTableId << "]\r\n"
	         << newTableId << ";"
	         << "0" << ";"   // TODO: Hard-coded Group.
	         << "0" << ";"   // TODO: Hard-coded Type.
			 << itimes << ";"
			 << itimes << ";"  // RED-time = Initial-time
			 << itimes << ";"  // BLACK-time = Initial-time
             << this->GetName() << ";"   // RED-Id
             << this->GetScore() << ";"  // RED-Score
             << "" << ";"       // BLACK-Id
             << "" << ";"       // BLACK-Score
			 << "\r\n";
    result = hoxRC_OK;

exit_label:
    wxLogDebug("%s: END.", FNAME);
    return result;
}

void 
hoxPlayer::StartConnection()
{
    const char* FNAME = "hoxPlayer::StartConnection";

    wxCHECK_RET( m_connection, "The connection must have been set." );

    m_connection->Start();
}

void 
hoxPlayer::ShutdownConnection()
{
    const char* FNAME = "hoxPlayer::ShutdownConnection";

    wxLogDebug("%s: Player [%s] requesting the Connection to be shutdowned...", 
        FNAME, this->GetName().c_str());
    hoxRequest* request = new hoxRequest( hoxREQUEST_SHUTDOWN, NULL );
    this->AddRequestToConnection( request );
}

void 
hoxPlayer::AddRequestToConnection( hoxRequest* request )
{ 
    const char* FNAME = "hoxPlayer::AddRequestToConnection";

    if ( m_connection == NULL )
    {
        wxLogWarning("%s: No connection set. Deleting the request.", FNAME);
        delete request;
        return;
    }

    if ( m_connection->AddRequest( request ) )  // success?
	{
		wxCHECK_RET(request, "The request cannot be NULL.");
		if ( request->sender != NULL )
		{
			++m_nOutstandingRequests;
			wxLogDebug("%s: After incremented, the number of outstanding requests = [%d].",
				FNAME, m_nOutstandingRequests);
		}
	}

    if ( request->type == hoxREQUEST_SHUTDOWN )
    {
        wxLogDebug("%s: Request the Connection thread to be shutdowned...", FNAME);
        m_connection->Shutdown();
    }
}

void 
hoxPlayer::DecrementOutstandingRequests() 
{ 
    const char* FNAME = "hoxPlayer::DecrementOutstandingRequests";

    if ( m_nOutstandingRequests == 0 )
    {
        wxLogDebug("%s: * INFO * No more outstanding requests to be decremented.", FNAME);
        return;
    }

    --m_nOutstandingRequests; 
    wxLogDebug("%s: After decremented, the number of outstanding requests = [%d].",
        FNAME, m_nOutstandingRequests);

    if ( m_nOutstandingRequests == 0 )
    {
		if ( m_siteClosing && m_roles.empty() )
		{
			_PostSite_ShutdownReady();
		}
	}
}

void 
hoxPlayer::_PostSite_ShutdownReady()
{
	const char* FNAME  = "hoxPlayer::_PostSite_ShutdownReady";
	wxLogDebug("%s: ENTER.", FNAME);

    m_site->Handle_ShutdownReadyFromPlayer( this->GetName() );
}

/************************* END OF FILE ***************************************/
