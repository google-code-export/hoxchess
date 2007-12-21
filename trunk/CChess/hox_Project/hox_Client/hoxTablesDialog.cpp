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
    ID_NEW_TABLE
};

// ----------------------------------------------------------------------------
// Declare event-handler table
// ----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(hoxTablesDialog, wxDialog)
    EVT_BUTTON(ID_JOIN_TABLE, hoxTablesDialog::OnButtonJoin)
    EVT_BUTTON(ID_NEW_TABLE, hoxTablesDialog::OnButtonNew)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxTablesDialog
//-----------------------------------------------------------------------------


hoxTablesDialog::hoxTablesDialog( wxWindow*       parent, 
                                  wxWindowID      id, 
                                  const wxString& title,
                                  const hoxNetworkTableInfoList& tableList)
        : wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, 
		           wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
        , m_selectedCommand( COMMAND_ID_UNKNOWN )
{
    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    m_tablesListBox = new wxListBox(this, wxID_ANY);
    wxString item;
	wxString redId;
	wxString blackId;
    for ( hoxNetworkTableInfoList::const_iterator it = tableList.begin(); 
                                                 it != tableList.end(); ++it )
    {
		redId = (it->redId.empty() ? "0" : it->redId );
		blackId = (it->blackId.empty() ? "0" : it->blackId );

        item = "";
        item << "#" << it->id 
             << " [" << it->status << "] "
			 << " [" << hoxUtility::FormatTime(it->initialTime.nGame) << "] "
			 << " [" << hoxUtility::GameTypeToString(it->gameType) << "] "
			 << redId << "(" << it->redScore << ")" << " vs. " 
			 << blackId << "(" << it->blackScore << ")";
        m_tablesListBox->Append( item, const_cast<hoxNetworkTableInfo*>( &(*it) ) );
    }
    // Select the 1st table, if any.
    if ( m_tablesListBox->GetCount() > 0 )
    {
        m_tablesListBox->SetSelection( 0 );
    }

    topsizer->Add( m_tablesListBox,
         1,            // make vertically stretchable
         wxEXPAND |    // make horizontally stretchable
         wxALL,        //   and make border all around
         10 );         // set border width to 10tBox );

    /* Buttons... */

    wxBoxSizer *buttonsizer = new wxBoxSizer( wxHORIZONTAL );

    buttonsizer->Add( new wxButton(this, ID_NEW_TABLE, _("&New Table")),
        0,                // make vertically unstretchable
        wxALIGN_CENTER ); // no border and centre horizontally);

    buttonsizer->AddSpacer(30);

    buttonsizer->Add( new wxButton(this, ID_JOIN_TABLE, _("&Join Table")),
        0,                // make vertically unstretchable
        wxALIGN_CENTER ); // no border and centre horizontally);

    buttonsizer->Add( new wxButton(this, wxID_CANCEL, _("&Close")),
        0,                // make vertically unstretchable
        wxALIGN_CENTER ); // no border and centre horizontally);

    topsizer->Add(buttonsizer, 
        0,                // make vertically unstretchable
        wxALIGN_CENTER ); // no border and centre horizontally);

    SetSizer( topsizer );      // use the sizer for layout
}

void 
hoxTablesDialog::OnButtonJoin(wxCommandEvent& event)
{
    const char* FNAME = "hoxTablesDialog::OnButtonJoin";
    int selection = m_tablesListBox->GetSelection();
    if ( selection != wxNOT_FOUND )
    {
        hoxNetworkTableInfo* tableInfo  /* TODO: better way than reinterpret_cast? */
            = wx_reinterpret_cast( hoxNetworkTableInfo*, 
                                   m_tablesListBox->GetClientData( selection ) );
        wxASSERT( tableInfo != NULL );
        m_selectId = tableInfo->id;
    }

    m_selectedCommand = COMMAND_ID_JOIN;
    wxLogDebug(wxString::Format("%s: Table-Id [%s] is selected to JOIN.", FNAME, m_selectId.c_str()));
    Close();
}

void 
hoxTablesDialog::OnButtonNew(wxCommandEvent& event)
{
    wxLogDebug("Return [NEW] as the selected command to the caller.");
    m_selectedCommand = COMMAND_ID_NEW;
    Close();
}


/************************* END OF FILE ***************************************/
