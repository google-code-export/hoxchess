/***************************************************************************
 *  Copyright 2007, 2008, 2009 Huy Phan  <huyphan@playxiangqi.com>         *
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
// Name:            hoxSite.cpp
// Created:         11/24/2007
//
// Description:     The Site.
/////////////////////////////////////////////////////////////////////////////

#include "hoxSite.h"
#include "MyApp.h"
#include "hoxUtil.h"
#include "hoxMyPlayer.h"
#include "hoxChesscapePlayer.h"
#include "hoxAIPlayer.h"
#include "MyFrame.h"
#include "MyChild.h"
#include "hoxNetworkAPI.h"
#include "hoxTablesDialog.h"
#include "hoxBoard.h"
#include "hoxSitesUI.h"

#include "hoxAIPlayer.h"
#include "hoxTSITOPlayer.h"
#include "hoxFoliumPlayer.h"
#include "hoxXQWLightPlayer.h"

#include <wx/progdlg.h>

/**
 * A progress Dialog with Timer built-in.
 *
 * Restriction according to wxWidgets's Documentation:
 * ---------------------------------------------------
 *      Note: A timer can only be used from the main thread.
 */
class hoxProgressDialog : public wxProgressDialog
                        , public wxTimer
{
public:
    hoxProgressDialog( const wxString& title,
                       const wxString& message,
                       int             maximum = 100,
                       wxWindow*       parent = NULL,
                       int             style = wxPD_AUTO_HIDE | wxPD_APP_MODAL
                                             | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME
                                             | wxPD_REMAINING_TIME )
        : wxProgressDialog( title, message, maximum, parent, style )
        , wxTimer()
        , m_maximum( maximum )
        , m_timerValue( 0 )
    {
        this->Start( hoxTIME_ONE_SECOND_INTERVAL );
    }

    virtual void Stop()
    {
        this->wxTimer::Stop();
        this->Update( m_maximum ); // Make sure to close the dialog.
    }

    virtual void Notify()
    {
        bool bContinued = this->Update( ++m_timerValue );
        if ( ! bContinued || m_timerValue >= m_maximum )
        {
            this->Stop();
        }
    }

private:
    int  m_maximum;      // Progress-MAXIMUM value.
    int  m_timerValue;   // Timer's value.
};

// --------------------------------------------------------------------------
// hoxSite
// --------------------------------------------------------------------------


hoxSite::hoxSite( hoxSiteType             type,
                  const hoxServerAddress& address )
        : m_type( type )
        , m_address( address)
        , m_dlgProgress( NULL )
		, m_siteDisconnecting( false )
        , m_player( NULL )
        , m_playersUI( NULL )
{
    m_playersUI = wxGetApp().GetFrame()->CreateNewSitePlayersUI();
    m_playersUI->SetOwner( this );
}

hoxSite::~hoxSite()
{
    wxCHECK_RET(m_playersUI, "Unexpected NULL pointer to players-UI");
    m_playersUI->SetOwner( NULL );
    wxGetApp().GetFrame()->DeleteSitePlayersUI( m_playersUI );
    m_playersUI = NULL;
}

hoxResult 
hoxSite::CloseTable( hoxTable_SPtr pTable )
{
    hoxSiteManager::GetInstance()->OnTableUIRemoved( this, pTable );
    m_tableMgr.RemoveTable( pTable );
    return hoxRC_OK;
}

hoxPlayer*
hoxSite::GetPlayerById( const wxString& sPlayerId,
                        const int       nScore )
{
    hoxPlayer* player = this->FindPlayer( sPlayerId );
    if ( player == NULL )
    {
	    player = m_playerMgr.CreateDummyPlayer( sPlayerId, nScore );
    }
    wxASSERT( player != NULL );
    return player;
}

void
hoxSite::OnPlayerLoggedIn( const wxString&       sPlayerId,
                           const int             nPlayerScore,
                           const hoxPlayerStatus playerStatus /* = hoxPLAYER_STATUS_UNKNOWN */ )
{
    wxLogDebug("%s: Add: [%s (%d)]", __FUNCTION__, sPlayerId.c_str(), nPlayerScore);
    hoxPlayerInfo_SPtr pPlayerInfo( new hoxPlayerInfo() );
    pPlayerInfo->id = sPlayerId;
    pPlayerInfo->score = nPlayerScore;

    m_onlinePlayers[sPlayerId] = pPlayerInfo;
    m_playersUI->AddPlayer( sPlayerId, nPlayerScore, playerStatus );
}

void
hoxSite::OnPlayerLoggedOut( const wxString& sPlayerId )
{
    wxLogDebug("%s: Remove: [%s]", __FUNCTION__, sPlayerId.c_str());
    m_onlinePlayers.erase(sPlayerId);
    m_playersUI->RemovePlayer( sPlayerId );
}

void
hoxSite::UpdateScoreOfOnlinePlayer( const wxString& sPlayerId,
                                    const int       nPlayerScore )
{
    hoxPlayerInfoMap::iterator it = m_onlinePlayers.find( sPlayerId );
    if ( it != m_onlinePlayers.end() ) // found?
    {
        wxLogDebug("%s: Update Score of [%s]: [%d] -> [%d]", __FUNCTION__,
            sPlayerId.c_str(), it->second->score, nPlayerScore);
        it->second->score = nPlayerScore;
        m_playersUI->UpdateScore( sPlayerId, nPlayerScore );
    }
    else
    {
        wxLogDebug("%s: *WARN* Player [%s] not found.", __FUNCTION__, sPlayerId.c_str());
    }
}

void
hoxSite::UpdateStatusOfOnlinePlayer( const wxString&       sPlayerId,
                                     const hoxPlayerStatus playerStatus )
{
    hoxPlayerInfoMap::iterator it = m_onlinePlayers.find( sPlayerId );
    if ( it != m_onlinePlayers.end() ) // found?
    {
        wxLogDebug("%s: Update Status of [%s]: [%d] -> [%d]", __FUNCTION__,
            sPlayerId.c_str(), it->second->status, playerStatus);
        it->second->status = playerStatus;
        m_playersUI->UpdateStatus( sPlayerId, playerStatus );
    }
    else
    {
        wxLogDebug("%s: *WARN* Player [%s] not found.", __FUNCTION__, sPlayerId.c_str());
    }
}

int
hoxSite::GetScoreOfOnlinePlayer( const wxString& sPlayerId ) const
{
    hoxPlayerInfoMap::const_iterator it = m_onlinePlayers.find( sPlayerId );
    if ( it != m_onlinePlayers.end() ) // found?
    {
        return it->second->score;
    }
    return hoxSCORE_UNKNOWN;
}

unsigned int
hoxSite::GetBoardFeatureFlags() const
{
	unsigned int flags = hoxBoard::hoxBOARD_FEATURE_ALL;

	return flags;
}

void
hoxSite::ShowProgressDialog( bool bShow /* = true */ )
{
    if ( bShow )
    {
        if ( m_dlgProgress != NULL ) 
        {
            m_dlgProgress->Destroy();  // NOTE: ... see wxWidgets' documentation.
            m_dlgProgress = NULL;
        }

        m_dlgProgress = new hoxProgressDialog(
            _("Progress dialog"),
            _("Wait until connection is established or press [Cancel]"),
            30,  // maximum
            wxGetApp().GetFrame()  // parent
            );
    }
    else /* Hide */
    {
        if ( m_dlgProgress != NULL )
        {
            m_dlgProgress->Stop();
        }
    }
}

hoxTable_SPtr
hoxSite::CreateNewTableWithGUI( const hoxNetworkTableInfo& tableInfo,
                                const wxString&            sSavedFile /* = "" */ )
{
    hoxTable_SPtr pTable;
    const wxString tableId = tableInfo.id;

    /* Create a GUI Frame for the new Table. */
    MyChild* childFrame = wxGetApp().GetFrame()->CreateFrameForTable( tableId );

    /* Create a new table with the newly created Frame. */
    pTable = m_tableMgr.CreateTable( tableId, 
                                     this,
                                     tableInfo.gameType,
                                     sSavedFile );
	pTable->SetInitialTime( tableInfo.initialTime );
    pTable->SetBlackTime( tableInfo.blackTime );
    pTable->SetRedTime( tableInfo.redTime );
	
	wxLogDebug("%s: Creating a new Board...", __FUNCTION__);
    unsigned int boardFeatureFlags = this->GetBoardFeatureFlags();
    const wxString sPiecePath = wxGetApp().GetOption("/Board/Piece/path");
    const wxString sBgColor = wxGetApp().GetOption("/Board/Color/background");
    const wxString sFgColor = wxGetApp().GetOption("/Board/Color/foreground");
	hoxBoard* pBoard = new hoxBoard( childFrame, 
                                     sPiecePath,
		                             pTable->GetReferee(),
                                     pTable,
                                     m_player->GetId(),
                                     sBgColor,
                                     sFgColor,
        					         wxDefaultPosition,
							         childFrame->GetSize(),
                                     boardFeatureFlags );
    pTable->ViewBoard( pBoard );
    
    bool bSound = wxGetApp().GetOption("sound") == "1";
    pBoard->EnableSound( bSound );

    childFrame->SetTable( pTable );
    childFrame->Show( true );

    hoxSiteManager::GetInstance()->OnTableUICreated( this, pTable );

    return pTable;
}

// --------------------------------------------------------------------------
// hoxLocalSite
// --------------------------------------------------------------------------

hoxLocalPlayer* 
hoxLocalSite::CreateLocalPlayer( const wxString& playerName )
{
	wxCHECK_MSG(m_player == NULL, NULL, "The player has already been set.");

	m_player = m_playerMgr.CreateLocalPlayer( playerName );
	return m_player;
}

unsigned int 
hoxLocalSite::GetCurrentActionFlags() const
{
	unsigned int flags = 0;

    /* Only allow ONE Practice Table. */
    const hoxTableList& tables = this->GetTables();
    if ( tables.empty() )
    {
        flags |= hoxSITE_ACTION_PRACTICE;
        flags |= hoxSITE_ACTION_OPEN;
    }

	return flags;
}

void
hoxLocalSite::OnLocalRequest_PRACTICE( const wxString& sSavedFile /* = "" */ )
{
    wxCHECK_RET( m_player != NULL, "Player is NULL" );

    /* Generate new unique IDs for:
     *   (1) This PRACTICE-Table, and
     *   (2) The new AI Player.
     */
    const wxString sTableId = hoxUtil::GenerateRandomString("PRACTICE_");
    const wxString sAIId    = hoxUtil::GenerateRandomString("AI_");

    /* Set the default Table's attributes. */

    hoxNetworkTableInfo tableInfo;

    tableInfo.id = sTableId;
    tableInfo.gameType = hoxGAME_TYPE_PRACTICE;

    const hoxTimeInfo timeInfo( 1500, 300, 20 );
	tableInfo.initialTime = timeInfo;
    tableInfo.redTime     = tableInfo.initialTime;
    tableInfo.blackTime   = tableInfo.initialTime;

    /* Create an "empty" PRACTICE Table. */

    wxLogDebug("%s: Create a PRACTICE Table [%s]...", __FUNCTION__, sTableId.c_str());
    hoxTable_SPtr pTable = this->CreateNewTableWithGUI( tableInfo, sSavedFile );

    /* Assign Players to the Table.
     *
     * NOTE: Hard-coded player-roles:
     *     + LOCAL player - play RED
     *     + AI player    - play BLACK
     */
    
    hoxResult result;

    result = m_player->JoinTableAs( pTable, hoxCOLOR_RED );
    wxASSERT( result == hoxRC_OK );

    hoxAIPlayer* pAIPlayer =
        new hoxXQWLightPlayer( sAIId, hoxPLAYER_TYPE_AI, 1500 );
        //new hoxFoliumPlayer( sAIId, hoxPLAYER_TYPE_AI, 1500 );
        //new hoxTSITOPlayer( sAIId, hoxPLAYER_TYPE_AI, 1500 );
        //new hoxAIPlayer( sAIId, hoxPLAYER_TYPE_AI, 1500 );

    pAIPlayer->SetSavedFile( sSavedFile );
    pAIPlayer->Start();

    result = pAIPlayer->JoinTableAs( pTable, hoxCOLOR_BLACK );
    wxASSERT( result == hoxRC_OK );
}


// --------------------------------------------------------------------------
// hoxRemoteSite
// --------------------------------------------------------------------------

hoxRemoteSite::hoxRemoteSite(const hoxServerAddress& address,
                             hoxSiteType             type /*= hoxSITE_TYPE_REMOTE*/)
        : hoxSite( type, address )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

hoxRemoteSite::~hoxRemoteSite()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

hoxLocalPlayer* 
hoxRemoteSite::CreateLocalPlayer( const wxString& playerName )
{
	wxCHECK_MSG(m_player == NULL, NULL, "The player has already been set.");

	m_player = m_playerMgr.CreateMyPlayer( playerName );
	return m_player;
}

void 
hoxRemoteSite::OnResponse_LOGIN( const hoxResponse_APtr& response )
{
    this->ShowProgressDialog( false );

    if ( response->code != hoxRC_OK )
    {
        wxString sMessage;
        sMessage.Printf("The response's code for [%s] is ERROR [%s: %s].", 
            hoxUtil::RequestTypeToString(response->type).c_str(), 
            hoxUtil::ResultToStr(response->code),
            response->content.c_str());
        ::wxMessageBox( sMessage, _("Login error"), wxOK | wxICON_EXCLAMATION );
    }
}

void
hoxRemoteSite::OnPlayersUIEvent( hoxPlayersUI::EventType eventType,
                                 const wxString&         sPlayerId )
{
    if ( m_player == NULL ) return;

    switch ( eventType )
    {
        case hoxPlayersUI::EVENT_TYPE_INFO:
        {
            m_player->QueryPlayerInfo( sPlayerId );
            break;
        }
        case hoxPlayersUI::EVENT_TYPE_INVITE:
        {
            m_player->InvitePlayer( sPlayerId );
            break;
        }
        default:
            wxLogDebug("%s: Unsupported eventType [%d].", __FUNCTION__, eventType);
    }
}

hoxResult
hoxRemoteSite::OnPlayerJoined( const wxString&  tableId,
                               const wxString&  playerId,
                               const int        playerScore,
				 			   const hoxColor   requestColor)
{
    hoxTable_SPtr pTable;
    hoxPlayer*    player = NULL;

	/* Lookup the Table.
     * Make sure that it must be already created.
     */
	pTable = this->FindTable( tableId );
	if ( pTable.get() == NULL )
	{
        wxLogDebug("%s: *** WARN *** Table [%s] NOT exist.", __FUNCTION__, tableId.c_str());
		return hoxRC_ERR;
	}

	/* Lookup the Player (create a new "dummy" player if necessary).
     */
    player = this->GetPlayerById( playerId, playerScore );
    wxASSERT( player != NULL );

    /* Attempt to join the table with the requested color.
     */
    return player->JoinTableAs( pTable, requestColor );
}

hoxResult 
hoxRemoteSite::JoinLocalPlayerToTable( const hoxNetworkTableInfo& tableInfo )
{
    hoxResult      result;
    hoxTable_SPtr  pTable;
    const wxString tableId = tableInfo.id;
    const wxString redId   = tableInfo.redId;
    const wxString blackId = tableInfo.blackId;
    hoxPlayer*     player  = NULL;  // Just a player holder.

	/* Create a table if necessary. */

    pTable = this->FindTable( tableId );
	if ( pTable.get() == NULL )
	{
        wxLogDebug("%s: Create a new Table [%s].", __FUNCTION__, tableId.c_str());
        pTable = this->CreateNewTableWithGUI( tableInfo );
	}

	/* Determine which color (or role) my player will have. */
	
	hoxColor myColor = hoxCOLOR_UNKNOWN;

	if      ( redId == m_player->GetId() )   myColor = hoxCOLOR_RED;
	else if ( blackId == m_player->GetId() ) myColor = hoxCOLOR_BLACK;
    else 	                                   myColor = hoxCOLOR_NONE;

	/****************************
	 * Assign players to table.
     ****************************/

    result = m_player->JoinTableAs( pTable, myColor );
    wxCHECK( result == hoxRC_OK, hoxRC_ERR  );

	/* Create additional "dummy" player(s) if required.
     */

    if ( !redId.empty() && pTable->GetRedPlayer() == NULL )
    {
        player = this->GetPlayerById( redId, ::atoi(tableInfo.redScore) );
        wxASSERT( player != NULL );
        result = player->JoinTableAs( pTable, hoxCOLOR_RED );
    }
    if ( !blackId.empty() && pTable->GetBlackPlayer() == NULL )
    {
        player = this->GetPlayerById( blackId, ::atoi(tableInfo.blackScore) );
	    wxASSERT( player != NULL );
        result = player->JoinTableAs( pTable, hoxCOLOR_BLACK );
    }

	return result;
}

hoxResult
hoxRemoteSite::DisplayListOfTables( const hoxNetworkTableInfoList& tableList )
{
    /* Show tables. */
    MyFrame* frame = wxGetApp().GetFrame();
	const unsigned int actionFlags = this->GetCurrentActionFlags();
    
    hoxTablesDialog tablesDlg( frame, wxID_ANY, "Tables", tableList, actionFlags );
    tablesDlg.ShowModal();
    
    hoxTablesDialog::CommandId selectedCommand = tablesDlg.GetSelectedCommand();
    const wxString selectedId = tablesDlg.GetSelectedId();

    /* Find out which command the use wants to execute... */

    switch( selectedCommand )
    {
        case hoxTablesDialog::COMMAND_ID_JOIN:
        {
            this->OnLocalRequest_JOIN( selectedId );
            break;
        }

        case hoxTablesDialog::COMMAND_ID_NEW:
        {
            this->OnLocalRequest_NEW();
            break;
        }

		case hoxTablesDialog::COMMAND_ID_REFRESH:
        {
            wxLogDebug("%s: Get the latest list of tables...", __FUNCTION__);
            if ( hoxRC_OK != this->QueryForNetworkTables() )
            {
                wxLogError("%s: Failed to get the list of tables.", __FUNCTION__);
            }
            break;
        }

        default:
            wxLogDebug("%s: No command is selected. Fine.", __FUNCTION__);
            break;
    }

    return hoxRC_OK;
}

hoxResult 
hoxRemoteSite::Connect()
{
    if ( this->IsConnected() )
    {
        wxLogDebug("%s: This site has been connected. END.", __FUNCTION__);
        return hoxRC_OK;
    }

    /* Start connecting... */

    this->ShowProgressDialog( true );

    return m_player->ConnectToServer();
}

hoxResult 
hoxRemoteSite::Disconnect()
{
	hoxResult result = hoxRC_OK;
    wxLogDebug("%s: ENTER.", __FUNCTION__);

	if ( m_siteDisconnecting )
	{
		wxLogDebug("%s: Site [%s] is already being disconnected. END.",
            __FUNCTION__, this->GetName().c_str());
		return hoxRC_OK;
	}
	m_siteDisconnecting = true;

	if ( m_player != NULL )
	{
		result = m_player->DisconnectFromServer();
	}

    return result;
}

hoxResult 
hoxRemoteSite::QueryForNetworkTables()
{
    if ( ! this->IsConnected() )
    {
        wxLogDebug("%s: This site has NOT been connected.", __FUNCTION__);
        return hoxRC_ERR;
    }

    return m_player->QueryForNetworkTables();
}

bool 
hoxRemoteSite::IsConnected() const
{
    return (    m_player != NULL
		     && m_player->GetConnection() != NULL
             && m_player->GetConnection()->IsConnected() );
}

void 
hoxRemoteSite::OnShutdownReadyFromLocalPlayer()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

	if ( m_player == NULL )
	{
		wxLogDebug("%s: Player is NULL. Shutdown must have already been processed.", __FUNCTION__);
		return;
	}

    /* Close all the Frames of Tables. */
    const hoxTableList& tables = this->GetTables();
    while ( !tables.empty() )
    {
        const wxString sTableId = tables.front()->GetId();
        wxLogDebug("%s: Delete Frame of Table [%s]...", __FUNCTION__, sTableId.c_str());
        wxGetApp().GetFrame()->DeleteFrameOfTable( sTableId );
            // NOTE: The call above already delete the Table.
    }

	/* Must set the local player to NULL immediately to handle "re-entrance"
	 * because the DISCONNECT call below can go to sleep...
	 */
	hoxLocalPlayer* localPlayer = m_player;
	m_player = NULL;

	localPlayer->ResetConnection();

    /* Inform the App. */
    wxCommandEvent event( hoxEVT_APP_SITE_CLOSE_READY );
    event.SetEventObject( this );
    wxPostEvent( &(wxGetApp()), event );
}

unsigned int 
hoxRemoteSite::GetCurrentActionFlags() const
{
	unsigned int flags = 0;

    if ( ! this->IsConnected() )
	{
		flags |= hoxSITE_ACTION_CONNECT;
	}
	else
    {
		flags |= hoxSITE_ACTION_DISCONNECT;
		flags |= hoxSITE_ACTION_LIST;
		flags |= hoxSITE_ACTION_NEW;
		flags |= hoxSITE_ACTION_JOIN;
    }

	return flags;
}

void
hoxRemoteSite::OnLocalRequest_JOIN( const wxString& sTableId )
{
    wxCHECK_RET( m_player != NULL, "Player is NULL" );

    wxLogDebug("%s: Ask the server to allow me to JOIN table = [%s]",
        __FUNCTION__, sTableId.c_str());

    if ( hoxRC_OK != m_player->JoinNetworkTable( sTableId ) )
    {
        wxLogError("%s: Failed to JOIN a network table [%s].", __FUNCTION__, sTableId.c_str());
    }
}

void
hoxRemoteSite::OnLocalRequest_NEW()
{
    wxCHECK_RET( m_player != NULL, "Player is NULL" );

    wxLogDebug("%s: Ask the server to open a new table.", __FUNCTION__);

    if ( hoxRC_OK != m_player->OpenNewNetworkTable() )
    {
        wxLogError("%s: Failed to open a NEW network table.", __FUNCTION__);
    }
}

void
hoxRemoteSite::OnLocalRequest_PRACTICE( const wxString& sSavedFile /* = "" */ )
{
    wxCHECK_RET( m_player != NULL, "Player is NULL" );

    wxLogDebug("%s: Create a new PRACTICE Table...", __FUNCTION__);

    /* FIXME: Do nothing for now. */
}

// --------------------------------------------------------------------------
// hoxChesscapeSite
// --------------------------------------------------------------------------

hoxChesscapeSite::hoxChesscapeSite( const hoxServerAddress& address )
        : hoxRemoteSite( address, hoxSITE_TYPE_CHESSCAPE )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

hoxChesscapeSite::~hoxChesscapeSite()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

hoxLocalPlayer* 
hoxChesscapeSite::CreateLocalPlayer( const wxString& playerName )
{
	wxCHECK_MSG(m_player == NULL, NULL, "The player has already been set.");

	m_player = m_playerMgr.CreateChesscapePlayer( playerName );
	return m_player;
}

unsigned int 
hoxChesscapeSite::GetCurrentActionFlags() const
{
	unsigned int flags = 0;

	/* Get flags from the parent-class. */
	flags = this->hoxRemoteSite::GetCurrentActionFlags();

    if ( this->IsConnected() )
    {
		/* Chesscape can only support 1-table-at-a-time.
		 * If the Player is actively playing (RED or BLACK),
         * then disable NEW and JOIN actions.
		 */
        wxString sTableId_NOT_USED;
        const hoxColor myRole = m_player->GetFrontRole( sTableId_NOT_USED );
        if ( myRole == hoxCOLOR_RED || myRole == hoxCOLOR_BLACK ) // playing?
	    {
		    flags &= ~hoxSITE_ACTION_NEW;
		    flags &= ~hoxSITE_ACTION_JOIN;
	    }
    }

	return flags;
}

unsigned int
hoxChesscapeSite::GetBoardFeatureFlags() const
{
	unsigned int flags = hoxBoard::hoxBOARD_FEATURE_ALL;

    /* Disable the RESET feature. */
    flags &= ~hoxBoard::hoxBOARD_FEATURE_RESET;

	return flags;
}

void
hoxChesscapeSite::OnLocalRequest_JOIN( const wxString& sTableId )
{
	/* Chesscape can only support 1-table-at-a-time.
	 * Thus, we need to close the Table that is observed (if any)
     * before joining another Table.
	 */

    wxString       sObservedTableId;
    const hoxColor myRole = m_player->GetFrontRole( sObservedTableId );

    if ( myRole == hoxCOLOR_RED || myRole == hoxCOLOR_BLACK ) // playing?
    {
        wxLogWarning("Action not allowed: Cannot join a Table while playing at another.");
        return;   // *** Exit immediately!
    }

    if ( myRole == hoxCOLOR_NONE )  // observing?
    {
        wxLogDebug("%s: Close the observed Table [%s] before joining another...",
            __FUNCTION__, sObservedTableId.c_str());
        wxGetApp().GetFrame()->DeleteFrameOfTable( sObservedTableId );
            /* NOTE: The call above already delete the Table.
             *       It also triggers the TABLE-CLOSING process.
             */
    }

    this->hoxRemoteSite::OnLocalRequest_JOIN( sTableId );
}

void
hoxChesscapeSite::OnLocalRequest_NEW()
{
	/* Chesscape can only support 1-table-at-a-time.
	 * Thus, we need to close the Table that is observed (if any)
     * before asking to create a new Table.
	 */

    wxString       sObservedTableId;
    const hoxColor myRole = m_player->GetFrontRole( sObservedTableId );

    if ( myRole == hoxCOLOR_RED || myRole == hoxCOLOR_BLACK ) // playing?
    {
        wxLogWarning("Action not allowed: Cannot create a new Table while playing at another.");
        return;   // *** Exit immediately!
    }

    if ( myRole == hoxCOLOR_NONE )  // observing?
    {
        wxLogDebug("%s: Close the observed Table [%s] before creating a new one...",
            __FUNCTION__, sObservedTableId.c_str());
        wxGetApp().GetFrame()->DeleteFrameOfTable( sObservedTableId );
            /* NOTE: The call above already delete the Table.
             *       It also triggers the TABLE-CLOSING process.
             */
    }

    this->hoxRemoteSite::OnLocalRequest_NEW();
}

///////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------
// hoxSiteManager
// --------------------------------------------------------------------------

/* Define (initialize) the single instance */
hoxSiteManager* 
hoxSiteManager::m_instance = NULL;

/* static */
hoxSiteManager* 
hoxSiteManager::GetInstance()
{
	if ( m_instance == NULL )
		m_instance = new hoxSiteManager();

	return m_instance;
}

/* private */
hoxSiteManager::hoxSiteManager()
        : m_sitesUI( NULL )
{
}

hoxSite* 
hoxSiteManager::CreateSite( hoxSiteType             siteType,
						    const hoxServerAddress& address,
				            const wxString&         userName,
						    const wxString&         password )
{
	hoxSite*           site = NULL;
    hoxLocalPlayer*    localPlayer = NULL;

	switch ( siteType )
	{
	case hoxSITE_TYPE_LOCAL:
	{
		site = new hoxLocalSite( address );
		break;
	}
	case hoxSITE_TYPE_REMOTE:
	{
		site = new hoxRemoteSite( address );
		break;
	}
	case hoxSITE_TYPE_CHESSCAPE:
	{
		site = new hoxChesscapeSite( address );
		break;
	}
	default:
        wxLogError("%s: Unsupported Site-Type [%d].", __FUNCTION__, siteType);
		return NULL;   // *** Exit with error immediately.
	}

    localPlayer = site->CreateLocalPlayer( userName );
	localPlayer->SetPassword( password );
    localPlayer->SetSite( site );
    localPlayer->Start();

    m_sites.push_back( site );
    if ( m_sitesUI != NULL ) m_sitesUI->AddSite( site );
	return site;
}

void
hoxSiteManager::CreateLocalSite()
{
    const hoxServerAddress localAddress("127.0.0.1", 0);
    const wxString         localUserName = "LOCAL_USER";
    const wxString         localPassword;
    this->CreateSite( hoxSITE_TYPE_LOCAL, 
            		  localAddress,
					  localUserName,
					  localPassword );
}

hoxSite*
hoxSiteManager::FindSite( const hoxServerAddress& address ) const
{
    for ( hoxSiteList::const_iterator it = m_sites.begin();
                                      it != m_sites.end(); ++it )
    {
        if ( (*it)->GetAddress() == address )
        {
            return (*it);
        }
    }

	return NULL;
}

void
hoxSiteManager::DeleteSite( hoxSite* site )
{
    wxCHECK_RET( site != NULL, "The Site must not be NULL." );
    
    wxLogDebug("%s: Deleting site [%s]...", __FUNCTION__, site->GetName().c_str());

    if ( m_sitesUI != NULL ) m_sitesUI->RemoveSite( site );
    delete site;
    m_sites.remove( site );
}

void
hoxSiteManager::DeleteLocalSite()
{
    hoxSite* localSite = NULL;

    for ( hoxSiteList::iterator it = m_sites.begin();
                                it != m_sites.end(); ++it )
    {
		if ( (*it)->GetType() == hoxSITE_TYPE_LOCAL )
        {
            localSite = (*it);
            break;
        }
    }

    if ( localSite != NULL )
    {
        this->DeleteSite( localSite );
    }
}

void 
hoxSiteManager::Close()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    for ( hoxSiteList::iterator it = m_sites.begin();
                                it != m_sites.end(); ++it )
    {
		(*it)->Disconnect();
    }
}

void
hoxSiteManager::OnTableUICreated( hoxSite*      site,
                                  hoxTable_SPtr pTable )
{
    if ( m_sitesUI != NULL ) m_sitesUI->AddTableToSite( site, pTable );
}

void
hoxSiteManager::OnTableUIRemoved( hoxSite*      site,
                                  hoxTable_SPtr pTable )
{
    if ( m_sitesUI != NULL ) m_sitesUI->RemoveTableFromSite( site, pTable );
}

/************************* END OF FILE ***************************************/
