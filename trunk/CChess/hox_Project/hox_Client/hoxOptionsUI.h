/////////////////////////////////////////////////////////////////////////////
// Name:        hoxOptionsUI.h
// Purpose:     The UI managing all the options/preferences of HOXChess.
// Author:      Huy Phan
// Created:     3/9/2009 6:58:33 AM
// Copyright:   Copyright 2007-2009 Huy Phan  <huyphan@playxiangqi.com>
// Licence:     GNU General Public License v3
/////////////////////////////////////////////////////////////////////////////

#ifndef _HOXOPTIONSUI_H_
#define _HOXOPTIONSUI_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/propdlg.h"
#include "wx/valgen.h"
#include "wx/clrpicker.h"
////@end includes

#include "MyApp.h"
#include "wx/bookctrl.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxColourPickerCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_HOXOPTIONSAPP 10018
#define ID_OPTIONS_MAIN 10002
#define ID_CHECKBOX_SOUND 10003
#define ID_CHECKBOX_WELCOME 10004
#define ID_CHECKBOX_LIST_ON_LOGIN 10001
#define ID_STATIC_TEXT_LANG 10005
#define ID_CHOICE_LANGUAGE 10006
#define ID_OPTIONS_BOARD 10007
#define ID_BOARD_IMAGE_LIST 10023
#define m_staticText31 10008
#define ID_COLOR_BACKGROUND 10009
#define m_staticText4 10010
#define ID_COLOR_FOREGROUND 10011
#define ID_PIECE_SET_LIST 10012
#define ID_PANEL_PIECE_PREVIEW 10014
#define ID_OPTIONS_AI 10015
#define ID_LISTBOX_ENGINES 10000
#define SYMBOL_HOXOPTIONSUI_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_HOXOPTIONSUI_TITLE _("Options")
#define SYMBOL_HOXOPTIONSUI_IDNAME ID_HOXOPTIONSAPP
#define SYMBOL_HOXOPTIONSUI_SIZE wxDefaultSize
#define SYMBOL_HOXOPTIONSUI_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * hoxOptionsUI class declaration
 */

class hoxOptionsUI: public wxPropertySheetDialog
{    
    DECLARE_DYNAMIC_CLASS( hoxOptionsUI )
    DECLARE_EVENT_TABLE()

public:
    hoxLanguageInfoVector m_langList;
    wxLanguage   m_language;
    wxString     m_sBoardImage;
    wxString     m_sBgColor;
    wxString     m_sFgColor;
    wxString     m_sPiece;
    wxString     m_sDefaultAI;

    bool         m_bAlreadySetSelectedPage;
    size_t   	 m_selectedPage;

    /// Constructors
    hoxOptionsUI();
    hoxOptionsUI( wxWindow* parent, wxWindowID id = SYMBOL_HOXOPTIONSUI_IDNAME, const wxString& caption = SYMBOL_HOXOPTIONSUI_TITLE, const wxPoint& pos = SYMBOL_HOXOPTIONSUI_POSITION, const wxSize& size = SYMBOL_HOXOPTIONSUI_SIZE, long style = SYMBOL_HOXOPTIONSUI_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_HOXOPTIONSUI_IDNAME, const wxString& caption = SYMBOL_HOXOPTIONSUI_TITLE, const wxPoint& pos = SYMBOL_HOXOPTIONSUI_POSITION, const wxSize& size = SYMBOL_HOXOPTIONSUI_SIZE, long style = SYMBOL_HOXOPTIONSUI_STYLE );

    /// Destructor
    ~hoxOptionsUI();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin hoxOptionsUI event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE_LANGUAGE
    void OnChoiceLanguageSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_BOARD_IMAGE_LIST
    void OnBoardImageListSelected( wxCommandEvent& event );

    /// wxEVT_COLOURPICKER_CHANGED event handler for ID_COLOR_BACKGROUND
    void OnColorBackgroundColourPickerChanged( wxColourPickerEvent& event );

    /// wxEVT_COLOURPICKER_CHANGED event handler for ID_COLOR_FOREGROUND
    void OnColorForegroundColourPickerChanged( wxColourPickerEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_PIECE_SET_LIST
    void OnPieceSetListSelected( wxCommandEvent& event );

    /// wxEVT_PAINT event handler for ID_PANEL_PIECE_PREVIEW
    void OnPaint( wxPaintEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_ENGINES
    void OnListboxEnginesSelected( wxCommandEvent& event );

////@end hoxOptionsUI event handler declarations

    void OnPageChanged( wxBookCtrlEvent& event );

////@begin hoxOptionsUI member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end hoxOptionsUI member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin hoxOptionsUI member variables
    wxCheckBox* m_checkBoxSound;
    wxChoice* m_choiceLanguage;
    wxListBox* m_boardImageList;
    wxColourPickerCtrl* m_colorPickerBackground;
    wxColourPickerCtrl* m_colorPickerForeground;
    wxListBox* m_pieceSetChoices;
    wxPanel* m_panelPiecePreview;
    wxListBox* m_listBoxEngines;
public:
    bool m_bSound;
    bool m_bWelcome;
    bool m_bTables;
////@end hoxOptionsUI member variables

private:
    void _InitLanguageChoices();
    void _InitBoardImageChoices();
    void _InitPieceSetChoices();
    void _InitAIChoices();
    wxArrayString _loadAvailableBoardImages() const;
    wxArrayString _loadAvailablePieceSets() const;
    void _DrawPiecePreview( wxPanel*      panel,
                            const wxPoint p1,
                            const wxPoint p2 );
    void _DrawOnePieceAt( wxDC&          dc,
                          hoxPieceType   pieceType,
                          hoxColor       pieceColor,
                          const wxPoint& pos );
    void _LoadBoardImage();

private:
    wxBitmap          m_boardBitmap;
    hoxBoardImageInfo m_boardInfo;
};

#endif
    // _HOXOPTIONSUI_H_
