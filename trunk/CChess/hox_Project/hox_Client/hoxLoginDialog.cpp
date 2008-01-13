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
// Name:            hoxLoginDialog.cpp
// Created:         12/28/2007
//
// Description:     The dialog to login a remote server.
/////////////////////////////////////////////////////////////////////////////

#include "hoxLoginDialog.h"
#include "hoxUtility.h"
#include "MyApp.h"    // wxGetApp()

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

enum
{
    HOX_ID_LOGIN = 100
};

// The Site-Type Selection
enum SiteTypeSelection
{
	/* NOTE: The numeric values must be maintained to match
	 *       with the indices of site-types.
	 */

    SITETYPE_SELECTION_CHESSCAPE    = 0,
    SITETYPE_SELECTION_HOXCHESS     = 1,
    SITETYPE_SELECTION_HTTP_POLLING = 2
};

static wxString s_siteTypes[] =
{
    "Chesscape.com",
    "HOXChess Server",
    "HTTP Polling (experiment!!!)"
};

// ----------------------------------------------------------------------------
// Declare event-handler table
// ----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(hoxLoginDialog, wxDialog)
    EVT_BUTTON(HOX_ID_LOGIN, hoxLoginDialog::OnButtonLogin)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxLoginDialog
//-----------------------------------------------------------------------------


hoxLoginDialog::hoxLoginDialog( wxWindow*       parent, 
                                wxWindowID      id, 
                                const wxString& title )
        : wxDialog( parent, id, title, wxDefaultPosition, wxDefaultSize, 
		            wxDEFAULT_DIALOG_STYLE )
        , m_selectedCommand( COMMAND_ID_CANCEL )
		, m_selectedSiteType( hoxSITE_TYPE_UNKNOWN )
		, m_selectedPort( 0 )
{
	/* Read existing login-info from Configuration. */

	wxConfig* config = wxGetApp().GetConfig();
	int      siteChoice    = SITETYPE_SELECTION_CHESSCAPE;
	wxString serverAddress = "games.chesscape.com";
	wxString serverPort    = "3534";
	wxString userName;
	wxString password;

	config->Read("/Sites/Chesscape/username", &userName);
	config->Read("/Sites/Chesscape/password", &password);
	_GetDefaultLoginInfo( siteChoice,
						  serverAddress,
						  serverPort,
						  userName,
						  password );

	/* Create a layout. */

    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

    /* Site-Type. */

    m_radioSiteTypes = new wxRadioBox( this, 
									   wxID_ANY, 
									   _("Site T&ype"), 
									   wxPoint(10,10), 
									   wxDefaultSize, 
									   WXSIZEOF(s_siteTypes), 
									   s_siteTypes, 
									   1, 
									   wxRA_SPECIFY_COLS );
    topSizer->Add( 
		m_radioSiteTypes,
		wxSizerFlags().Border(wxALL, 10).Align(wxALIGN_LEFT).Expand());

	m_radioSiteTypes->SetSelection( siteChoice );

	/* Server-Address. */

	wxBoxSizer* addressSizer = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, _T("Server &Address")), 
		wxHORIZONTAL );

    m_textCtrlAddress = new wxTextCtrl( 
		this, 
		wxID_ANY,
        serverAddress,
        wxDefaultPosition,
        wxSize(200, wxDefaultCoord ));

	m_textCtrlPort = new wxTextCtrl(
		this, 
		wxID_ANY,
        serverPort,
        wxDefaultPosition,
        wxSize(50, wxDefaultCoord ));

    addressSizer->Add( 
		new wxStaticText(this, wxID_ANY, _T("Name/IP: ")),
		wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

    addressSizer->Add( 
		m_textCtrlAddress,
        wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

	addressSizer->AddSpacer(30);

    addressSizer->Add( 
		new wxStaticText(this, wxID_ANY, _T("Port: ")),
        wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

	addressSizer->Add( 
		m_textCtrlPort,
        wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

    topSizer->Add(
		addressSizer, 
		wxSizerFlags().Border(wxALL, 10).Align(wxALIGN_LEFT).Expand());

	/* User-Login. */

	wxBoxSizer* loginSizer = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, _T("Login &Info")), 
		wxHORIZONTAL );

    m_textCtrlUserName = new wxTextCtrl( 
		this, 
		wxID_ANY,
        userName,   // default value
        wxDefaultPosition,
        wxSize(130, wxDefaultCoord ));

	m_textCtrlPassword = new wxTextCtrl(
		this, 
		wxID_ANY,
        password, // default value
        wxDefaultPosition,
        wxSize(100, wxDefaultCoord ),
		wxTE_PASSWORD);

    loginSizer->Add( 
		new wxStaticText(this, wxID_ANY, _T("Username: ")),
		wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

    loginSizer->Add( 
		m_textCtrlUserName,
        wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

	loginSizer->AddSpacer(30);

    loginSizer->Add( 
		new wxStaticText(this, wxID_ANY, _T("Password: ")),
        wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

	loginSizer->Add( 
		m_textCtrlPassword,
        wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

    topSizer->Add(
		loginSizer, 
		wxSizerFlags().Border(wxALL, 10).Align(wxALIGN_LEFT).Expand());

    /* Buttons */

    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );

    buttonSizer->Add( 
		new wxButton(this, HOX_ID_LOGIN, _("&Login")),
        0,                // make vertically unstretchable
        wxALIGN_CENTER ); // no border and centre horizontally);

    buttonSizer->AddSpacer(20);

    buttonSizer->Add( 
		new wxButton(this, wxID_CANCEL, _("&Cancel")),
        0,                // make vertically unstretchable
        wxALIGN_CENTER ); // no border and centre horizontally);

    topSizer->Add(
		buttonSizer,
		wxSizerFlags().Border(wxALL, 10).Align(wxALIGN_CENTER));

    SetSizer( topSizer );      // use the sizer for layout
	topSizer->SetSizeHints( this );   // set size hints to honour minimum size
}

void 
hoxLoginDialog::OnButtonLogin(wxCommandEvent& event)
{
	/* Determine the selected Server-Type */

    switch ( m_radioSiteTypes->GetSelection() )
    {
        case SITETYPE_SELECTION_CHESSCAPE:  
			m_selectedSiteType = hoxSITE_TYPE_CHESSCAPE; 
			break;

        case SITETYPE_SELECTION_HOXCHESS:  
			m_selectedSiteType = hoxSITE_TYPE_REMOTE; 
			break;

        case SITETYPE_SELECTION_HTTP_POLLING:
			m_selectedSiteType = hoxSITE_TYPE_HTTP; 
			break;

        default:
            wxFAIL_MSG( _("Unexpected radio box selection") );
    }

	/* Determine Server-Address (Name/IP and Port) */

	m_selectedAddress = m_textCtrlAddress->GetValue();
	m_selectedPort = ::atoi( m_textCtrlPort->GetValue().c_str() );
	if ( m_selectedPort <= 0  )
	{
		wxLogError("The port [%s] is invalid.", m_textCtrlPort->GetValue().c_str() );
		return;
	}

	/* Determine UserName and Password. */

	m_selectedUserName = m_textCtrlUserName->GetValue();
	m_selectedPassword = m_textCtrlPassword->GetValue();

	/* Return the LOGIN command. */

    m_selectedCommand = COMMAND_ID_LOGIN;

	/* Save login-info for next-time use */

	_SaveDefaultLoginInfo( m_radioSiteTypes->GetSelection(),
						   m_selectedAddress,
						   m_textCtrlPort->GetValue(),
						   m_selectedUserName,
						   m_selectedPassword );

    Close();
}

bool 
hoxLoginDialog::_GetDefaultLoginInfo( int&      siteChoice,
									  wxString& serverAddress,
									  wxString& serverPort,
									  wxString& userName,
									  wxString& password )
{
	const char* FNAME = "hoxLoginDialog::_GetDefaultLoginInfo";

	/* Read the existing settings from Configuration. */
	wxConfig* config = wxGetApp().GetConfig();

	if ( ! config->Read( "/Sites/siteChoice", &siteChoice) )
		return false;  // not found.

	/* Based on the choice, read the server-info + user-Info. */

	wxString siteKey;

    switch ( siteChoice )
    {
        case SITETYPE_SELECTION_CHESSCAPE:    siteKey = "Chesscape";   break;
        case SITETYPE_SELECTION_HOXCHESS:     siteKey = "HOXChess";    break;
        case SITETYPE_SELECTION_HTTP_POLLING: siteKey = "HttpPolling"; break;
        default:
			wxLogDebug("%s: Unknown site choice [%d].", FNAME, siteChoice);
			return false;
    }

	config->SetPath( "/Sites/" + siteKey );

	if ( ! config->Read("serverAddress", &serverAddress) )
		return false;  // not found.

	if ( ! config->Read("serverPort", &serverPort) )
		return false;  // not found.

	if ( ! config->Read("username", &userName) )
		return false;  // not found.

	if ( ! config->Read("password", &password) )
		return false;  // not found.

	return true;   // found old settings?
}

bool 
hoxLoginDialog::_SaveDefaultLoginInfo( const int       siteChoice,
									   const wxString& serverAddress,
									   const wxString& serverPort,
									   const wxString& userName,
									   const wxString& password )
{
	const char* FNAME = "hoxLoginDialog::_SaveDefaultLoginInfo";

	/* Read the existing settings from Configuration. */
	wxConfig* config = wxGetApp().GetConfig();

	config->Write("/Sites/siteChoice", siteChoice);

	/* Based on the choice, read the server-info + user-Info. */

	wxString siteKey;

    switch ( siteChoice )
    {
        case SITETYPE_SELECTION_CHESSCAPE:    siteKey = "Chesscape";   break;
        case SITETYPE_SELECTION_HOXCHESS:     siteKey = "HOXChess";    break;
        case SITETYPE_SELECTION_HTTP_POLLING: siteKey = "HttpPolling"; break;
        default:
			wxLogDebug("%s: Unknown site choice [%d].", FNAME, siteChoice);
			return false;
    }

	config->SetPath( "/Sites/" + siteKey );

	config->Write("serverAddress", serverAddress);
	config->Write("serverPort", serverPort);
	config->Write("username", userName);
	config->Write("password", password);

	return true;   // found old settings?
}

/************************* END OF FILE ***************************************/
