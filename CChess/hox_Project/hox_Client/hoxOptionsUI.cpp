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
#include <wx/notebook.h>

/* Event table. */
BEGIN_EVENT_TABLE(hoxOptionsUI, wxPropertySheetDialog)
    EVT_CHECKBOX(ID_SOUND, hoxOptionsUI::OnSound)
    EVT_BUTTON(ID_LANG, hoxOptionsUI::OnLanguage)
    EVT_COLOURPICKER_CHANGED(wxID_ANY, hoxOptionsUI::OnColorChanged)
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

    notebook->AddPage( generalPage, _("General"), true /* select */ );
    notebook->AddPage( boardPage, _("Board"), false );

    this->LayoutDialog();
}

wxPanel*
hoxOptionsUI::_CreateGeneralPage( wxWindow* parent )
{
    wxPanel* panel = new wxPanel(parent);

    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

    /* ----------- Sounds. */

    wxBoxSizer* soundSizer = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer* item0 = new wxBoxSizer( wxHORIZONTAL );
    m_soundCheck = new wxCheckBox( panel, ID_SOUND, _("&Sounds") );
    m_soundCheck->SetValue( m_data.m_bSound );
    item0->Add( m_soundCheck,
                wxSizerFlags().Border(wxRIGHT, 10) );
    soundSizer->Add( item0, 0, wxGROW|wxALL, 0 );

    /* ----------- Language. */

    wxBoxSizer* langSizer = new wxBoxSizer( wxHORIZONTAL );
    langSizer->Add( 
        new wxButton( panel, ID_LANG, _("Change &Language") ) );

    /* Done. */
    topSizer->Add( soundSizer, 0, wxGROW|wxALIGN_CENTRE|wxALL, 5 );
    topSizer->AddSpacer( 40 );  // Add some spaces in between.
    topSizer->Add( langSizer, 0, wxGROW|wxALIGN_CENTRE|wxALL, 5 );

    panel->SetSizer( topSizer );
    topSizer->Fit( panel );

    return panel;
}

wxPanel*
hoxOptionsUI::_CreateBoardPage( wxWindow* parent )
{
    wxPanel* panel = new wxPanel(parent);

    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer* item0 = new wxBoxSizer( wxVERTICAL );

    /* ----------- Background Color. */

    wxBoxSizer* bgSizer = new wxBoxSizer( wxHORIZONTAL );
    m_bgBox = new wxColourPickerCtrl( panel, ID_BG_COLOR );
    m_bgBox->SetColour( wxColor(m_data.m_sBgColor) );
    bgSizer->Add( new wxStaticText(panel, wxID_STATIC, _("&Background color:")),
                  0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    bgSizer->Add(m_bgBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(bgSizer, 0, wxGROW|wxALL, 0);

    /* ----------- Foreground Color. */

    wxBoxSizer* fgSizer = new wxBoxSizer( wxHORIZONTAL );
    m_fgBox = new wxColourPickerCtrl( panel, ID_FG_COLOR );
    m_fgBox->SetColour( wxColor(m_data.m_sFgColor) );
    fgSizer->Add( new wxStaticText(panel, wxID_STATIC, _("&Foreground color:")),
                  0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    fgSizer->Add(m_fgBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(fgSizer, 0, wxGROW|wxALL, 0);

    /* Done. */
    topSizer->Add( item0, 0, wxGROW|wxALIGN_CENTRE|wxALL, 5 );

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
    wxGetApp().SelectAndSaveLanguage( true /* bRestartIfChange */ );
}

void
hoxOptionsUI::OnColorChanged( wxColourPickerEvent& event )
{
    wxString& whichColor = ( event.GetId() == ID_BG_COLOR
                            ? m_data.m_sBgColor
                            : m_data.m_sFgColor );
    whichColor = event.GetColour().GetAsString( wxC2S_CSS_SYNTAX );
}

/************************* END OF FILE ***************************************/
