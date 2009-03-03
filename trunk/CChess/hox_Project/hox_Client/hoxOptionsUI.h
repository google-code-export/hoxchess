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
// Name:            hoxOptionsUI.h
// Created:         10/20/2008
//
// Description:     The UI containing the App's Options.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_OPTIONS_UI_H__
#define __INCLUDED_HOX_OPTIONS_UI_H__

#include <wx/wx.h>
#include <wx/propdlg.h>
#include <wx/clrpicker.h> // Color Picker Dialog

// ---------------------------------------------------------------------------
// hoxOptionsUI class
// ---------------------------------------------------------------------------

class hoxOptionsUI : public wxPropertySheetDialog
{
public:
    class OptionsData
    {
    public:
        bool         m_bSound;
        wxLanguage   m_language;
        wxString     m_sBgColor;
        wxString     m_sFgColor;
        wxString     m_sPiece;
        wxString     m_sDefaultAI;
    };

public:
    hoxOptionsUI( wxWindow* parent, const OptionsData& data );
    virtual ~hoxOptionsUI() {}

    OptionsData GetData() const { return m_data; }

protected:
    void OnSound( wxCommandEvent& event );
    void OnLanguage( wxCommandEvent& event );
    void OnColorChanged( wxColourPickerEvent& event );
    void OnPiece( wxCommandEvent& event );
    void OnDefaultAI( wxCommandEvent& event );

private:
    wxPanel* _CreateGeneralPage( wxWindow* parent );
    wxPanel* _CreateBoardPage( wxWindow* parent );
    wxPanel* _CreateAIPage( wxWindow* parent );

    wxArrayString _loadAvailablePieceSets() const;

public:
    OptionsData           m_data;

    wxCheckBox*           m_soundCheck;
    wxTextCtrl*           m_languageTextCtrl;
    wxColourPickerCtrl*   m_bgBox;  // Background
    wxColourPickerCtrl*   m_fgBox;  // Foreground
    wxTextCtrl*           m_pieceTextCtrl;
    wxTextCtrl*           m_defaultAITextCtrl;

private:

    DECLARE_EVENT_TABLE()

}; // END of hoxOptionsUI

#endif /* __INCLUDED_HOX_OPTIONS_UI_H__ */
