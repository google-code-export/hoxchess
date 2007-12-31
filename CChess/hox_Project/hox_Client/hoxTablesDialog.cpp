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
// Name:            hoxTablesDialog.cpp
// Created:         10/16/2007
//
// Description:     The dialog of a list of Tables.
/////////////////////////////////////////////////////////////////////////////

#include "hoxTablesDialog.h"
#include "hoxUtility.h"

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

enum
{
    ID_JOIN_TABLE =  100,
    ID_NEW_TABLE,
	ID_REFRESH_LIST
};

// ----------------------------------------------------------------------------
// Declare event-handler table
// ----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(hoxTablesDialog, wxDialog)
    EVT_BUTTON(ID_JOIN_TABLE, hoxTablesDialog::OnButtonJoin)
    EVT_BUTTON(ID_NEW_TABLE, hoxTablesDialog::OnButtonNew)
	EVT_BUTTON(ID_REFRESH_LIST, hoxTablesDialog::OnButtonRefresh)

	EVT_MOVE(hoxTablesDialog::OnMove)
	EVT_SIZE(hoxTablesDialog::OnSize)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxTablesDialog
//-----------------------------------------------------------------------------

/* Initialize class-member variables */
wxPoint hoxTablesDialog::s_lastPosition(-1, -1);
wxSize  hoxTablesDialog::s_lastSize(0, 0);

int wxCALLBACK MyCompareFunction( long item1, 
								  long item2, 
								  long sortData /* not used */)
{
    if (item1 < item2)  return -1;
    if (item1 > item2)  return 1;
    return 0;
}

hoxTablesDialog::hoxTablesDialog( wxWindow*       parent, 
                                  wxWindowID      id, 
                                  const wxString& title,
                                  const hoxNetworkTableInfoList& tableList)
        : wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, 
		           wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
        , m_selectedCommand( COMMAND_ID_UNKNOWN )
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

	/* Create a List-Control to display the table-list. */

	m_listCtrlTables = new wxListCtrl(
		this,
		wxID_ANY,
		wxDefaultPosition,
		wxDefaultSize,
		wxLC_REPORT | wxLC_SINGLE_SEL);

    wxString columns[] =
    {
        "Table",
        "Group",
        "Timer",
		"Type",
		"Red Player",
		"Black Player"
    };

	long     colIndex = 0;

	for ( colIndex = 0; colIndex < WXSIZEOF( columns ); ++colIndex )
	{
		m_listCtrlTables->InsertColumn( colIndex, columns[colIndex] );
	}

	long     itemIndex = 0;
	wxString redId;
	wxString blackId;

	for ( hoxNetworkTableInfoList::const_iterator it = tableList.begin(); 
												  it != tableList.end(); 
												++it )
	{
		redId   = (it->redId.empty()   ? "0" : it->redId );
		blackId = (it->blackId.empty() ? "0" : it->blackId );

		colIndex = 0;
		itemIndex = m_listCtrlTables->InsertItem(itemIndex, wxString::Format("#%s", it->id) );
		m_listCtrlTables->SetItemData( itemIndex, ::atoi( it->id.c_str() ) );

		m_listCtrlTables->SetItem(itemIndex, ++colIndex, wxString::Format("%d", it->status));
		m_listCtrlTables->SetItem(itemIndex, ++colIndex, hoxUtility::FormatTime(it->initialTime.nGame));
		m_listCtrlTables->SetItem(itemIndex, ++colIndex, hoxUtility::GameTypeToString(it->gameType));
		m_listCtrlTables->SetItem(itemIndex, ++colIndex, (redId << " (" << it->redScore << ")") );
		m_listCtrlTables->SetItem(itemIndex, ++colIndex, (blackId << " (" << it->blackScore << ")") );
		
		++itemIndex;
	}

	/* Set the columns' width. */

	for ( colIndex = 0; colIndex < WXSIZEOF( columns ); ++colIndex )
	{
		m_listCtrlTables->SetColumnWidth( colIndex, wxLIST_AUTOSIZE_USEHEADER );
	}
	if ( m_listCtrlTables->GetItemCount() > 0 )
	{
		for ( colIndex = 2; colIndex < WXSIZEOF( columns ); ++colIndex )
		{
			m_listCtrlTables->SetColumnWidth( colIndex, wxLIST_AUTOSIZE );
		}
	}

    /* Select the 1st table, if any. */

    if ( m_listCtrlTables->GetItemCount() > 0 )
    {
        m_listCtrlTables->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    }

	/* Sort the list. */
    m_listCtrlTables->SortItems(MyCompareFunction, 0);

    topSizer->Add( 
		m_listCtrlTables,
		wxSizerFlags(1).Expand().Border(wxALL, 10));

    /* Buttons... */

    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );

    buttonSizer->Add( 
		new wxButton(this, ID_REFRESH_LIST, _("&Refresh")),
		wxSizerFlags().Align(wxALIGN_CENTER));

    buttonSizer->Add( 
		new wxButton(this, ID_NEW_TABLE, _("&New Table")),
		wxSizerFlags().Align(wxALIGN_CENTER));

    buttonSizer->AddSpacer(30);

    buttonSizer->Add( 
		new wxButton(this, ID_JOIN_TABLE, _("&Join Table")),
		wxSizerFlags().Align(wxALIGN_CENTER));

    buttonSizer->Add( 
		new wxButton(this, wxID_CANCEL, _("&Close")),
		wxSizerFlags().Align(wxALIGN_CENTER));

    topSizer->Add(
		buttonSizer, 
		wxSizerFlags().Align(wxALIGN_CENTER));

    SetSizer( topSizer );      // use the sizer for layout

	/* Use the last Position and Size, if available. */

	if ( hoxTablesDialog::s_lastPosition.x >= 0 )
	{
		this->SetPosition( hoxTablesDialog::s_lastPosition );
	}

	if ( hoxTablesDialog::s_lastSize.x > 0 )
	{
		this->SetSize( hoxTablesDialog::s_lastSize );
	}
}

void 
hoxTablesDialog::OnButtonJoin(wxCommandEvent& event)
{
    const char* FNAME = "hoxTablesDialog::OnButtonJoin";

	/* Get the 1st selected item. */

	long selectedIndex = m_listCtrlTables->GetNextItem( -1, 
		                                                wxLIST_NEXT_ALL,
                                                        wxLIST_STATE_SELECTED );
	if ( selectedIndex == -1 ) // No selection?
	{
		wxLogWarning("You need to select a Table.");
		return;
	}

	long selectedId = m_listCtrlTables->GetItemData( selectedIndex );
	m_selectId.Printf("%ld", selectedId);

    m_selectedCommand = COMMAND_ID_JOIN;
    wxLogDebug("%s: Table-Id [%s] is selected to JOIN.", FNAME, m_selectId.c_str());
    Close();
}

void 
hoxTablesDialog::OnButtonNew(wxCommandEvent& event)
{
    m_selectedCommand = COMMAND_ID_NEW;
    Close();
}

void 
hoxTablesDialog::OnButtonRefresh(wxCommandEvent& event)
{
    m_selectedCommand = COMMAND_ID_REFRESH;
    Close();
}

void 
hoxTablesDialog::OnMove(wxMoveEvent& event)
{
	hoxTablesDialog::s_lastPosition = event.GetPosition();
	event.Skip(); // Let the search for the event handler should continue.
}

void 
hoxTablesDialog::OnSize(wxSizeEvent& event)
{
	hoxTablesDialog::s_lastSize = event.GetSize();
	event.Skip(); // Let the search for the event handler should continue.
}

/************************* END OF FILE ***************************************/
