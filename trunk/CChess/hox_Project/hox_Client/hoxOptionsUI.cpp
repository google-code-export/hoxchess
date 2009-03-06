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
// Name:            hoxOptionsUI.cpp
// Created:         10/20/2008
//
// Description:     The UI containing the App's Options.
/////////////////////////////////////////////////////////////////////////////

#include "hoxOptionsUI.h"
#include "MyApp.h"    // wxGetApp()
#include "hoxAIPluginMgr.h"
#include "hoxUtil.h"
#include <wx/dir.h>

enum
{
    ID_SOUND    = (wxID_HIGHEST + 1),
    ID_WELCOME,
    ID_LANGUAGE,
    ID_BG_COLOR,   // Background
    ID_FG_COLOR,   // Foreground
    ID_CHOOSE_PIECE,
    ID_CHOOSE_AI,  // Default AI
};

BEGIN_EVENT_TABLE(hoxOptionsUI, wxPropertySheetDialog)
    EVT_CHECKBOX(ID_SOUND, hoxOptionsUI::OnSound)
    EVT_CHECKBOX(ID_WELCOME, hoxOptionsUI::OnWelcome)
    EVT_BUTTON(ID_LANGUAGE, hoxOptionsUI::OnLanguage)
    EVT_COLOURPICKER_CHANGED(wxID_ANY, hoxOptionsUI::OnColorChanged)
    EVT_BUTTON(ID_CHOOSE_PIECE, hoxOptionsUI::OnPiece)
    EVT_BUTTON(ID_CHOOSE_AI, hoxOptionsUI::OnDefaultAI)

    EVT_PAINT            (hoxOptionsUI::OnPaint)
    EVT_ERASE_BACKGROUND (hoxOptionsUI::OnEraseBackground)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------
// hoxOptionsUI class
// ---------------------------------------------------------------------------

hoxOptionsUI::hoxOptionsUI( wxWindow*          parent,
                            const OptionsData& data )
{
    m_data = data;

    this->Create( parent, wxID_ANY, _("Preferences") );

    this->CreateButtons( wxOK | wxCANCEL );

    wxBookCtrlBase* notebook = this->GetBookCtrl();

    wxPanel* generalPage = _CreateGeneralPage( notebook );
    wxPanel* boardPage   = _CreateBoardPage( notebook );
    wxPanel* aiPage      = _CreateAIPage( notebook );

    notebook->AddPage( generalPage, _("General"), true /* select */ );
    notebook->AddPage( boardPage,   _("Board"), false );
    notebook->AddPage( aiPage,      _("AI"), false );

    this->LayoutDialog();
}

wxPanel*
hoxOptionsUI::_CreateGeneralPage( wxWindow* parent )
{
    wxPanel* panel = new wxPanel(parent);

    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* miscSizer = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, ""), 
		wxVERTICAL );

    /* ----------- Sounds. */

    wxBoxSizer* soundSizer = new wxBoxSizer( wxHORIZONTAL );
    m_soundCheck = new wxCheckBox( panel, ID_SOUND, _("Enable &Sounds") );
    m_soundCheck->SetValue( m_data.m_bSound );
    soundSizer->Add( m_soundCheck );
    miscSizer->Add( soundSizer, wxSizerFlags().Border(wxALL, 5) );

    /* ----------- Welcome. */
    wxBoxSizer* welcomeSizer = new wxBoxSizer( wxHORIZONTAL );
    m_welcomeCheck = new wxCheckBox( panel, ID_WELCOME, _("Show &Welcome Dialog at startup") );
    m_welcomeCheck->SetValue( m_data.m_bWelcome );
    welcomeSizer->Add( m_welcomeCheck );
    miscSizer->Add( welcomeSizer, wxSizerFlags().Border(wxALL, 5) );

    topSizer->Add( miscSizer, 
                   wxSizerFlags().Expand().Border(wxALL, 10));

    /* ----------- Language. */

    wxBoxSizer* langSizer = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, _("Language")), 
		wxHORIZONTAL );

    const wxString sLangDesc = wxGetApp().GetLanguageDesc( m_data.m_language );
    m_languageTextCtrl = new wxTextCtrl( 
		panel, wxID_ANY,
        sLangDesc,
        wxDefaultPosition, wxSize(150, wxDefaultCoord),
        wxTE_READONLY );

    langSizer->Add( 
        new wxStaticText(panel, wxID_ANY, _("Current &Language:")),
		wxSizerFlags().Align(wxALIGN_CENTER).Border(wxALL, 5));

    langSizer->Add( 
		m_languageTextCtrl,
        wxSizerFlags().Align(wxALIGN_CENTER).Border(wxALL, 5));

    langSizer->Add( 
		new wxButton(panel, ID_LANGUAGE, _("&Choose...")),
        wxSizerFlags().Align(wxALIGN_CENTER).Border(wxALL, 10));

    topSizer->Add( langSizer, 
                   wxSizerFlags().Expand().Border(wxALL, 10));

    /* Done. */

    panel->SetSizer( topSizer );
    topSizer->Fit( panel );

    return panel;
}

wxPanel*
hoxOptionsUI::_CreateBoardPage( wxWindow* parent )
{
    wxPanel* panel = new wxPanel(parent);

    wxBoxSizer* topSizer = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer* settingsSizer = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer* previewSizer = new wxStaticBoxSizer(
        new wxStaticBox( panel, wxID_ANY, _("Preview") ), wxVERTICAL);

    wxBoxSizer* colorSizer = new wxStaticBoxSizer(
		new wxStaticBox( panel, wxID_ANY, _("Colors")), wxVERTICAL );

    /* ----------- Background Color. */

    wxBoxSizer* bgSizer = new wxBoxSizer( wxHORIZONTAL );
    m_bgBox = new wxColourPickerCtrl(
        panel, ID_BG_COLOR,
        wxColor(m_data.m_sBgColor),
        wxDefaultPosition, wxSize(wxDefaultCoord, 30) );
    bgSizer->Add( new wxStaticText(panel, wxID_ANY, _("&Background:")),
                  wxSizerFlags().Center().Border(wxALL, 5) );
    bgSizer->Add( m_bgBox,
                  wxSizerFlags().Center().Border(wxALL, 5));
    colorSizer->Add( bgSizer,
                     wxSizerFlags().Border(wxALL, 0));

    /* ----------- Foreground Color. */

    wxBoxSizer* fgSizer = new wxBoxSizer( wxHORIZONTAL );
    m_fgBox = new wxColourPickerCtrl(
        panel, ID_FG_COLOR,
        wxColor(m_data.m_sFgColor),
        wxDefaultPosition, wxSize(wxDefaultCoord, 30) );
    fgSizer->Add( new wxStaticText(panel, wxID_STATIC, _("&Foreground:")),
                  wxSizerFlags().Center().Border(wxALL, 5) );
    fgSizer->Add( m_fgBox,
                  wxSizerFlags().Center().Border(wxALL, 5) );
    colorSizer->Add( fgSizer,
                     wxSizerFlags().Border(wxALL, 0) );

    /* ----------- Piece. */
    wxBoxSizer* pieceSizer = new wxStaticBoxSizer(
        new wxStaticBox( panel, wxID_ANY, _("Piece")), wxHORIZONTAL );

    m_pieceTextCtrl = new wxTextCtrl( 
		panel, wxID_ANY,
        m_data.m_sPiece,
        wxDefaultPosition, wxSize(150, wxDefaultCoord),
        wxTE_READONLY );

    pieceSizer->Add( 
        new wxStaticText(panel, wxID_ANY, _("&Set:")),
		wxSizerFlags().Align(wxALIGN_CENTER).Border(wxALL, 5));

    pieceSizer->Add( 
		m_pieceTextCtrl,
        wxSizerFlags().Align(wxALIGN_CENTER).Border(wxALL, 5));

    pieceSizer->Add( 
		new wxButton(panel, ID_CHOOSE_PIECE, _("&Choose...")),
        wxSizerFlags().Align(wxALIGN_CENTER).Border(wxALL, 5));

    settingsSizer->Add( colorSizer, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
    settingsSizer->Add( pieceSizer, wxSizerFlags().Expand().Border(wxALL, 0));

    /* ----------- The Preview panel. */
    m_previewPanel = new wxPanel( panel, wxID_ANY, wxDefaultPosition,
                                  wxSize(90, 170),
                                  wxBORDER_SUNKEN );
    previewSizer->Add( m_previewPanel,
                       wxSizerFlags().Proportion(1).Border(wxALL, 5));
    m_previewPanel->SetBackgroundColour( *wxLIGHT_GREY );

    /* Done. */
    topSizer->Add( settingsSizer, wxSizerFlags().Expand().Border(wxALL, 10));
    topSizer->Add( previewSizer, wxSizerFlags().Expand().Border(wxALL, 10));

    panel->SetSizer( topSizer );
    topSizer->Fit( panel );
    return panel;
}

wxPanel*
hoxOptionsUI::_CreateAIPage( wxWindow* parent )
{
    wxPanel* panel = new wxPanel(parent);

    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

	/* AI. */

	wxBoxSizer* boxSizer = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, _("Artificial Intelligence")), 
		wxHORIZONTAL );

    m_defaultAITextCtrl = new wxTextCtrl( 
		panel, wxID_ANY,
        m_data.m_sDefaultAI,
        wxDefaultPosition, wxSize(150, wxDefaultCoord),
        wxTE_READONLY );

    boxSizer->Add( 
        new wxStaticText(panel, wxID_ANY, _("&Default engine:")),
		wxSizerFlags().Align(wxALIGN_CENTER).Border(wxALL, 5));

    boxSizer->Add( 
		m_defaultAITextCtrl,
        wxSizerFlags().Align(wxALIGN_CENTER).Border(wxALL, 5));

    boxSizer->Add( 
		new wxButton(panel, ID_CHOOSE_AI, _("&Choose...")),
        wxSizerFlags().Align(wxALIGN_CENTER).Border(wxALL, 10));

    topSizer->Add( boxSizer, 
                   wxSizerFlags().Expand().Border(wxALL, 10));

    /* Done. */

    panel->SetSizer( topSizer );
    topSizer->Fit( panel );

    return panel;
}

void
hoxOptionsUI::OnSound( wxCommandEvent& event )
{
    m_data.m_bSound = event.IsChecked();
}

void
hoxOptionsUI::OnWelcome( wxCommandEvent& event )
{
    m_data.m_bWelcome = event.IsChecked();
}

void
hoxOptionsUI::OnLanguage( wxCommandEvent& event )
{
    wxLanguage language = wxGetApp().SelectLanguage();
    if ( language != wxLANGUAGE_UNKNOWN )
    {
        m_data.m_language = language;
        const wxString sLangDesc = wxGetApp().GetLanguageDesc( m_data.m_language );
        m_languageTextCtrl->SetValue( sLangDesc );
    }
}

void
hoxOptionsUI::OnColorChanged( wxColourPickerEvent& event )
{
    wxString& whichColor = ( event.GetId() == ID_BG_COLOR
                            ? m_data.m_sBgColor
                            : m_data.m_sFgColor );
    whichColor = event.GetColour().GetAsString( wxC2S_CSS_SYNTAX );
}

void
hoxOptionsUI::OnPiece( wxCommandEvent& event )
{
    const wxArrayString pieceSets = _loadAvailablePieceSets();
    if ( pieceSets.IsEmpty() )
    {
        ::wxMessageBox( _("There is no Piece-Set available."),
                        _("Piece-Set Selection"),
                        wxOK | wxICON_EXCLAMATION );
        return;
    }

    const int nChoice = ::wxGetSingleChoiceIndex(
                            _("Select the Piece-Set"),
                            _("Piece-Set Selection"),
                            pieceSets );
    if ( nChoice != -1 )
    {
        const wxString sSetName = pieceSets[nChoice];
        m_data.m_sPiece = wxString(PIECE_SETS_PATH) + "/" + sSetName;
        m_pieceTextCtrl->SetValue( m_data.m_sPiece );
        this->Refresh();  // Repaint the "Preview" of Piece-Set.
    }
}

void
hoxOptionsUI::OnDefaultAI( wxCommandEvent& event )
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

    const int nChoice = ::wxGetSingleChoiceIndex(
                            _("Select the default AI engine"),
                            _("AI Selection"),
                            aiNames );
    if ( nChoice != -1 )
    {
        m_data.m_sDefaultAI = aiNames[nChoice];
        m_defaultAITextCtrl->SetValue( m_data.m_sDefaultAI );
    }
}

void
hoxOptionsUI::OnPaint( wxPaintEvent& event )
{
    _DrawPiecePreview( m_previewPanel );
    event.Skip(); // Let the search for the event handler should continue.
}

void
hoxOptionsUI::OnEraseBackground( wxEraseEvent& event )
{
    _DrawPiecePreview( m_previewPanel );
    event.Skip(); // Let the search for the event handler should continue.
}

void
hoxOptionsUI::_DrawPiecePreview( wxPanel* panel )
{
    wxCHECK_RET( panel, "The panel must not be NULL" );

    hoxUtil::SetPiecesPath( m_data.m_sPiece ); // FIXME: Need to fix this.
    wxClientDC dc( panel );

    wxPoint pos(5, 5);
    _DrawOnePieceAt( dc, hoxPIECE_KING, hoxCOLOR_RED, pos );

    pos.y += 75;
    _DrawOnePieceAt( dc, hoxPIECE_KING, hoxCOLOR_BLACK, pos );
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

    dc.Blit( pos.x, pos.y, 
             bitmap.GetWidth(), bitmap.GetHeight(),
             &memDC, 0, 0, wxCOPY, true);
}

wxArrayString
hoxOptionsUI::_loadAvailablePieceSets() const
{
    wxArrayString pieceSets;

    wxString sPiecesDir = PIECE_SETS_PATH;
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

/************************* END OF FILE ***************************************/
