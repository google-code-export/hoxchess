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
// Name:            hoxOptionsUI.h
// Created:         10/20/2008
//
// Description:     The UI containing the App's Options.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_OPTIONS_UI_H__
#define __INCLUDED_HOX_OPTIONS_UI_H__

#include <wx/wx.h>
#include <wx/propdlg.h>

// ---------------------------------------------------------------------------
// hoxOptionsUI class
// ---------------------------------------------------------------------------

class hoxOptionsUI : public wxPropertySheetDialog
{
public:
    class OptionsData
    {
    public:
        bool      m_bSound;
        wxString  m_sBgColor;
        wxString  m_sFgColor;
    };

public:
    hoxOptionsUI( wxWindow* parent, const OptionsData& data );
    virtual ~hoxOptionsUI() {}

    virtual int ShowModal();

    OptionsData GetData() const { return m_data; }

protected:
    void OnSound( wxCommandEvent& event );
    void OnBgColor( wxCommandEvent& event );
    void OnFgColor( wxCommandEvent& event );

private:
    wxPanel* _CreateGeneralPage( wxWindow* parent );
    wxPanel* _CreateBoardPage( wxWindow* parent );

public:
    OptionsData  m_data;

    wxCheckBox*  m_soundCheck;
    wxButton*    m_bgBox;  // Background
    wxButton*    m_fgBox;  // Foreground

private:

    enum {
        ID_SOUND = 100 ,
        ID_BG_COLOR,   // Background
        ID_FG_COLOR    // Foreground
    };

    DECLARE_EVENT_TABLE()

}; // END of hoxOptionsUI

#endif /* __INCLUDED_HOX_OPTIONS_UI_H__ */
