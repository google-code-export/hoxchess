/***************************************************************************
 *  Copyright 2007, 2008 Huy Phan  <huyphan@playxiangqi.com>               *
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
// Name:            MyChild.cpp
// Created:         10/20/2007
//
// Description:     The child Window for the Client.
/////////////////////////////////////////////////////////////////////////////

#include "MyChild.h"
#include "MyFrame.h"
#include "MyApp.h"    // To access wxGetApp()
#include "hoxTable.h"
#include "hoxSite.h"
#include "hoxPlayer.h"
#include "hoxTableMgr.h"

#if !defined(__WXMSW__)
    #include "icons/chart.xpm"
#endif

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------


// Note that MDI_NEW_TABLE and MDI_ABOUT commands get passed
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

MyChild::MyChild( wxMDIParentFrame* parent, 
				  const wxString&   title,
                  const wxPoint&    pos /* = wxDefaultPosition */,
			      const wxSize&     size /* = wxDefaultSize */ )
       : wxMDIChildFrame( parent, 
                          wxID_ANY, 
                          title, 
                          pos, 
                          size,
                          wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE )
        , m_table( NULL )
{
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
    // Associate the menu bar with the frame
    wxMenuBar* menu_bar = MyFrame::Create_Menu_Bar( true /* hasTable */);
    SetMenuBar( menu_bar );
}

void 
MyChild::_SetupStatusBar()
{
    CreateStatusBar();
    SetStatusText( this->GetTitle() );
}

void 
MyChild::OnQuit( wxCommandEvent& event )
{
    Close(true);
}

void 
MyChild::OnToggle( wxCommandEvent& event )
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
    
    MyFrame* parent = wxGetApp().GetFrame();
    wxCHECK_RET( parent, "We should be able to cast...");
    bool bAllowedToClose =  parent->OnChildClose( this, m_table );

    if ( !bAllowedToClose )
    {
        wxLogDebug("%s: The parent did not allow me to close. END.", FNAME);
        return;
    }

    hoxSite* site = m_table->GetSite();
    site->CloseTable( m_table );
    m_table = NULL;

    parent->UpdateSiteTreeUI();

    event.Skip(); // let the search for the event handler should continue...

    wxLogDebug("%s: END.", FNAME);
}

void MyChild::OnSize(wxSizeEvent& event)
{
    //wxString mySize = wxString::Format("%d x %d", event.GetSize().GetWidth(), event.GetSize().GetHeight());
    //wxLogStatus(mySize);
    event.Skip(); // let the search for the event handler should continue...
}

void
MyChild::SetTable(hoxTable* table)
{
    wxCHECK_RET( m_table == NULL, "A table has already been set." );
    wxASSERT( table != NULL );
    m_table = table;
}

hoxSite* 
MyChild::GetSite() const
{
    if ( m_table != NULL )
        return m_table->GetSite();

    return NULL;
}

/************************* END OF FILE ***************************************/
