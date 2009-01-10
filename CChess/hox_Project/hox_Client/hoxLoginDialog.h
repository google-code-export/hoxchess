/***************************************************************************
 *  Copyright 2007, 2008, 2009 Huy Phan  <huyphan@playxiangqi.com>         *
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
// Name:            hoxLoginDialog.h
// Created:         12/28/2007
//
// Description:     The dialog to login a remote server.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_LOGIN_DIALOG_H_
#define __INCLUDED_HOX_LOGIN_DIALOG_H_

#include <wx/wx.h>
#include "hoxTypes.h"


// ----------------------------------------------------------------------------
// The Login-Dialog class
// ----------------------------------------------------------------------------

class hoxLoginDialog : public wxDialog
{
public:
    enum CommandId
    {
	    COMMAND_ID_CANCEL,
        COMMAND_ID_LOGIN
    };

    hoxLoginDialog( wxWindow*        parent, 
                    wxWindowID       id, 
                    const wxString&  title );

    void OnSiteSelected(wxCommandEvent& event);
    void OnButtonLogin(wxCommandEvent& event);

    CommandId      GetSelectedCommand()  const { return m_selectedCommand; }
	hoxSiteType    GetSelectedSiteType() const { return m_selectedSiteType; }
	const wxString GetSelectedAddress()  const { return m_selectedAddress; }
	int            GetSelectedPort()     const { return m_selectedPort; }
	const wxString GetSelectedUserName() const { return m_selectedUserName; }
	const wxString GetSelectedPassword() const { return m_selectedPassword; }

private:
	bool _GetDefaultLoginInfo( int& siteChoice );
	bool _SaveDefaultLoginInfo( const int siteChoice );
    const wxString _GenerateGuestUserName();

private:
	wxRadioBox*  m_radioSiteTypes;
	wxTextCtrl*  m_textCtrlAddress;
	wxTextCtrl*  m_textCtrlPort;
	wxTextCtrl*  m_textCtrlUserName;
	wxTextCtrl*  m_textCtrlPassword;

    CommandId    m_selectedCommand;
	hoxSiteType  m_selectedSiteType;
	wxString     m_selectedAddress;
	int          m_selectedPort;
	wxString     m_selectedUserName;
	wxString     m_selectedPassword;

    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_LOGIN_DIALOG_H_ */
