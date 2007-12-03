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
#include "hoxSocketServer.h"
#include "hoxServer.h"
#include "hoxUtility.h"
#include "hoxSocketConnection.h"
#include "hoxHttpConnection.h"
#include "MyFrame.h"
#include "MyChild.h"
#include "hoxNetworkAPI.h"
#include "hoxTablesDialog.h"

DEFINE_EVENT_TYPE(hoxEVT_SITE_PLAYER_SHUTDOWN_DONE)

BEGIN_EVENT_TABLE(hoxResponseHandler, wxEvtHandler)
    EVT_COMMAND(wxID_ANY, hoxEVT_SITE_PLAYER_SHUTDOWN_DONE, hoxResponseHandler::OnShutdownDone_FromPlayer)
    EVT_COMMAND(wxID_ANY, hoxEVT_CONNECTION_RESPONSE, hoxResponseHandler::OnConnectionResponse)
END_EVENT_TABLE()


void 
hoxResponseHandler::OnShutdownDone_FromPlayer( wxCommandEvent& event )
{
    const char* FNAME = "hoxResponseHandler::OnShutdownDone_FromPlayer";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxPlayer* player = wx_reinterpret_cast(hoxPlayer*, event.GetEventObject());
    wxCHECK_RET(player, "Player cannot be NULL.");

    m_site->Handle_ShutdownDoneFromPlayer( player );
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

void 
hoxSite::OnSystemShutdown() 
{ 
    const char* FNAME = "hoxSite::OnSystemShutdown";

    wxLogDebug("%s: ENTER.", FNAME);

    /* Inform all players about the SHUTDOWN. */
    m_playerMgr.OnSystemShutdown();

    //if ( m_playerMgr.GetNumberOfPlayers() > 0 )
    //{
    //    return;
    //}
}

hoxResult 
hoxSite::CloseTable(hoxTable* table)
{
    m_tableMgr.RemoveTable( table );
    return hoxRESULT_OK;
}

void 
hoxSite::Handle_ShutdownDoneFromPlayer( hoxPlayer* player )
{
    const char* FNAME = "hoxSite::Handle_ShutdownDoneFromPlayer";
    wxLogDebug("%s: ENTER.", FNAME);

    wxLogDebug("%s: Removing this player [%s] from the system...", 
        FNAME, player->GetName().c_str());

    this->DeletePlayer( player );

    /* Initiate the App if there is no more active players. */
    if ( m_playerMgr.GetNumberOfPlayers() == 0 )
    {
        wxCommandEvent event( hoxEVT_APP_SITE_SHUTDOWN_READY );
        event.SetEventObject( this );
        wxPostEvent( &(wxGetApp()), event );
    }
}

// --------------------------------------------------------------------------
// hoxLocalSite
// --------------------------------------------------------------------------


hoxLocalSite::hoxLocalSite(const hoxServerAddress& address)
        : hoxSite( hoxSITE_TYPE_LOCAL, address )
        , m_server( NULL )
        , m_socketServer( NULL )
        , m_isOpened( false )
        , m_player( NULL )
{
    // Create a "host" player representing this machine.
    wxString playerName = hoxUtility::GenerateRandomString();
    playerName += "_HOST";
    m_player = m_playerMgr.CreateHostPlayer( playerName );
}

hoxLocalSite::~hoxLocalSite()
{
    this->Close();
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

    /* Start the socket-manager */

    m_server = new hoxServer( this );

    if ( m_server->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError("%s: Failed to create Server thread.", FNAME);
        return hoxRESULT_ERR;
    }
    wxASSERT_MSG( !m_server->GetThread()->IsDetached(), "The Server thread must be joinable.");

    m_server->GetThread()->Run();

    /* Start the socket-server */

    wxASSERT_MSG( m_socketServer == NULL, "The socket-server should not have been created.");

    m_socketServer = new hoxSocketServer( m_address.port,
                                          m_server,
                                          this );

    if ( m_socketServer->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError("%s: Failed to create socker-server thread.", FNAME);
        return hoxRESULT_ERR;
    }
    wxASSERT_MSG( !m_socketServer->GetThread()->IsDetached(), "The socket-server thread must be joinable.");

    m_socketServer->GetThread()->Run();

    m_isOpened = true;

    return hoxRESULT_OK;
}

hoxResult
hoxLocalSite::Close()
{
    const char* FNAME = "hoxLocalSite::Close";

    if ( m_socketServer != NULL )
    {
        wxLogDebug("%s: Request the socket-server thread to be shutdowned...", FNAME);
        m_socketServer->RequestShutdown();
        wxThread::ExitCode exitCode = m_socketServer->GetThread()->Wait();
        wxLogDebug("%s: The socket-server thread was shutdowned with exit-code = [%d].", FNAME, exitCode);
        delete m_socketServer;
        m_socketServer = NULL;
    }

    if ( m_server != NULL )
    {
        wxLogDebug("%s: Request the Server thread to be shutdowned...", FNAME);
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_SHUTDOWN, NULL );
        m_server->AddRequest( request );
        wxThread::ExitCode exitCode = m_server->GetThread()->Wait();
        wxLogDebug("%s: The Server thread was shutdowned with exit-code = [%d].", FNAME, exitCode);
        delete m_server;
        m_server = NULL;
    }

    m_isOpened = false;

    return hoxRESULT_OK;
}

hoxResult 
hoxLocalSite::CreateNewTable( wxString& newTableId )
{
    return this->CreateNewTableAsPlayer( newTableId, 
                                         m_player );
}

hoxResult 
hoxLocalSite::CreateNewTableAsPlayer( wxString&  newTableId, 
                                      hoxPlayer* player )
{
    const char* FNAME = "hoxLocalSite::CreateNewTableAsPlayer";
    hoxTable* newTable = NULL;
    wxString  tableId;  // TODO: we should generate our table-Id.

    wxLogDebug("%s: ENTER.", FNAME);

    /* Create a GUI Frame for the new Table. */
    MyFrame* frame = wxGetApp().GetFrame();
    MyChild* childFrame = frame->CreateFrameForTable( tableId );

    /* Create a new table with newly created Frame. */
    newTable = m_tableMgr.CreateTableWithFrame( childFrame, 
                                                tableId );
    childFrame->SetTable( newTable );
    childFrame->Show( false /*true*/ );

    newTableId = newTable->GetId();

    // Add the specified player to the table.
    hoxResult result = player->JoinTable( newTable );
    wxASSERT( result == hoxRESULT_OK  );
    wxASSERT_MSG( player->HasRole( 
                            hoxRole(newTable->GetId(), hoxPIECE_COLOR_RED) ),
                  _("Player must play RED"));

    // Update UI.
    frame->UpdateSiteTreeUI();

    return hoxRESULT_OK;
}

void 
hoxLocalSite::DeletePlayer( hoxPlayer* player )
{
	if ( m_player == player )
	{
		m_player = NULL;
	}

	this->hoxSite::DeletePlayer( player );
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

    m_player = _CreateLocalPlayer();
}

hoxRemoteSite::~hoxRemoteSite()
{
    const char* FNAME = "hoxRemoteSite::~hoxRemoteSite";
    wxLogDebug("%s: ENTER.", FNAME);

    this->Close();
}

hoxLocalPlayer* 
hoxRemoteSite::_CreateLocalPlayer()
{
    hoxLocalPlayer* localPlayer = NULL;
    hoxConnection* connection = NULL;

    wxString playerName = hoxUtility::GenerateRandomString();

    if ( m_type == hoxSITE_TYPE_HTTP )
    {
        localPlayer = m_playerMgr.CreateHTTPPlayer( playerName );
        connection = new hoxHttpConnection( m_address.name, 
                                            m_address.port );
    }
    else
    {
        localPlayer = m_playerMgr.CreateMyPlayer( playerName );
        connection = new hoxSocketConnection( m_address.name, 
                                              m_address.port );
    }

    localPlayer->SetConnection( connection );

    return localPlayer;
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
    newTable = m_tableMgr.CreateTableWithFrame( childFrame, 
                                                newTableId );
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

    name.Printf("Remote %s:%d", m_address.name.c_str(), m_address.port);
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
    hoxResult result;
    wxLogDebug("%s: ENTER.", FNAME);

	if ( m_player != NULL )
	{
		result = m_player->DisconnectFromNetworkServer( NULL /*m_responseHandler*/ );
		if ( result != hoxRESULT_OK )
		{
			wxLogError("%s: Failed to disconnect from remote server.", FNAME);
			return hoxRESULT_ERR;
		}
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
    return (    m_player->GetConnection() != NULL
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
    table = m_tableMgr.CreateTableWithFrame( childFrame, 
                                             tableId );
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

    this->Close();
}

/************************* END OF FILE ***************************************/
