/***************************************************************************
 *  Copyright 2007 Huy Phan  <huyphan@playxiangqi.com>                     *
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
#include "hoxServer.h"
#include "hoxUtility.h"
#include "hoxSocketConnection.h"
#include "hoxHttpConnection.h"
#include "MyFrame.h"
#include "MyChild.h"
#include "hoxNetworkAPI.h"
#include "hoxTablesDialog.h"

DEFINE_EVENT_TYPE(hoxEVT_SITE_PLAYER_SHUTDOWN_READY)

BEGIN_EVENT_TABLE(hoxResponseHandler, wxEvtHandler)
	EVT_COMMAND(wxID_ANY, hoxEVT_SITE_PLAYER_SHUTDOWN_READY, hoxResponseHandler::OnShutdownReady_FromPlayer)
    EVT_COMMAND(wxID_ANY, hoxEVT_CONNECTION_RESPONSE, hoxResponseHandler::OnConnectionResponse)
END_EVENT_TABLE()


void 
hoxResponseHandler::OnShutdownReady_FromPlayer( wxCommandEvent& event )
{
    const char* FNAME = "hoxResponseHandler::OnShutdownReady_FromPlayer";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxPlayer* player = wx_reinterpret_cast(hoxPlayer*, event.GetEventObject());
    wxCHECK_RET(player, "Player cannot be NULL.");

    m_site->Handle_ShutdownReadyFromPlayer( player );
}

void 
hoxResponseHandler::OnConnectionResponse( wxCommandEvent& event )
{
    const char* FNAME = "hoxResponseHandler::OnConnectionResponse";

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    if ( m_site->GetType() != hoxSITE_TYPE_LOCAL ) // remote site?
    {
        hoxRemoteSite* remoteSite = (hoxRemoteSite*) m_site;
        remoteSite->Handle_ConnectionResponse( response.release() );
    }
}


// --------------------------------------------------------------------------
// hoxSite
// --------------------------------------------------------------------------


hoxSite::hoxSite( hoxSiteType             type,
                  const hoxServerAddress& address )
        : m_type( type )
        , m_address( address)
        , m_responseHandler( NULL )
        , m_dlgProgress( NULL )
		, m_siteClosing( false )
{
    m_playerMgr.SetSite( this );
    m_tableMgr.SetSite( this );

    m_responseHandler = new hoxResponseHandler( this );
}

hoxSite::~hoxSite()
{
    delete m_responseHandler;
}

void 
hoxSite::DeletePlayer( hoxPlayer* player )
{ 
	m_playerMgr.DeletePlayer( player ); 
}

hoxResult 
hoxSite::CloseTable(hoxTable* table)
{
    m_tableMgr.RemoveTable( table );
    return hoxRESULT_OK;
}

void 
hoxSite::Handle_ShutdownReadyFromPlayer( hoxPlayer* player )
{
    const char* FNAME = "hoxSite::Handle_ShutdownReadyFromPlayer";
    
	wxLogDebug("%s: ENTER. Do nothing. END.", FNAME);
}

// --------------------------------------------------------------------------
// hoxLocalSite
// --------------------------------------------------------------------------


hoxLocalSite::hoxLocalSite(const hoxServerAddress& address)
        : hoxSite( hoxSITE_TYPE_LOCAL, address )
        , m_server( NULL )
        , m_isOpened( false )
		, m_nNextTableId( 0 )
{
}

hoxLocalSite::~hoxLocalSite()
{
}

const wxString 
hoxLocalSite::GetName() const
{
    wxString name;

    name.Printf("Local Site (%d)", m_address.port);
    return name;
}

hoxResult 
hoxLocalSite::OpenServer()
{
    const char* FNAME = "hoxLocalSite::OpenServer";
    wxLogDebug("%s: ENTER.", FNAME);

    if ( m_server != NULL )
    {
        wxLogDebug("%s: The server have been created. END.", FNAME);
        return hoxRESULT_OK;
    }

    m_server = new hoxServer( this );

	if ( hoxRESULT_OK !=  m_server->StartServer( m_address.port ) )
	{
        wxLogError("%s: Failed to start socker-server thread.", FNAME);
        return hoxRESULT_ERR;
	}

    m_isOpened = true;

    return hoxRESULT_OK;
}

hoxResult
hoxLocalSite::Close()
{
    const char* FNAME = "hoxLocalSite::Close";

	if ( m_siteClosing )
	{
		wxLogDebug("%s: Site [%s] is already being closed. END.", FNAME, this->GetName().c_str());
		return hoxRESULT_OK;
	}
	m_siteClosing = true;

	m_playerMgr.OnSiteClosing();

	if ( m_playerMgr.GetNumberOfPlayers() == 0 )	
	{
		_DoCloseSite();
	}

    return hoxRESULT_OK;
}

hoxResult 
hoxLocalSite::CreateNewTableAsPlayer( wxString&  newTableId, 
                                      hoxPlayer* player )
{
    const char* FNAME = "hoxLocalSite::CreateNewTableAsPlayer";
    hoxTable* newTable = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

	/* Generate a new Table-Id */
	newTableId = _GenerateTableId();

    /* Create a new table without a frame. */
    newTable = m_tableMgr.CreateTable( newTableId );

    /* Add the specified player to the table. */
    hoxResult result = player->JoinTable( newTable );
    wxASSERT( result == hoxRESULT_OK  );
    wxASSERT_MSG( player->HasRole( 
                            hoxRole(newTable->GetId(), hoxPIECE_COLOR_RED) ),
                  _("Player must play RED"));

    /* Update UI. */
    wxGetApp().GetFrame()->UpdateSiteTreeUI();

    return hoxRESULT_OK;
}

void 
hoxLocalSite::Handle_ShutdownReadyFromPlayer( hoxPlayer* player )
{
    const char* FNAME = "hoxLocalSite::Handle_ShutdownReadyFromPlayer";
    wxLogDebug("%s: ENTER. (%s)", FNAME, player->GetName().c_str());

	this->DeletePlayer( player );

    /* Perform the actual closing. */
	if ( m_playerMgr.GetNumberOfPlayers() == 0 )	
	{
		_DoCloseSite();
	}
}

void 
hoxLocalSite::_DoCloseSite()
{
	const char* FNAME = "hoxLocalSite::_DoCloseSite";

	wxLogDebug("%s: ENTER.", FNAME);

	if ( m_server != NULL )
	{
		m_server->CloseServer();
		delete m_server;
		m_server = NULL;
	}

	m_isOpened = false;

	wxCommandEvent event( hoxEVT_APP_SITE_CLOSE_READY );
	event.SetEventObject( this );
	wxPostEvent( &(wxGetApp()), event );
}

const wxString 
hoxLocalSite::_GenerateTableId()
{
	++m_nNextTableId;
	return wxString::Format("%d", m_nNextTableId);
}


// --------------------------------------------------------------------------
// hoxRemoteSite
// --------------------------------------------------------------------------

hoxRemoteSite::hoxRemoteSite(const hoxServerAddress& address,
                             hoxSiteType             type /*= hoxSITE_TYPE_REMOTE*/)
        : hoxSite( type, address )
        , m_player( NULL )
{
    const char* FNAME = "hoxRemoteSite::hoxRemoteSite";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxRemoteSite::~hoxRemoteSite()
{
    const char* FNAME = "hoxRemoteSite::~hoxRemoteSite";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxLocalPlayer* 
hoxRemoteSite::CreateLocalPlayer( const wxString& playerName )
{
	wxCHECK_MSG(m_player == NULL, NULL, "The player has already been set.");

	m_player = m_playerMgr.CreateMyPlayer( playerName );
	return m_player;
}

void 
hoxRemoteSite::Handle_ConnectionResponse( hoxResponse* pResponse )
{
    const char* FNAME = "hoxRemoteSite::Handle_ConnectionResponse";

    wxLogDebug("%s: ENTER.", FNAME);

    const std::auto_ptr<hoxResponse> response( pResponse ); // take care memory leak!

    if ( m_dlgProgress != NULL )
    {
        bool wasCanceled = !m_dlgProgress->Pulse();
        m_dlgProgress->Update(100);  // make sure to close the dialog.
        if ( wasCanceled )
        {
            wxLogDebug("%s: Connection has been canceled.", FNAME);
            return;
        }
    }

    if ( response->code != hoxRESULT_OK )
    {
        wxLogDebug("%s: The response's code is ERROR. END.", FNAME);
        return;
    }

    switch ( response->type )
    {
        case hoxREQUEST_TYPE_CONNECT:
            this->OnResponse_Connect( response->content );
            break;

        case hoxREQUEST_TYPE_NEW:
            this->OnResponse_New( response->content );
            break;

        case hoxREQUEST_TYPE_LIST:
            this->OnResponse_List( response->content );
            break;

        case hoxREQUEST_TYPE_JOIN:
            this->OnResponse_Join( response->content );
            break;

        default:
            wxLogError("%s: Unknown type [%s].", 
                FNAME, hoxUtility::RequestTypeToString(response->type).c_str() );
            break;
    }
}

void 
hoxRemoteSite::OnResponse_Connect( const wxString& responseStr )
{
    const char* FNAME = "hoxRemoteSite::OnResponse_Connect";
    int        returnCode = -1;
    wxString   returnMsg;
    hoxResult  result;

    wxLogDebug("%s: Parsing SEND-CONNECT's response...", FNAME);

    result = hoxNetworkAPI::ParseSimpleResponse( responseStr,
                                                 returnCode,
                                                 returnMsg );
    if ( result != hoxRESULT_OK || returnCode != 0 )
    {
        wxLogError("%s: Failed to parse CONNECT's response. [%d] [%s]", 
            FNAME, returnCode, returnMsg.c_str());
        return;
    }

	//wxGetApp().GetFrame()->UpdateSiteTreeUI();
}

void 
hoxRemoteSite::OnResponse_New( const wxString& responseStr )
{
    const char* FNAME = "hoxRemoteSite::OnResponse_New";
    wxString newTableId;
    hoxResult result;
    
    result = hoxNetworkAPI::ParseNewNetworkTable( responseStr,
                                                  newTableId );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse for NEW's response.", FNAME);
        return;
    }

    hoxTable* newTable = NULL;

    /* Create a GUI Frame for the new Table. */
    MyFrame* frame = wxGetApp().GetFrame();
    MyChild* childFrame = frame->CreateFrameForTable( newTableId );

    /* Create a new table with newly created Frame. */
    newTable = m_tableMgr.CreateTable( newTableId, 
                                       childFrame );
    childFrame->SetTable( newTable );
    childFrame->Show( true );

    // Add the HOST player to the table.
    result = m_player->JoinTable( newTable );
    wxASSERT( result == hoxRESULT_OK  );
    wxASSERT_MSG( m_player->HasRole( 
                        hoxRole(newTable->GetId(), hoxPIECE_COLOR_RED) ),
                  _("Player must play RED"));

    // Update UI.
    frame->UpdateSiteTreeUI();
}

void 
hoxRemoteSite::OnResponse_List( const wxString& responseStr )
{
    const char* FNAME = "hoxRemoteSite::OnResponse_List";
    hoxNetworkTableInfoList tableList;
    hoxResult               result;

    wxLogDebug("%s: ENTER.", FNAME);

    result = hoxNetworkAPI::ParseNetworkTables( responseStr,
                                                tableList );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse LIST's response [%s].", 
            FNAME, responseStr.c_str());
        return;
    }

    /* Show tables. */
    MyFrame* frame = wxGetApp().GetFrame();
    hoxTablesDialog tablesDlg( frame, wxID_ANY, "Tables", tableList);
    tablesDlg.ShowModal();
    hoxTablesDialog::CommandId selectedCommand = tablesDlg.GetSelectedCommand();
    wxString selectedId = tablesDlg.GetSelectedId();

    /* Find out which command the use wants to execute... */

    switch( selectedCommand )
    {
        case hoxTablesDialog::COMMAND_ID_JOIN:
        {
            wxLogDebug("%s: Ask the server to allow me to JOIN table = [%s]", FNAME, selectedId.c_str());
            hoxNetworkTableInfo tableInfo;
            result = m_player->JoinNetworkTable( selectedId, m_responseHandler );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to JOIN a network table [%s].", FNAME, selectedId.c_str());
            }
            break;
        }

        case hoxTablesDialog::COMMAND_ID_NEW:
        {
            wxLogDebug("%s: Ask the server to open a new table.", FNAME);
            wxString newTableId;
            result = m_player->OpenNewNetworkTable( m_responseHandler );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to open a NEW network table.", FNAME);
            }
            break;
        }

        default:
            wxLogDebug("%s: No command is selected. Fine.", FNAME);
            break;
    }

    hoxUtility::FreeNetworkTableInfoList( tableList );
}

void 
hoxRemoteSite::OnResponse_Join( const wxString& responseStr )
{
    const char* FNAME = "hoxRemoteSite::OnResponse_Join";
    hoxNetworkTableInfo tableInfo;
    hoxResult result;

    wxLogDebug("%s: ENTER.", FNAME);

    result = hoxNetworkAPI::ParseJoinNetworkTable( responseStr,
                                                   tableInfo );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse JOIN's response [%s].", 
            FNAME, responseStr.c_str());
        return;
    }
    else
    {
        wxLogDebug("Successfully joined the network table [%s].", 
            tableInfo.id.c_str());
        this->JoinExistingTable( tableInfo );
    }
}

const wxString 
hoxRemoteSite::GetName() const
{
    wxString name;

	name.Printf("%s:%d", m_address.name.c_str(), m_address.port);
	//if ( ! this->IsConnected() )
	//{
	//	name += " (disconnected)";		
	//}

    return name;
}

hoxResult 
hoxRemoteSite::Connect()
{
    const char* FNAME = "hoxRemoteSite::Connect";
    hoxResult result;
    MyFrame* frame = wxGetApp().GetFrame();

    if ( this->IsConnected() )
    {
        wxLogDebug("%s: This site has been connected. END.", FNAME);
        return hoxRESULT_OK;
    }

    /* Start connecting... */

    if ( m_dlgProgress != NULL ) 
    {
        m_dlgProgress->Destroy();  // NOTE: ... see wxWidgets' documentation.
        m_dlgProgress = NULL;
    }

    m_dlgProgress = new wxProgressDialog(
        "Progress dialog",
        "Wait until connnection is established or press [Cancel]",
        100,
        frame,  // parent
        wxPD_AUTO_HIDE | wxPD_CAN_ABORT
        );
    m_dlgProgress->SetSize( wxSize(500, 150) );
    m_dlgProgress->Pulse();

    result = m_player->ConnectToNetworkServer( m_responseHandler );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to connect to server.", FNAME);
        return hoxRESULT_ERR;
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxRemoteSite::Close()
{
    const char* FNAME = "hoxRemoteSite::Close";
    wxLogDebug("%s: ENTER.", FNAME);

	if ( m_siteClosing )
	{
		wxLogDebug("%s: Site [%s] is already being closed. END.", FNAME, this->GetName().c_str());
		return hoxRESULT_OK;
	}
	m_siteClosing = true;

	if ( m_player != NULL )
	{
        wxCommandEvent event( hoxEVT_PLAYER_SITE_CLOSING );
        event.SetString( "The site is being closed" );
        event.SetEventObject( &(wxGetApp()) );
        wxPostEvent( m_player , event );
	}

    return hoxRESULT_OK;
}

hoxResult 
hoxRemoteSite::QueryForNetworkTables()
{
    const char* FNAME = "hoxRemoteSite::QueryForNetworkTables";
    hoxResult result;

    if ( ! this->IsConnected() )
    {
        wxLogDebug("%s: This site has NOT been connected.", FNAME);
        return hoxRESULT_ERR;
    }

    result = m_player->QueryForNetworkTables( m_responseHandler );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to query for tables.", FNAME);
        return hoxRESULT_ERR;
    }

    return hoxRESULT_OK;
}

bool 
hoxRemoteSite::IsConnected() const
{
    return (    m_player != NULL
		     && m_player->GetConnection() != NULL
             && m_player->GetConnection()->IsConnected() );
}

hoxResult 
hoxRemoteSite::CreateNewTable( wxString& newTableId )
{
    const char* FNAME = "hoxRemoteSite::CreateNewTable";
    hoxResult result;

    result = m_player->OpenNewNetworkTable( m_responseHandler );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to open a new Table on the remote server.", FNAME);
        return hoxRESULT_ERR;
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxRemoteSite::JoinExistingTable( const hoxNetworkTableInfo& tableInfo )
{
    const char* FNAME = "hoxRemoteSite::JoinExistingTable";
    hoxResult result;

    wxLogDebug("%s: ENTER.", FNAME);

    /*******************************************************
     * Check to see which side (RED or BLACK) we will play
     * and who the other player, if any, is.
     *******************************************************/

    bool     playRed = false;   // Do I play RED?
    wxString otherPlayerId;     // Who is the other player?

    if ( tableInfo.redId == m_player->GetName() )
    {
        playRed = true;
        otherPlayerId = tableInfo.blackId;
    }
    else if ( tableInfo.blackId == m_player->GetName() )
    {
        playRed = false;
        otherPlayerId = tableInfo.redId;
    }
    else
    {
        wxLogError("%s: I should have secured a seat in table [%s].", FNAME, tableInfo.id.c_str());
        return hoxRESULT_ERR;
    }

    /***********************/
    /* Create a new table. */
    /***********************/

    wxLogDebug("%s: Creating a new table JOINING an existing network table...", FNAME);

    hoxTable* table = NULL;
    wxString  tableId = tableInfo.id;

    /* Create a GUI Frame for the new Table. */
    MyFrame* frame = wxGetApp().GetFrame();
    MyChild* childFrame = frame->CreateFrameForTable( tableId );

    /* Create a new table with newly created Frame. */
    table = m_tableMgr.CreateTable( tableId, 
                                    childFrame );
    childFrame->SetTable( table );
    childFrame->Show( true );

    /***********************/
    /* Setup players       */
    /***********************/

    // The other player will be a DUMMY player.

    hoxPlayer* red_player = NULL;
    hoxPlayer* black_player = NULL;
    hoxPlayer* other_player = NULL;

    if ( ! otherPlayerId.empty() )
    {
        other_player = m_playerMgr.CreateDummyPlayer( otherPlayerId );
    }

    if ( playRed )  // Do I play RED?
    {
        red_player = m_player;
        black_player = other_player;
    }
    else
    {
        black_player = m_player;
        red_player = other_player;
    }

    /* Join the players at the table.
     */

    if ( red_player != NULL )
    {
        result = red_player->JoinTable( table );
        wxASSERT( result == hoxRESULT_OK  );
        wxASSERT_MSG( red_player->HasRole( hoxRole(table->GetId(), 
                                                   hoxPIECE_COLOR_RED) ),
                      _("Player must play RED"));
    }

    if ( black_player != NULL )
    {
        result = black_player->JoinTable( table );
        wxASSERT( result == hoxRESULT_OK  );
        wxASSERT_MSG( black_player->HasRole( hoxRole(table->GetId(), 
                                                     hoxPIECE_COLOR_BLACK) ),
                      _("Player must play BLACK"));
    }

    // Toggle board if I play BLACK.
    if ( !playRed )
    {
        table->ToggleViewSide();
    }

	frame->UpdateSiteTreeUI();

    return hoxRESULT_OK;
}

void 
hoxRemoteSite::DeletePlayer( hoxPlayer* player )
{
	if ( m_player == player )
	{
		m_player = NULL;
	}

	this->hoxSite::DeletePlayer( player );
}

void 
hoxRemoteSite::Handle_ShutdownReadyFromPlayer( hoxPlayer* player )
{
    const char* FNAME = "hoxRemoteSite::Handle_ShutdownReadyFromPlayer";
    wxLogDebug("%s: ENTER.", FNAME);

	if ( m_player == NULL )
	{
		wxLogDebug("%s: Player is NULL. Shutdown must have already been processed.", FNAME);
		return;
	}

	/* Must set the local player to NULL immediately to handle "re-entrance"
	 * because the DISCONNECT call below can go to sleep...
	 */
	hoxLocalPlayer* localPlayer = m_player;
	m_player = NULL;

	hoxResult result = localPlayer->DisconnectFromNetworkServer( NULL /*m_responseHandler*/ );
	if ( result != hoxRESULT_OK )
	{
		wxLogError("%s: Failed to disconnect from remote server.", FNAME);
		return;
	}

    /* Inform the App. */
    wxCommandEvent event( hoxEVT_APP_SITE_CLOSE_READY );
    event.SetEventObject( this );
    wxPostEvent( &(wxGetApp()), event );
}

// --------------------------------------------------------------------------
// hoxHTTPSite
// --------------------------------------------------------------------------

hoxHTTPSite::hoxHTTPSite( const hoxServerAddress& address )
        : hoxRemoteSite( address, hoxSITE_TYPE_HTTP )
{
    const char* FNAME = "hoxHTTPSite::hoxHTTPSite";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxHTTPSite::~hoxHTTPSite()
{
    const char* FNAME = "hoxHTTPSite::~hoxHTTPSite";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxLocalPlayer* 
hoxHTTPSite::CreateLocalPlayer( const wxString& playerName )
{
	wxCHECK_MSG(m_player == NULL, NULL, "The player has already been set.");

	m_player = m_playerMgr.CreateHTTPPlayer( playerName );
	return m_player;
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
{

}

hoxSiteManager::~hoxSiteManager()
{

}

hoxSite* 
hoxSiteManager::CreateSite( hoxSiteType             siteType,
						    const hoxServerAddress& address )
{
	hoxSite* site = NULL;

	switch ( siteType )
	{
	case hoxSITE_TYPE_LOCAL:
	{
        hoxLocalSite* localSite = new hoxLocalSite( address );
		site = localSite;
		break;
	}
	case hoxSITE_TYPE_REMOTE:
	{
		hoxRemoteSite* remoteSite = new hoxRemoteSite( address );

		wxString playerName = hoxUtility::GenerateRandomString();
		hoxLocalPlayer* localPlayer = remoteSite->CreateLocalPlayer( playerName );
        hoxConnection* connection = new hoxSocketConnection( address.name, 
                                                             address.port );
		localPlayer->SetConnection( connection );
		site = remoteSite;
		break;
	}
	case hoxSITE_TYPE_HTTP:
	{
		hoxRemoteSite* remoteSite = new hoxHTTPSite( address );

		wxString playerName = hoxUtility::GenerateRandomString();
		hoxLocalPlayer* localPlayer = remoteSite->CreateLocalPlayer( playerName );
        hoxConnection* connection = new hoxHttpConnection( address.name, 
                                                           address.port );
		localPlayer->SetConnection( connection );
		site = remoteSite;
		break;
	}
	default:
		break;
	}

    m_sites.push_back( site );
	return site;
}

hoxRemoteSite*
hoxSiteManager::FindRemoteSite( const hoxServerAddress& address ) const
{
    /* Search for existing REMOTE site. */
    for ( hoxSiteList::const_iterator it = m_sites.begin();
                                      it != m_sites.end(); ++it )
    {
        if (   (*it)->GetType() != hoxSITE_TYPE_LOCAL
            && (*it)->GetAddress() == address )
        {
            return (hoxRemoteSite*) (*it);
        }
    }

	return NULL;
}

void
hoxSiteManager::DeleteSite( hoxSite* site )
{
    const char* FNAME = "hoxSiteManager::DeleteSite";

    wxCHECK_RET( site != NULL, "The Site must not be NULL." );
    
    wxLogDebug("%s: Deleting site [%s]...", FNAME, site->GetName().c_str());

    delete site;
    m_sites.remove( site );
}

void 
hoxSiteManager::Close()
{
    const char* FNAME = "hoxSiteManager::Close";
    wxLogDebug("%s: ENTER.", FNAME);

    for ( hoxSiteList::iterator it = m_sites.begin();
                                it != m_sites.end(); ++it )
    {
		(*it)->Close();
    }
}

/************************* END OF FILE ***************************************/
