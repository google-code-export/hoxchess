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

// Note that MDI_NEW_TABLE and MDI_ABOUT commands get passed
// to the parent window for processing, so no need to
// duplicate event handlers here.
BEGIN_EVENT_TABLE(MyChild, wxMDIChildFrame)
    EVT_MENU(MDI_CHILD_QUIT, MyChild::OnQuit)
    EVT_MENU(MDI_TOGGLE, MyChild::OnToggleView)
    EVT_CLOSE(MyChild::OnClose)
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
{
    wxMenuBar* menu_bar = MyFrame::Create_Menu_Bar( true /* hasTable */);
    SetMenuBar( menu_bar );

    //CreateStatusBar();
    //SetStatusText( this->GetTitle() );

    // This should work for MDI frames as well as for normal ones.
    SetSizeHints(100, 100);
}

void 
MyChild::OnQuit( wxCommandEvent& event )
{
    Close(true);
}

void 
MyChild::OnToggleView( wxCommandEvent& event )
{
    if ( m_pTable ) m_pTable->ToggleViewSide();
}

void 
MyChild::OnClose( wxCloseEvent& event )
{
    wxCHECK_RET( m_pTable, "The table must have been set." );
    wxGetApp().GetFrame()->OnChildClose( event, this, m_pTable );
}

void
MyChild::SetTable( hoxTable_SPtr pTable )
{
    wxCHECK_RET( m_pTable.get() == NULL, "A table has already been set." );
    wxASSERT( pTable != NULL );
    m_pTable = pTable;
}

hoxSite* 
MyChild::GetSite() const
{
    return m_pTable ? m_pTable->GetSite()
                    : NULL;
}

bool
MyChild::IsMyTable( const wxString& sTableId ) const
{
    return m_pTable ? m_pTable->GetId() == sTableId
                    : false;
}

/************************* END OF FILE ***************************************/
