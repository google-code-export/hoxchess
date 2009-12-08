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
// Name:            hoxTablesDialog.cpp
// Created:         10/16/2007
//
// Description:     The dialog of a list of Tables.
/////////////////////////////////////////////////////////////////////////////

#include "hoxTablesDialog.h"
#include "hoxUtil.h"
#include "MyApp.h"    // wxGetApp()

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

enum
{
	ID_CLOSE_DIALOG = wxID_CANCEL,
    ID_JOIN_TABLE   =  100,
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
    EVT_BUTTON(ID_CLOSE_DIALOG, hoxTablesDialog::OnButtonClose)

    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, hoxTablesDialog::OnListItemDClick)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxTablesDialog
//-----------------------------------------------------------------------------

int wxCALLBACK
_CompareTablesCallBack( long item1, 
                        long item2, 
                        long sortData /* not used */)
{
    if (item1 < item2)  return -1;
    if (item1 > item2)  return 1;
    return 0;
}

hoxTablesDialog::hoxTablesDialog( wxWindow*                      parent, 
                                  wxWindowID                     id, 
                                  const wxString&                title,
                                  const hoxNetworkTableInfoList& tableList,
								  unsigned int                   actionFlags )
        : wxDialog( parent, id, title, wxDefaultPosition, wxDefaultSize, 
		            wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
        , m_actionFlags( actionFlags )
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

	/* Create a List-Control to display the table-list. */

	m_listCtrlTables = new wxListCtrl( this, wxID_ANY,
		                               wxDefaultPosition, wxDefaultSize,
		                               wxLC_REPORT | wxLC_SINGLE_SEL);
    long colIndex = 0;
    m_listCtrlTables->InsertColumn( colIndex++, _("Table") );
    m_listCtrlTables->InsertColumn( colIndex++, _("Group") );
    m_listCtrlTables->InsertColumn( colIndex++, _("Type") );
    m_listCtrlTables->InsertColumn( colIndex++, _("Timer") );
    m_listCtrlTables->InsertColumn( colIndex++, _("Red Player") );
    m_listCtrlTables->InsertColumn( colIndex++, _("Black Player") );

    const int nColums = m_listCtrlTables->GetColumnCount();

    long     itemIndex = 0;
    wxString groupInfo;
    wxString redInfo;
    wxString blackInfo;

    for ( hoxNetworkTableInfoList::const_iterator it = tableList.begin(); 
											      it != tableList.end(); ++it )
    {
	    groupInfo = ( it->group == hoxGAME_GROUP_PUBLIC ? "Public" : "Private" );

        if ( it->redId.empty() ) redInfo = "*";
        else  redInfo = it->redId + " (" << it->redScore << ")";

        if ( it->blackId.empty() ) blackInfo = "*";
        else  blackInfo = it->blackId + " (" << it->blackScore << ")";

	    colIndex = 0;
	    itemIndex = m_listCtrlTables->InsertItem(itemIndex, it->id);
	    m_listCtrlTables->SetItemData( itemIndex, ::atoi( it->id.c_str() ) );

	    m_listCtrlTables->SetItem(itemIndex, ++colIndex, groupInfo);
	    m_listCtrlTables->SetItem(itemIndex, ++colIndex, hoxUtil::GameTypeToString(it->gameType));

        m_listCtrlTables->SetItem(itemIndex, ++colIndex,
              hoxUtil::FormatTime(it->initialTime.nGame) + " | " 
            + hoxUtil::FormatTime(it->initialTime.nMove) + " | " 
            + hoxUtil::FormatTime(it->initialTime.nFree));

	    m_listCtrlTables->SetItem(itemIndex, ++colIndex, redInfo );
	    m_listCtrlTables->SetItem(itemIndex, ++colIndex, blackInfo );

	    ++itemIndex;
    }

	/* Set the columns' width. */

    for ( colIndex = 0; colIndex < nColums; ++colIndex )
    {
        m_listCtrlTables->SetColumnWidth( colIndex, wxLIST_AUTOSIZE_USEHEADER );
    }
    if ( m_listCtrlTables->GetItemCount() > 0 )
    {
        for ( colIndex = 2; colIndex < nColums; ++colIndex )
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
    m_listCtrlTables->SortItems( _CompareTablesCallBack, 0 /* sortData */ );

    topSizer->Add( m_listCtrlTables,
		           wxSizerFlags(1).Expand().Border(wxALL, 10));

    /* Buttons... */
	wxButton* buttonRefresh = new wxButton(this, ID_REFRESH_LIST, _("&Refresh"));
	wxButton* buttonNew     = new wxButton(this, ID_NEW_TABLE,    _("&New Table"));
	wxButton* buttonJoin    = new wxButton(this, ID_JOIN_TABLE,   _("&Join Table"));
	wxButton* buttonClose   = new wxButton(this, ID_CLOSE_DIALOG, _("&Close"));

    /* Disable certain buttons based on the input Action Flags. */
    buttonNew->Enable(  (m_actionFlags & hoxSITE_ACTION_NEW)  != 0 );
    buttonJoin->Enable( (m_actionFlags & hoxSITE_ACTION_JOIN) != 0 );

    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );

    buttonSizer->Add( buttonRefresh, wxSizerFlags().Align(wxALIGN_CENTER));
    buttonSizer->Add( buttonNew, wxSizerFlags().Align(wxALIGN_CENTER));
    buttonSizer->AddSpacer(30);
    buttonSizer->Add( buttonJoin, wxSizerFlags().Align(wxALIGN_CENTER));
    buttonSizer->Add( buttonClose, wxSizerFlags().Align(wxALIGN_CENTER));

    topSizer->Add( buttonSizer, wxSizerFlags().Align(wxALIGN_CENTER));
    SetSizer( topSizer );      // use the sizer for layout

    /* Use the last Position and Size, if available. */
    wxPoint lastPosition; 
    wxSize  lastSize;
    if ( _GetDefaultLayout( lastPosition, lastSize ) )
    {
        this->SetPosition( lastPosition );
        this->SetSize( lastSize );
    }
}

void 
hoxTablesDialog::OnButtonJoin( wxCommandEvent& event )
{
    if ( (m_actionFlags & hoxSITE_ACTION_JOIN) == 0 )
    {
        wxLogDebug("%s: JOIN action is disabled. Ignore command.", __FUNCTION__);
        return;
    }

	/* Get the 1st selected item. */

	long selectedIndex = m_listCtrlTables->GetNextItem( -1, 
		                                                wxLIST_NEXT_ALL,
                                                        wxLIST_STATE_SELECTED );
	if ( selectedIndex == -1 ) // No selection?
	{
		wxLogWarning("You need to select a Table.");
		return;
	}

	const long selectedId = m_listCtrlTables->GetItemData( selectedIndex );
	m_selectId.Printf("%ld", selectedId);

    _OnDialogClosed( COMMAND_ID_JOIN );
}

void 
hoxTablesDialog::OnButtonNew( wxCommandEvent& event )
{
    _OnDialogClosed( COMMAND_ID_NEW );
}

void 
hoxTablesDialog::OnButtonRefresh( wxCommandEvent& event )
{
    _OnDialogClosed( COMMAND_ID_REFRESH );
}

void 
hoxTablesDialog::OnButtonClose( wxCommandEvent& event )
{
    /* NOTE: This handler must be explicitly defined so that
     *       we could save the (size,location) of the dialog.
     *       Relying solely on wxID_CANCEL is not enough
     *       (to trigger the 'Close' handler).
     */

    _SaveDefaultLayout( this->GetPosition(), this->GetSize() );
    event.Skip(); // Let the search for the event handler should continue.
}

void 
hoxTablesDialog::OnSiteDeleted()
{
    _OnDialogClosed( COMMAND_ID_SITE_DELETED );
}

void
hoxTablesDialog::OnListItemDClick( wxListEvent& event )
{
    wxCommandEvent DUMMY_event;
    this->OnButtonJoin( DUMMY_event );
}

void
hoxTablesDialog::_OnDialogClosed( int rc )
{
    _SaveDefaultLayout( this->GetPosition(), this->GetSize() );
    EndDialog( rc );
}

bool 
hoxTablesDialog::_GetDefaultLayout( wxPoint& position, 
								    wxSize&  size )
{
	position = wxPoint( -1, -1 );
	size = wxSize( 0, 0 );

	// Read the existing layout from Configuration.
	wxConfig* config = wxGetApp().GetConfig();

	if ( ! config->Read("/Layout/TablesDialog/position/x", &position.x) )
		return false;  // not found.

	if ( ! config->Read("/Layout/TablesDialog/position/y", &position.y) )
		return false;  // not found.

	if ( ! config->Read("/Layout/TablesDialog/size/x", &size.x) )
		return false;  // not found.

	if ( ! config->Read("/Layout/TablesDialog/size/y", &size.y) )
		return false;  // not found.

	return true;   // found old layout?
}

void
hoxTablesDialog::_SaveDefaultLayout( const wxPoint& position,
                                     const wxSize&  size )
{
	wxConfig* config = wxGetApp().GetConfig();

	config->Write("/Layout/TablesDialog/position/x", position.x);
	config->Write("/Layout/TablesDialog/position/y", position.y);

	config->Write("/Layout/TablesDialog/size/x", size.x);
	config->Write("/Layout/TablesDialog/size/y", size.y);
}

/************************* END OF FILE ***************************************/
