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
#include <wx/notebook.h>

/* Windows ID */
enum
{
    ID_SOUND = 100 ,
    ID_LANGUAGE,
    ID_BG_COLOR,   // Background
    ID_FG_COLOR,   // Foreground
    ID_CHOOSE_AI,  // Default AI
};

/* Event table. */
BEGIN_EVENT_TABLE(hoxOptionsUI, wxPropertySheetDialog)
    EVT_CHECKBOX(ID_SOUND, hoxOptionsUI::OnSound)
    EVT_BUTTON(ID_LANGUAGE, hoxOptionsUI::OnLanguage)
    EVT_COLOURPICKER_CHANGED(wxID_ANY, hoxOptionsUI::OnColorChanged)
    EVT_BUTTON(ID_CHOOSE_AI, hoxOptionsUI::OnDefaultAI)
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
    notebook->AddPage( boardPage, _("Board"), false );
    notebook->AddPage( aiPage, _("AI"), false );

    this->LayoutDialog();
}

wxPanel*
hoxOptionsUI::_CreateGeneralPage( wxWindow* parent )
{
    wxPanel* panel = new wxPanel(parent);

    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

    /* ----------- Sounds. */

    wxBoxSizer* miscSizer = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, ""), 
		wxVERTICAL );

    wxBoxSizer* soundSizer = new wxBoxSizer( wxHORIZONTAL );
    m_soundCheck = new wxCheckBox( panel, ID_SOUND, _("Enable &Sounds") );
    m_soundCheck->SetValue( m_data.m_bSound );
    soundSizer->Add( m_soundCheck,
                     wxSizerFlags().Border(wxRIGHT, 100) );
    miscSizer->Add( soundSizer,
                    wxSizerFlags().Border(wxALL, 10) );
    
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

    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* colorSizer = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, _("Colors")), 
		wxVERTICAL );

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
                     wxSizerFlags().Border(wxALL, 5));

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
                     wxSizerFlags().Border(wxALL, 5) );

    /* Done. */
    topSizer->Add( colorSizer, 
                   wxSizerFlags().Expand().Border(wxALL, 10));

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
hoxOptionsUI::OnDefaultAI( wxCommandEvent& event )
{
    wxArrayString aiNames = hoxAIPluginMgr::GetInstance()->GetNamesOfAllAIPlugins();

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

/************************* END OF FILE ***************************************/
