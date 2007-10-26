/////////////////////////////////////////////////////////////////////////////
// Name:            hoxTablesDialog.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/16/2007
//
// Description:     The dialog of a list of Tables.
/////////////////////////////////////////////////////////////////////////////

#include "hoxTablesDialog.h"
#include "hoxLocalPlayer.h"

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
        : wxDialog(parent, id, title)
        , m_selectedCommand( COMMAND_ID_UNKNOWN )
{
    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    m_tablesListBox = new wxListBox(this, wxID_ANY);
    wxString item;
    for ( hoxNetworkTableInfoList::const_iterator it = tableList.begin(); 
                                                 it != tableList.end(); ++it )
    {
        item = "";
        item << "Table #" << (*it)->id 
             << " (status: " << (*it)->status << ") "
             << (*it)->redId << " vs. " << (*it)->blackId;
        m_tablesListBox->Append( item, (*it) );
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
hoxTablesDialog::OnButtonJoin(wxCommandEvent& WXUNUSED(event))
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
    wxLogDebug(wxString::Format("%s: Table-Id [%s] is selected to JOIN.", FNAME, m_selectId));
    Close();
}

void 
hoxTablesDialog::OnButtonNew(wxCommandEvent& WXUNUSED(event))
{
    wxLogDebug("Return [NEW] as the selected command to the caller.");
    m_selectedCommand = COMMAND_ID_NEW;
    Close();
}


/************************* END OF FILE ***************************************/
