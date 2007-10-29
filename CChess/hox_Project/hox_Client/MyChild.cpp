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
    //EVT_MENU(MDI_CHANGE_SIZE, MyChild::OnChangeSize)

    EVT_CLOSE(MyChild::OnClose)
END_EVENT_TABLE()


// ---------------------------------------------------------------------------
// MyChild
// ---------------------------------------------------------------------------

MyChild::MyChild(wxMDIParentFrame *parent, const wxString& title)
       : wxMDIChildFrame( parent, 
                          wxID_ANY, 
                          title, 
                          wxDefaultPosition, 
                          wxSize(400, 500),
                          wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE )
        , m_table( NULL )
{
    this->Maximize();

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

    if ( m_table != NULL )
    {
        /* We should send a signal inform all players about this event.
         * In the mean time, use "brute force"!!!
         */

        hoxPlayer* redPlayer = m_table->GetRedPlayer();
        hoxPlayer* blackPlayer = m_table->GetBlackPlayer();

        if ( redPlayer != NULL ) redPlayer->LeaveTable( m_table );
        if ( blackPlayer != NULL ) blackPlayer->LeaveTable( m_table );
        hoxTableMgr::GetInstance()->RemoveTable( m_table );
    }

    wxLogDebug("%s: END.", FNAME);
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
    //option_menu->AppendSeparator();
    //option_menu->Append(MDI_CHANGE_SIZE, _T("Resize frame\tCtrl-S"));

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

//void 
//MyChild::OnChangeSize(wxCommandEvent& WXUNUSED(event))
//{
//    SetClientSize(100, 100);
//}

void 
MyChild::OnClose(wxCloseEvent& event)
{
    const char* FNAME = "MyChild::OnClose";
    wxLogDebug("%s: ENTER.", FNAME);

    wxGetApp().m_nChildren--;

    /* If one of the players is the HTTP player,
     * then inform the HTTP server that the player is leaving this table.
     */
    wxASSERT( m_table != NULL );
    hoxHttpPlayer* httpPlayer = wxGetApp().m_httpPlayer;
    if (   httpPlayer != NULL
        && (   m_table->GetRedPlayer() == httpPlayer
            || m_table->GetBlackPlayer() == httpPlayer) )
    {
        wxLogDebug("%s: Info the HTTP server about my leaving my table.", FNAME);
        const wxString tableId = m_table->GetId(); 
        wxASSERT( this->GetParent() );
        hoxResult result = httpPlayer->LeaveNetworkTable( tableId, this->GetParent() );
        if ( result != hoxRESULT_OK ) // failed?
        {
            wxLogError("%s: Failed to inform HTTP server about my leaving the table [%s].", FNAME, tableId);
        }
        httpPlayer->LeaveTable( m_table );
    }

    /* If one of the players is the MY player,
     * then inform the the server that the player is leaving this table.
     */
    wxASSERT( m_table != NULL );
    hoxMyPlayer* myPlayer = wxGetApp().m_myPlayer;
    if (   myPlayer != NULL
        && (   m_table->GetRedPlayer() == myPlayer
            || m_table->GetBlackPlayer() == myPlayer) )
    {
        wxLogDebug(wxString::Format("%s: Info the server about my leaving my table.", FNAME));
        const wxString tableId = m_table->GetId(); 
        wxASSERT( this->GetParent() );
        hoxResult result = myPlayer->LeaveNetworkTable( tableId, this->GetParent() );
        if ( result != hoxRESULT_OK ) // failed?
        {
            wxLogError(wxString::Format("%s: Failed to inform the server about my leaving the table [%s].", FNAME, tableId));
        }
        myPlayer->LeaveTable( m_table );
    }

    event.Skip(); // let the search for the event handler should continue...

    wxLogDebug("%s: END.", FNAME);
}

void
MyChild::SetTable(hoxTable* table)
{
    wxASSERT_MSG( m_table == NULL, "A table has already been set." );
    wxASSERT( table != NULL );
    m_table = table;
}


/************************* END OF FILE ***************************************/
