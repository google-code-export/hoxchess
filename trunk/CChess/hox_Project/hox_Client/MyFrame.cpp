/////////////////////////////////////////////////////////////////////////////
// Name:            MyFrame.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/02/2007
/////////////////////////////////////////////////////////////////////////////

#include "MyFrame.h"
#include "MyChild.h"
#include "MyApp.h"    // To access wxGetApp()
#include "hoxTable.h"
#include "hoxPlayer.h"
#include "hoxTableMgr.h"
#include "hoxPlayerMgr.h"
#include "hoxTablesDialog.h"
#include "hoxUtility.h"
#include "hoxWWWThread.h"

#if !defined(__WXMSW__)
    #include "icons/sample.xpm"
#endif

#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/help.xpm"


/* A LOCAL player that represents this host to connect
 * with a single remote server.
 * TODO: Currently, this App can only connect 1 remote server!!!
 * FIXME: Who will cleanup me???
 */
hoxLocalPlayer* gs_localPlayer  = NULL;

/* Define my custom events */
DEFINE_EVENT_TYPE(hoxEVT_FRAME_LOG_MSG)

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

#define ID_WINDOW_LOG    103


BEGIN_EVENT_TABLE(MyFrame, wxMDIParentFrame)
    EVT_MENU(MDI_ABOUT, MyFrame::OnAbout)
    EVT_MENU(MDI_NEW_WINDOW, MyFrame::OnNewWindow)

    EVT_MENU(MDI_OPEN_SERVER, MyFrame::OnOpenServer)
    EVT_MENU(MDI_CONNECT_SERVER, MyFrame::OnConnectServer)
    //EVT_MENU(MDI_QUERY_TABLES, MyFrame::OnQueryTables)
    EVT_MENU(MDI_DISCONNECT_SERVER, MyFrame::OnDisconnectServer)

    EVT_MENU(MDI_CONNECT_WWW_SERVER, MyFrame::OnConnectWWWServer)

    EVT_MENU(MDI_QUIT, MyFrame::OnQuit)

    EVT_CLOSE(MyFrame::OnClose)
    EVT_SIZE(MyFrame::OnSize)
    EVT_SASH_DRAGGED(ID_WINDOW_LOG, MyFrame::OnSashDrag)

    //EVT_COMMAND(wxID_ANY, hoxEVT_WWW_RESPONSE, MyFrame::OnWWWResponse)
    EVT_COMMAND(wxID_ANY, hoxEVT_WWW_RESPONSE, MyFrame::OnMYResponse)
    EVT_COMMAND(wxID_ANY, hoxEVT_FRAME_LOG_MSG, MyFrame::OnFrameLogMsgEvent)

    EVT_SOCKET(CLIENT_SOCKET_ID,  MyFrame::OnClientSocketEvent)

END_EVENT_TABLE()

// ---------------------------------------------------------------------------
// MyFrame
// ---------------------------------------------------------------------------

// Define my frame constructor
MyFrame::MyFrame(wxWindow *parent,
                 const wxWindowID id,
                 const wxString& title,
                 const wxPoint& pos,
                 const wxSize& size,
                 const long style)
       : wxMDIParentFrame(parent, id, title, pos, size, style)
{
    const char* FNAME = "MyFrame::MyFrame";
    this->SetIcon(wxICON(sample));

    // A window containing our log-text.
    m_logWindow = new wxSashLayoutWindow(this, ID_WINDOW_LOG,
                               wxDefaultPosition, wxDefaultSize/*wxSize(200, 30)*/,
                               wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    m_logWindow->SetDefaultSize(wxSize(1000, 180));
    m_logWindow->SetOrientation(wxLAYOUT_HORIZONTAL);
    m_logWindow->SetAlignment(wxLAYOUT_BOTTOM);
    m_logWindow->SetBackgroundColour(wxColour(0, 0, 255));
    m_logWindow->SetSashVisible(wxSASH_TOP, true);

    m_logText = new wxTextCtrl( m_logWindow, wxID_ANY, _T(""),
                                wxDefaultPosition, wxDefaultSize,
                                wxTE_MULTILINE | wxSUNKEN_BORDER );

    // Create toolbar.
    CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_HORIZONTAL);
    InitToolBar(GetToolBar());

    // Accelerators
    wxAcceleratorEntry entries[3];
    entries[0].Set(wxACCEL_CTRL, (int) 'N', MDI_NEW_WINDOW);
    entries[1].Set(wxACCEL_CTRL, (int) 'X', MDI_QUIT);
    entries[2].Set(wxACCEL_CTRL, (int) 'A', MDI_ABOUT);
    wxAcceleratorTable accel(3, entries);
    SetAcceleratorTable(accel);

    // Progress dialog.
    m_dlgProgress = NULL;

    wxLogDebug(wxString::Format(_("[%d (%d)] %s: HOX Client ready."), 
                    wxThread::GetCurrentId, wxThread::IsMain(), FNAME));
}

MyFrame::~MyFrame()
{
}

void MyFrame::OnClose(wxCloseEvent& event)
{
    event.Skip();
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close();
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event) )
{
    wxMessageBox( wxString::Format(
                    _("Welcome to HOX Client!\n"
                      "Author: Huy Phan (c) 2007\n"
                      "\n"
                      "This application is powered by %s, running under %s."),
                    wxVERSION_STRING,
                    wxGetOsDescription().c_str()
                 ),
                 _("About HOX Client"),
                 wxOK | wxICON_INFORMATION,
                 this);
}

void MyFrame::OnNewWindow(wxCommandEvent& WXUNUSED(event) )
{
    const char* FNAME = "MyFrame::OnNewWindow";
    hoxTable* table = NULL;

    table = _CreateNewTable( "" );  // An Id will be generated.
    wxLogDebug(wxString::Format("%s: Created a new table [%s].", FNAME, table->GetId()));

    // Add the HOST player to the table.
    hoxResult result = wxGetApp().GetHostPlayer()->JoinTable( table );
    wxASSERT( result == hoxRESULT_OK  );
    wxASSERT_MSG( wxGetApp().GetHostPlayer()->HasRole( hoxRole(table->GetId(), 
                                                         hoxPIECE_COLOR_RED) ),
                  _("Player must play RED"));
}

void MyFrame::OnOpenServer(wxCommandEvent& WXUNUSED(event) )
{
    wxLogDebug(_("Open a server..."));
    wxGetApp().OpenServer();
}

void MyFrame::OnConnectServer(wxCommandEvent& WXUNUSED(event) )
{
    const char* FNAME = "MyFrame::OnConnectServer";
    hoxResult result;

    if ( m_dlgProgress != NULL ) 
    {
        m_dlgProgress->Destroy();  // NOTE: ... see wxWidgets' documentation.
        m_dlgProgress = NULL;
    }

    m_dlgProgress = new wxProgressDialog
                        (
                         _T("Progress dialog"),
                         _T("Wait until connnection is established or press [Cancel]"),
                         100,
                         this,
                         wxPD_AUTO_HIDE | wxPD_CAN_ABORT
                        );
    m_dlgProgress->SetSize( wxSize(500, 150) );
    m_dlgProgress->Pulse();

    wxLogDebug(_("Create a WWW LOCAL player to connect to a remote server..."));
    if ( wxGetApp().m_myPlayer != NULL )
    {
        wxLogDebug("Delete the existing MY player.");
        hoxPlayerMgr::GetInstance()->DeletePlayer( wxGetApp().m_myPlayer );
        wxGetApp().m_myPlayer = NULL;
    }

    hoxNetworkTableInfoList tableList;
    wxString playerName = hoxUtility::GenerateRandomString();
    wxGetApp().m_myPlayer = 
        hoxPlayerMgr::GetInstance()->CreateMyPlayer( playerName );

    result = wxGetApp().m_myPlayer->ConnectToNetworkServer( 
                                "127.0.0.1", 
                                 hoxNETWORK_DEFAULT_SERVER_PORT,
                                 this );
    if ( result != hoxRESULT_OK )
    {
        wxLogError(wxString::Format("%s: Failed to connect to server.", FNAME));
        return;
    }

    m_dlgProgress->Pulse();
}

void MyFrame::OnConnectWWWServer(wxCommandEvent& WXUNUSED(event) )
{
    const char* FNAME = "MyFrame::OnConnectWWWServer";
    wxLogDebug(_("Connect to the WWW server..."));
    hoxResult result;

    if ( m_dlgProgress != NULL ) 
    {
        m_dlgProgress->Destroy();  // NOTE: ... see wxWidgets' documentation.
        m_dlgProgress = NULL;
    }

    m_dlgProgress = new wxProgressDialog
                        (
                         _T("Progress dialog"),
                         _T("Wait until connnection is established or press [Cancel]"),
                         100,
                         this,
                         wxPD_AUTO_HIDE | wxPD_CAN_ABORT
                        );
    m_dlgProgress->SetSize( wxSize(500, 150) );
    m_dlgProgress->Pulse();

    /* Create a WWW network player */

    wxLogDebug(_("Create a WWW LOCAL player to connect to a remote server..."));
    if ( wxGetApp().m_wwwLocalPlayer != NULL )
    {
        wxLogDebug("Delete the existing local player.");
        hoxPlayerMgr::GetInstance()->DeletePlayer( wxGetApp().m_wwwLocalPlayer );
        wxGetApp().m_wwwLocalPlayer = NULL;
    }

    hoxNetworkTableInfoList tableList;
    wxString playerName = hoxUtility::GenerateRandomString();
    wxGetApp().m_wwwLocalPlayer = 
        hoxPlayerMgr::GetInstance()->CreateWWWLocalPlayer( playerName );

    result = wxGetApp().m_wwwLocalPlayer->ConnectToNetworkServer( 
                                "www.playxiangqi.com", 
                                 80 /* port */,
                                 this );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("Failed to connect to WWW server.");
        return;
    }

    m_dlgProgress->Pulse();
}

void MyFrame::OnQueryTables(wxCommandEvent& WXUNUSED(event) )
{
    hoxResult result;
    hoxNetworkTableInfoList tableList;

    wxLogDebug(_("About to query for the list of tables..."));
    wxASSERT( gs_localPlayer != NULL );
    result = gs_localPlayer->QueryForNetworkTables( tableList );

    // Show tables.
    hoxTablesDialog tablesDlg( this, wxID_ANY, "Tables", tableList);
    tablesDlg.ShowModal();
    wxString selectedId = tablesDlg.GetSelectedId();
    wxLogDebug(wxString::Format(_("The selected ID = [%s]")), selectedId);

    hoxUtility::FreeNetworkTableInfoList( tableList );

    // Perform a "join table" action if a table is selected.
    if ( ! selectedId.empty() )
    {
        this->DoJoinTable( selectedId );
    }
}

void MyFrame::DoJoinTable(const wxString& tableId)
{
    hoxResult result;

    /***********************/
    /* Send a JOIN request */
    /***********************/

    wxLogDebug(wxString::Format(_("About to join the table [%s]..."), tableId));
    wxASSERT( gs_localPlayer != NULL );
    result = gs_localPlayer->JoinNetworkTable( tableId );

    if ( result != hoxRESULT_OK )
    {
        wxLogWarning(_("Failed to send Join-Table command."));
        return;
    }

    /***********************/
    /* Create a new table. */
    /***********************/

    wxLogDebug(_T("Create a new table to play with network player."));
    hoxTable* table = _CreateNewTable( tableId );

    /***********************/
    /* Setup players       */
    /***********************/

    /* Right now, assume that the network ('remote') player is always RED
     * because he owns the table.
     */

    /* Just create a 'proxy' player that represents the 
     * remote network player.
     */

    // The remote player must joint first because the API JoinTable()
    // below will try to fill the RED position first.
    hoxPlayer* red_player = 
            hoxPlayerMgr::GetInstance()->CreatePlayer("(Dummy) Red Player",
                                                      hoxPLAYER_TYPE_DUMMY );
    result = red_player->JoinTable( table );
    wxASSERT( result == hoxRESULT_OK  );
    wxASSERT_MSG( red_player->HasRole( hoxRole(table->GetId(), 
                                               hoxPIECE_COLOR_RED) ),
                  _("Player must play RED"));

    // NOTE: The App's player will play BLACK.
    //       It should be noticed that a HOST player is able
    //       to play/view multiple tables at the same time.

    hoxPlayer* black_player = gs_localPlayer;
    result = black_player->JoinTable( table );
    wxASSERT( result == hoxRESULT_OK  );
    wxASSERT_MSG( black_player->HasRole( hoxRole(table->GetId(), 
                                                 hoxPIECE_COLOR_BLACK) ),
                  _("Player must play BLACK"));

    // Change the view side in which BLACK is at the bottom of the screen.
    table->ToggleViewSide();
}

void MyFrame::DoJoinNewWWWTable(const wxString& tableId)
{
    hoxResult result;

    wxASSERT( wxGetApp().m_wwwLocalPlayer != NULL );

    /***********************/
    /* Create a new table. */
    /***********************/

    wxLogDebug(_T("Create a new table connecting to WWW server..."));
    hoxTable* table = _CreateNewTable( tableId );

    /***********************/
    /* Setup players       */
    /***********************/

    /* Since we open this NEW table, we will play RED.
     */

    hoxPlayer* red_player = wxGetApp().m_wwwLocalPlayer;

    result = red_player->JoinTable( table );
    wxASSERT( result == hoxRESULT_OK  );
    wxASSERT_MSG( red_player->HasRole( hoxRole(table->GetId(), 
                                               hoxPIECE_COLOR_RED) ),
                  _("Player must play RED"));

    /* NOTE: The other player is <EMPTY> 
     */
}

void MyFrame::DoJoinExistingWWWTable(const hoxNetworkTableInfo& tableInfo)
{
    const char* FNAME = "MyFrame::DoJoinExistingWWWTable";
    hoxResult result;

    wxASSERT( wxGetApp().m_wwwLocalPlayer != NULL );

    /*******************************************************/
    /* Check to see which side (RED or BLACK) we will play
    /* and who the other player, if any, is.
    /*******************************************************/

    bool     playRed = false;   // Do I play RED?
    wxString otherPlayerId;     // Who is the other player?

    if ( tableInfo.redId == wxGetApp().m_wwwLocalPlayer->GetName() )
    {
        playRed = true;
        otherPlayerId = tableInfo.blackId;
    }
    else if ( tableInfo.blackId == wxGetApp().m_wwwLocalPlayer->GetName() )
    {
        playRed = false;
        otherPlayerId = tableInfo.redId;
    }
    else
    {
        wxLogError(wxString::Format("I should have secured a seat in table [%s].", FNAME, tableInfo.id));
        return;
    }

    /***********************/
    /* Create a new table. */
    /***********************/

    wxLogDebug(_("Create a new table JOINING an existing network table..."));
    hoxTable* table = _CreateNewTable( tableInfo.id );

    /***********************/
    /* Setup players       */
    /***********************/

    // The other player will be a DUMMY player.

    hoxPlayer* red_player = NULL;
    hoxPlayer* black_player = NULL;

    if ( playRed )  // Do I play RED?
    {
        red_player = wxGetApp().m_wwwLocalPlayer;
        if ( ! otherPlayerId.empty() )
        {
            black_player = hoxPlayerMgr::GetInstance()->CreatePlayer( 
                                                        otherPlayerId,
                                                        hoxPLAYER_TYPE_DUMMY );
        }
    }
    else
    {
        black_player = wxGetApp().m_wwwLocalPlayer;
        if ( ! otherPlayerId.empty() )
        {
            red_player = hoxPlayerMgr::GetInstance()->CreatePlayer( 
                                                        otherPlayerId,
                                                        hoxPLAYER_TYPE_DUMMY );
        }
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
}

void MyFrame::DoJoinExistingMYTable(const hoxNetworkTableInfo& tableInfo)
{
    const char* FNAME = "MyFrame::DoJoinExistingMYTable";
    hoxResult result;

    wxASSERT( wxGetApp().m_myPlayer != NULL );

    /*******************************************************/
    /* Check to see which side (RED or BLACK) we will play
    /* and who the other player, if any, is.
    /*******************************************************/

    bool     playRed = false;   // Do I play RED?
    wxString otherPlayerId;     // Who is the other player?

    if ( tableInfo.redId == wxGetApp().m_myPlayer->GetName() )
    {
        playRed = true;
        otherPlayerId = tableInfo.blackId;
    }
    else if ( tableInfo.blackId == wxGetApp().m_myPlayer->GetName() )
    {
        playRed = false;
        otherPlayerId = tableInfo.redId;
    }
    else
    {
        wxLogError(wxString::Format("I should have secured a seat in table [%s].", FNAME, tableInfo.id));
        return;
    }

    /***********************/
    /* Create a new table. */
    /***********************/

    wxLogDebug(_("Create a new table JOINING an existing network table..."));
    hoxTable* table = _CreateNewTable( tableInfo.id );

    /***********************/
    /* Setup players       */
    /***********************/

    // The other player will be a DUMMY player.

    hoxPlayer* red_player = NULL;
    hoxPlayer* black_player = NULL;

    if ( playRed )  // Do I play RED?
    {
        red_player = wxGetApp().m_myPlayer;
        if ( ! otherPlayerId.empty() )
        {
            black_player = hoxPlayerMgr::GetInstance()->CreatePlayer( 
                                                        otherPlayerId,
                                                        hoxPLAYER_TYPE_DUMMY );
        }
    }
    else
    {
        black_player = wxGetApp().m_myPlayer;
        if ( ! otherPlayerId.empty() )
        {
            red_player = hoxPlayerMgr::GetInstance()->CreatePlayer( 
                                                        otherPlayerId,
                                                        hoxPLAYER_TYPE_DUMMY );
        }
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

    // Start listen for new Moves.
    wxGetApp().m_myPlayer->StartListenForMoves( this );
}

void MyFrame::OnDisconnectServer(wxCommandEvent& WXUNUSED(event) )
{
    hoxResult result;

    wxLogDebug(_("About to disconnect from the remote server..."));
    if ( gs_localPlayer != NULL )
    {
        result = gs_localPlayer->DisconnectFromNetwork();
        wxLogDebug(_("Disconnected from the remote server."));
    }
}

void MyFrame::OnSize(wxSizeEvent& event)
{
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
}

void MyFrame::OnSashDrag(wxSashEvent& event)
{
    if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
        return;

    m_logWindow->SetDefaultSize(wxSize(1000, event.GetDragRect().height));

    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);

    // Leaves bits of itself behind sometimes
    GetClientWindow()->Refresh();
}

void MyFrame::InitToolBar(wxToolBar* toolBar)
{
    wxBitmap bitmaps[8];

    bitmaps[0] = wxBitmap( new_xpm );
    bitmaps[1] = wxBitmap( open_xpm );
    bitmaps[2] = wxBitmap( save_xpm );
    bitmaps[6] = wxBitmap( help_xpm );

    toolBar->AddTool(MDI_NEW_WINDOW, _T("New"), bitmaps[0], _T("New file"));
    toolBar->AddTool(1, _T("Open"), bitmaps[1], _T("Open file"));
    toolBar->AddTool(2, _T("Save"), bitmaps[2], _T("Save file"));
    toolBar->AddSeparator();
    toolBar->AddTool(MDI_ABOUT, _T("About"), bitmaps[6], _T("Help"));

    toolBar->Realize();
}

void
MyFrame::SetupMenu()
{
    // File menu.
    wxMenu *file_menu = new wxMenu;

    file_menu->Append(MDI_NEW_WINDOW, _T("&New window\tCtrl-N"), _T("Create a new child window"));
    file_menu->AppendSeparator();
    file_menu->Append(MDI_OPEN_SERVER, _T("&Open Server\tCtrl-O"), _T("Open server for remote access"));
    file_menu->Append(MDI_CONNECT_SERVER, _T("Connect Server\tCtrl-L"), _T("Connect to remote server"));
    //file_menu->Append(MDI_QUERY_TABLES, _T("&Query Tables\t"), _T("Query for list of tables"));
    file_menu->Append(MDI_DISCONNECT_SERVER, _T("&Disconnect Server\tCtrl-D"), _T("Disconnect from remote server"));
    file_menu->AppendSeparator();
    file_menu->Append(MDI_QUIT, _T("&Exit\tAlt-X"), _T("Quit the program"));

    // Server menu.
    wxMenu *server_menu = new wxMenu;

    server_menu->Append(MDI_CONNECT_WWW_SERVER, _T("Connect WWW Serve&r\tCtrl-R"), _T("Connect to remote WWW server"));

    // Help menu.
    wxMenu *help_menu = new wxMenu;
    help_menu->Append(MDI_ABOUT, _T("&About\tF1"));

    wxMenuBar *menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, _T("&File"));
    menu_bar->Append(server_menu, _T("&Server"));
    menu_bar->Append(help_menu, _T("&Help"));

    // Associate the menu bar with the frame
    this->SetMenuBar(menu_bar);
}

void
MyFrame::SetupStatusBar()
{
    this->CreateStatusBar();
}

void 
MyFrame::OnWWWResponse(wxCommandEvent& event) 
{
    const char* FNAME = "MyFrame::OnWWWResponse";

    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId, wxThread::IsMain(), FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    bool wasCanceled = !m_dlgProgress->Pulse();
    m_dlgProgress->Update(100);  // make sure to close the dialog.
    if ( wasCanceled )
    {
        wxLogDebug("%s: Connection has been canceled.", FNAME);
        return;
    }

    switch ( response->type )
    {
        case hoxREQUEST_TYPE_CONNECT:
            _OnWWWResponse_Connect( response->content );
            break;

        case hoxREQUEST_TYPE_LIST:
            _OnWWWResponse_List( response->content );
            break;

        case hoxREQUEST_TYPE_NEW:
            _OnWWWResponse_New( response->content );
            break;

        case hoxREQUEST_TYPE_JOIN:
            _OnWWWResponse_Join( response->content );
            break;

        case hoxREQUEST_TYPE_LEAVE:
            _OnWWWResponse_Leave( response->content );
            break;

        default:
            wxLogError("%s: Unknown type [%d].", response->type );
            break;
    }
}

void 
MyFrame::OnMYResponse(wxCommandEvent& event) 
{
    const char* FNAME = "MyFrame::OnMYResponse";

    wxLogDebug("[%d (%d)] %s: ENTER.", wxThread::GetCurrentId, wxThread::IsMain(), FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    bool wasCanceled = !m_dlgProgress->Pulse();
    m_dlgProgress->Update(100);  // make sure to close the dialog.
    if ( wasCanceled )
    {
        wxLogDebug("%s: Connection has been canceled.", FNAME);
        return;
    }

    switch ( response->type )
    {
        case hoxREQUEST_TYPE_CONNECT:
            _OnMYResponse_Connect( response->content );
            break;

        case hoxREQUEST_TYPE_LIST:
            _OnMYResponse_List( response->content );
            break;

        case hoxREQUEST_TYPE_NEW:
            _OnWWWResponse_New( response->content );
            break;

        case hoxREQUEST_TYPE_JOIN:
            _OnMYResponse_Join( response->content );
            break;

        case hoxREQUEST_TYPE_LEAVE:
            _OnWWWResponse_Leave( response->content );
            break;

        default:
            wxLogError("%s: Unknown type [%d].", response->type );
            break;
    }
}

void 
MyFrame::OnClientSocketEvent(wxSocketEvent& event)
{
    const char* FNAME = "MyFrame::OnClientSocketEvent";
    wxString msg;
    msg.Printf("%s: ", FNAME);

    switch(event.GetSocketEvent())
    {
        case wxSOCKET_INPUT      : msg.Append("wxSOCKET_INPUT"); break;
        case wxSOCKET_LOST       : msg.Append("wxSOCKET_LOST"); break;
        case wxSOCKET_CONNECTION : msg.Append("wxSOCKET_CONNECTION"); break;
        default                  : msg.Append("Unexpected event !"); break;
    }
    wxLogDebug(msg);

    //wxSocketBase* sock = event.GetSocket();

    wxGetApp().m_myPlayer->ProcessIncomingData( event );
}

void 
MyFrame::OnFrameLogMsgEvent( wxCommandEvent &event )
{
    wxString msg = event.GetString();
    m_logText->AppendText( msg << "\n");
}

void 
MyFrame::_OnWWWResponse_Connect( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnWWWResponse_Connect";
    int        returnCode = -1;
    wxString   returnMsg;
    hoxResult  result;

    wxLogDebug(wxString::Format("%s: Parsing SEND-CONNECT's response...", FNAME));

    result = hoxWWWThread::parse_string_for_simple_response( responseStr,
                                                             returnCode,
                                                             returnMsg );
    if ( result != hoxRESULT_OK )
    {
        wxLogError(wxString::Format("%s: Failed to parse for SEND-CONNECT's response.", FNAME));
        return;
    }
    else if ( returnCode != 0 )
    {
        wxLogError(wxString::Format("%s: Send CONNECT to server failed. [%s]", FNAME, returnMsg));
        return;
    }

    /* Get the list of tables from the WWW server */
    result = wxGetApp().m_wwwLocalPlayer->QueryForNetworkTables( this );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to query for LIST of tables from WWW server.", FNAME);
        return;
    }
}

void 
MyFrame::_OnMYResponse_Connect( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnMYResponse_Connect";
    int        returnCode = -1;
    wxString   returnMsg;
    hoxResult  result;

    wxLogDebug(wxString::Format("%s: Parsing SEND-CONNECT's response...", FNAME));

    result = hoxWWWThread::parse_string_for_simple_response( responseStr,
                                                             returnCode,
                                                             returnMsg );
    if ( result != hoxRESULT_OK )
    {
        wxLogError(wxString::Format("%s: Failed to parse for SEND-CONNECT's response.", FNAME));
        return;
    }
    else if ( returnCode != 0 )
    {
        wxLogError(wxString::Format("%s: Send CONNECT to server failed. [%s]", FNAME, returnMsg));
        return;
    }

    /* Get the list of tables from the WWW server */
    result = wxGetApp().m_myPlayer->QueryForNetworkTables( this );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to query for LIST of tables from the server.", FNAME);
        return;
    }
}

void 
MyFrame::_OnMYResponse_List( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnMYResponse_List";
    wxLogDebug(wxString::Format("%s: Parsing SEND-LIST's response...", FNAME));
    hoxNetworkTableInfoList tableList;
    hoxResult result;

    result = hoxWWWThread::parse_string_for_network_tables( responseStr,
                                                            tableList );
    if ( result != hoxRESULT_OK )
    {
        wxLogError(wxString::Format("%s: Failed to parse for SEND-LIST's response.", FNAME));
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
            wxLogDebug(wxString::Format("%s: Ask the server to allow me to JOIN table = [%s]", FNAME, selectedId));
            hoxNetworkTableInfo tableInfo;
            result = wxGetApp().m_myPlayer->JoinNetworkTable( selectedId, this );
            if ( result != hoxRESULT_OK )
            {
                wxLogError(wxString::Format("%s: Failed to JOIN a network table [%s].", FNAME, selectedId));
            }
            break;
        }

        case hoxTablesDialog::COMMAND_ID_NEW:
        {
            wxLogDebug("Ask the WWW server to open a new table.");
            wxString newTableId;
            result = wxGetApp().m_wwwLocalPlayer->OpenNewNetworkTable( this );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("Failed to open NEW network table.");
            }
            break;
        }

        default:
            wxLogDebug(wxString::Format("%s: No command is selected. Fine.", FNAME));
            break;
    }

    hoxUtility::FreeNetworkTableInfoList( tableList );
}

void 
MyFrame::_OnMYResponse_Join( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnMYResponse_Join";
    wxLogDebug(wxString::Format("%s: Parsing SEND-JOIN's response...", FNAME));
    hoxNetworkTableInfo tableInfo;
    hoxResult result;

    result = hoxWWWThread::parse_string_for_join_network_table( responseStr,
                                                                tableInfo );
    if ( result != hoxRESULT_OK )
    {
        wxLogError(wxString::Format("%s: Failed to parse for SEND-JOIN's response.", FNAME));
        return;
    }
    else
    {
        wxLogDebug(wxString::Format("Successfully joined the network table [%s].", tableInfo.id));
        this->DoJoinExistingMYTable( tableInfo );
    }
}

void 
MyFrame::_OnWWWResponse_List( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnWWWResponse_List";
    wxLogDebug(wxString::Format("%s: Parsing SEND-LIST's response...", FNAME));
    hoxNetworkTableInfoList tableList;
    hoxResult result;

    result = hoxWWWThread::parse_string_for_network_tables( responseStr,
                                                            tableList );
    if ( result != hoxRESULT_OK )
    {
        wxLogError(wxString::Format("%s: Failed to parse for SEND-LIST's response.", FNAME));
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
            wxLogDebug(wxString::Format(_("Ask WWW server to allow me to JOIN table = [%s]")), selectedId);
            hoxNetworkTableInfo tableInfo;
            result = wxGetApp().m_wwwLocalPlayer->JoinNetworkTable( selectedId, this );
            if ( result != hoxRESULT_OK )
            {
                wxLogError(wxString::Format("%s: Failed to JOIN a network table [%s].", FNAME, selectedId));
            }
            break;
        }

        case hoxTablesDialog::COMMAND_ID_NEW:
        {
            wxLogDebug("Ask the WWW server to open a new table.");
            wxString newTableId;
            result = wxGetApp().m_wwwLocalPlayer->OpenNewNetworkTable( this );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("Failed to open NEW network table.");
            }
            break;
        }

        default:
            wxLogDebug("No command is selected. Fine.");
            break;
    }

    hoxUtility::FreeNetworkTableInfoList( tableList );
}

void 
MyFrame::_OnWWWResponse_New( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnWWWResponse_New";
    wxLogDebug(wxString::Format("%s: Parsing SEND-NEW's response...", FNAME));
    wxString newTableId;
    hoxResult result;

    result = hoxWWWThread::parse_string_for_new_network_table( responseStr,
                                                               newTableId );
    if ( result != hoxRESULT_OK )
    {
        wxLogError(wxString::Format("%s: Failed to parse for SEND-NEW's response.", FNAME));
        return;
    }

    wxLogDebug(wxString::Format("WWW server created a new table with ID = [%s].", newTableId));
    this->DoJoinNewWWWTable( newTableId );
}

void 
MyFrame::_OnWWWResponse_Join( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnWWWResponse_Join";
    wxLogDebug(wxString::Format("%s: Parsing SEND-JOIN's response...", FNAME));
    hoxNetworkTableInfo tableInfo;
    hoxResult result;

    result = hoxWWWThread::parse_string_for_join_network_table( responseStr,
                                                                tableInfo );
    if ( result != hoxRESULT_OK )
    {
        wxLogError(wxString::Format("%s: Failed to parse for SEND-JOIN's response.", FNAME));
        return;
    }
    else
    {
        wxLogDebug(wxString::Format("Successfully joined the network table [%s].", tableInfo.id));
        this->DoJoinExistingWWWTable( tableInfo );
    }
}

void 
MyFrame::_OnWWWResponse_Leave( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnWWWResponse_Leave";
    int        returnCode = -1;
    wxString   returnMsg;
    hoxResult  result;

    wxLogDebug(wxString::Format("%s: Parsing SEND-LEAVE's response...", FNAME));

    result = hoxWWWThread::parse_string_for_simple_response( responseStr,
                                                             returnCode,
                                                             returnMsg );
    if ( result != hoxRESULT_OK )
    {
        wxLogError(wxString::Format("%s: Failed to parse for SEND-LEAVE's response.", FNAME));
        return;
    }
    else if ( returnCode != 0 )
    {
        wxLogError(wxString::Format("%s: Send LEAVE to server failed. [%s]", FNAME, returnMsg));
        return;
    }
}

hoxTable* 
MyFrame::_CreateNewTable( const wxString& tableId )
{
    hoxTable* newTable = NULL;
    MyChild*  subframe = NULL;
    wxString  tableTitle;

    wxGetApp().m_nChildren++; // for tracking purpose.

    tableTitle = tableId;  // Use the table's Id as the Title.

    // Generate a table's table if required.
    if ( tableTitle.empty() )
    {
        tableTitle.Printf(_("%d"), wxGetApp().m_nChildren );
    }

    subframe = new MyChild( this, tableTitle );

    // Create a new table. Use the title as the table-id.
    newTable = hoxTableMgr::GetInstance()->CreateTable( subframe, tableTitle );
    subframe->SetTable( newTable );

    subframe->Show( true );
    return newTable;
}

/************************* END OF FILE ***************************************/
