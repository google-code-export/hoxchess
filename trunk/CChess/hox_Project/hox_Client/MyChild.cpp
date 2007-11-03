/////////////////////////////////////////////////////////////////////////////
// Name:            MyChild.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/20/2007
//
// Description:     The child Window for the Client.
/////////////////////////////////////////////////////////////////////////////

#include "MyChild.h"
#include "MyFrame.h"
#include "MyApp.h"    // To access wxGetApp()
#include "hoxTable.h"
#include "hoxPlayer.h"
#include "hoxTableMgr.h"
#include "hoxPlayerMgr.h"
#if !defined(__WXMSW__)
    #include "icons/chart.xpm"
#endif

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------


// Note that MDI_NEW_WINDOW and MDI_ABOUT commands get passed
// to the parent window for processing, so no need to
// duplicate event handlers here.
BEGIN_EVENT_TABLE(MyChild, wxMDIChildFrame)
    EVT_MENU(MDI_CHILD_QUIT, MyChild::OnQuit)
    EVT_MENU(MDI_TOGGLE, MyChild::OnToggle)
    EVT_CLOSE(MyChild::OnClose)
    EVT_SIZE(MyChild::OnSize)
END_EVENT_TABLE()


// ---------------------------------------------------------------------------
// MyChild
// ---------------------------------------------------------------------------

MyChild::MyChild(wxMDIParentFrame *parent, const wxString& title)
       : wxMDIChildFrame( parent, 
                          wxID_ANY, 
                          title, 
                          wxDefaultPosition, 
                          wxSize(666, 586),
                          wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE )
        , m_table( NULL )
{
    //this->Maximize();

    _SetupMenu();
    _SetupStatusBar();

    SetIcon(wxICON(chart));

    // this should work for MDI frames as well as for normal ones
    SetSizeHints(100, 100);
}

MyChild::~MyChild()
{
    const char* FNAME = "MyChild::~MyChild";
    wxLogDebug("%s: ENTER.", FNAME);
}

void 
MyChild::_SetupMenu()
{
    /* File menu */

    wxMenu *file_menu = new wxMenu;

    file_menu->Append(MDI_NEW_WINDOW, _T("&New window"));
    file_menu->AppendSeparator();
    file_menu->Append(MDI_OPEN_SERVER, _T("&Open Server\tCtrl-O"), _T("Open server for remote access"));
    file_menu->Append(MDI_CONNECT_SERVER, _T("&Connect Server\tCtrl-C"), _T("Connect to remote server"));
    file_menu->Append(MDI_DISCONNECT_SERVER, _T("&Disconnect Server\tCtrl-D"), _T("Disconnect from remote server"));
    file_menu->AppendSeparator();
    file_menu->Append(MDI_CHILD_QUIT, _T("&Close child"), _T("Close this window"));
    file_menu->Append(MDI_QUIT, _T("&Exit"));

    /* Child menu */

    wxMenu *option_menu = new wxMenu;

    option_menu->Append(MDI_TOGGLE, _T("&Toggle View\tCtrl-T"));

    /* Help menu */

    wxMenu *help_menu = new wxMenu;
    help_menu->Append(MDI_ABOUT, _T("&About"));

    /* Setup the main menu bar */

    wxMenuBar *menu_bar = new wxMenuBar;

    menu_bar->Append(file_menu, _T("&File"));
    menu_bar->Append(option_menu, _T("&Child"));
    menu_bar->Append(help_menu, _T("&Help"));

    // Associate the menu bar with the frame
    SetMenuBar(menu_bar);
}

void 
MyChild::_SetupStatusBar()
{
    CreateStatusBar();
    SetStatusText( this->GetTitle() );
}

void 
MyChild::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void 
MyChild::OnToggle(wxCommandEvent& WXUNUSED(event))
{
    if ( m_table != NULL )
        m_table->ToggleViewSide();
}

void 
MyChild::OnClose(wxCloseEvent& event)
{
    const char* FNAME = "MyChild::OnClose";
    wxLogDebug("%s: ENTER.", FNAME);

    wxCHECK_RET( m_table, "The table must have been set." );
    
    MyFrame* parent = wxDynamicCast(this->GetParent(), MyFrame);
    wxCHECK_RET( parent, "We should be able to cast...");
    bool bAllowedToClose =  parent->OnChildClose( this, m_table );

    if ( !bAllowedToClose )
    {
        wxLogDebug("%s: The parent did not allow me to close. END.", FNAME);
        return;
    }

    hoxTableMgr::GetInstance()->RemoveTable( m_table );
    m_table = NULL;

    event.Skip(); // let the search for the event handler should continue...

    wxLogDebug("%s: END.", FNAME);
}

void MyChild::OnSize(wxSizeEvent& event)
{
    wxString mySize = wxString::Format("%d x %d", event.GetSize().GetWidth(), event.GetSize().GetHeight());
    wxLogStatus(mySize);
    event.Skip(); // let the search for the event handler should continue...
}

void
MyChild::SetTable(hoxTable* table)
{
    wxCHECK_RET( m_table == NULL, "A table has already been set." );
    wxASSERT( table != NULL );
    m_table = table;
}


/************************* END OF FILE ***************************************/
