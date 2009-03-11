/////////////////////////////////////////////////////////////////////////////
// Name:        hoxWelcomeUI.h
// Purpose:     
// Author:      Huy Phan
// Modified by: 
// Created:     03/05/2009 07:16:53
// RCS-ID:      
// Copyright:   Copyright 2007-2009 Huy Phan  <huyphan@playxiangqi.com>
// Licence:     GNU General Public License v3
/////////////////////////////////////////////////////////////////////////////

#ifndef _HOXWELCOMEUI_H_
#define _HOXWELCOMEUI_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/html/htmlwin.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DLG_WELCOME 10000
#define ID_HTMLWINDOW 10003
#define ID_CHK_SHOW_WELCOME 10007
#define ID_BTN_PLAY 10001
#define ID_BTN_LOGIN 10002
#define ID_BTN_OPTIONS 10004
#define SYMBOL_HOXWELCOMEUI_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_HOXWELCOMEUI_TITLE _("Getting Started")
#define SYMBOL_HOXWELCOMEUI_IDNAME ID_DLG_WELCOME
#define SYMBOL_HOXWELCOMEUI_SIZE wxSize(400, 300)
#define SYMBOL_HOXWELCOMEUI_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * hoxWelcomeUI class declaration
 */

class hoxWelcomeUI: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( hoxWelcomeUI )
    DECLARE_EVENT_TABLE()

public:
    enum CommandId
    {
        COMMAND_ID_PRACTICE = (wxID_HIGHEST + 1),
        COMMAND_ID_REMOTE,
		COMMAND_ID_OPTIONS
    };

public:
    /// Constructors
    hoxWelcomeUI();
    hoxWelcomeUI( wxWindow* parent, wxWindowID id = SYMBOL_HOXWELCOMEUI_IDNAME, const wxString& caption = SYMBOL_HOXWELCOMEUI_TITLE, const wxPoint& pos = SYMBOL_HOXWELCOMEUI_POSITION, const wxSize& size = SYMBOL_HOXWELCOMEUI_SIZE, long style = SYMBOL_HOXWELCOMEUI_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_HOXWELCOMEUI_IDNAME, const wxString& caption = SYMBOL_HOXWELCOMEUI_TITLE, const wxPoint& pos = SYMBOL_HOXWELCOMEUI_POSITION, const wxSize& size = SYMBOL_HOXWELCOMEUI_SIZE, long style = SYMBOL_HOXWELCOMEUI_STYLE );

    /// Destructor
    ~hoxWelcomeUI();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Check if this Dialog should be shown in the next startup.
    bool ShowNextStartup() const ;

////@begin hoxWelcomeUI event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTN_PLAY
    void OnBtnPlayClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTN_LOGIN
    void OnBtnLoginClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTN_OPTIONS
    void OnBtnOptionsClick( wxCommandEvent& event );

////@end hoxWelcomeUI event handler declarations

////@begin hoxWelcomeUI member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end hoxWelcomeUI member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin hoxWelcomeUI member variables
    wxCheckBox* m_showNextTimeCheck;
////@end hoxWelcomeUI member variables

protected:
    void OnHtmlLinkClicked( wxHtmlLinkEvent& event );
};

#endif
    // _HOXWELCOMEUI_H_
