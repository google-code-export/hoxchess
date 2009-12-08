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
// Name:            hoxOptionDialog.cpp
// Created:         03/30/2008
//
// Description:     The dialog to change the Options of a Table.
/////////////////////////////////////////////////////////////////////////////

#include "hoxOptionDialog.h"
#include "hoxTable.h"
#include "hoxUtil.h"

BEGIN_EVENT_TABLE(hoxOptionDialog, wxDialog)
    EVT_BUTTON(wxID_OK, hoxOptionDialog::OnButtonSave)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxLoginDialog
//-----------------------------------------------------------------------------

hoxOptionDialog::hoxOptionDialog( wxWindow*           parent, 
                                  wxWindowID          id, 
                                  const wxString&     title,
                                  const hoxTable_SPtr pTable,
                                  unsigned int        optionFlags /* = 0 */ )
        : wxDialog( parent, id, title )
		, m_pTable( pTable )
{
    const hoxTimeInfo currentTimeInfo = m_pTable->GetInitialTime();

    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

	/* Rated/Non-Rated Game. */

    wxBoxSizer* ratedGameSizer = new wxBoxSizer( wxHORIZONTAL );

    m_checkBoxRatedGame = new wxCheckBox(this, wxID_ANY, _("&Rated Game"));
    m_bRatedGame = ( m_pTable->GetGameType() == hoxGAME_TYPE_RATED );
    m_checkBoxRatedGame->SetValue( m_bRatedGame );

    ratedGameSizer->Add( m_checkBoxRatedGame,
                         wxSizerFlags().Align(wxALIGN_LEFT) );

    topSizer->Add( ratedGameSizer, 
		wxSizerFlags().Border(wxALL, 10).Align(wxALIGN_LEFT).Expand());

	/* Timers. */

	wxBoxSizer* timerSizer = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, _("&Timers")), 
		wxHORIZONTAL );

    m_textCtrlTime_Game = new wxTextCtrl( 
		this, wxID_ANY,
        wxString::Format("%d", currentTimeInfo.nGame / 60), // default value
        wxDefaultPosition, wxSize(50, wxDefaultCoord ));

    m_textCtrlTime_Move = new wxTextCtrl( 
		this, wxID_ANY,
        wxString::Format("%d", currentTimeInfo.nMove), // default value
        wxDefaultPosition, wxSize(50, wxDefaultCoord ));

    m_textCtrlTime_Free = new wxTextCtrl( 
		this, wxID_ANY,
        wxString::Format("%d", currentTimeInfo.nFree), // default value
        wxDefaultPosition, wxSize(50, wxDefaultCoord ));

    timerSizer->Add( new wxStaticText(this, wxID_ANY, _("Game: \n(minutes)")),
		wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));
    timerSizer->Add( m_textCtrlTime_Game,
        wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

    timerSizer->AddSpacer(20);
    timerSizer->Add( new wxStaticText(this, wxID_ANY, _("Move: \n(seconds)")),
		wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));
    timerSizer->Add( m_textCtrlTime_Move,
        wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

    timerSizer->AddSpacer(20);
    timerSizer->Add( new wxStaticText(this, wxID_ANY, _("Free: \n(seconds)")),
		wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));
    timerSizer->Add( m_textCtrlTime_Free,
        wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP, 10));

    topSizer->Add( timerSizer, 
		wxSizerFlags().Border(wxALL, 10).Align(wxALIGN_LEFT).Expand());

    /* Buttons */

    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );
	wxButton* buttonSave   = new wxButton(this, wxID_OK, _("&Save Options"));
    wxButton* buttonCancel = new wxButton(this, wxID_CANCEL, _("&Cancel"));

	/* Disable certain buttons based on the input Option Flags. */
	buttonSave->Enable( (optionFlags & hoxOPTION_READONLY_FLAG) == 0 );

    buttonSizer->Add( buttonCancel,
                      wxSizerFlags().Proportion(0).Align(wxALIGN_CENTER));
    buttonSizer->AddSpacer(20);
    buttonSizer->Add( buttonSave,
                      wxSizerFlags().Proportion(0).Align(wxALIGN_CENTER));

    topSizer->Add( buttonSizer,
		           wxSizerFlags().Border(wxALL, 10).Align(wxALIGN_CENTER));

    SetSizer( topSizer );      // use the sizer for layout
	topSizer->SetSizeHints( this );   // set size hints to honour minimum size
}

void 
hoxOptionDialog::OnButtonSave(wxCommandEvent& event)
{
    wxString sValue;

    /* Determine the new Rated/Non-Rated Game option. */
    m_bRatedGame = m_checkBoxRatedGame->GetValue();

	/* Determine the new Timers. */

    sValue = m_textCtrlTime_Game->GetValue();
    m_newTimeInfo.nGame = ::atoi( sValue.c_str() ) * 60; // ... to seconds
	if ( m_newTimeInfo.nGame <= 0  )
	{
		wxLogError("Game Time [%s] is invalid.", sValue.c_str());
		return;
	}

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * NOTE: Allow Move-time to be zero since some site such as Chesscape
     *       supports the so-called "infinite" Move-Time.
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     */
    sValue = m_textCtrlTime_Move->GetValue();
    m_newTimeInfo.nMove = ::atoi( sValue.c_str() );
	if ( m_newTimeInfo.nMove < 0  )
	{
		wxLogError("Move Time [%s] is invalid.", sValue.c_str());
		return;
	}

    sValue = m_textCtrlTime_Free->GetValue();
    m_newTimeInfo.nFree = ::atoi( sValue.c_str() );
	if ( m_newTimeInfo.nFree <= 0  )
	{
		wxLogError("Free Time [%s] is invalid.", sValue.c_str());
		return;
	}

    wxLogDebug("%s: Table [%s]: New Rated-Game = [%d], Timers = [%s].", __FUNCTION__,
        m_pTable->GetId().c_str(), m_bRatedGame,
        hoxUtil::TimeInfoToString(m_newTimeInfo).c_str());

    EndDialog( wxID_OK );
}

/************************* END OF FILE ***************************************/
