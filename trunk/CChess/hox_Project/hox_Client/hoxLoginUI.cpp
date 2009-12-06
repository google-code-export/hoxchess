/////////////////////////////////////////////////////////////////////////////
// Name:        hoxLoginUI.cpp
// Purpose:     
// Author:      Huy Phan
// Modified by: 
// Created:     3/14/2009 8:59:35 PM
// RCS-ID:      
// Copyright:   Copyright 2007-2009 Huy Phan  <huyphan@playxiangqi.com>
// Licence:     GNU General Public License v3
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "../../hox_Project/hox_Client/hoxLoginUI.h"
#include "hoxUtil.h"
#include "MyApp.h"

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
    { hoxSITE_TYPE_REMOTE, "HOXChess", "PlayXiangqi.com", "games.playxiangqi.com", "80",   "", "" },
    { hoxSITE_TYPE_CHESSCAPE, "Chesscape", "Chesscape.com", "games.chesscape.com", "3534", "", "" },
};


////@begin XPM images
////@end XPM images


/*!
 * hoxLoginUI type definition
 */

IMPLEMENT_DYNAMIC_CLASS( hoxLoginUI, wxDialog )


/*!
 * hoxLoginUI event table definition
 */

BEGIN_EVENT_TABLE( hoxLoginUI, wxDialog )

////@begin hoxLoginUI event table entries
    EVT_RADIOBUTTON( ID_RADIO_PLAYXIANGQI, hoxLoginUI::OnRadioSiteTypeSelected )

    EVT_RADIOBUTTON( ID_RADIO_CHESSCAPE, hoxLoginUI::OnRadioSiteTypeSelected )

    EVT_BUTTON( ID_BTN_GUEST, hoxLoginUI::OnLoginClick )

    EVT_BUTTON( wxID_OK, hoxLoginUI::OnLoginClick )

////@end hoxLoginUI event table entries

END_EVENT_TABLE()


/*!
 * hoxLoginUI constructors
 */

hoxLoginUI::hoxLoginUI()
{
    Init();
}

hoxLoginUI::hoxLoginUI( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * hoxLoginUI creator
 */

bool hoxLoginUI::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin hoxLoginUI creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end hoxLoginUI creation

    switch ( m_siteType )
    {
        case hoxSITE_TYPE_REMOTE:
            m_radioPlayXiangqi->SetValue(true);
            break;
        case hoxSITE_TYPE_CHESSCAPE:
            m_radioChesscape->SetValue(true);
            break;
        default:
            break;
    }

    /* Initialize the values. */
    m_textIP->SetValue( m_sIP );
    m_textPort->SetValue( m_sPort );
    m_textUsername->SetValue( m_sUsername );
    m_textPassword->SetValue( m_sPassword );

    return true;
}


/*!
 * hoxLoginUI destructor
 */

hoxLoginUI::~hoxLoginUI()
{
////@begin hoxLoginUI destruction
////@end hoxLoginUI destruction
}


/*!
 * Member initialisation
 */

void hoxLoginUI::Init()
{
////@begin hoxLoginUI member initialisation
    m_nType = 0;
    m_radioPlayXiangqi = NULL;
    m_radioChesscape = NULL;
    m_textIP = NULL;
    m_textPort = NULL;
    m_textUsername = NULL;
    m_textPassword = NULL;
////@end hoxLoginUI member initialisation
    
	/* Read existing login-info from Configuration. */
	m_nType = 0;

    if ( ! _GetDefaultLoginInfo( m_nType ) ) // not found
    {
        m_nType = SITETYPE_SELECTION_HOXCHESS; // Set the default
    }
    
    /* Setup the default value. */
    m_siteType  = s_siteList[m_nType].type;
    m_sUsername = s_siteList[m_nType].sUsername;
    m_sPassword = s_siteList[m_nType].sPassword;
    m_sIP       = s_siteList[m_nType].sAddress;
    m_sPort     = s_siteList[m_nType].sPort;
}


/*!
 * Control creation for hoxLoginUI
 */

void hoxLoginUI::CreateControls()
{    
////@begin hoxLoginUI content construction
    hoxLoginUI* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, _T(""));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxGROW|wxALL, 5);

    wxGridSizer* itemGridSizer4 = new wxGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer3->Add(itemGridSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_radioPlayXiangqi = new wxRadioButton( itemDialog1, ID_RADIO_PLAYXIANGQI, _("PlayXiangqi"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_radioPlayXiangqi->SetValue(true);
    itemGridSizer4->Add(m_radioPlayXiangqi, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl6 = new wxHyperlinkCtrl( itemDialog1, ID_HYPERLINKCTRL, _("www.playxiangqi.com"), _T("www.playxiangqi.com"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemGridSizer4->Add(itemHyperlinkCtrl6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_radioChesscape = new wxRadioButton( itemDialog1, ID_RADIO_CHESSCAPE, _("Chesscape"), wxDefaultPosition, wxDefaultSize, 0 );
    m_radioChesscape->SetValue(false);
    itemGridSizer4->Add(m_radioChesscape, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl8 = new wxHyperlinkCtrl( itemDialog1, hoxHYPERLINKCTRL, _("www.chesscape.com"), _T("www.chesscape.com"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemGridSizer4->Add(itemHyperlinkCtrl8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCollapsiblePane* itemCollapsiblePane9 = new wxCollapsiblePane( itemDialog1, ID_COLLAPSIBLEPANE, _("Server &Address"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE );
    itemBoxSizer2->Add(itemCollapsiblePane9, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer10Static = new wxStaticBox(itemCollapsiblePane9->GetPane(), wxID_ANY, _T(""));
    wxStaticBoxSizer* itemStaticBoxSizer10 = new wxStaticBoxSizer(itemStaticBoxSizer10Static, wxHORIZONTAL);
    itemCollapsiblePane9->GetPane()->SetSizer(itemStaticBoxSizer10);

    wxStaticText* itemStaticText11 = new wxStaticText( itemCollapsiblePane9->GetPane(), wxID_STATIC, _("Name/IP:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer10->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_textIP = new wxTextCtrl( itemCollapsiblePane9->GetPane(), ID_TEXTCTRL_SERVER_NAME, _T(""), wxDefaultPosition, wxSize(200, -1), 0 );
    itemStaticBoxSizer10->Add(m_textIP, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemStaticBoxSizer10->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemCollapsiblePane9->GetPane(), wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer10->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_textPort = new wxTextCtrl( itemCollapsiblePane9->GetPane(), ID_TEXTCTRL_SERVER_PORT, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemStaticBoxSizer10->Add(m_textPort, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer16Static = new wxStaticBox(itemDialog1, wxID_ANY, _T(""));
    wxStaticBoxSizer* itemStaticBoxSizer16 = new wxStaticBoxSizer(itemStaticBoxSizer16Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer16, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText17 = new wxStaticText( itemDialog1, wxID_STATIC, _("Username:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer16->Add(itemStaticText17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_textUsername = new wxTextCtrl( itemDialog1, ID_TEXTCTRL_USERNAME, _T(""), wxDefaultPosition, wxSize(130, -1), 0 );
    itemStaticBoxSizer16->Add(m_textUsername, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemStaticBoxSizer16->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText20 = new wxStaticText( itemDialog1, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer16->Add(itemStaticText20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_textPassword = new wxTextCtrl( itemDialog1, ID_TEXTCTRL_PASSWORD, _T(""), wxDefaultPosition, wxSize(100, -1), wxTE_PASSWORD );
    itemStaticBoxSizer16->Add(m_textPassword, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer23, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton24 = new wxButton( itemDialog1, ID_BTN_GUEST, _("Login as &Guest"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemButton24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer23->Add(15, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton26 = new wxButton( itemDialog1, wxID_OK, _("&Login"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton26->SetDefault();
    itemBoxSizer23->Add(itemButton26, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton27 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemButton27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_textPort->SetValidator( wxTextValidator(wxFILTER_DIGITS, & m_sPort) );
////@end hoxLoginUI content construction
}


/*!
 * Should we show tooltips?
 */

bool hoxLoginUI::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap hoxLoginUI::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin hoxLoginUI bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end hoxLoginUI bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon hoxLoginUI::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin hoxLoginUI icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end hoxLoginUI icon retrieval
}




/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIO_PLAYXIANGQI
 */

void hoxLoginUI::OnRadioSiteTypeSelected( wxCommandEvent& event )
{
    switch ( event.GetId() )
    {
        case ID_RADIO_PLAYXIANGQI:
            m_siteType = hoxSITE_TYPE_REMOTE;
            m_nType = 0;
            break;
        case ID_RADIO_CHESSCAPE:
            m_siteType = hoxSITE_TYPE_CHESSCAPE;
            m_nType = 1;
            break;
        default:
            wxLogError("%s: Unknown site ID [%d].", __FUNCTION__, event.GetId());
            return;
    }

    /* Double check here. */
    if ( m_nType < 0 || m_nType >= SITETYPE_SELECTION_MAX )
    {
        wxLogError("%s: Logic error - Invalid site choice [%d].", __FUNCTION__, m_nType);
        return;
    }

    m_textIP->SetValue( s_siteList[m_nType].sAddress );
    m_textPort->SetValue( s_siteList[m_nType].sPort );
    m_textUsername->SetValue( s_siteList[m_nType].sUsername );
    m_textPassword->SetValue( s_siteList[m_nType].sPassword );

}

bool 
hoxLoginUI::_GetDefaultLoginInfo( int& siteChoice )
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
hoxLoginUI::_SaveDefaultLoginInfo( const int siteChoice )
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
hoxLoginUI::_GenerateGuestUserName() const
{
    const unsigned int MAX_GUEST_ID = 10000;

    const int randNum = hoxUtil::GenerateRandomNumber( MAX_GUEST_ID );
    wxString sGuestId;
    sGuestId << hoxGUEST_PREFIX << "hx" << randNum;

    return sGuestId;
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void hoxLoginUI::OnLoginClick( wxCommandEvent& event )
{
	m_sIP   = m_textIP->GetValue();
    m_sPort = m_textPort->GetValue();
	const int nPort = ::atoi( m_sPort.c_str() );
	if ( nPort <= 0  )
	{
		wxLogError("The port [%s] is invalid.", m_sPort.c_str() );
		return;
	}
    
    m_sUsername = (   event.GetId() == ID_BTN_GUEST
                    ? _GenerateGuestUserName()
                    : m_textUsername->GetValue() );
    m_sPassword = m_textPassword->GetValue();

	/* Save login-info for next-time use */

    s_siteList[m_nType].sAddress  = m_sIP;
    s_siteList[m_nType].sPort     = m_sPort;
    if ( event.GetId() == wxID_OK ) // normal login?
    {
        s_siteList[m_nType].sUsername = m_sUsername;
        s_siteList[m_nType].sPassword = m_sPassword;
    }

	_SaveDefaultLoginInfo( m_nType );

    EndDialog( wxID_OK );
}

