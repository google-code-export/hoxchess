/////////////////////////////////////////////////////////////////////////////
// Name:        hoxLoginUI.h
// Purpose:     
// Author:      Huy Phan
// Modified by: 
// Created:     3/14/2009 8:59:35 PM
// RCS-ID:      
// Copyright:   Copyright 2007-2009 Huy Phan  <huyphan@playxiangqi.com>
// Licence:     GNU General Public License v3
/////////////////////////////////////////////////////////////////////////////

#ifndef _HOXLOGINUI_H_
#define _HOXLOGINUI_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/hyperlink.h"
#include "wx/collpane.h"
#include "wx/valtext.h"
////@end includes

#include "hoxTypes.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_HOXLOGINUI 10000
#define ID_RADIO_PLAYXIANGQI 10006
#define ID_HYPERLINKCTRL 10008
#define ID_RADIO_CHESSCAPE 10007
#define hoxHYPERLINKCTRL 10009
#define ID_COLLAPSIBLEPANE 10010
#define ID_TEXTCTRL_SERVER_NAME 10011
#define ID_TEXTCTRL_SERVER_PORT 10012
#define ID_TEXTCTRL_USERNAME 10004
#define ID_TEXTCTRL_PASSWORD 10005
#define ID_BTN_GUEST 10001
#define SYMBOL_HOXLOGINUI_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_HOXLOGINUI_TITLE _("Login to a remote server")
#define SYMBOL_HOXLOGINUI_IDNAME ID_HOXLOGINUI
#define SYMBOL_HOXLOGINUI_SIZE wxSize(400, 300)
#define SYMBOL_HOXLOGINUI_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * hoxLoginUI class declaration
 */

class hoxLoginUI: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( hoxLoginUI )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    hoxLoginUI();
    hoxLoginUI( wxWindow* parent, wxWindowID id = SYMBOL_HOXLOGINUI_IDNAME, const wxString& caption = SYMBOL_HOXLOGINUI_TITLE, const wxPoint& pos = SYMBOL_HOXLOGINUI_POSITION, const wxSize& size = SYMBOL_HOXLOGINUI_SIZE, long style = SYMBOL_HOXLOGINUI_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_HOXLOGINUI_IDNAME, const wxString& caption = SYMBOL_HOXLOGINUI_TITLE, const wxPoint& pos = SYMBOL_HOXLOGINUI_POSITION, const wxSize& size = SYMBOL_HOXLOGINUI_SIZE, long style = SYMBOL_HOXLOGINUI_STYLE );

    /// Destructor
    ~hoxLoginUI();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin hoxLoginUI event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIO_PLAYXIANGQI
    void OnRadioSiteTypeSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTN_GUEST
    void OnLoginClick( wxCommandEvent& event );

////@end hoxLoginUI event handler declarations

////@begin hoxLoginUI member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end hoxLoginUI member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin hoxLoginUI member variables
    wxRadioButton* m_radioPlayXiangqi;
    wxRadioButton* m_radioChesscape;
    wxTextCtrl* m_textIP;
    wxTextCtrl* m_textPort;
    wxTextCtrl* m_textUsername;
    wxTextCtrl* m_textPassword;
public:
    wxString m_sUsername;
    wxString m_sPassword;
    wxString m_sIP;
    wxString m_sPort;
private:
    int m_nType;
////@end hoxLoginUI member variables
public:
    hoxSiteType  m_siteType;

private:
    bool _GetDefaultLoginInfo( int& siteChoice );
    bool _SaveDefaultLoginInfo( const int siteChoice );
    const wxString _GenerateGuestUserName() const;

};

#endif
    // _HOXLOGINUI_H_
