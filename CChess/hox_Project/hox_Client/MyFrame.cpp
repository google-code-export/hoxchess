/////////////////////////////////////////////////////////////////////////////
// Name:            MyFrame.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/02/2007
//
// Description:     The main Frame of the App.
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
#include "hoxNetworkAPI.h"

#if !defined(__WXMSW__)
    #include "icons/sample.xpm"
#endif

#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/help.xpm"


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
    EVT_MENU(MDI_DISCONNECT_SERVER, MyFrame::OnDisconnectServer)

    EVT_MENU(MDI_CONNECT_HTTP_SERVER, MyFrame::OnConnectHTTPServer)

    EVT_MENU(MDI_QUIT, MyFrame::OnQuit)

    EVT_CLOSE(MyFrame::OnClose)
    EVT_SIZE(MyFrame::OnSize)
    EVT_SASH_DRAGGED(ID_WINDOW_LOG, MyFrame::OnSashDrag)

    EVT_COMMAND(wxID_ANY, hoxEVT_HTTP_RESPONSE, MyFrame::OnHTTPResponse)
    EVT_COMMAND(wxID_ANY, hoxEVT_CONNECTION_RESPONSE, MyFrame::OnMYResponse)
    EVT_COMMAND(wxID_ANY, hoxEVT_FRAME_LOG_MSG, MyFrame::OnFrameLogMsgEvent)

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

    wxLogDebug("%s: ENTER.", FNAME);

    SetIcon(wxICON(sample));

    // A window containing our log-text.
    m_logWindow = new wxSashLayoutWindow(this, ID_WINDOW_LOG,
                               wxDefaultPosition, wxDefaultSize,
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

    m_nChildren = 0;

    wxLogDebug("%s: HOX Chess ready.", FNAME);
}

MyFrame::~MyFrame()
{
}

void MyFrame::OnClose(wxCloseEvent& event)
{
    while( ! m_children.empty() )
    {
        MyChild* child = m_children.front();
        child->Close( true /* force */ );
        // NOTE: The call above already delete the child.
    }
    event.Skip();
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close();
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event) )
{
    wxMessageBox( wxString::Format(
                    _("Welcome to HOX Chess!\n"
                      "\n"
                      "Author: Huy Phan (c) 2007\n"
                      "Version: 0.0.2.0\n"
                      "\n"
                      "This application is powered by %s, running under %s."),
                    wxVERSION_STRING,
                    wxGetOsDescription().c_str()
                 ),
                 _("About HOX Chess"),
                 wxOK | wxICON_INFORMATION,
                 this);
}

void MyFrame::OnNewWindow(wxCommandEvent& WXUNUSED(event) )
{
    const char* FNAME = "MyFrame::OnNewWindow";
    hoxTable* table = NULL;

    table = _CreateNewTable( "" );  // An Id will be generated.
    wxLogDebug("%s: Created a new table [%s].", FNAME, table->GetId());

    // Add the HOST player to the table.
    hoxResult result = wxGetApp().GetHostPlayer()->JoinTable( table );
    wxASSERT( result == hoxRESULT_OK  );
    wxASSERT_MSG( wxGetApp().GetHostPlayer()->HasRole( hoxRole(table->GetId(), 
                                                       hoxPIECE_COLOR_RED) ),
                  _("Player must play RED"));
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

void MyFrame::OnOpenServer(wxCommandEvent& WXUNUSED(event) )
{
    const char* FNAME = "MyFrame::OnOpenServer";
    wxLogDebug("%s: ENTER.", FNAME);
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
                         _("Progress dialog"),
                         _("Wait until connnection is established or press [Cancel]"),
                         100,
                         this,
                         wxPD_AUTO_HIDE | wxPD_CAN_ABORT
                        );
    m_dlgProgress->SetSize( wxSize(500, 150) );
    m_dlgProgress->Pulse();

    hoxNetworkTableInfoList tableList;
    hoxMyPlayer* myPlayer = wxGetApp().GetMyPlayer();

    result = myPlayer->ConnectToNetworkServer( "127.0.0.1", 
                                               hoxNETWORK_DEFAULT_SERVER_PORT,
                                               this );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to connect to server.", FNAME);
        return;
    }

    m_dlgProgress->Pulse();
}

void MyFrame::OnConnectHTTPServer(wxCommandEvent& WXUNUSED(event) )
{
    const char* FNAME = "MyFrame::OnConnectHTTPServer";
    hoxResult result;

    wxLogDebug("%s: ENTER.", FNAME);

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

    hoxNetworkTableInfoList tableList;
    hoxHttpPlayer* httpPlayer = wxGetApp().GetHTTPPlayer();

    result = httpPlayer->ConnectToNetworkServer( HOX_HTTP_SERVER_HOSTNAME, 
                                                 HOX_HTTP_SERVER_PORT,
                                                 this );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%: Failed to connect to HTTP server.", FNAME);
        return;
    }

    m_dlgProgress->Pulse();
}

void MyFrame::DoJoinNewHTTPTable(const wxString& tableId)
{
    const char* FNAME = "MyFrame::DoJoinNewHTTPTable";
    hoxResult result;

    wxLogDebug("%s: ENTER.", FNAME);

    /***********************/
    /* Create a new table. */
    /***********************/

    wxLogDebug("%s: Creating a new table connecting to HTTP server.", FNAME);
    hoxTable* table = _CreateNewTable( tableId );

    /***********************/
    /* Setup players       */
    /***********************/

    /* Since we open this NEW table, we will play RED.
     */

    hoxPlayer* red_player = wxGetApp().GetHTTPPlayer();

    result = red_player->JoinTable( table );
    wxASSERT( result == hoxRESULT_OK  );
    wxASSERT_MSG( red_player->HasRole( hoxRole(table->GetId(), 
                                               hoxPIECE_COLOR_RED) ),
                  "Player must play RED");

    /* NOTE: The other player is <EMPTY> 
     */
}

void MyFrame::DoJoinExistingHTTPTable(const hoxNetworkTableInfo& tableInfo)
{
    const char* FNAME = "MyFrame::DoJoinExistingHTTPTable";
    hoxResult result;

    hoxHttpPlayer* httpPlayer = wxGetApp().GetHTTPPlayer();

    /*******************************************************/
    /* Check to see which side (RED or BLACK) we will play
    /* and who the other player, if any, is.
    /*******************************************************/

    bool     playRed = false;   // Do I play RED?
    wxString otherPlayerId;     // Who is the other player?

    if ( tableInfo.redId == httpPlayer->GetName() )
    {
        playRed = true;
        otherPlayerId = tableInfo.blackId;
    }
    else if ( tableInfo.blackId == httpPlayer->GetName() )
    {
        playRed = false;
        otherPlayerId = tableInfo.redId;
    }
    else
    {
        wxLogError("%s: I should have secured a seat in table [%s].", FNAME, tableInfo.id);
        return;
    }

    /***********************/
    /* Create a new table. */
    /***********************/

    wxLogDebug("%s: Creating a new table JOINING an existing network table...", FNAME);
    hoxTable* table = _CreateNewTable( tableInfo.id );

    /***********************/
    /* Setup players       */
    /***********************/

    // The other player will be a DUMMY player.

    hoxPlayer* red_player = NULL;
    hoxPlayer* black_player = NULL;

    if ( playRed )  // Do I play RED?
    {
        red_player = httpPlayer;
        if ( ! otherPlayerId.empty() )
        {
            black_player = hoxPlayerMgr::GetInstance()->CreatePlayer( 
                                                        otherPlayerId,
                                                        hoxPLAYER_TYPE_DUMMY );
        }
    }
    else
    {
        black_player = httpPlayer;
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

    hoxMyPlayer* myPlayer = wxGetApp().GetMyPlayer();

    /*******************************************************/
    /* Check to see which side (RED or BLACK) we will play
    /* and who the other player, if any, is.
    /*******************************************************/

    bool     playRed = false;   // Do I play RED?
    wxString otherPlayerId;     // Who is the other player?

    if ( tableInfo.redId == myPlayer->GetName() )
    {
        playRed = true;
        otherPlayerId = tableInfo.blackId;
    }
    else if ( tableInfo.blackId == myPlayer->GetName() )
    {
        playRed = false;
        otherPlayerId = tableInfo.redId;
    }
    else
    {
        wxLogError("%s: I should have secured a seat in table [%s].", FNAME, tableInfo.id);
        return;
    }

    /***********************/
    /* Create a new table. */
    /***********************/

    wxLogDebug("%s: Creating a new table JOINING an existing network table...", FNAME);
    hoxTable* table = _CreateNewTable( tableInfo.id );

    /***********************/
    /* Setup players       */
    /***********************/

    // The other player will be a DUMMY player.

    hoxPlayer* red_player = NULL;
    hoxPlayer* black_player = NULL;

    if ( playRed )  // Do I play RED?
    {
        red_player = myPlayer;
        if ( ! otherPlayerId.empty() )
        {
            black_player = hoxPlayerMgr::GetInstance()->CreatePlayer( 
                                                        otherPlayerId,
                                                        hoxPLAYER_TYPE_DUMMY );
        }
    }
    else
    {
        black_player = myPlayer;
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
    myPlayer->StartListenForMoves();
}

void MyFrame::DoJoinNewMYTable(const wxString& tableId)
{
    const char* FNAME = "MyFrame::DoJoinNewMYTable";
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

    hoxPlayer* red_player = wxGetApp().GetMyPlayer();

    result = red_player->JoinTable( table );
    wxASSERT( result == hoxRESULT_OK  );
    wxASSERT_MSG( red_player->HasRole( hoxRole(table->GetId(), 
                                               hoxPIECE_COLOR_RED) ),
                  _("Player must play RED"));

    /* NOTE: The other player is <EMPTY> 
     */
}

void MyFrame::OnDisconnectServer(wxCommandEvent& WXUNUSED(event) )
{
    const char* FNAME = "MyFrame::OnDisconnectServer";
    wxLogDebug("%s: ENTER. *** Do nothing. ***", FNAME);
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
    file_menu->Append(MDI_DISCONNECT_SERVER, _T("&Disconnect Server\tCtrl-D"), _T("Disconnect from remote server"));
    file_menu->AppendSeparator();
    file_menu->Append(MDI_QUIT, _T("&Exit\tAlt-X"), _T("Quit the program"));

    // Server menu.
    wxMenu *server_menu = new wxMenu;

    server_menu->Append(MDI_CONNECT_HTTP_SERVER, _T("Connect HTTP Serve&r\tCtrl-R"), _T("Connect to remote HTTP server"));

    // Help menu.
    wxMenu *help_menu = new wxMenu;
    help_menu->Append(MDI_ABOUT, _T("&About\tF1"));

    wxMenuBar *menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, _T("&File"));
    menu_bar->Append(server_menu, _T("&Server"));
    menu_bar->Append(help_menu, _T("&Help"));

    // Associate the menu bar with the frame
    SetMenuBar(menu_bar);
}

void
MyFrame::SetupStatusBar()
{
    CreateStatusBar();
}

void 
MyFrame::OnHTTPResponse(wxCommandEvent& event) 
{
    const char* FNAME = "MyFrame::OnHTTPResponse";

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    bool wasCanceled = !m_dlgProgress->Pulse();
    m_dlgProgress->Update(100);  // make sure to close the dialog.
    if ( wasCanceled )
    {
        wxLogDebug("%s: Connection has been canceled.", FNAME);
        return;
    }

    if ( response->code != hoxRESULT_OK )
    {
        wxLogDebug("%s: The response's code is ERROR. END.", FNAME);
        return;
    }

    switch ( response->type )
    {
        case hoxREQUEST_TYPE_CONNECT:
            _OnHTTPResponse_Connect( response->content );
            break;

        case hoxREQUEST_TYPE_LIST:
            _OnHTTPResponse_List( response->content );
            break;

        case hoxREQUEST_TYPE_NEW:
            _OnHTTPResponse_New( response->content );
            break;

        case hoxREQUEST_TYPE_JOIN:
            _OnHTTPResponse_Join( response->content );
            break;

        case hoxREQUEST_TYPE_LEAVE:
            _OnHTTPResponse_Leave( response->content );
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

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    bool wasCanceled = !m_dlgProgress->Pulse();
    m_dlgProgress->Update(100);  // make sure to close the dialog.
    if ( wasCanceled )
    {
        wxLogDebug("%s: Connection has been canceled.", FNAME);
        return;
    }

    if ( response->code != hoxRESULT_OK )
    {
        wxLogDebug("%s: The response's code is ERROR. END.", FNAME);
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
            _OnMYResponse_New( response->content );
            break;

        case hoxREQUEST_TYPE_JOIN:
            _OnMYResponse_Join( response->content );
            break;

        case hoxREQUEST_TYPE_LEAVE:
            _OnHTTPResponse_Leave( response->content );
            break;

        default:
            wxLogError("%s: Unknown type [%d].", response->type );
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
MyFrame::_OnHTTPResponse_Connect( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnHTTPResponse_Connect";
    int        returnCode = -1;
    wxString   returnMsg;
    hoxResult  result;

    wxLogDebug("%s: Parsing SEND-CONNECT's response...", FNAME);

    result = hoxNetworkAPI::ParseSimpleResponse( responseStr,
                                                 returnCode,
                                                 returnMsg );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse for SEND-CONNECT's response.", FNAME);
        return;
    }
    else if ( returnCode != 0 )
    {
        wxLogError("%s: Send CONNECT to server failed. [%s]", FNAME, returnMsg);
        return;
    }

    /* Get the list of tables from the HTTP server */
    result = wxGetApp().GetHTTPPlayer()->QueryForNetworkTables( this );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to query for LIST of tables from HTTP server.", FNAME);
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

    wxLogDebug("%s: Parsing SEND-CONNECT's response...", FNAME);

    result = hoxNetworkAPI::ParseSimpleResponse( responseStr,
                                                 returnCode,
                                                 returnMsg );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse for SEND-CONNECT's response.", FNAME);
        return;
    }
    else if ( returnCode != 0 )
    {
        wxLogError("%s: Send CONNECT to server failed. [%s]", FNAME, returnMsg);
        return;
    }

    /* Get the list of tables from the server */
    result = wxGetApp().GetMyPlayer()->QueryForNetworkTables( this );
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
            wxLogDebug("%s: Ask the server to allow me to JOIN table = [%s]", FNAME, selectedId);
            hoxNetworkTableInfo tableInfo;
            result = wxGetApp().GetMyPlayer()->JoinNetworkTable( selectedId, this );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to JOIN a network table [%s].", FNAME, selectedId);
            }
            break;
        }

        case hoxTablesDialog::COMMAND_ID_NEW:
        {
            wxLogDebug("%s: Ask the server to open a new table.", FNAME);
            wxString newTableId;
            result = wxGetApp().GetMyPlayer()->OpenNewNetworkTable( this );
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
MyFrame::_OnMYResponse_Join( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnMYResponse_Join";
    wxLogDebug("%s: Parsing SEND-JOIN's response...", FNAME);
    hoxNetworkTableInfo tableInfo;
    hoxResult result;

    result = hoxNetworkAPI::ParseJoinNetworkTable( responseStr,
                                                   tableInfo );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse for SEND-JOIN's response [%s].", FNAME, responseStr);
        return;
    }
    else
    {
        wxLogDebug("Successfully joined the network table [%s].", tableInfo.id);
        this->DoJoinExistingMYTable( tableInfo );
    }
}

void 
MyFrame::_OnMYResponse_New( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnMYResponse_New";
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

    wxLogDebug("The server created a new table with ID = [%s].", newTableId);
    this->DoJoinNewMYTable( newTableId );
}

void 
MyFrame::_OnHTTPResponse_List( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnHTTPResponse_List";
    hoxNetworkTableInfoList tableList;
    hoxResult result;

    wxLogDebug("%s: Parsing SEND-LIST's response...", FNAME);

    result = hoxNetworkAPI::ParseNetworkTables( responseStr,
                                                tableList );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse SEND-LIST's response.", FNAME);
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
            wxLogDebug("%s: Asking HTTP server to allow me to JOIN table = [%s]...", FNAME, selectedId);
            hoxNetworkTableInfo tableInfo;
            result = wxGetApp().GetHTTPPlayer()->JoinNetworkTable( selectedId, this );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to JOIN the network table [%s].", FNAME, selectedId);
            }
            break;
        }

        case hoxTablesDialog::COMMAND_ID_NEW:
        {
            wxLogDebug("%s: Asking the HTTPserver to open a new table...", FNAME);
            wxString newTableId;
            result = wxGetApp().GetHTTPPlayer()->OpenNewNetworkTable( this );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to open NEW network table.", FNAME);
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
MyFrame::_OnHTTPResponse_New( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnHTTPResponse_New";
    wxString newTableId;
    hoxResult result;

    wxLogDebug("%s: Parsing SEND-NEW's response...", FNAME);

    result = hoxNetworkAPI::ParseNewNetworkTable( responseStr,
                                                  newTableId );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse SEND-NEW's response.", FNAME);
        return;
    }

    wxLogDebug("%s: HTTP server created a new table with ID = [%s].", FNAME, newTableId);
    this->DoJoinNewHTTPTable( newTableId );
}

void 
MyFrame::_OnHTTPResponse_Join( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnHTTPResponse_Join";
    hoxNetworkTableInfo tableInfo;
    hoxResult result;

    wxLogDebug("%s: Parsing SEND-JOIN's response...", FNAME);

    result = hoxNetworkAPI::ParseJoinNetworkTable( responseStr,
                                                   tableInfo );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse SEND-JOIN's response.", FNAME);
        return;
    }
    else
    {
        wxLogDebug("%s: Successfully joined the network table [%s].", FNAME, tableInfo.id);
        this->DoJoinExistingHTTPTable( tableInfo );
    }
}

void 
MyFrame::_OnHTTPResponse_Leave( const wxString& responseStr )
{
    const char* FNAME = "MyFrame::_OnHTTPResponse_Leave";
    int        returnCode = -1;
    wxString   returnMsg;
    hoxResult  result;

    wxLogDebug("%s: Parsing SEND-LEAVE's response...", FNAME);

    result = hoxNetworkAPI::ParseSimpleResponse( responseStr,
                                                 returnCode,
                                                 returnMsg );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse for SEND-LEAVE's response.", FNAME);
        return;
    }
    else if ( returnCode != 0 )
    {
        wxLogError("%s: Send LEAVE to server failed. [%s]", FNAME, returnMsg);
        return;
    }
}

hoxTable* 
MyFrame::_CreateNewTable( const wxString& tableId )
{
    hoxTable* newTable = NULL;
    MyChild*  subframe = NULL;
    wxString  tableTitle;

    m_nChildren++; // for tracking purpose.

    tableTitle = tableId;  // Use the table's Id as the Title.

    // Generate a table's table if required.
    if ( tableTitle.empty() )
    {
        tableTitle.Printf("%d", m_nChildren );
    }

    subframe = new MyChild( this, tableTitle );
    m_children.push_back( subframe );

    // Create a new table. Use the title as the table-id.
    newTable = hoxTableMgr::GetInstance()->CreateTable( subframe, tableTitle );
    subframe->SetTable( newTable );

    subframe->Show( true );
    return newTable;
}

/************************* END OF FILE ***************************************/
