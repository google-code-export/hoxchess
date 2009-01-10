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
#include "hoxLoginDialog.h"
#include "hoxSitesUI.h"
#include "hoxPlayersUI.h"
#include "hoxOptionsUI.h"
#include "hoxBoard.h"

#include <wx/splitter.h>

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

IMPLEMENT_DYNAMIC_CLASS(MyFrame, wxMDIParentFrame)

/* Define my custom events */
DEFINE_EVENT_TYPE(hoxEVT_FRAME_LOG_MSG)

BEGIN_EVENT_TABLE(MyFrame, wxMDIParentFrame)
    EVT_MENU(MDI_ABOUT, MyFrame::OnAbout)
    EVT_MENU(MDI_NEW_TABLE, MyFrame::OnNewTable)
    EVT_MENU(MDI_CLOSE_TABLE, MyFrame::OnCloseTable)
	EVT_UPDATE_UI(MDI_NEW_TABLE, MyFrame::OnUpdateNewTable)
    EVT_UPDATE_UI(MDI_CLOSE_TABLE, MyFrame::OnUpdateCloseTable)

    EVT_MENU(MDI_DISCONNECT_SERVER, MyFrame::OnDisconnectServer)
	EVT_UPDATE_UI(MDI_DISCONNECT_SERVER, MyFrame::OnUpdateDisconnectServer)
    EVT_MENU(MDI_CONNECT_SERVER, MyFrame::OnConnectServer)
    EVT_MENU(MDI_LIST_TABLES, MyFrame::OnListTables)
	EVT_UPDATE_UI(MDI_LIST_TABLES, MyFrame::OnUpdateListTables)

    EVT_MENU(MDI_SHOW_SERVERS_WINDOW, MyFrame::OnShowServersWindow)
    EVT_UPDATE_UI(MDI_SHOW_SERVERS_WINDOW, MyFrame::OnUpdateServersWindow)

    EVT_MENU(MDI_SHOW_LOG_WINDOW, MyFrame::OnShowLogWindow)
    EVT_UPDATE_UI(MDI_SHOW_LOG_WINDOW, MyFrame::OnUpdateLogWindow)

    EVT_MENU(MDI_PRACTICE, MyFrame::OnPractice)
    EVT_UPDATE_UI(MDI_PRACTICE, MyFrame::OnUpdatePractice)

    EVT_MENU(MDI_QUIT, MyFrame::OnQuit)

    EVT_CLOSE(MyFrame::OnClose)
    EVT_SIZE(MyFrame::OnSize)
    EVT_SASH_DRAGGED(ID_WINDOW_SITES, MyFrame::OnServersSashDrag)
    EVT_SASH_DRAGGED(ID_WINDOW_LOG, MyFrame::OnLogSashDrag)

    EVT_COMMAND(wxID_ANY, hoxEVT_FRAME_LOG_MSG, MyFrame::OnFrameLogMsgEvent)

    EVT_CONTEXT_MENU(MyFrame::OnContextMenu)
    EVT_MENU(MDI_SOUND, MyFrame::OnToggleSound)
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
{
    SetIcon( wxICON(hoxchess) );

    _CreateSitesUI();
    _CreateLogUI();

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

    SetMenuBar( MyFrame::Create_Menu_Bar() );
    CreateStatusBar();

	wxLogStatus("%s %s is ready.", HOX_APP_NAME, HOX_VERSION);
}

void 
MyFrame::OnClose(wxCloseEvent& event)
{
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER.", FNAME);

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
    wxMessageBox( wxString::Format(
                    _("%s %s\n"
                      "\n"
                      "Author: Huy Phan (c) 2007, 2008, 2009\n"
                      "Email: huyphan@playxiangqi.com\n"
                      "\n"
                      "This application is powered by %s, running under %s.\n"
                      "\n"),
				    HOX_APP_NAME,
                    HOX_VERSION,
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
    hoxSite* selectedSite = _GetSelectedSite();
    if ( selectedSite != NULL )
    {
        selectedSite->OnLocalRequest_NEW();
    }
}

void 
MyFrame::OnCloseTable( wxCommandEvent& event )
{
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER.", FNAME);

    MyChild* child = wxDynamicCast(this->GetActiveChild(), MyChild);
    if ( child != NULL )
    {
        wxLogDebug("%s: Closing the active Table [%s]...", FNAME, child->GetTitle().c_str());
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
MyFrame::OnUpdateCloseTable( wxUpdateUIEvent& event )
{
    event.Enable( this->GetActiveChild() != NULL );
}

void 
MyFrame::OnUpdateListTables( wxUpdateUIEvent& event )
{
    hoxSite* selectedSite = _GetSelectedSite();
    event.Enable( selectedSite != NULL );
}

void
MyFrame::OnChildClose( wxCloseEvent& event,
                       MyChild*      child, 
                       hoxTable_SPtr pTable )
{
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER.", FNAME);

    wxCHECK_RET( pTable.get() != NULL, "The table is NULL." );

	/* Save the layout. */
	_SaveDefaultTableLayout( child->GetSize() );

    pTable->OnClose_FromSystem();
    m_children.remove( child );

    /* Inform the Site. */
    hoxSite* site = pTable->GetSite();
    site->CloseTable( pTable );

    event.Skip(); // let the search for the event handler should continue...

    wxLogDebug("%s: END.", FNAME);
}

void 
MyFrame::OnDisconnectServer( wxCommandEvent& event )
{
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER.", FNAME);

    /* Find out which site is selected. */

    hoxSite* selectedSite = _GetSelectedSite();
    if ( selectedSite == NULL )
    {
        wxLogDebug("%s: No site is selected. Do nothing. END.", FNAME);
        return;
    }

    /* Go through my children to see which ones belong to the site and
     * trigger a close event.
     */
    _CloseChildrenOfSite( selectedSite );

    /* Disconnect the site itself. */
    selectedSite->Disconnect();
}

void 
MyFrame::OnUpdateDisconnectServer(wxUpdateUIEvent& event)
{
    hoxSite*  selectedSite = _GetSelectedSite();
	bool bEnabled = ( selectedSite != NULL
                   && selectedSite->GetType() != hoxSITE_TYPE_LOCAL );
    event.Enable( bEnabled );
}

void 
MyFrame::OnConnectServer( wxCommandEvent& event )
{
    const char* FNAME = __FUNCTION__;

	/* Ask the user for the server' address and login-info. */

    hoxLoginDialog loginDlg( this, wxID_ANY, 
                             "Connect to a remote server" );
	loginDlg.ShowModal();

	if ( loginDlg.GetSelectedCommand() != hoxLoginDialog::COMMAND_ID_LOGIN )
	{
		wxLogDebug("%s: Login has been canceled.", FNAME);
		return;
	}

	const hoxSiteType siteType = loginDlg.GetSelectedSiteType();
	const hoxServerAddress serverAddress( loginDlg.GetSelectedAddress(),
		                                  loginDlg.GetSelectedPort() );
	const wxString userName = loginDlg.GetSelectedUserName();
	const wxString password = loginDlg.GetSelectedPassword();


    /* Start connecting... */
    
	wxGetApp().ConnectToServer( siteType,
		                        serverAddress,
							    userName,
								password );
}

void 
MyFrame::OnShowServersWindow( wxCommandEvent& event )
{
    m_sitesWindow->Show( ! m_sitesWindow->IsShown() );

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
MyFrame::OnListTables( wxCommandEvent& event )
{
    const char* FNAME = __FUNCTION__;

    hoxSite* selectedSite = _GetSelectedSite();
    if ( selectedSite == NULL )
        return;

    hoxResult result = selectedSite->QueryForNetworkTables();
    if ( result != hoxRC_OK )
    {
        wxLogError("%s: Failed to query for LIST of tables from the server.", FNAME);
        return;
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
MyFrame::OnLogSashDrag(wxSashEvent& event)
{
    if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
        return;

    m_logWindow->SetDefaultSize(wxSize(-1, event.GetDragRect().height));

    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);

    // Leaves bits of itself behind sometimes
    GetClientWindow()->Refresh();
}

void 
MyFrame::InitToolBar(wxToolBar* toolBar)
{
    toolBar->AddTool( MDI_CONNECT_SERVER, _("Connect"), 
                      wxBitmap(connect_xpm), _("Connect Server"));

    toolBar->AddTool( MDI_DISCONNECT_SERVER, _("Disconnect"), 
                      wxBitmap(disconnect_xpm), _("Disconnect Server"));

    toolBar->AddSeparator();

	toolBar->AddTool( MDI_LIST_TABLES, _("List"), 
		              wxBitmap(folder_open_xpm), _("List Tables"));

    toolBar->AddTool( MDI_NEW_TABLE, _("New"), 
		              wxBitmap(new_xpm), _("Open Table"));

    toolBar->AddTool( MDI_CLOSE_TABLE, _("Close"), 
		              wxBitmap(delete_xpm), _("Close Table"));

    toolBar->AddSeparator();

    toolBar->AddTool( MDI_SOUND, _("Sounds"),
                      hoxUtil::LoadImage("speaker.png"),
		              _("Toggle sounds"), wxITEM_CHECK);
    toolBar->ToggleTool( MDI_SOUND,
                         wxGetApp().GetOption("sound") == "1" );

    toolBar->AddTool( MDI_ABOUT, _("About"), 
		              wxBitmap(help_xpm), _("Help"));

    toolBar->AddTool( MDI_QUIT, _("Exit"), 
		              wxBitmap(quit_xpm), _("Quit the Program"));

    toolBar->Realize();
}

/* static */
wxMenuBar* 
MyFrame::Create_Menu_Bar(bool hasTable /* = false */)
{
    /* File menu. */
    wxMenu* file_menu = new wxMenu;
    file_menu->Append(MDI_CONNECT_SERVER, _("Connect Server...\tCtrl-L"), _("Connect to remote server"));
    file_menu->Append(MDI_DISCONNECT_SERVER, _("&Disconnect Server\tCtrl-D"), _("Disconnect from remote server"));
    file_menu->AppendSeparator();
	file_menu->Append(MDI_LIST_TABLES, _("List &Tables\tCtrl-T"), _("Get the list of tables"));
    file_menu->Append(MDI_NEW_TABLE, _("&New Table\tCtrl-N"), _("Create New Table"));
    file_menu->Append(MDI_PRACTICE, _("&Practice with Computer\tCtrl-P"),
                                    _("Practice with your local Computer"));
    file_menu->Append(MDI_CLOSE_TABLE, _("&Close Table\tCtrl-C"), _("Close Table"));
    file_menu->AppendSeparator();
    file_menu->Append(MDI_QUIT, _("&Exit\tAlt-X"), _("Quit the program"));

    /* View menu. */
    wxMenu* view_menu = new wxMenu;
    view_menu->AppendCheckItem(MDI_SHOW_SERVERS_WINDOW, _("Site&s View\tCtrl-S"));
    if ( hasTable )
    {
        view_menu->Append(MDI_TOGGLE, _("Toggle Table &View\tCtrl-V"));
    }
    view_menu->AppendSeparator();
    view_menu->AppendCheckItem(MDI_SHOW_LOG_WINDOW, _("Lo&g Window\tCtrl-G"));

    /* Tools menu. */
    wxMenu* tools_menu = new wxMenu;
    tools_menu->Append(MDI_OPTIONS, _("&Options...\tCtrl-O"));

    /* Help menu. */
    wxMenu* help_menu = new wxMenu;
    help_menu->Append(MDI_ABOUT, _("&About HOXChess...\tF1"));

    /* The main menu bar */
    wxMenuBar* menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, _("&File"));
    menu_bar->Append(view_menu, _("&View"));
    menu_bar->Append(tools_menu, _("&Tools"));
    menu_bar->Append(help_menu, _("&Help"));

    return menu_bar;
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
			menu.Append(MDI_CONNECT_SERVER, _("Connect Remote Server...\tCtrl-L"), 
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
    }
	else
	{
		menu.Append(MDI_CONNECT_SERVER, _("Connect Remote Server...\tCtrl-L"), 
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
    hoxOptionsUI::OptionsData optionData;
    optionData.m_bSound = (wxGetApp().GetOption("sound") == "1");
    optionData.m_sBgColor = wxGetApp().GetOption("/Board/Color/background");
    optionData.m_sFgColor = wxGetApp().GetOption("/Board/Color/foreground");

    hoxOptionsUI optionsDialog( this, optionData );

    if ( optionsDialog.ShowModal() == wxID_OK )
    {
        optionData = optionsDialog.GetData();
        wxGetApp().SetOption( "sound",
                              optionData.m_bSound ? "1" : "0" );
        this->GetToolBar()->ToggleTool( MDI_SOUND, optionData.m_bSound );

        wxGetApp().SetOption( "/Board/Color/background",
                              optionData.m_sBgColor );
        wxGetApp().SetOption( "/Board/Color/foreground",
                              optionData.m_sFgColor );

        // Apply the new Options to the Active Table.
        MyChild* child = wxDynamicCast(this->GetActiveChild(), MyChild);
        if ( child != NULL )
        {
            hoxTable_SPtr pTable = child->GetTable();
            hoxBoard* pBoardUI = pTable->GetBoardUI();
            if ( pBoardUI )
            {
                pBoardUI->EnableSound( optionData.m_bSound );
                pBoardUI->SetBgColor( wxColor(optionData.m_sBgColor) );
                pBoardUI->SetFgColor( wxColor(optionData.m_sFgColor) );
            }
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

	if ( m_children.empty() )
	{
		defaultPosition = wxPoint(0, 0);
	}

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

    wxSplitterWindow* splitter = new wxSplitterWindow(
                                    m_sitesWindow, wxID_ANY,
                                    wxDefaultPosition, wxDefaultSize,
                                    wxSP_3D | wxSP_LIVE_UPDATE );
    m_sitesUI = new hoxSitesUI( splitter );
    hoxSiteManager::GetInstance()->SetUI( m_sitesUI );

    m_playersUI = new hoxPlayersUI( splitter );
    m_playersUI->Disable();  // Disable until a Site exists.

    splitter->SplitHorizontally( m_sitesUI, m_playersUI, 100 );
}

void
MyFrame::_CreateLogUI()
{
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
    hoxTable_SPtr selectedTable;  // Not used!
    return m_sitesUI->GetSelectedSite( selectedTable );
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

/************************* END OF FILE ***************************************/
