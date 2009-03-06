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
// Name:            hoxLoginDialog.cpp
// Created:         12/28/2007
//
// Description:     The dialog to login a remote server.
/////////////////////////////////////////////////////////////////////////////

#include "hoxLoginDialog.h"
#include "hoxUtil.h"
#include "MyApp.h"    // wxGetApp()

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

enum
{
    HOX_ID_SITE_TYPE = 100,
    HOX_ID_LOGIN,
    HOX_ID_GUEST
};

enum SiteTypeSelection
{
	/* NOTE: The numeric values must be maintained to match
	 *       with the indices of site-types.
	 */

    SITETYPE_SELECTION_HOXCHESS     = 0,
    SITETYPE_SELECTION_CHESSCAPE    = 1,

    SITETYPE_SELECTION_MAX    // MAX Site marker!!!
};

struct SiteInfo
{
    hoxSiteType type;
    wxString    sId;
    wxString    sDisplayName;
    wxString    sAddress;
    wxString    sPort;
    wxString    sUsername;
    wxString    sPassword;
};

/* Default sites info. */
static SiteInfo s_siteList[] =
{
    { hoxSITE_TYPE_REMOTE, "HOXChess", "PlayXiangqi.com", "games.playxiangqi.com", "8000", "", "" },
    { hoxSITE_TYPE_CHESSCAPE, "Chesscape", "Chesscape.com", "games.chesscape.com", "3534", "", "" },
};

// ----------------------------------------------------------------------------
// Declare event-handler table
// ----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(hoxLoginDialog, wxDialog)
    EVT_RADIOBOX(HOX_ID_SITE_TYPE, hoxLoginDialog::OnSiteSelected)
    EVT_BUTTON(HOX_ID_LOGIN, hoxLoginDialog::OnButtonLogin)
    EVT_BUTTON(HOX_ID_GUEST, hoxLoginDialog::OnButtonLogin)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxLoginDialog
//-----------------------------------------------------------------------------


hoxLoginDialog::hoxLoginDialog( wxWindow*       parent, 
                                wxWindowID      id, 
                                const wxString& title )
        : wxDialog( parent, id, title )
        , m_selectedCommand( COMMAND_ID_CANCEL )
		, m_selectedSiteType( hoxSITE_TYPE_UNKNOWN )
		, m_selectedPort( 0 )
{
	/* Read existing login-info from Configuration. */

	int   siteChoice = 0;

    if ( ! _GetDefaultLoginInfo( siteChoice ) ) // not found
    {
        siteChoice = SITETYPE_SELECTION_HOXCHESS; // Set the default
    }

    /* Get the list of Display Names of Sites. */

    wxArrayString siteTypes;
    for ( int i = 0; i < SITETYPE_SELECTION_MAX; ++i )
    {
        siteTypes.Add( s_siteList[i].sDisplayName );
    }

	/* Create a layout. */

    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

    /* Site-Type. */

    m_radioSiteTypes = new wxRadioBox( this, 
									   HOX_ID_SITE_TYPE, 
									   _("Site T&ype"), 
									   wxPoint(10,10), 
									   wxDefaultSize, 
                                       siteTypes,
									   1, 
									   wxRA_SPECIFY_COLS );
    topSizer->Add( 
		m_radioSiteTypes,
		wxSizerFlags().Border(wxALL, 10).Align(wxALIGN_LEFT).Expand());

	m_radioSiteTypes->SetSelection( siteChoice );

	/* Server-Address. */

	wxBoxSizer* addressSizer = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, _("Server &Address")), 
		wxHORIZONTAL );

    m_textCtrlAddress = new wxTextCtrl( 
		this, 
		wxID_ANY,
        s_siteList[siteChoice].sAddress,
        wxDefaultPosition,
        wxSize(200, wxDefaultCoord ));

	m_textCtrlPort = new wxTextCtrl(
		this, 
		wxID_ANY,
        s_siteList[siteChoice].sPort,
        wxDefaultPosition,
        wxSize(50, wxDefaultCoord ));

    addressSizer->Add( 
		new wxStaticText(this, wxID_ANY, _("Name/IP: ")),
		wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

    addressSizer->Add( 
		m_textCtrlAddress,
        wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

	addressSizer->AddSpacer(30);

    addressSizer->Add( 
		new wxStaticText(this, wxID_ANY, _("Port: ")),
        wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

	addressSizer->Add( 
		m_textCtrlPort,
        wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

    topSizer->Add(
		addressSizer, 
		wxSizerFlags().Border(wxALL, 10).Align(wxALIGN_LEFT).Expand());

	/* User-Login. */

	wxBoxSizer* loginSizer = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, _("Login &Info")), 
		wxHORIZONTAL );

    m_textCtrlUserName = new wxTextCtrl( 
		this, 
		wxID_ANY,
        s_siteList[siteChoice].sUsername,
        wxDefaultPosition,
        wxSize(130, wxDefaultCoord ));

	m_textCtrlPassword = new wxTextCtrl(
		this, 
		wxID_ANY,
        s_siteList[siteChoice].sPassword,
        wxDefaultPosition,
        wxSize(100, wxDefaultCoord ),
		wxTE_PASSWORD);

    loginSizer->Add( 
		new wxStaticText(this, wxID_ANY, _("Username: ")),
		wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

    loginSizer->Add( 
		m_textCtrlUserName,
        wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

	loginSizer->AddSpacer(30);

    loginSizer->Add( 
		new wxStaticText(this, wxID_ANY, _("Password: ")),
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
		new wxButton(this, HOX_ID_GUEST, _("&Guest")),
        0,                // make vertically unstretchable
        wxALIGN_CENTER ); // no border and centre horizontally

    buttonSizer->AddSpacer(40);

    buttonSizer->Add( 
		new wxButton(this, HOX_ID_LOGIN, _("&Login")),
        0,                // make vertically unstretchable
        wxALIGN_CENTER ); // no border and centre horizontally

    buttonSizer->AddSpacer(15);

    buttonSizer->Add( 
		new wxButton(this, wxID_CANCEL, _("&Cancel")),
        0,                // make vertically unstretchable
        wxALIGN_CENTER ); // no border and centre horizontally

    topSizer->Add(
		buttonSizer,
		wxSizerFlags().Border(wxALL, 10).Align(wxALIGN_CENTER));

    SetSizer( topSizer );      // use the sizer for layout
	topSizer->SetSizeHints( this );   // set size hints to honour minimum size
}

void 
hoxLoginDialog::OnSiteSelected(wxCommandEvent& event)
{
    int nSelectedType = m_radioSiteTypes->GetSelection();

    if ( nSelectedType < 0 || nSelectedType >= SITETYPE_SELECTION_MAX )
    {
        wxLogError("%s: Unknown site choice [%d].", __FUNCTION__, nSelectedType);
        return;
    }

    m_textCtrlAddress->SetValue( s_siteList[nSelectedType].sAddress );
    m_textCtrlPort->SetValue( s_siteList[nSelectedType].sPort );
    m_textCtrlUserName->SetValue( s_siteList[nSelectedType].sUsername );
    m_textCtrlPassword->SetValue( s_siteList[nSelectedType].sPassword );
}

void 
hoxLoginDialog::OnButtonLogin(wxCommandEvent& event)
{
	/* Determine the selected Site-Type */

    int nSelectedType = m_radioSiteTypes->GetSelection();

    if ( nSelectedType < 0 || nSelectedType >= SITETYPE_SELECTION_MAX )
    {
        wxLogError("%s: Unknown site choice [%d].", __FUNCTION__, nSelectedType);
        return;
    }

    m_selectedSiteType = s_siteList[nSelectedType].type;

	/* Determine Server-Address (Name/IP and Port) */

	m_selectedAddress = m_textCtrlAddress->GetValue();
	m_selectedPort = ::atoi( m_textCtrlPort->GetValue().c_str() );
	if ( m_selectedPort <= 0  )
	{
		wxLogError("The port [%s] is invalid.", m_textCtrlPort->GetValue().c_str() );
		return;
	}

	/* Determine UserName and Password. */

    if ( event.GetId() == HOX_ID_LOGIN )
    {
	    m_selectedUserName = m_textCtrlUserName->GetValue();
    }
    else
    {
        m_selectedUserName = _GenerateGuestUserName();
    }
	m_selectedPassword = m_textCtrlPassword->GetValue();

	/* Return the LOGIN command. */

    m_selectedCommand = COMMAND_ID_LOGIN;

	/* Save login-info for next-time use */

    s_siteList[nSelectedType].sAddress  = m_selectedAddress;
    s_siteList[nSelectedType].sPort     = m_textCtrlPort->GetValue();
    if ( event.GetId() == HOX_ID_LOGIN )
    {
        s_siteList[nSelectedType].sUsername = m_selectedUserName;
        s_siteList[nSelectedType].sPassword = m_selectedPassword;
    }

	_SaveDefaultLoginInfo( nSelectedType );

    Close();
}

bool 
hoxLoginDialog::_GetDefaultLoginInfo( int& siteChoice )
{
	wxConfig* config = wxGetApp().GetConfig();

	if ( ! config->Read( "/Sites/siteChoice", &siteChoice) )
		return false;  // not found.

	/* Based on the choice, read the server-info + user-Info. */

    if ( siteChoice < 0 || siteChoice >= SITETYPE_SELECTION_MAX )
    {
        wxLogError("%s: Unknown site choice [%d].", __FUNCTION__, siteChoice);
        return false;
    }

    for ( int i = 0; i < SITETYPE_SELECTION_MAX; ++i )
    {
	    config->SetPath( "/Sites/" + s_siteList[i].sId );

        config->Read("serverAddress", &(s_siteList[i].sAddress));
	    config->Read("serverPort",    &(s_siteList[i].sPort));
	    config->Read("username",      &(s_siteList[i].sUsername));
	    config->Read("password",      &(s_siteList[i].sPassword));
    }

	return true;   // found old settings?
}

bool 
hoxLoginDialog::_SaveDefaultLoginInfo( const int siteChoice )
{
	wxConfig* config = wxGetApp().GetConfig();

	config->Write("/Sites/siteChoice", siteChoice);

    for ( int i = 0; i < SITETYPE_SELECTION_MAX; ++i )
    {
	    config->SetPath( "/Sites/" + s_siteList[i].sId );

        config->Write("serverAddress", s_siteList[i].sAddress);
	    config->Write("serverPort",    s_siteList[i].sPort);
	    config->Write("username",      s_siteList[i].sUsername);
	    config->Write("password",      s_siteList[i].sPassword);
    }

	return true;   // OK
}

const wxString
hoxLoginDialog::_GenerateGuestUserName()
{
    const unsigned int MAX_GUEST_ID = 10000;

    const int randNum = hoxUtil::GenerateRandomNumber( MAX_GUEST_ID );
    wxString sGuestId;
    sGuestId << hoxGUEST_PREFIX << "hox" << randNum;

    return sGuestId;
}

/************************* END OF FILE ***************************************/
