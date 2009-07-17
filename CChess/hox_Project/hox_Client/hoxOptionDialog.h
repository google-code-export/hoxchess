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
// Name:            hoxOptionDialog.h
// Created:         03/30/2008
//
// Description:     The dialog to change the Options of a Table.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_OPTION_DIALOG_H__
#define __INCLUDED_HOX_OPTION_DIALOG_H__

#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// The Option-Dialog class
// ----------------------------------------------------------------------------

class hoxOptionDialog : public wxDialog
{
public:
    /**
     * The Dialog's flags.
     */
    enum OptionFlagEnum
    {
	    /* NOTE: The numeric values are of the 32-bitmap ones. */

        hoxOPTION_READONLY_FLAG        = ( (unsigned int) 1 )
                /* Read-Only options */
    };

    hoxOptionDialog( wxWindow*           parent, 
                     wxWindowID          id, 
                     const wxString&     title,
                     const hoxTable_SPtr pTable,
                     unsigned int        optionFlags = 0 );

    void OnButtonSave(wxCommandEvent& event);

    bool           IsRatedGame() const { return m_bRatedGame; }
	hoxTimeInfo    GetNewTimeInfo() const { return m_newTimeInfo; }

private:
    wxCheckBox*         m_checkBoxRatedGame;

    wxTextCtrl*         m_textCtrlTime_Game;
    wxTextCtrl*         m_textCtrlTime_Move;
    wxTextCtrl*         m_textCtrlTime_Free;

    const hoxTable_SPtr m_pTable;
    bool                m_bRatedGame;  // Rated/Non-Rated Game.
    hoxTimeInfo         m_newTimeInfo;

    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_OPTION_DIALOG_H__ */
