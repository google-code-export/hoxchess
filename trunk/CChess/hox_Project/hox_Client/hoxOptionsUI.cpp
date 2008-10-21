/***************************************************************************
 *  Copyright 2007, 2008 Huy Phan  <huyphan@playxiangqi.com>               *
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
#include "MyApp.h"         // wxGetApp
#include <wx/notebook.h>
#include <wx/colordlg.h>  // Color Dialog

/* Event table. */
BEGIN_EVENT_TABLE(hoxOptionsUI, wxPropertySheetDialog)
    EVT_CHECKBOX(ID_SOUND, hoxOptionsUI::OnSound)
    EVT_BUTTON(ID_BG_COLOR, hoxOptionsUI::OnBgColor)
    EVT_BUTTON(ID_FG_COLOR, hoxOptionsUI::OnFgColor)
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
    wxBoxSizer* item0 = new wxBoxSizer( wxVERTICAL );

    /* ----------- Sounds. */

    wxBoxSizer* soundSizer = new wxBoxSizer( wxHORIZONTAL );
    m_soundCheck = new wxCheckBox( panel, ID_SOUND, _("&Sounds") );
    soundSizer->Add( m_soundCheck,
                     0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    item0->Add( soundSizer, 0, wxGROW|wxALL, 0 );

    /* Done. */
    topSizer->Add( item0, 1, wxGROW|wxALIGN_CENTRE|wxALL, 5 );

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
    m_bgBox = new wxButton( panel, ID_BG_COLOR, wxEmptyString,
                            wxDefaultPosition, wxSize(50, wxDefaultCoord ) );
    bgSizer->Add( new wxStaticText(panel, wxID_STATIC, _("&Background color:")),
                  0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    bgSizer->Add(m_bgBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(bgSizer, 0, wxGROW|wxALL, 0);

    /* ----------- Foreground Color. */

    wxBoxSizer* fgSizer = new wxBoxSizer( wxHORIZONTAL );
    m_fgBox = new wxButton( panel, ID_FG_COLOR, wxEmptyString,
                            wxDefaultPosition, wxSize(50, wxDefaultCoord ) );
    fgSizer->Add( new wxStaticText(panel, wxID_STATIC, _("&Foreground color:")),
                  0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    fgSizer->Add(m_fgBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    item0->Add(fgSizer, 0, wxGROW|wxALL, 0);

    /* Done. */
    topSizer->Add( item0, 1, wxGROW|wxALIGN_CENTRE|wxALL, 5 );

    panel->SetSizer( topSizer );
    topSizer->Fit( panel );

    return panel;
}

int
hoxOptionsUI::ShowModal()
{
    m_soundCheck->SetValue( m_data.m_bSound );
    m_bgBox->SetBackgroundColour( wxColor(m_data.m_sBgColor) );
    m_fgBox->SetBackgroundColour( wxColor(m_data.m_sFgColor) );

    return wxPropertySheetDialog::ShowModal();
}

void
hoxOptionsUI::OnSound( wxCommandEvent& event )
{
    m_data.m_bSound = event.IsChecked();
}

void
hoxOptionsUI::OnBgColor( wxCommandEvent& event )
{
    wxColourData colorData;
    colorData.SetColour( m_data.m_sBgColor );

    wxColourDialog dialog(this, &colorData);
    if (dialog.ShowModal() == wxID_OK)
    {
        colorData = dialog.GetColourData();
        wxColor newColor = colorData.GetColour();
        m_data.m_sBgColor = newColor.GetAsString( wxC2S_CSS_SYNTAX );
        m_bgBox->SetBackgroundColour( newColor );
    }
}

void
hoxOptionsUI::OnFgColor( wxCommandEvent& event )
{
    wxColourData colorData;
    colorData.SetColour( m_data.m_sFgColor );

    wxColourDialog dialog(this, &colorData);
    if (dialog.ShowModal() == wxID_OK)
    {
        colorData = dialog.GetColourData();
        wxColor newColor = colorData.GetColour();
        m_data.m_sFgColor = newColor.GetAsString( wxC2S_CSS_SYNTAX );
        m_fgBox->SetBackgroundColour( newColor );
    }
}

/************************* END OF FILE ***************************************/
