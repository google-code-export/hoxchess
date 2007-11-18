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
// Name:            hoxTablesDialog.h
// Created:         10/16/2007
//
// Description:     The dialog of a list of Tables.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_TABLES_DIALOG_H_
#define __INCLUDED_HOX_TABLES_DIALOG_H_

#include "wx/wx.h"
#include "hoxTypes.h"


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

    void OnButtonJoin(wxCommandEvent& event);
    void OnButtonNew(wxCommandEvent& event);

    CommandId GetSelectedCommand() const { return m_selectedCommand; }
    wxString GetSelectedId() const { return m_selectId; }

private:
    CommandId  m_selectedCommand;

    wxListBox* m_tablesListBox;
    wxString   m_selectId;

    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_TABLES_DIALOG_H_ */
