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
// Name:            hoxTablesDialog.h
// Created:         10/16/2007
//
// Description:     The dialog of a list of Tables.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_TABLES_DIALOG_H__
#define __INCLUDED_HOX_TABLES_DIALOG_H__

#include <wx/wx.h>
#include <wx/listctrl.h>
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
        COMMAND_ID_NEW,
		COMMAND_ID_REFRESH
    };

    hoxTablesDialog( wxWindow*                      parent, 
                     wxWindowID                     id, 
                     const wxString&                title,
                     const hoxNetworkTableInfoList& tableList,
					 unsigned int                   actionFlags );

    void OnButtonJoin(wxCommandEvent& event);
    void OnButtonNew(wxCommandEvent& event);
	void OnButtonRefresh(wxCommandEvent& event);
	void OnButtonClose(wxCommandEvent& event);

	void OnClose(wxCloseEvent& event);
    void OnListItemDClick(wxListEvent& event);

    CommandId GetSelectedCommand() const { return m_selectedCommand; }
    wxString GetSelectedId() const { return m_selectId; }

private:
	bool _GetDefaultLayout( wxPoint& position, wxSize& size );
	bool _SaveDefaultLayout( const wxPoint& position, const wxSize& size );

private:
	wxListCtrl*   m_listCtrlTables;

    CommandId     m_selectedCommand;
    wxString      m_selectId;

    unsigned int  m_actionFlags;

    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_TABLES_DIALOG_H__ */
