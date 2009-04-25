/////////////////////////////////////////////////////////////////////////////
// Name:        hoxOptionsUI.cpp
// Purpose:     The UI managing all the options/preferences of HOXChess.
// Author:      Huy Phan
// Created:     3/9/2009 6:58:33 AM
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
#include "wx/bookctrl.h"
////@end includes

#include "hoxOptionsUI.h"
#include "hoxAIPluginMgr.h"
#include "hoxUtil.h"
#include <wx/dir.h>

////@begin XPM images
////@end XPM images


/*!
 * hoxOptionsUI type definition
 */

IMPLEMENT_DYNAMIC_CLASS( hoxOptionsUI, wxPropertySheetDialog )


/*!
 * hoxOptionsUI event table definition
 */

BEGIN_EVENT_TABLE( hoxOptionsUI, wxPropertySheetDialog )

////@begin hoxOptionsUI event table entries
    EVT_CHOICE( ID_CHOICE_LANGUAGE, hoxOptionsUI::OnChoiceLanguageSelected )

    EVT_COLOURPICKER_CHANGED( ID_COLOR_BACKGROUND, hoxOptionsUI::OnColorBackgroundColourPickerChanged )

    EVT_COLOURPICKER_CHANGED( ID_COLOR_FOREGROUND, hoxOptionsUI::OnColorForegroundColourPickerChanged )

    EVT_CHOICE( ID_PIECE_SET_CHOICES, hoxOptionsUI::OnPieceSetChoicesSelected )

    EVT_LISTBOX( ID_LISTBOX_ENGINES, hoxOptionsUI::OnListboxEnginesSelected )

////@end hoxOptionsUI event table entries

    EVT_BOOKCTRL_PAGE_CHANGED( wxID_ANY, hoxOptionsUI::OnPageChanged )

END_EVENT_TABLE()


/*!
 * hoxOptionsUI constructors
 */

hoxOptionsUI::hoxOptionsUI()
{
    Init();
}

hoxOptionsUI::hoxOptionsUI( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * hoxOptionsUI creator
 */

bool hoxOptionsUI::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin hoxOptionsUI creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    SetSheetStyle(wxPROPSHEET_DEFAULT);
    wxPropertySheetDialog::Create( parent, id, caption, pos, size, style );

    CreateButtons(wxOK|wxCANCEL);
    CreateControls();
    LayoutDialog();
    Centre();
////@end hoxOptionsUI creation
    return true;
}


/*!
 * hoxOptionsUI destructor
 */

hoxOptionsUI::~hoxOptionsUI()
{
////@begin hoxOptionsUI destruction
////@end hoxOptionsUI destruction
}


/*!
 * Member initialisation
 */

void hoxOptionsUI::Init()
{
////@begin hoxOptionsUI member initialisation
    m_bSound = true;
    m_bWelcome = true;
    m_bTables = true;
    m_checkBoxSound = NULL;
    m_choiceLanguage = NULL;
    m_colorPickerBackground = NULL;
    m_colorPickerForeground = NULL;
    m_pieceSetChoices = NULL;
    m_panelPiecePreview = NULL;
    m_listBoxEngines = NULL;
////@end hoxOptionsUI member initialisation

    m_bAlreadySetSelectedPage = false;
    m_selectedPage = 0;
}


/*!
 * Control creation for hoxOptionsUI
 */

void hoxOptionsUI::CreateControls()
{    
////@begin hoxOptionsUI content construction
    //hoxOptionsUI* itemPropertySheetDialog1 = this;

    wxImageList* itemPropertySheetDialog1ImageList = new wxImageList(22, 22, true, 3);
    {
        wxIcon itemPropertySheetDialog1Icon0(GetIconResource(wxT("../resource/images/go-home.png")));
        itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon0);
        wxIcon itemPropertySheetDialog1Icon1(GetIconResource(wxT("../resource/images/preferences.png")));
        itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon1);
        wxIcon itemPropertySheetDialog1Icon2(GetIconResource(wxT("../resource/images/system-users.png")));
        itemPropertySheetDialog1ImageList->Add(itemPropertySheetDialog1Icon2);
    }
    GetBookCtrl()->AssignImageList(itemPropertySheetDialog1ImageList);

    wxPanel* itemPanel2 = new wxPanel( GetBookCtrl(), m_panelMain, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemPanel2->SetSizer(itemBoxSizer3);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemPanel2, wxID_ANY, _("General"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, 0, wxGROW|wxALL, 5);
    m_checkBoxSound = new wxCheckBox( itemPanel2, ID_CHECKBOX_SOUND, _("Enable &Sounds"), wxDefaultPosition, wxDefaultSize, 0 );
    m_checkBoxSound->SetValue(true);
    itemStaticBoxSizer4->Add(m_checkBoxSound, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox6 = new wxCheckBox( itemPanel2, ID_CHECKBOX_WELCOME, _("Show &Welcome Dialog at startup"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox6->SetValue(true);
    itemStaticBoxSizer4->Add(itemCheckBox6, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox7 = new wxCheckBox( itemPanel2, ID_CHECKBOX_LIST_ON_LOGIN, _("Show List of &Tables at login"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox7->SetValue(true);
    itemStaticBoxSizer4->Add(itemCheckBox7, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer8Static = new wxStaticBox(itemPanel2, wxID_ANY, _("Language"));
    wxStaticBoxSizer* itemStaticBoxSizer8 = new wxStaticBoxSizer(itemStaticBoxSizer8Static, wxHORIZONTAL);
    itemBoxSizer3->Add(itemStaticBoxSizer8, 1, wxGROW|wxALL, 5);
    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel2, ID_STATIC_TEXT_LANG, _("Current Language:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer8->Add(itemStaticText9, 0, wxALIGN_TOP|wxALL, 5);

    wxArrayString m_choiceLanguageStrings;
    m_choiceLanguage = new wxChoice( itemPanel2, ID_CHOICE_LANGUAGE, wxDefaultPosition, wxDefaultSize, m_choiceLanguageStrings, 0 );
    itemStaticBoxSizer8->Add(m_choiceLanguage, 0, wxALIGN_TOP|wxALL, 5);

    GetBookCtrl()->AddPage(itemPanel2, _("Main"), false, 0);

    wxPanel* itemPanel11 = new wxPanel( GetBookCtrl(), m_panelBoard, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel11->SetSizer(itemBoxSizer12);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer12->Add(itemBoxSizer13, 1, wxGROW, 5);
    wxStaticBox* itemStaticBoxSizer14Static = new wxStaticBox(itemPanel11, wxID_ANY, _("Colors"));
    wxStaticBoxSizer* itemStaticBoxSizer14 = new wxStaticBoxSizer(itemStaticBoxSizer14Static, wxVERTICAL);
    itemBoxSizer13->Add(itemStaticBoxSizer14, 1, wxGROW|wxALL, 5);
    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer14->Add(itemBoxSizer15, 1, wxGROW|wxLEFT|wxRIGHT|wxTOP, 5);
    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel11, m_staticText31, _("Background:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemStaticText16, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);

    m_colorPickerBackground = new wxColourPickerCtrl( itemPanel11, ID_COLOR_BACKGROUND, wxColour(0, 0, 0), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemBoxSizer15->Add(m_colorPickerBackground, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer14->Add(itemBoxSizer18, 1, wxGROW|wxALL, 5);
    wxStaticText* itemStaticText19 = new wxStaticText( itemPanel11, m_staticText4, _("Foreground:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemStaticText19, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);

    m_colorPickerForeground = new wxColourPickerCtrl( itemPanel11, ID_COLOR_FOREGROUND, wxColour(0, 0, 0), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE );
    itemBoxSizer18->Add(m_colorPickerForeground, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);

    wxStaticBox* itemStaticBoxSizer21Static = new wxStaticBox(itemPanel11, wxID_ANY, _("Pieces"));
    wxStaticBoxSizer* itemStaticBoxSizer21 = new wxStaticBoxSizer(itemStaticBoxSizer21Static, wxVERTICAL);
    itemBoxSizer13->Add(itemStaticBoxSizer21, 1, wxGROW|wxALL, 5);
    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer21->Add(itemBoxSizer22, 1, wxGROW|wxALL, 5);
    wxStaticText* itemStaticText23 = new wxStaticText( itemPanel11, m_staticText5, _("Set:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemStaticText23, 0, wxALIGN_TOP|wxALL, 5);

    wxArrayString m_pieceSetChoicesStrings;
    m_pieceSetChoices = new wxChoice( itemPanel11, ID_PIECE_SET_CHOICES, wxDefaultPosition, wxDefaultSize, m_pieceSetChoicesStrings, 0 );
    itemBoxSizer22->Add(m_pieceSetChoices, 0, wxALIGN_TOP|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer25Static = new wxStaticBox(itemPanel11, wxID_ANY, _("Preview"));
    wxStaticBoxSizer* itemStaticBoxSizer25 = new wxStaticBoxSizer(itemStaticBoxSizer25Static, wxHORIZONTAL);
    itemBoxSizer12->Add(itemStaticBoxSizer25, 0, wxGROW|wxALL, 5);
    m_panelPiecePreview = new wxPanel( itemPanel11, ID_PANEL_PIECE_PREVIEW, wxDefaultPosition, wxSize(100, 180), wxTAB_TRAVERSAL );
    m_panelPiecePreview->SetBackgroundColour(wxColour(128, 128, 192));
    itemStaticBoxSizer25->Add(m_panelPiecePreview, 0, wxGROW|wxALL, 5);

    GetBookCtrl()->AddPage(itemPanel11, _("Board"), false, 1);

    wxPanel* itemPanel27 = new wxPanel( GetBookCtrl(), m_panelAI, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxVERTICAL);
    itemPanel27->SetSizer(itemBoxSizer28);

    wxStaticBox* itemStaticBoxSizer29Static = new wxStaticBox(itemPanel27, wxID_ANY, _("Artificial Intelligence"));
    wxStaticBoxSizer* itemStaticBoxSizer29 = new wxStaticBoxSizer(itemStaticBoxSizer29Static, wxVERTICAL);
    itemBoxSizer28->Add(itemStaticBoxSizer29, 1, wxGROW|wxALL, 5);
    wxBoxSizer* itemBoxSizer30 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer29->Add(itemBoxSizer30, 1, wxGROW|wxALL, 5);
    wxStaticText* itemStaticText31 = new wxStaticText( itemPanel27, m_staticText3, _("Default Engine:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer30->Add(itemStaticText31, 0, wxALIGN_TOP|wxALL, 5);

    wxArrayString m_listBoxEnginesStrings;
    m_listBoxEngines = new wxListBox( itemPanel27, ID_LISTBOX_ENGINES, wxDefaultPosition, wxSize(150, -1), m_listBoxEnginesStrings, wxLB_SINGLE );
    itemBoxSizer30->Add(m_listBoxEngines, 0, wxALIGN_TOP|wxALL, 5);

    GetBookCtrl()->AddPage(itemPanel27, _("AI"), false, 2);

    // Set validators
    m_checkBoxSound->SetValidator( wxGenericValidator(& m_bSound) );
    itemCheckBox6->SetValidator( wxGenericValidator(& m_bWelcome) );
    itemCheckBox7->SetValidator( wxGenericValidator(& m_bTables) );
    // Connect events and objects
    m_panelPiecePreview->Connect(ID_PANEL_PIECE_PREVIEW, wxEVT_PAINT, wxPaintEventHandler(hoxOptionsUI::OnPaint), NULL, this);
////@end hoxOptionsUI content construction

    _InitLanguageChoices();
    m_colorPickerBackground->SetColour( m_sBgColor );
    m_colorPickerForeground->SetColour( m_sFgColor );
    _InitPieceSetChoices();
    _InitAIChoices();
    if ( m_selectedPage >=0 && m_selectedPage < GetBookCtrl()->GetPageCount() )
    {
        GetBookCtrl()->SetSelection( m_selectedPage );
        m_bAlreadySetSelectedPage = true;
    }
}


/*!
 * Should we show tooltips?
 */

bool hoxOptionsUI::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap hoxOptionsUI::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin hoxOptionsUI bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end hoxOptionsUI bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon hoxOptionsUI::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin hoxOptionsUI icon retrieval
    wxUnusedVar(name);
    if (name == _T("../resource/images/go-home.png"))
    {
        wxIcon icon(wxString(HOX_PATH) + _T("../resource/images/go-home.png"), wxBITMAP_TYPE_PNG);
        return icon;
    }
    else if (name == _T("../resource/images/preferences.png"))
    {
        wxIcon icon(wxString(HOX_PATH) + _T("../resource/images/preferences.png"), wxBITMAP_TYPE_PNG);
        return icon;
    }
    else if (name == _T("../resource/images/system-users.png"))
    {
        wxIcon icon(wxString(HOX_PATH) + _T("../resource/images/system-users.png"), wxBITMAP_TYPE_PNG);
        return icon;
    }
    return wxNullIcon;
////@end hoxOptionsUI icon retrieval
}


void hoxOptionsUI::_InitLanguageChoices()
{
    for ( size_t i = 0; i < m_langList.size(); ++i )
    {
        m_choiceLanguage->Append( m_langList[i].desc );
        if ( m_language == m_langList[i].code )
        {
            m_choiceLanguage->SetSelection(i);
        }
    }
}

void hoxOptionsUI::_InitPieceSetChoices()
{
    const wxArrayString pieceSets = _loadAvailablePieceSets();
    if ( pieceSets.IsEmpty() )
    {
        ::wxMessageBox( _("There is no Piece-Set available."),
                        _("Piece-Set Selection"),
                        wxOK | wxICON_EXCLAMATION );
        return;
    }

    m_pieceSetChoices->Insert( pieceSets, 0 );
    m_pieceSetChoices->SetStringSelection( m_sPiece.AfterLast('/') );
}

void hoxOptionsUI::_InitAIChoices()
{
    const wxArrayString aiNames =
        hoxAIPluginMgr::GetInstance()->GetNamesOfAllAIPlugins();

    if ( aiNames.IsEmpty() )
    {
        ::wxMessageBox( _("There is no AI Engine available."),
                        _("AI Selection"),
                        wxOK | wxICON_EXCLAMATION );
        return;
    }
    
    m_listBoxEngines->InsertItems( aiNames, 0 );
    m_listBoxEngines->SetStringSelection( m_sDefaultAI );
}

wxArrayString
hoxOptionsUI::_loadAvailablePieceSets() const
{
    wxArrayString pieceSets;

    wxString sPiecesDir = HOX_PATH + wxString(PIECE_SETS_PATH);
    wxLogDebug("%s: Get Piece-Sets from [%s].", __FUNCTION__, sPiecesDir.c_str());
    wxDir dir(sPiecesDir);
	if ( !dir.IsOpened() )
	{
        wxLogWarning("%s: Fail to open Piece-Sets folder [%s].", __FUNCTION__, sPiecesDir.c_str());
		return pieceSets; // Just return the current set.
	}

    wxString  sFolder;  // ... also used as the name of the Piece-Set.

	for ( bool cont = dir.GetFirst(&sFolder, wxEmptyString, wxDIR_DIRS);
	           cont == true;
               cont = dir.GetNext(&sFolder) )
    {
        wxLogDebug("%s: Piece-Set = [%s].", __FUNCTION__, sFolder.c_str());
        pieceSets.Add( sFolder );
	}

    return pieceSets;
}

void
hoxOptionsUI::_DrawPiecePreview( wxPanel*      panel,
                                 const wxPoint p1,
                                 const wxPoint p2 )
{
    wxCHECK_RET( panel, "The panel must not be NULL" );

    hoxUtil::SetPiecesPath( m_sPiece ); // FIXME: Need to fix this.
    wxClientDC dc( panel );

    _DrawOnePieceAt( dc, hoxPIECE_ELEPHANT, hoxCOLOR_RED, p1 );
    _DrawOnePieceAt( dc, hoxPIECE_ELEPHANT, hoxCOLOR_BLACK, p2 );
}

void
hoxOptionsUI::_DrawOnePieceAt( wxDC&          dc,
                               hoxPieceType   pieceType,
                               hoxColor       pieceColor,
                               const wxPoint& pos )
{
    wxImage  image;
    wxBitmap bitmap;
    if ( hoxRC_OK == hoxUtil::LoadPieceImage( pieceType, pieceColor,
                                              image ) )
    {
        bitmap = wxBitmap(image);
    }
    if ( ! bitmap.Ok() )
      return;

    wxMemoryDC memDC;
    memDC.SelectObject( const_cast<wxBitmap&>(bitmap) );

    wxCoord x = pos.x - (bitmap.GetWidth() / 2);
    wxCoord y = pos.y - (bitmap.GetHeight() / 2);
    dc.Blit( x, y, 
             bitmap.GetWidth(), bitmap.GetHeight(),
             &memDC, 0, 0, wxCOPY, true );
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE_LANGUAGE
 */

void hoxOptionsUI::OnChoiceLanguageSelected( wxCommandEvent& event )
{
    size_t nChoice = event.GetInt();
    if ( nChoice >= 0 && nChoice < m_langList.size() )
    {
        m_language = m_langList[nChoice].code;
    }
}


/*!
 * wxEVT_COLOURPICKER_CHANGED event handler for ID_COLOR_BACKGROUND
 */

void hoxOptionsUI::OnColorBackgroundColourPickerChanged( wxColourPickerEvent& event )
{
    m_sBgColor = event.GetColour().GetAsString( wxC2S_CSS_SYNTAX );
    m_panelPiecePreview->Refresh();  // repaint...
}


/*!
 * wxEVT_COLOURPICKER_CHANGED event handler for ID_COLOR_FOREGROUND
 */

void hoxOptionsUI::OnColorForegroundColourPickerChanged( wxColourPickerEvent& event )
{
    m_sFgColor = event.GetColour().GetAsString( wxC2S_CSS_SYNTAX );
    m_panelPiecePreview->Refresh();  // repaint...
}


/*!
 * wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_ENGINES
 */

void hoxOptionsUI::OnListboxEnginesSelected( wxCommandEvent& event )
{
    m_sDefaultAI = event.GetString();
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_PIECE_SET_CHOICES
 */

void hoxOptionsUI::OnPieceSetChoicesSelected( wxCommandEvent& event )
{
    m_sPiece = wxString(PIECE_SETS_PATH) + "/" + event.GetString();
    m_panelPiecePreview->Refresh();  // repaint...
}


/*!
 * wxEVT_PAINT event handler for ID_PANEL_PIECE_PREVIEW
 */

void hoxOptionsUI::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc(m_panelPiecePreview);
    const wxSize sz = m_panelPiecePreview->GetClientSize();
    dc.SetBrush( wxBrush( wxColor(m_sBgColor) ) );
    dc.SetPen( wxPen( wxColor(m_sFgColor) ) );

    dc.DrawRectangle( 0, 0, sz.x, sz.y );;
    
    const wxPoint p1(50, 40);
    const wxPoint p2(50, 140);
    
    dc.DrawLine( p1.x, 0, p1.x, sz.y );
    dc.DrawLine( 0, p1.y, sz.x, p1.y );
    dc.DrawLine( 0, p2.y, sz.x, p2.y );

    _DrawPiecePreview( m_panelPiecePreview, p1, p2 );
}

void
hoxOptionsUI::OnPageChanged( wxBookCtrlEvent& event )
{
    // To avoid revert to the default selection of 0 upon initialized.
    if ( m_bAlreadySetSelectedPage )
    {
        m_selectedPage = event.GetSelection();
    }
}
