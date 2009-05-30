/***************************************************************************
 *  Copyright 2007-2009 Huy Phan  <huyphan@playxiangqi.com>                *
 *                      Bharatendra Boddu (bharathendra at yahoo dot com)  *
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

#include "MyFrame.h"
#include "MyChild.h"
#include "MyApp.h"    // To access wxGetApp()
#include "hoxTable.h"
#include "hoxUtil.h"
#include "hoxLoginUI.h"
#include "hoxSitesUI.h"
#include "hoxPlayersUI.h"
#include "hoxOptionsUI.h"
#include "hoxBoard.h"
#include "hoxLog.h"
#include "hoxSavedTable.h"
#include "hoxAIPluginMgr.h"
#include "hoxWelcomeUI.h"
#include <wx/aboutdlg.h>

#if !defined(__WXMSW__)
    #include "../resource/icons/hoxchess.xpm"
#endif

#include "../resource/bitmaps/new.xpm"
#include "../resource/bitmaps/help.xpm"
#include "../resource/bitmaps/quit.xpm"
#include "../resource/bitmaps/delete.xpm"
#include "../resource/bitmaps/folder_open.xpm"
#include "../resource/bitmaps/connect.xpm"
#include "../resource/bitmaps/disconnect.xpm"
#include "../resource/bitmaps/save.xpm"

IMPLEMENT_DYNAMIC_CLASS(MyFrame, wxMDIParentFrame)

BEGIN_EVENT_TABLE(MyFrame, wxMDIParentFrame)
    EVT_MENU(MDI_ABOUT,          MyFrame::OnAbout)
    EVT_MENU(MDI_NEW_TABLE,      MyFrame::OnNewTable)
    EVT_MENU(MDI_CLOSE_TABLE,    MyFrame::OnCloseTable)
    EVT_UPDATE_UI(MDI_CLOSE_TABLE, MyFrame::OnUpdateCloseTable)
	EVT_UPDATE_UI(MDI_NEW_TABLE, MyFrame::OnUpdateNewTable)

    EVT_MENU(MDI_DISCONNECT_SERVER,      MyFrame::OnDisconnectServer)
	EVT_UPDATE_UI(MDI_DISCONNECT_SERVER, MyFrame::OnUpdateRemoteSiteRequired)
    EVT_MENU(MDI_CONNECT_SERVER,         MyFrame::OnConnectServer)
    EVT_MENU(MDI_LIST_TABLES,            MyFrame::OnListTables)
	EVT_UPDATE_UI(MDI_LIST_TABLES,       MyFrame::OnUpdateRemoteSiteRequired)

    EVT_MENU(MDI_SHOW_SERVERS_WINDOW,      MyFrame::OnShowServersWindow)
    EVT_UPDATE_UI(MDI_SHOW_SERVERS_WINDOW, MyFrame::OnUpdateServersWindow)
    EVT_MENU(MDI_SHOW_LOG_WINDOW,          MyFrame::OnShowLogWindow)
    EVT_UPDATE_UI(MDI_SHOW_LOG_WINDOW,     MyFrame::OnUpdateShowLogWindow)

    EVT_MENU(MDI_PRACTICE,              MyFrame::OnPractice)
    EVT_UPDATE_UI(MDI_PRACTICE,         MyFrame::OnUpdatePractice)
    EVT_MENU(MDI_SAVE_TABLE,            MyFrame::OnSaveTable)
    EVT_MENU(MDI_OPEN_SAVED_TABLE,      MyFrame::OnOpenSavedTable)
    EVT_UPDATE_UI(MDI_OPEN_SAVED_TABLE, MyFrame::OnUpdateOpenSavedTable)

    EVT_MENU(MDI_QUIT, MyFrame::OnQuit)
    EVT_IDLE(MyFrame::OnIdle)
    EVT_CLOSE(MyFrame::OnClose)
    EVT_SIZE(MyFrame::OnSize)
    EVT_SASH_DRAGGED(ID_WINDOW_SITES, MyFrame::OnServersSashDrag)

    EVT_CONTEXT_MENU(MyFrame::OnContextMenu)
    EVT_MENU(MDI_SOUND,   MyFrame::OnToggleSound)
    EVT_MENU(MDI_OPTIONS, MyFrame::OnOptions)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------
// MyFrame
// ---------------------------------------------------------------------------

MyFrame::MyFrame( wxWindow*        parent,
                  const wxWindowID id,
                  const wxString&  title,
                  const wxPoint&   pos,
                  const wxSize&    size,
                  const long       style )
       : wxMDIParentFrame( parent, id, title, pos, size, style )
       , m_bShowWelcomeChecked( false )
{
    m_log = new hoxLog(this, _("Log Window"), false);

    SetIcon( wxICON(hoxchess) );

    _CreateSitesUI();

    // Create toolbar.
    CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_HORIZONTAL);
    InitToolBar(GetToolBar());

    // Accelerators
    wxAcceleratorEntry entries[2];
    entries[0].Set(wxACCEL_CTRL, (int) 'N', MDI_NEW_TABLE);
    entries[1].Set(wxACCEL_CTRL, (int) 'X', MDI_QUIT);
    wxAcceleratorTable accel(WXSIZEOF(entries), entries);
    SetAcceleratorTable(accel);

    SetMenuBar( MyFrame::Create_Menu_Bar() );
    // This shows that the standard window menu may be customized:
    wxMenu* const windowMenu = GetWindowMenu();
    if ( windowMenu )
    {
        windowMenu->Delete(wxID_MDI_WINDOW_ARRANGE_ICONS);
        SetWindowMenu(windowMenu);
    }

    CreateStatusBar();

	wxLogStatus("%s %s is ready.", HOX_APP_NAME, HOX_VERSION);
}

void
MyFrame::OnIdle(wxIdleEvent& event)
{
    if ( ! m_bShowWelcomeChecked )
    {
        m_bShowWelcomeChecked = true;
        if ( wxGetApp().GetOption("welcome") == "1" )
        {
            _ShowWelcomeDialog();
        }
    }
}

void 
MyFrame::OnClose(wxCloseEvent& event)
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    while( ! m_children.empty() )
    {
        MyChild* child = m_children.front();
        child->Close( true /* force */ );
        // NOTE: The call above already delete the child.
    }

    /* Inform all sites about the CLOSING. */
    wxGetApp().OnSystemClose();

	if ( hoxSiteManager::GetInstance()->GetNumberOfSites() > 0 )
    {
        // *** Delay the shutdown until all sites are closed.
        return;
    }

	/* Save the current layout. */
	wxGetApp().SaveDefaultFrameLayout( this->GetPosition(), 
		                               this->GetSize() );
	_SaveDefaultSitesLayout( m_sitesWindow->GetSize().x );

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
    wxAboutDialogInfo info;
    
    info.SetName( HOX_APP_NAME );
    info.SetVersion( HOX_VERSION );
    info.SetDescription( wxString::Format(
        _("An open source Xiangqi (Chinese Chess) desktop application.          \n"
          "Powered by %s, under %s      "
          ), wxVERSION_STRING, wxGetOsDescription().c_str() )
    );
    info.SetCopyright( "(C) 2007-2009 Huy Phan, PlayXiangqi" );
    info.AddDeveloper( "Huy Phan" );
    info.AddDeveloper( "Wangmao Lin" );
    info.AddDeveloper( "Darick Le" );
    info.SetWebSite( "http://hoxchess.googlecode.com", "HOXChess on GoogleCode" );
    info.SetLicence(
        "                GNU General Public License v3                          \n"
        "           =======================================                     \n"
        "\n"
        "  HOXChess is free software: you can redistribute it and/or modify     \n"
        "  it under the terms of the GNU General Public License as published by \n"
        "  the Free Software Foundation, either version 3 of the License, or    \n"
        "  (at your option) any later version.                                  \n"
        "\n"
        "  HOXChess is distributed in the hope that it will be useful,          \n"
        "  but WITHOUT ANY WARRANTY; without even the implied warranty of       \n"
        "  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        \n"
        "  GNU General Public License for more details.                         \n"
    );
    info.AddTranslator( "Kathleen Mak - Chinese (Simplified)" );
    info.AddTranslator( "Huy Phan - Vietnamese" );

    wxAboutBox(info);
}

void 
MyFrame::OnNewTable( wxCommandEvent& event )
{
    hoxSite* selectedSite = _GetSelectedSite();
    if ( selectedSite != NULL )
    {
        selectedSite->OnLocalRequest_NEW();
    }
}

void 
MyFrame::OnCloseTable( wxCommandEvent& event )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    MyChild* child = wxDynamicCast(this->GetActiveChild(), MyChild);
    if ( child != NULL )
    {
        wxLogDebug("%s: Closing the active Table [%s]...", __FUNCTION__, child->GetTitle().c_str());
        child->Close( true /* force */ );
    }
}

void 
MyFrame::OnUpdateNewTable( wxUpdateUIEvent& event )
{
	bool enableMenu = false;
    hoxSite* selectedSite = _GetSelectedSite();

    if ( selectedSite != NULL )
	{
		unsigned int actionFlags = selectedSite->GetCurrentActionFlags();
		if ( (actionFlags & hoxSITE_ACTION_NEW) != 0 )
		{
			enableMenu = true;
		}
	}

    event.Enable( enableMenu );
}

void
MyFrame::OnChildClose( wxCloseEvent& event,
                       MyChild*      child, 
                       hoxTable_SPtr pTable )
{
    wxCHECK_RET( pTable, "The table is NULL." );

    _SaveDefaultTableLayout( child->GetSize() );

    pTable->OnClose_FromSystem();
    m_children.remove( child );

    hoxSite* site = pTable->GetSite();
    site->CloseTable( pTable );

    event.Skip(); // let the search for the event handler continue...
}

void 
MyFrame::OnDisconnectServer( wxCommandEvent& event )
{
    hoxSite* selectedSite = _GetSelectedSite();
    if ( selectedSite == NULL )
    {
        wxLogDebug("%s: No site is selected. Do nothing. END.", __FUNCTION__);
        return;
    }

    /* Go through my children to see which ones belong to the site and
     * trigger a close event.
     */
    _CloseChildrenOfSite( selectedSite );

    selectedSite->Disconnect();
}

void 
MyFrame::OnUpdateRemoteSiteRequired(wxUpdateUIEvent& event)
{
    hoxSite*  selectedSite = _GetSelectedSite();
    event.Enable(   selectedSite != NULL
                  && selectedSite->GetType() != hoxSITE_TYPE_LOCAL );
}

void 
MyFrame::OnUpdateCloseTable(wxUpdateUIEvent& event)
{
    MyChild* child = wxDynamicCast( this->GetActiveChild(), MyChild );
    event.Enable( child != NULL );
}

void 
MyFrame::OnConnectServer( wxCommandEvent& event )
{
    hoxLoginUI loginDlg( this );
    if ( loginDlg.ShowModal() == wxID_CANCEL ) return;

    const hoxServerAddress serverAddress( loginDlg.m_sIP,
                                          ::atoi( loginDlg.m_sPort.c_str() ) );

    wxGetApp().ConnectToServer( loginDlg.m_siteType, serverAddress,
                                loginDlg.m_sUsername, loginDlg.m_sPassword );
}

void 
MyFrame::OnShowServersWindow( wxCommandEvent& event )
{
    m_sitesWindow->Show( ! m_sitesWindow->IsShown() );

    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);

    GetClientWindow()->Refresh(); // Leaves bits of itself behind sometimes
}

void 
MyFrame::OnUpdateServersWindow( wxUpdateUIEvent& event )
{
    event.Check( m_sitesWindow->IsShown() );
}

void 
MyFrame::OnShowLogWindow( wxCommandEvent& event )
{
    m_log->Show( event.IsChecked() );
}

void
MyFrame::OnUpdateShowLogWindow( wxUpdateUIEvent& event )
{
    event.Check( m_log->IsShown() );
}

void 
MyFrame::OnPractice( wxCommandEvent& event )
{
    hoxSite* selectedSite = _GetSelectedSite();
    if ( selectedSite != NULL )
    {
        selectedSite->OnLocalRequest_PRACTICE();
    }
}

void 
MyFrame::OnUpdatePractice( wxUpdateUIEvent& event )
{
	bool bEnabled = false;
    hoxSite* selectedSite = _GetSelectedSite();

    if ( selectedSite != NULL )
	{
		unsigned int actionFlags = selectedSite->GetCurrentActionFlags();
		if ( (actionFlags & hoxSITE_ACTION_PRACTICE) != 0 )
		{
			bEnabled = true;
		}
	}

    event.Enable( bEnabled );
}

void
MyFrame::OnSaveTable( wxCommandEvent& event )
{
    MyChild* child = wxDynamicCast( this->GetActiveChild(), MyChild );
    if ( child == NULL ) return;

	hoxTable_SPtr pTable = child->GetTable();

	const wxString fileName =
        wxFileSelector( _("Please choose a file to save"),
                        wxEmptyString,
                        wxEmptyString,
                        "xml",
                        "XML files (*.xml)|*.xml|All files (*)|*",
                        wxFD_SAVE|wxFD_OVERWRITE_PROMPT );
	if ( fileName.empty() )
		return;

    _SaveCurrentTableToDisk( pTable, fileName );
}

void
MyFrame::OnOpenSavedTable( wxCommandEvent& event )
{
    hoxSite* selectedSite = _GetSelectedSite();
    if ( selectedSite == NULL ) return;

    const wxString fileName =
        wxFileSelector( _("Please choose a file to open"),
                        wxEmptyString,
                        wxEmptyString,
                        "xml",
                        "XML files (*.xml)|*.xml|All files (*)|*",
                        wxFD_OPEN );
    if ( fileName.empty() )
	    return;

    selectedSite->OnLocalRequest_PRACTICE( fileName );
}

void 
MyFrame::OnUpdateOpenSavedTable( wxUpdateUIEvent& event )
{
    bool bEnabled = false;
    hoxSite* selectedSite = _GetSelectedSite();

    if ( selectedSite != NULL )
    {
	    unsigned int actionFlags = selectedSite->GetCurrentActionFlags();
	    if ( (actionFlags & hoxSITE_ACTION_OPEN) != 0 )
	    {
            bEnabled = true;
	    }
    }
    event.Enable( bEnabled );
}

void 
MyFrame::OnListTables( wxCommandEvent& event )
{
    hoxSite* selectedSite = _GetSelectedSite();
    if ( selectedSite )
    {
        selectedSite->QueryForTables();
    }
}

void 
MyFrame::OnSize(wxSizeEvent& event)
{
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
}

void 
MyFrame::OnServersSashDrag(wxSashEvent& event)
{
    if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
        return;

    m_sitesWindow->SetDefaultSize(wxSize(event.GetDragRect().width, -1));

    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);

    // Leaves bits of itself behind sometimes
    GetClientWindow()->Refresh();
}

void 
MyFrame::InitToolBar(wxToolBar* toolBar)
{
    toolBar->AddTool( MDI_CONNECT_SERVER, "Connect", 
                      wxBitmap(connect_xpm), _("Connect Server"));
    toolBar->AddTool( MDI_DISCONNECT_SERVER, "Disconnect", 
                      wxBitmap(disconnect_xpm), _("Disconnect Server"));
    toolBar->AddSeparator();

	toolBar->AddTool( MDI_LIST_TABLES, "List", 
		              wxBitmap(folder_open_xpm), _("List Tables"));
    toolBar->AddTool( MDI_NEW_TABLE, "New", 
		              wxBitmap(new_xpm), _("Open Table"));
    toolBar->AddTool( MDI_CLOSE_TABLE, "Close", 
		              wxBitmap(delete_xpm), _("Close Table"));
    toolBar->AddSeparator();

    toolBar->AddTool( MDI_SOUND, "Sounds",
                      hoxUtil::LoadImage("speaker.png"),
		              _("Toggle Sounds"), wxITEM_CHECK);
    toolBar->ToggleTool( MDI_SOUND,
                         wxGetApp().GetOption("sound") == "1" );
    toolBar->AddTool( MDI_QUIT, "Exit", 
		              wxBitmap(quit_xpm), _("Quit the program"));

    toolBar->Realize();
}

/* static */
wxMenuBar* 
MyFrame::Create_Menu_Bar(bool hasTable /* = false */)
{
    /* File menu. */
    wxMenu* file_menu = new wxMenu;
    Add_Menu_Item( file_menu,
                   MDI_CONNECT_SERVER, _("Connect Server...\tCtrl-L"), _("Connect to remote server"),
                   connect_xpm );
    file_menu->Append(MDI_DISCONNECT_SERVER, _("&Disconnect Server\tCtrl-D"), _("Disconnect from remote server"));
    file_menu->AppendSeparator();
    file_menu->Append(MDI_LIST_TABLES, _("List &Tables\tCtrl-T"), _("Get the list of tables"));
    file_menu->Append(MDI_NEW_TABLE, _("&New Table\tCtrl-N"), _("Create New Table"));
    if ( hasTable )
    {
        file_menu->Append(MDI_CLOSE_TABLE, _("&Close Table\tCtrl-C"), _("Close Table"));
    }
    file_menu->AppendSeparator();
    file_menu->Append(MDI_PRACTICE, _("&Practice with Computer\tCtrl-P"),
                                    _("Practice with your local Computer"));
    if ( hasTable )
    {
        Add_Menu_Item( file_menu,
                       MDI_SAVE_TABLE, _("&Save Table..."), _("Save the current table"),
                       save_xpm );
    }
    file_menu->Append(MDI_OPEN_SAVED_TABLE, _("Open Saved Table..."), _("Open a saved table"));
    file_menu->AppendSeparator();
    file_menu->Append(MDI_QUIT, _("&Exit\tAlt-X"), _("Quit the program"));

    /* View menu. */
    wxMenu* view_menu = new wxMenu;
    view_menu->AppendCheckItem(MDI_SHOW_SERVERS_WINDOW, _("Site&s View\tCtrl-S"));
    if ( hasTable )
    {
        view_menu->Append(MDI_TOGGLE, _("Toggle Table &View\tCtrl-V"), _("Toggle Table View"));
    }
    view_menu->AppendSeparator();
    view_menu->AppendCheckItem(MDI_SHOW_LOG_WINDOW, _("Lo&g Window\tCtrl-G"), _("Log Window"));

    /* Tools menu. */
    wxMenu* tools_menu = file_menu;   // Use File menu instead on Mac
#ifndef __WXMAC__
    tools_menu = new wxMenu;
#endif
    tools_menu->Append(MDI_OPTIONS, _("&Options...\tCtrl-O"), _("Set options"));

    /* Help menu. */
    wxMenu* help_menu = new wxMenu;
    Add_Menu_Item( help_menu,
                   MDI_ABOUT, _("&About HOXChess...\tF1"), wxEmptyString,
                   help_xpm );

    /* The main menu bar */
    wxMenuBar* menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, _("&File"));
    menu_bar->Append(view_menu, _("&View"));
#ifndef __WXMAC__
    menu_bar->Append(tools_menu, _("&Tools"));
#endif
    menu_bar->Append(help_menu, _("&Help"));

    return menu_bar;
}

/* static */
void
MyFrame::Add_Menu_Item( wxMenu*         parentMenu,
                        const int       id,
                        const wxString& name,
                        const wxString& help,
                        const wxBitmap& bitmap )
{
    wxMenuItem* item = new wxMenuItem( parentMenu, id, name, help );
    item->SetBitmap( bitmap );
    parentMenu->Append( item );
}

void 
MyFrame::OnContextMenu( wxContextMenuEvent& event )
{ 
    wxPoint point = event.GetPosition(); 
    point = ScreenToClient(point);
    
    wxMenu menu;

    hoxTable_SPtr selectedTable;
    hoxSite* selectedSite = _GetSelectedSiteAndTable(selectedTable);

    if ( selectedTable.get() != NULL )
    {
        menu.Append(MDI_CLOSE_TABLE, _("&Close Table\tCtrl-C"), 
			                         _("Close Table"));
    }
    else if ( selectedSite != NULL )
    {
		unsigned int actionFlags = selectedSite->GetCurrentActionFlags();

		if ( (actionFlags & hoxSITE_ACTION_CONNECT) != 0 )
		{
			menu.Append(MDI_CONNECT_SERVER, _("Connect Server...\tCtrl-L"), 
				                            _("Connect to remote server"));
		}
		if ( (actionFlags & hoxSITE_ACTION_DISCONNECT) != 0 )
		{
            menu.Append(MDI_DISCONNECT_SERVER, _("&Disconnect Server\tCtrl-D"), 
                                               _("Disconnect from remote server"));
		}
		if ( (actionFlags & hoxSITE_ACTION_LIST) != 0 )
		{
            menu.Append(MDI_LIST_TABLES, _("List &Tables\tCtrl-T"), 
                                         _("Get the list of tables"));
		}
		if ( (actionFlags & hoxSITE_ACTION_NEW) != 0 )
		{
			menu.Append(MDI_NEW_TABLE, _("&New Table\tCtrl-N"), 
				                       _("Create New Table"));
		}
		if ( (actionFlags & hoxSITE_ACTION_PRACTICE) != 0 )
		{
			menu.Append(MDI_PRACTICE, _("&Practice with Computer\tCtrl-P"),
                                      _("Practice with your local Computer"));
		}
		if ( (actionFlags & hoxSITE_ACTION_OPEN) != 0 )
		{
            menu.Append(MDI_OPEN_SAVED_TABLE, _("Open Saved Table..."), 
                                              _("Open a saved table"));
		}
    }
	else
	{
		menu.Append(MDI_CONNECT_SERVER, _("Connect Server...\tCtrl-L"), 
			                            _("Connect to remote server"));
	}

    PopupMenu(&menu, point.x, point.y);
}

void
MyFrame::OnToggleSound( wxCommandEvent& event )
{
    bool bSound = wxGetApp().GetOption("sound") == "1";
    bSound = !bSound;
    wxGetApp().SetOption( "sound", bSound ? "1" : "0" );

    hoxTable_SPtr pTable;
    for ( MyChildList::const_iterator it = m_children.begin();
                                      it != m_children.end(); ++it )
    {
        pTable = (*it)->GetTable();
        hoxBoard* pBoardUI = pTable->GetBoardUI();
        if ( pBoardUI ) pBoardUI->EnableSound( bSound );
    }
}

void
MyFrame::OnOptions( wxCommandEvent& event )
{
    /* STEP 1: Show options ............................ */

    hoxOptionsUI dlg;

    dlg.m_bSound       = (wxGetApp().GetOption("sound") == "1");
    dlg.m_bWelcome     = (wxGetApp().GetOption("welcome") == "1");
    dlg.m_bTables      = (wxGetApp().GetOption("showTables") == "1");
    dlg.m_language     = wxGetApp().GetCurrentLanguage();
    dlg.m_sBoardImage  = wxGetApp().GetOption("/Board/Image/path");
    dlg.m_sBgColor     = wxGetApp().GetOption("/Board/Color/background");
    dlg.m_sFgColor     = wxGetApp().GetOption("/Board/Color/foreground");
    dlg.m_sPiece       = wxGetApp().GetOption("/Board/Piece/path");
    dlg.m_sDefaultAI   = hoxAIPluginMgr::GetInstance()->GetDefaultPluginName();

    const wxString selectedPage = wxGetApp().GetOption("optionsPage");
    long lSelectedPage = 0;
    if ( ! selectedPage.ToLong( &lSelectedPage ) ) lSelectedPage = 0;
    dlg.m_selectedPage = (size_t) lSelectedPage;

    wxGetApp().GetLanguageList( dlg.m_langList );

    dlg.Create( this );
    if ( dlg.ShowModal() != wxID_OK ) return;

    /* STEP 2: Save options ............................ */

    wxGetApp().SetOption( "sound", dlg.m_bSound ? "1" : "0" );
    this->GetToolBar()->ToggleTool( MDI_SOUND, dlg.m_bSound );

    wxGetApp().SetOption( "welcome", dlg.m_bWelcome ? "1" : "0" );
    wxGetApp().SetOption( "showTables", dlg.m_bTables ? "1" : "0" );

    if ( dlg.m_language != wxGetApp().GetCurrentLanguage() )
    {
        wxGetApp().SaveCurrentLanguage( dlg.m_language );
        ::wxMessageBox( _("You must restart the program for this change to take effect"),
                        _("Required Action"), wxOK | wxICON_INFORMATION );
    }

    wxGetApp().SetOption( "/Board/Image/path",       dlg.m_sBoardImage );
    wxGetApp().SetOption( "/Board/Color/background", dlg.m_sBgColor );
    wxGetApp().SetOption( "/Board/Color/foreground", dlg.m_sFgColor );
    wxGetApp().SetOption( "/Board/Piece/path",       dlg.m_sPiece );

    hoxAIPluginMgr::SetDefaultPluginName( dlg.m_sDefaultAI );
    wxGetApp().SetOption( "defaultAI", dlg.m_sDefaultAI );

    wxGetApp().SetOption( "optionsPage", wxString::Format("%d", dlg.m_selectedPage) );

    // Apply the new Options to the Active Table.
    MyChild* child = wxDynamicCast(this->GetActiveChild(), MyChild);
    if ( child != NULL )
    {
        hoxTable_SPtr pTable = child->GetTable();
        hoxBoard* pBoardUI = pTable->GetBoardUI();
        if ( pBoardUI )
        {
            pBoardUI->EnableSound( dlg.m_bSound );
            pBoardUI->SetBackgroundImage( dlg.m_sBoardImage );
            pBoardUI->SetBgColor( wxColor(dlg.m_sBgColor) );
            pBoardUI->SetFgColor( wxColor(dlg.m_sFgColor) );
            pBoardUI->SetPiecesPath( dlg.m_sPiece );
            pBoardUI->Repaint();
        }
    }
}

MyChild* 
MyFrame::CreateFrameForTable( const wxString& sTableId )
{
    MyChild*  childFrame = NULL;
    wxString  windowTitle;

    wxASSERT( ! sTableId.empty() );

    /* Generate the Window's title for the new Table. */
    windowTitle.Printf("Table #%s", sTableId.c_str());

	/* Load the default layout. */
	wxPoint defaultPosition = wxDefaultPosition;
	wxSize  defaultSize;

#ifdef WIN32
	if ( m_children.empty() )
	{
		defaultPosition = wxPoint(0, 0);
	}
#endif

	if ( ! _GetDefaultTableLayout( defaultSize ) ) // not exist?
	{
		defaultSize = wxSize(666, 586);
	}

    childFrame = new MyChild( this, 
	                          windowTitle, 
	                          defaultPosition, 
						      defaultSize );
    m_children.push_back( childFrame );

    return childFrame;
}

void
MyFrame::OnSiteSelectionChanged( hoxSite* selectedSite )
{
    wxASSERT( selectedSite );

    hoxPlayersUI* playersUI = selectedSite->GetPlayersUI();
    if ( playersUI )
    {
        wxLogDebug("%s: Set active Players-UI to [%s].", __FUNCTION__, selectedSite->GetName().c_str());
        _SetActiveSitePlayersUI( playersUI );
    }

    if ( selectedSite->GetType() != hoxSITE_TYPE_LOCAL )
    {
        wxMenuBar* frameMenu = this->GetMenuBar();
        frameMenu->Enable( MDI_DISCONNECT_SERVER, true );
        frameMenu->Enable( MDI_LIST_TABLES, true );
        frameMenu->Enable( MDI_NEW_TABLE, true );
    }
}

void
MyFrame::_SetActiveSitePlayersUI( hoxPlayersUI* newPlayersUI )
{
    wxWindow* currentPlayersUI = m_sitesSplitter->GetWindow2();
    bool bNeedToSplit = false;

    if ( currentPlayersUI == NULL )
    {
        bNeedToSplit = true;
    }
    else if ( currentPlayersUI != newPlayersUI )
    {
        /* NOTE: I tried to use wxSplitterWindow::ReplaceWindow() acccording to
         *       wxWidgets online documentation's recommendation but the method
         *       does not work.
         *
         *   m_sitesSplitter->ReplaceWindow( currentPlayersUI, newPlayersUI );
         */

        m_sitesSplitter->Unsplit( currentPlayersUI );
        bNeedToSplit = true;
    }

    if ( bNeedToSplit )
    {
        m_sitesSplitter->SplitHorizontally( m_sitesUI, newPlayersUI, 100 );
    }
}

hoxPlayersUI*
MyFrame::CreateNewSitePlayersUI()
{
    return new hoxPlayersUI( m_sitesSplitter, hoxPlayersUI::UI_TYPE_SITE );
}

void
MyFrame::DeleteSitePlayersUI( hoxPlayersUI* toDeletePlayersUI )
{
    wxWindow* currentPlayersUI = m_sitesSplitter->GetWindow2();
    if ( currentPlayersUI && currentPlayersUI == toDeletePlayersUI )
    {
        m_sitesSplitter->Unsplit( toDeletePlayersUI );
    }

    toDeletePlayersUI->Close( true /* force to close */ );
}

void
MyFrame::DeleteFrameOfTable( const wxString& sTableId )
{
    MyChild* foundChild = NULL;

    for ( MyChildList::const_iterator it = m_children.begin();
                                      it != m_children.end(); ++it )
    {
        if ( (*it)->IsMyTable( sTableId ) )
        {
            foundChild = ( *it );
            break;
        }
    }

    if ( foundChild != NULL )
    {
        foundChild->Close( true /* force */ );
        // NOTE: The call above already delete the child.
    }
}

void
MyFrame::_CreateSitesUI()
{
	int defaultSizeX;
	if ( ! _GetDefaultSitesLayout( defaultSizeX ) ) // not found?
	{
		defaultSizeX = 200;
	}

    // A window to the left of the client window
    m_sitesWindow = new wxSashLayoutWindow( this, 
		                                    ID_WINDOW_SITES,
                                            wxDefaultPosition, 
											wxDefaultSize,
                                            wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN );
    m_sitesWindow->SetDefaultSize(wxSize(defaultSizeX, -1));
    m_sitesWindow->SetOrientation(wxLAYOUT_VERTICAL);
    m_sitesWindow->SetAlignment(wxLAYOUT_LEFT);
    m_sitesWindow->SetBackgroundColour(wxColour(0, 0, 255));
    m_sitesWindow->SetSashVisible(wxSASH_RIGHT, true);
    m_sitesWindow->SetExtraBorderSize(2);

    m_sitesSplitter = new wxSplitterWindow(
                                    m_sitesWindow, wxID_ANY,
                                    wxDefaultPosition, wxDefaultSize,
                                    wxSP_3D | wxSP_LIVE_UPDATE );
    m_sitesUI = new hoxSitesUI( m_sitesSplitter );
    hoxSiteManager::GetInstance()->SetUI( m_sitesUI );
    m_sitesSplitter->Initialize( m_sitesUI );
}

void
MyFrame::_ShowWelcomeDialog()
{
    hoxWelcomeUI welcomeDlg( this );
    const int nCommandId = welcomeDlg.ShowModal();

    /* NOTE: Do this setting first before the "Options" dialog
     *       could be invoked.
     */
    wxGetApp().SetOption( "welcome",
                          welcomeDlg.ShowNextStartup() ? "1" : "0" );

    switch ( nCommandId )
    {
        case hoxWelcomeUI::COMMAND_ID_PRACTICE:
        {
            this->ProcessCommand( MDI_PRACTICE ); break;
        }
        case hoxWelcomeUI::COMMAND_ID_REMOTE:
        {
            this->ProcessCommand( MDI_CONNECT_SERVER ); break;
        }
        case hoxWelcomeUI::COMMAND_ID_OPTIONS:
        {
            this->ProcessCommand( MDI_OPTIONS ); break;
        }
        default: break;
    }
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
MyFrame::_GetSelectedSite() const
{
    return m_sitesUI->GetSelectedSite();
}

hoxSite* 
MyFrame::_GetSelectedSiteAndTable(hoxTable_SPtr& selectedTable) const
{
    return m_sitesUI->GetSelectedSite( selectedTable );
}

bool 
MyFrame::_GetDefaultSitesLayout( int& sizeX )
{
	sizeX = 0;

	wxConfig* config = wxGetApp().GetConfig();

	if ( ! config->Read("/Layout/Sites/size/x", &sizeX) )
		return false;  // not found.

	return true;   // found old layout?
}

bool 
MyFrame::_SaveDefaultSitesLayout( const int sizeX )
{
	wxConfig* config = wxGetApp().GetConfig();
	config->Write("/Layout/Sites/size/x", sizeX);
	return true;
}

bool 
MyFrame::_GetDefaultTableLayout( wxSize& size )
{
	size = wxSize( 0, 0 );

	wxConfig* config = wxGetApp().GetConfig();

	if ( ! config->Read("/Layout/Table/size/x", &size.x) )
		return false;  // not found.

	if ( ! config->Read("/Layout/Table/size/y", &size.y) )
		return false;  // not found.

	return true;   // found old layout?
}

bool 
MyFrame::_SaveDefaultTableLayout( const wxSize& size )
{
	wxConfig* config = wxGetApp().GetConfig();

	config->Write("/Layout/Table/size/x", size.x);
	config->Write("/Layout/Table/size/y", size.y);

	return true;
}

void
MyFrame::_SaveCurrentTableToDisk( const hoxTable_SPtr& pTable,
                                  const wxString&      fileName ) const
{
    hoxMoveList      moveList;
    hoxGameState     gameState;

    pTable->GetReferee()->GetHistoryMoves( moveList );
    pTable->GetReferee()->GetGameState( gameState );

    hoxSavedTable savedTable( fileName );
    savedTable.SaveGameState( pTable->GetId(), moveList,
                              gameState.pieceList, gameState.nextColor );
}

/************************* END OF FILE ***************************************/
