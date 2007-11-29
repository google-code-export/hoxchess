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
// Name:            MyFrame.cpp
// Created:         10/02/2007
//
// Description:     The main Frame of the App.
/////////////////////////////////////////////////////////////////////////////

#include <wx/numdlg.h>
#include "MyFrame.h"
#include "MyChild.h"
#include "MyApp.h"    // To access wxGetApp()
#include "hoxTable.h"
#include "hoxPlayer.h"
#include "hoxTableMgr.h"
#include "hoxPlayerMgr.h"
#include "hoxTablesDialog.h"
#include "hoxUtility.h"
#include "hoxNetworkAPI.h"
#include "hoxSocketConnection.h"
#include "hoxHttpConnection.h"
#include "hoxLocalPlayer.h"

#if !defined(__WXMSW__)
    #include "icons/hoxchess.xpm"
#endif

#include "bitmaps/new.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/quit.xpm"
#include "bitmaps/delete.xpm"

IMPLEMENT_DYNAMIC_CLASS(MyFrame, wxMDIParentFrame)

/* Define my custom events */
DEFINE_EVENT_TYPE(hoxEVT_FRAME_LOG_MSG)

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

#define hoxVERSION_STRING  "0.1.0.1"

BEGIN_EVENT_TABLE(MyFrame, wxMDIParentFrame)
    EVT_MENU(MDI_ABOUT, MyFrame::OnAbout)
    EVT_MENU(MDI_NEW_TABLE, MyFrame::OnNewTable)
    EVT_MENU(MDI_CLOSE_TABLE, MyFrame::OnCloseTable)
    EVT_UPDATE_UI(MDI_CLOSE_TABLE, MyFrame::OnUpdateCloseTable)

    EVT_MENU(MDI_OPEN_SERVER, MyFrame::OnOpenServer)
    EVT_MENU(MDI_CLOSE_SERVER, MyFrame::OnCloseServer)
    EVT_MENU(MDI_CONNECT_SERVER, MyFrame::OnConnectServer)
    EVT_MENU(MDI_DISCONNECT_SERVER, MyFrame::OnDisconnectServer)
    EVT_MENU(MDI_LIST_TABLES, MyFrame::OnListTables)

    EVT_MENU(MDI_CONNECT_HTTP_SERVER, MyFrame::OnConnectHTTPServer)

    EVT_MENU(MDI_SHOW_SERVERS_WINDOW, MyFrame::OnShowServersWindow)
    EVT_UPDATE_UI(MDI_SHOW_SERVERS_WINDOW, MyFrame::OnUpdateServersWindow)

    EVT_MENU(MDI_SHOW_LOG_WINDOW, MyFrame::OnShowLogWindow)
    EVT_UPDATE_UI(MDI_SHOW_LOG_WINDOW, MyFrame::OnUpdateLogWindow)

    EVT_MENU(MDI_QUIT, MyFrame::OnQuit)

    EVT_CLOSE(MyFrame::OnClose)
    EVT_SIZE(MyFrame::OnSize)
    EVT_SASH_DRAGGED(ID_WINDOW_SITES, MyFrame::OnServersSashDrag)
    EVT_SASH_DRAGGED(ID_WINDOW_LOG, MyFrame::OnLogSashDrag)

    EVT_COMMAND(wxID_ANY, hoxEVT_FRAME_LOG_MSG, MyFrame::OnFrameLogMsgEvent)

    EVT_CONTEXT_MENU(MyFrame::OnContextMenu)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------
// MyFrame
// ---------------------------------------------------------------------------

MyFrame::MyFrame()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

MyFrame::MyFrame( wxWindow*        parent,
                  const wxWindowID id,
                  const wxString&  title,
                  const wxPoint&   pos,
                  const wxSize&    size,
                  const long       style )
       : wxMDIParentFrame( parent, id, title, pos, size, style )
{
    const char* FNAME = "MyFrame::MyFrame";

    wxLogDebug("%s: ENTER.", FNAME);

    SetIcon( wxICON(hoxchess) );

    // A window to the left of the client window
    m_sitesWindow = new wxSashLayoutWindow(this, ID_WINDOW_SITES,
                               wxDefaultPosition, wxDefaultSize,
                               wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    m_sitesWindow->SetDefaultSize(wxSize(200, -1));
    m_sitesWindow->SetOrientation(wxLAYOUT_VERTICAL);
    m_sitesWindow->SetAlignment(wxLAYOUT_LEFT);
    m_sitesWindow->SetBackgroundColour(wxColour(0, 0, 255));
    m_sitesWindow->SetSashVisible(wxSASH_RIGHT, true);
    m_sitesWindow->SetExtraBorderSize(2);

    m_sitesTree = new wxTreeCtrl( m_sitesWindow, 
                                    ID_TREE_SITES,
                                    wxDefaultPosition, wxDefaultSize,
                                    wxTR_DEFAULT_STYLE | wxNO_BORDER);

    wxTreeItemId root = m_sitesTree->AddRoot( "Sites" );
    this->UpdateSiteTreeUI();

    // A window containing our log-text.
    m_logWindow = new wxSashLayoutWindow( 
                            this, 
                            ID_WINDOW_LOG,
                            wxDefaultPosition, 
                            wxDefaultSize,
                            wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN );
    m_logWindow->SetDefaultSize(wxSize(-1, 180));  // TODO: Hard-coded.
    m_logWindow->SetOrientation(wxLAYOUT_HORIZONTAL);
    m_logWindow->SetAlignment(wxLAYOUT_BOTTOM);
    m_logWindow->SetBackgroundColour(wxColour(0, 0, 255));
    m_logWindow->SetSashVisible(wxSASH_TOP, true);
    m_logWindow->Show( false );

    m_logText = new wxTextCtrl( 
            m_logWindow, wxID_ANY, 
            "A Log Window (currently not being used due to its instabilities)",
            wxDefaultPosition, wxDefaultSize,
            wxTE_MULTILINE | wxSUNKEN_BORDER );

    // Create toolbar.
    CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_HORIZONTAL);
    InitToolBar(GetToolBar());

    // Accelerators
    wxAcceleratorEntry entries[3];
    entries[0].Set(wxACCEL_CTRL, (int) 'N', MDI_NEW_TABLE);
    entries[1].Set(wxACCEL_CTRL, (int) 'X', MDI_QUIT);
    entries[2].Set(wxACCEL_CTRL, (int) 'A', MDI_ABOUT);
    wxAcceleratorTable accel(3, entries);
    SetAcceleratorTable(accel);

    // Progress dialog.
    m_dlgProgress = NULL;

    m_nChildren = 0;

    this->SetupMenu();
    this->SetupStatusBar();

    wxLogStatus(_("HOXChess is ready."));
}

MyFrame::~MyFrame()
{
}

void 
MyFrame::OnClose(wxCloseEvent& event)
{
    const char* FNAME = "MyFrame::OnClose";

    wxLogDebug("%s: ENTER.", FNAME);

    while( ! m_children.empty() )
    {
        MyChild* child = m_children.front();
        child->Close( true /* force */ );
        // NOTE: The call above already delete the child.
    }

    /* Inform all players about the SHUTDOWN. */
    hoxPlayerMgr::GetInstance()->OnSystemShutdown();

    if ( hoxPlayerMgr::GetInstance()->GetNumberOfPlayers() > 0 )
    {
        // *** Postpone the shutdown until all players leave the system.
        return;
    }

    /* Forward this Close event to let the App close the entire system. */
    event.Skip();
}

void 
MyFrame::OnQuit( wxCommandEvent& event )
{
    Close();
}

void 
MyFrame::OnAbout( wxCommandEvent& event )
{
    wxMessageBox( wxString::Format(
                    _("HOXChess %s\n"
                      "\n"
                      "Author: Huy Phan (c) 2007\n"
                      "Email: huyphan@playxiangqi.com\n"
                      "\n"
                      "This application is powered by %s, running under %s.\n"
                      "\n"),
                    hoxVERSION_STRING,
                    wxVERSION_STRING,
                    wxGetOsDescription().c_str()
                 ),
                 _("About HOXChess"),
                 wxOK | wxICON_INFORMATION,
                 this );
}

void 
MyFrame::OnNewTable( wxCommandEvent& event )
{
    const char* FNAME = "MyFrame::OnNewTable";

    hoxTable* selectedTable = NULL;
    hoxSite* selectedSite = _GetSelectedSite(selectedTable);

    if ( selectedSite == NULL )
        return;

    wxString newTableId;
    if ( hoxRESULT_OK != selectedSite->CreateNewTable( newTableId ) )
    {
        wxLogError("%s: Failed to create a new Table.", FNAME);
    }
}

void 
MyFrame::OnCloseTable( wxCommandEvent& event )
{
    const char* FNAME = "MyFrame::OnCloseTable";

    wxLogDebug("%s: ENTER.", FNAME);

    MyChild* child = wxDynamicCast(this->GetActiveChild(), MyChild);
    if ( child != NULL )
    {
        wxLogDebug("%s: Closing the active Table [%s]...", FNAME, child->GetTitle().c_str());
        child->Close( true /* force */ );
    }
}

// Update the menu's UI.
void 
MyFrame::OnUpdateCloseTable(wxUpdateUIEvent& event)
{
    event.Enable( this->GetActiveChild() != NULL );
}

bool
MyFrame::OnChildClose( MyChild* child, 
                       hoxTable* table )
{
    const char* FNAME = "MyFrame::OnChildClose";

    wxLogDebug("%s: ENTER.", FNAME);

    wxCHECK_MSG( table != NULL, true, "The table is NULL." );

    m_nChildren--;
    table->OnClose_FromSystem();
    m_children.remove( child );

    wxLogDebug("%s: END.", FNAME);
    return true;
}

void 
MyFrame::OnOpenServer( wxCommandEvent& event )
{
    const char* FNAME = "MyFrame::OnOpenServer";
    wxLogDebug("%s: ENTER.", FNAME);

    /* Ask user for server's port-number... */

    long minPort = 1024;
    long maxPort = 64000;
    long defaultPort = hoxNETWORK_DEFAULT_SERVER_PORT;

    long nPort = wxGetNumberFromUser( 
        _("Enter the port at which the server will be listening:"),
        wxString::Format("Port [%d - %d]", minPort, maxPort), 
        _("Server's Port ..."),
        defaultPort,
        minPort,
        maxPort,
        this,  // parent 
        wxDefaultPosition 
        );

    if ( nPort == -1 ) // invalid or user cancels?
    {
        wxLogDebug("%s: User enters an invalid port or has canceled.", FNAME);
        return;
    }

    wxGetApp().OpenServer( (int) nPort );
    wxLogStatus("Server listening for connections at port [%d].", (int)nPort);
}

void 
MyFrame::OnCloseServer( wxCommandEvent& event )
{
    const char* FNAME = "MyFrame::OnCloseServer";
    wxLogDebug("%s: ENTER.", FNAME);

    /* Find out which site is selected. */

    hoxTable* selectedTable = NULL;
    hoxSite* selectedSite = _GetSelectedSite(selectedTable);

    if ( selectedSite == NULL )
    {
        wxLogError("%s: A site must be selected. END.", FNAME);
        return;
    }

    /* Go through my children to see which ones belong to the site and
     * trigger a close event.
     */
    _CloseChildrenOfSite( selectedSite );

    /* Close the site itself. */
    wxGetApp().CloseServer();
}

void 
MyFrame::OnConnectServer( wxCommandEvent& event )
{
    const char* FNAME = "MyFrame::OnConnectServer";
    hoxResult result;

    /* Ask the user for the server' address. */

    hoxServerAddress serverAddress;

    result = _GetServerAddressFromUser( 
                    _("Enter the address of an HOXChess server:"),
                    _("Connect to a server ..."),
                    hoxServerAddress( "127.0.0.1", 
                                      hoxNETWORK_DEFAULT_SERVER_PORT ),
                    serverAddress );

    if ( result != hoxRESULT_OK )
        return;

    /* Start connecting... */
    wxGetApp().ConnectRemoteServer( serverAddress );
}

void MyFrame::OnConnectHTTPServer( wxCommandEvent& event )
{
    const char* FNAME = "MyFrame::OnConnectHTTPServer";
    hoxResult result;
    hoxHttpPlayer* httpPlayer = wxGetApp().GetHTTPPlayer();

    wxLogDebug("%s: ENTER.", FNAME);

    /* If the connection has been established, then go ahead
     * to query for the list of tables.
     */

    if (   httpPlayer->GetConnection() != NULL
        && httpPlayer->GetConnection()->IsConnected() )
    {
        /* Get the list of tables from the server */
        result = httpPlayer->QueryForNetworkTables( this );
        if ( result != hoxRESULT_OK )
        {
            wxLogError("%s: Failed to query for LIST of tables from the server.", FNAME);
            return;
        }
        return;
    }

    /***********************************************/
    /* Otherwise, try to establish a connection... */
    /***********************************************/

    /* Ask the user for the server' address. */

    hoxServerAddress serverAddress;

    result = _GetServerAddressFromUser( 
                    _("This is an EXPERIMENT feature.\n"
                      "The default HTTP server specified below is not yet ready.\n"
                      "\n"
                      "Enter the address of an HOXChess server (HTTP polling):"),
                    _("EXPERIMENT - Connect to a HTTP-based (polling) server ..."),
                    hoxServerAddress( HOX_HTTP_SERVER_HOSTNAME, 
                                      HOX_HTTP_SERVER_PORT ),
                    serverAddress );

    if ( result != hoxRESULT_OK )
        return;

    /* Start connecting... */

    if ( m_dlgProgress != NULL ) 
    {
        m_dlgProgress->Destroy();  // NOTE: ... see wxWidgets' documentation.
        m_dlgProgress = NULL;
    }

    m_dlgProgress = new wxProgressDialog
                        (
                         "Progress dialog",
                         "Wait until connnection is established or press [Cancel]",
                         100,
                         this,
                         wxPD_AUTO_HIDE | wxPD_CAN_ABORT
                        );
    m_dlgProgress->SetSize( wxSize(500, 150) );
    m_dlgProgress->Pulse();

    // Create a new connection for the HTTP player.
    wxLogDebug("%s: Connecting to HTTP server [%s:%d]...", 
        FNAME, serverAddress.name.c_str(), serverAddress.port);
    hoxConnection* connection = new hoxHttpConnection( serverAddress.name, 
                                                       serverAddress.port );
    httpPlayer->SetConnection( connection );

    result = httpPlayer->ConnectToNetworkServer( this );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to connect to HTTP server.", FNAME);
        return;
    }

    m_dlgProgress->Pulse();
}

void 
MyFrame::OnShowServersWindow( wxCommandEvent& event )
{
    m_sitesWindow->Show( ! m_sitesWindow->IsShown() );
    m_sitesWindow->SetDefaultSize(wxSize(200, -1));

    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);

    // Leaves bits of itself behind sometimes
    GetClientWindow()->Refresh();
}

void 
MyFrame::OnUpdateServersWindow( wxUpdateUIEvent& event )
{
    event.Check( m_sitesWindow->IsShown() );
}

void 
MyFrame::OnShowLogWindow( wxCommandEvent& event )
{
    m_logWindow->Show( ! m_logWindow->IsShown() );
    m_logWindow->SetDefaultSize(wxSize(-1, 180));

    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
}

void 
MyFrame::OnUpdateLogWindow( wxUpdateUIEvent& event )
{
    event.Check( m_logWindow->IsShown() );
}

void 
MyFrame::DoJoinExistingTable( const hoxNetworkTableInfo& tableInfo,
                              hoxLocalPlayer*            localPlayer )
{
    const char* FNAME = "MyFrame::DoJoinExistingTable";

    hoxSite* site = localPlayer->GetSite();
    hoxRemoteSite* remoteSite = wxDynamicCast( site, hoxRemoteSite );
    if ( remoteSite != NULL )
    {
        if ( hoxRESULT_OK != remoteSite->JoinExistingTable( tableInfo ) )
        {
            wxLogError("%s: Failed to join existing network table.", FNAME);
            return;
        }
        this->UpdateSiteTreeUI();
    }
}

void 
MyFrame::DoJoinNewTable( const wxString& tableId,
                         hoxLocalPlayer* localPlayer )
{
    const char* FNAME = "MyFrame::DoJoinNewTable";
    hoxResult result;

    /***********************/
    /* Create a new table. */
    /***********************/

    wxLogDebug("%s: Create a new table connecting to the server...", FNAME);
    hoxTable* table = _CreateNewTable( tableId );

    /***********************/
    /* Setup players       */
    /***********************/

    /* Since we open this NEW table, we will play RED.
     */

    result = localPlayer->JoinTable( table );
    wxASSERT( result == hoxRESULT_OK  );
    wxASSERT_MSG( localPlayer->HasRole( hoxRole(table->GetId(), 
                                                hoxPIECE_COLOR_RED) ),
                  _("Player must play RED"));

    /* NOTE: The other player is <EMPTY> 
     */
}

void MyFrame::OnDisconnectServer( wxCommandEvent& event )
{
    const char* FNAME = "MyFrame::OnDisconnectServer";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxTable* selectedTable = NULL;
    hoxSite* selectedSite = _GetSelectedSite(selectedTable);

    if ( selectedSite == NULL )
        return;

    /* Go through my children to see which ones belong to the site and
     * trigger a close event.
     */
    _CloseChildrenOfSite( selectedSite );

    /* Close the site itself. */
    if ( ! selectedSite->IsLocal() )
    {
        hoxRemoteSite* remoteSite = (hoxRemoteSite*) selectedSite;
        wxGetApp().DisconnectRemoteServer( remoteSite );
    }
}

void MyFrame::OnListTables( wxCommandEvent& event )
{
    const char* FNAME = "MyFrame::OnListTables";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxTable* selectedTable = NULL;
    hoxSite* selectedSite = _GetSelectedSite(selectedTable);

    if ( selectedSite == NULL )
        return;

    if ( ! selectedSite->IsLocal() )
    {
        hoxRemoteSite* remoteSite = (hoxRemoteSite*) selectedSite;

        /* Get the list of tables from the server */
        hoxResult result = remoteSite->QueryForNetworkTables( /*this*/ );
        if ( result != hoxRESULT_OK )
        {
            wxLogError("%s: Failed to query for LIST of tables from the server.", FNAME);
            return;
        }
    }
}

void MyFrame::OnSize(wxSizeEvent& event)
{
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
    //wxString mySize = wxString::Format("%d x %d", event.GetSize().GetWidth(), event.GetSize().GetHeight());
    //wxLogStatus(mySize);
}

void MyFrame::OnServersSashDrag(wxSashEvent& event)
{
    if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
        return;

    m_sitesWindow->SetDefaultSize(wxSize(event.GetDragRect().width, -1));

    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);

    // Leaves bits of itself behind sometimes
    GetClientWindow()->Refresh();
}

void MyFrame::OnLogSashDrag(wxSashEvent& event)
{
    if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
        return;

    m_logWindow->SetDefaultSize(wxSize(-1, event.GetDragRect().height));

    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);

    // Leaves bits of itself behind sometimes
    GetClientWindow()->Refresh();
}

void MyFrame::InitToolBar(wxToolBar* toolBar)
{
    wxBitmap bitmaps[8];

    bitmaps[0] = wxBitmap( new_xpm );
    bitmaps[1] = wxBitmap( delete_xpm );
    bitmaps[2] = wxBitmap( help_xpm );
    bitmaps[3] = wxBitmap( quit_xpm );
    //bitmaps[4] = wxBitmap( open_xpm );
    //bitmaps[5] = wxBitmap( save_xpm );

    toolBar->AddTool(MDI_NEW_TABLE, _T("New"), bitmaps[0], _("Open Table"));
    toolBar->AddTool(MDI_CLOSE_TABLE, _("Close"), bitmaps[1], _("Close Table"));
    toolBar->AddSeparator();
    toolBar->AddTool(MDI_ABOUT, _("About"), bitmaps[2], _("Help"));
    toolBar->AddTool(MDI_QUIT, _("Exit"), bitmaps[3], _("Quit the Program"));
    //toolBar->AddTool(4, _("Open"), bitmaps[4], _("Open file"));
    //toolBar->AddTool(5, _("Save"), bitmaps[5], _("Save file"));

    toolBar->Realize();
}

void
MyFrame::SetupMenu()
{
    // Associate the menu bar with the frame
    wxMenuBar* menu_bar = MyFrame::Create_Menu_Bar();
    SetMenuBar( menu_bar );
}

/* static */
wxMenuBar* 
MyFrame::Create_Menu_Bar(bool hasTable /* = false */)
{
    // File menu.
    wxMenu* file_menu = new wxMenu;
    file_menu->Append(MDI_CONNECT_SERVER, _("Connect Server...\tCtrl-L"), _("Connect to remote server"));
    file_menu->Append(MDI_DISCONNECT_SERVER, _("&Disconnect Server\tCtrl-D"), _("Disconnect from remote server"));
    file_menu->AppendSeparator();
    file_menu->Append(MDI_CONNECT_HTTP_SERVER, _("Connect HTTP Serve&r...\tCtrl-R"));
    file_menu->AppendSeparator();
    file_menu->Append(MDI_NEW_TABLE, _("&New Table\tCtrl-N"), _("Create New Table"));
    file_menu->Append(MDI_CLOSE_TABLE, _("&Close Table\tCtrl-C"), _("Close Table"));
    file_menu->AppendSeparator();
    file_menu->Append(MDI_QUIT, _("&Exit\tAlt-X"), _("Quit the program"));

    /* Server menu. */
    wxMenu* server_menu = new wxMenu;
    server_menu->Append(MDI_OPEN_SERVER, _("&Open Server...\tCtrl-O"), _("Open server for remote access"));
    server_menu->AppendSeparator();
    server_menu->AppendCheckItem(MDI_SHOW_SERVERS_WINDOW, _("Server&s Window\tCtrl-S"));

    /* View menu (only if a Table exits) */
    wxMenu* view_menu = NULL;
    if ( hasTable )
    {
        view_menu = new wxMenu;
        view_menu->Append(MDI_TOGGLE, _("&Toggle Table View\tCtrl-T"));
    }

    /* Help menu. */
    wxMenu* help_menu = new wxMenu;
    help_menu->AppendCheckItem(MDI_SHOW_LOG_WINDOW, _("Lo&g Window\tCtrl-G"));
    help_menu->AppendSeparator();
    help_menu->Append(MDI_ABOUT, _("&About HOXChess\tF1"));

    /* The main menu bar */
    wxMenuBar* menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, _("&File"));
    menu_bar->Append(server_menu, _("&Server"));
    if ( view_menu != NULL )
        menu_bar->Append(view_menu, _("&View"));
    menu_bar->Append(help_menu, _("&Help"));

    return menu_bar;
}

void
MyFrame::SetupStatusBar()
{
    CreateStatusBar();
}

void
MyFrame::Handle_PlayerResponse( hoxResponse*    pResponse,
                                hoxLocalPlayer* localPlayer ) 
{
    const char* FNAME = "MyFrame::Handle_PlayerResponse";

    wxLogDebug("%s: ENTER.", FNAME);

    const std::auto_ptr<hoxResponse> response( pResponse ); // take care memory leak!

    if ( response->code != hoxRESULT_OK )
    {
        wxLogDebug("%s: The response's code is ERROR. END.", FNAME);
        // Delete the connection if it is bad.
        if ( response->type == hoxREQUEST_TYPE_CONNECT )
        {
            wxLogDebug("%s: Deleting this BAD connection...", FNAME);
            localPlayer->ResetConnection();
        }
        return;
    }

    switch ( response->type )
    {
        case hoxREQUEST_TYPE_CONNECT:
            _OnResponse_Connect( response->content, localPlayer );
            break;

        case hoxREQUEST_TYPE_LIST:
            _OnResponse_List( response->content, localPlayer );
            break;

        case hoxREQUEST_TYPE_NEW:
            _OnResponse_New( response->content, localPlayer );
            break;

        case hoxREQUEST_TYPE_JOIN:
            _OnResponse_Join( response->content, localPlayer );
            break;

        case hoxREQUEST_TYPE_LEAVE:
            _OnResponse_Leave( response->content );
            break;

        default:
            wxLogError("%s: Unknown type [%d].", FNAME, response->type );
            break;
    }
}

void 
MyFrame::OnFrameLogMsgEvent( wxCommandEvent &event )
{
    wxString msg = event.GetString();
    m_logText->AppendText( msg << "\n");
}

void 
MyFrame::OnContextMenu( wxContextMenuEvent& event )
{ 
    wxPoint point = event.GetPosition(); 
    point = ScreenToClient(point);
    
    wxMenu menu;
    bool serverItemsInserted = false;
    bool remoteItemsInserted = false;
    bool tableItemsInserted = false;

    if ( wxGetApp().m_localSite == NULL )
    {
        menu.Append(MDI_OPEN_SERVER, _("&Open Server...\tCtrl-O"), 
                                     _("Open server for remote access"));
        serverItemsInserted = true;
    }

    hoxTable* selectedTable = NULL;
    hoxSite* selectedSite = _GetSelectedSite(selectedTable);

    if ( selectedTable != NULL )
    {
        menu.Append(MDI_CLOSE_TABLE, _("&Close Table\tCtrl-C"), _("Close Table"));
        tableItemsInserted = true;
    }
    else if ( selectedSite != NULL )
    {
        if ( selectedSite->IsLocal() )
        {
            hoxLocalSite* localSite = (hoxLocalSite*) selectedSite;
            if ( localSite->IsOpened() )
            {
                menu.Append(MDI_CLOSE_SERVER, _("&Close Server...\tCtrl-C"), 
                                              _("Close local server"));
                serverItemsInserted = true;
            }
        }
        else /* Remote */
        {
            hoxRemoteSite* remoteSite = (hoxRemoteSite*) selectedSite;
            if ( remoteSite->IsConnected() )
            {
                menu.Append(MDI_DISCONNECT_SERVER, _("&Disconnect Server\tCtrl-D"), 
                                                   _("Disconnect from remote server"));
                menu.Append(MDI_LIST_TABLES, _("&Query for Tables\tCtrl-Q"), 
                                                   _("Query for the list of tables"));
                remoteItemsInserted = true;
            }

            menu.Append(MDI_NEW_TABLE, _("&New Table\tCtrl-N"), _("Create New Table"));
        }
    }

    if ( ! tableItemsInserted )
    {
        if ( serverItemsInserted )
            menu.AppendSeparator();

        if ( !remoteItemsInserted )
            menu.Append(MDI_CONNECT_SERVER, _("Connect Remote Server...\tCtrl-L"), _("Connect to remote server"));
    }

    PopupMenu(&menu, point.x, point.y);
}

void 
MyFrame::_OnResponse_Connect( const wxString& responseStr,
                              hoxLocalPlayer* localPlayer )
{
    const char* FNAME = "MyFrame::_OnResponse_Connect";
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
            FNAME,  returnCode, returnMsg.c_str());
        return;
    }
}

void 
MyFrame::_OnResponse_List( const wxString& responseStr,
                           hoxLocalPlayer* localPlayer )
{
    const char* FNAME = "MyFrame::_OnResponse_List";
    hoxNetworkTableInfoList tableList;
    hoxResult               result;

    wxLogDebug("%s: Parsing LIST's response...", FNAME);

    result = hoxNetworkAPI::ParseNetworkTables( responseStr,
                                                tableList );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse the LIST's response.", FNAME);
        return;
    }

    /* Show tables. */

    hoxTablesDialog tablesDlg( this, wxID_ANY, "Tables", tableList);
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
            result = localPlayer->JoinNetworkTable( selectedId, this );
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
            result = localPlayer->OpenNewNetworkTable( this );
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
MyFrame::_OnResponse_Join( const wxString& responseStr,
                           hoxLocalPlayer* localPlayer )
{
    const char* FNAME = "MyFrame::_OnResponse_Join";
    wxLogDebug("%s: Parsing SEND-JOIN's response...", FNAME);
    hoxNetworkTableInfo tableInfo;
    hoxResult result;

    result = hoxNetworkAPI::ParseJoinNetworkTable( responseStr,
                                                   tableInfo );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse for SEND-JOIN's response [%s].", FNAME, responseStr.c_str());
        return;
    }
    else
    {
        wxLogDebug("Successfully joined the network table [%s].", tableInfo.id.c_str());
        this->DoJoinExistingTable( tableInfo, localPlayer );
    }
}

void 
MyFrame::_OnResponse_New( const wxString& responseStr,
                          hoxLocalPlayer* localPlayer )
{
    const char* FNAME = "MyFrame::_OnResponse_New";
    wxString newTableId;
    hoxResult result;

    wxLogDebug("%s: Parsing SEND-NEW's response...", FNAME);

    result = hoxNetworkAPI::ParseNewNetworkTable( responseStr,
                                                  newTableId );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse for SEND-NEW's response.", FNAME);
        return;
    }

    wxLogDebug("The system creating a new table with ID = [%s]...", newTableId.c_str());
    this->DoJoinNewTable( newTableId, localPlayer );
}

void 
MyFrame::_OnResponse_Leave( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnResponse_Leave";
    int        returnCode = -1;
    wxString   returnMsg;
    hoxResult  result;

    wxLogDebug("%s: Parsing SEND-LEAVE's response...", FNAME);

    result = hoxNetworkAPI::ParseSimpleResponse( responseStr,
                                                 returnCode,
                                                 returnMsg );
    if ( result != hoxRESULT_OK || returnCode != 0 )
    {
        wxLogError("%s: Failed to parse LEAVE's response. [%d] [%s]", 
            FNAME,  returnCode, returnMsg.c_str());
        return;
    }
}

hoxResult
MyFrame::_GetServerAddressFromUser( 
                        const wxString&         message,
                        const wxString&         caption,
                        const hoxServerAddress& defaultAddress,
                        hoxServerAddress&       serverAddress )
{
    const char* FNAME = "MyFrame::_GetServerAddressFromUser";

    const wxString defaultInput =
        wxString::Format("%s:%d", defaultAddress.name.c_str(), 
                                  defaultAddress.port);

    wxString userText = wxGetTextFromUser( message,
                                           caption,
                                           defaultInput,
                                           this /* Parent */ );
    if ( userText.empty() ) // user canceled?
    {
        wxLogDebug("%s: The user has canceled the connection.", FNAME);
        return hoxRESULT_ERR;
    }

    if ( ! hoxUtility::ParseServerAddress( userText,
                                           serverAddress ) )
    {
        wxLogError("The server's address [%s] is invalid .", userText.c_str());
        return hoxRESULT_ERR;
    }

    return hoxRESULT_OK;
}

MyChild* 
MyFrame::CreateFrameForTable( wxString& requestTableId )
{
    const char* FNAME = "MyFrame::CreateFrameForTable";
    MyChild*  childFrame = NULL;
    wxString  effectiveTableId;
    wxString  windowTitle;

    m_nChildren++; // for tracking purpose.

    /* Generate a table's Id if required. */
    if ( requestTableId.empty() )
    {
        requestTableId.Printf("%d", m_nChildren );
        wxLogDebug("%s: Generated a new table-Id [%s].", FNAME, requestTableId.c_str());  
    }

    /* TODO: We should check if the new Id has already been taken
     *       by an existing Table.
     */

    /* Generate the Window's title for the new Table. */
    windowTitle.Printf("Table #%s", requestTableId.c_str());

    childFrame = new MyChild( this, windowTitle );
    m_children.push_back( childFrame );

    return childFrame;
}

hoxTable* 
MyFrame::_CreateNewTable( const wxString& tableId )
{
    hoxTable* newTable = NULL;
    MyChild*  childFrame = NULL;
    wxString  effectiveTableId;

    /* Create a GUI Frame for the new Table. */
    effectiveTableId = tableId;
    childFrame = this->CreateFrameForTable( effectiveTableId );

    /* Create a new table with newly created Frame. */
    newTable = hoxTableMgr::GetInstance()->CreateTableWithFrame( childFrame, 
                                                                 effectiveTableId );
    childFrame->SetTable( newTable );
    childFrame->Show( true );

    return newTable;
}

void 
MyFrame::UpdateSiteTreeUI()
{
    wxTreeItemId  rootId = m_sitesTree->GetRootItem();
    wxTreeItemId  siteId;
    wxTreeItemId  tableId;

    /* Delete the old tree's content */
    m_sitesTree->DeleteChildren( rootId );

    /* Add new Sites */
    const hoxSiteList& sites = wxGetApp().m_sites;

    for ( hoxSiteList::const_iterator it = sites.begin();
                                      it != sites.end(); ++it )
    {
        siteId = m_sitesTree->AppendItem(rootId, (*it)->GetName());
        SiteTreeItemData* itemData = new SiteTreeItemData( *it );
        m_sitesTree->SetItemData(siteId, itemData);

        const hoxTableList& tables = (*it)->GetTables();
        for ( hoxTableList::const_iterator table_it = tables.begin();
                                           table_it != tables.end(); 
                                         ++table_it )
        {
            wxString tableStr;
            tableStr.Printf("Table #%s", (*table_it)->GetId().c_str());
            tableId = m_sitesTree->AppendItem(siteId, tableStr);

            TableTreeItemData* itemData = new TableTreeItemData( *table_it );
            m_sitesTree->SetItemData(tableId, itemData);
        }
    }

    /* Make Sites visible. */
    m_sitesTree->Expand(rootId);
}

void     
MyFrame::_CloseChildrenOfSite(hoxSite* site)
{
    wxCHECK_RET(site != NULL, "Site must not be NULL.");

    /* Go through my children to see which ones belong to the site.
     */
    MyChildList closeList;
    for ( MyChildList::const_iterator it = m_children.begin();
                                      it != m_children.end(); ++it )
    {
        if ( (*it)->GetSite() == site )
        {
            closeList.push_back( *it );
        }
    }

    /* Trigger a close event.
     */
    for ( MyChildList::const_iterator it = closeList.begin();
                                      it != closeList.end(); ++it )
    {
        (*it)->Close( true /* force */ );
        // NOTE: The call above already delete the child.
    }
}

hoxSite* 
MyFrame::_GetSelectedSite(hoxTable*& selectedTable) const
{
    selectedTable = NULL;

    hoxSite* selectedSite = NULL;

    wxTreeItemId selectedItem = m_sitesTree->GetSelection();
    wxTreeItemId rootId = m_sitesTree->GetRootItem();
    int depth = -1;

    // Get item's depth.
    wxTreeItemId itemId = selectedItem;
    while ( itemId.IsOk() )
    {
        ++depth;
        if ( itemId == rootId )
            break;
        itemId = m_sitesTree->GetItemParent( itemId );
    }

    //wxLogDebug("***** Depth = [%d]", depth);    

    if ( depth == 2 ) // table selected?
    {
        wxTreeItemData* itemData = m_sitesTree->GetItemData(selectedItem);
        TableTreeItemData* tableData = (TableTreeItemData*) itemData;
        selectedTable = tableData->GetTable();
        selectedSite = selectedTable->GetSite();
    }
    else if ( depth == 1 ) // site selected?
    {
        wxTreeItemData* itemData = m_sitesTree->GetItemData(selectedItem);
        SiteTreeItemData* siteData = (SiteTreeItemData*) itemData;
        selectedSite = siteData->GetSite();
        selectedTable = NULL;
    }
    else
    {
        selectedSite = NULL;
        selectedTable = NULL;
    }

    /// --------
    //if ( selectedItem.IsOk() )
    //{
    //    // Check first first-level children.
    //    if ( m_sitesTree->GetItemParent( selectedItem ) == rootId )
    //    {
    //        wxTreeItemData* itemData = m_sitesTree->GetItemData(selectedItem);
    //        SiteTreeItemData* siteData = (SiteTreeItemData*) itemData;
    //        selectedSite = siteData->GetSite();
    //    }
    //}

    return selectedSite;
}

/************************* END OF FILE ***************************************/
