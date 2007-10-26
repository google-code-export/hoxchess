/////////////////////////////////////////////////////////////////////////////
// Name:            hoxTablesDialog.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/16/2007
//
// Description:     The dialog of a list of Tables.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_TABLES_DIALOG_H_
#define __INCLUDED_HOX_TABLES_DIALOG_H_

#include "wx/wx.h"
#include "hoxTypes.h"

/* Forward declarations */
class hoxWWWPlayer;

// ----------------------------------------------------------------------------
// The Tables-Dialog class
// ----------------------------------------------------------------------------

class hoxTablesDialog : public wxDialog
{
public:
    enum CommandId
    {
        COMMAND_ID_UNKNOWN = -1,
        COMMAND_ID_JOIN,
        COMMAND_ID_NEW
    };

    hoxTablesDialog( wxWindow*                      parent, 
                     wxWindowID                     id, 
                     const wxString&                title,
                     const hoxNetworkTableInfoList& tableList );

    void OnButtonJoin(wxCommandEvent& WXUNUSED(event));
    void OnButtonNew(wxCommandEvent& WXUNUSED(event));

    CommandId GetSelectedCommand() const { return m_selectedCommand; }
    wxString GetSelectedId() const { return m_selectId; }

private:
    CommandId  m_selectedCommand;

    wxListBox* m_tablesListBox;
    wxString   m_selectId;

    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_TABLES_DIALOG_H_ */
