/***************************************************************************
 *  Copyright 2007 Huy Phan  <huyphan@playxiangqi.com>                     *
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
// Name:            hoxPlayerEvent.h
// Created:         10/10/2007
//
// Description:     The Player-Event.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_PLAYER_EVENT_H_
#define __INCLUDED_HOX_PLAYER_EVENT_H_

#include "wx/wx.h"
#include "hoxEnums.h"
#include "hoxTypes.h"
#include "hoxPiece.h"

// ----------------------------------------------------------------------
//  hoxPlayerEvent
// ----------------------------------------------------------------------

class hoxPlayerEvent : public wxNotifyEvent
{
public:
    hoxPlayerEvent( wxEventType commandType = wxEVT_NULL, int id = wxID_ANY );

    // accessors

    wxString GetTableId() { return m_tableId; }
    void SetTableId(const wxString tableId) { m_tableId = tableId; }

    hoxPiece *GetPiece() { return m_pPiece; }
    void SetPiece(hoxPiece* piece) { m_pPiece = piece; }

    hoxPosition GetOldPosition() { return m_oldPosition; }
    void SetOldPosition(hoxPosition pos) { m_oldPosition = pos; }

    hoxPosition GetPosition() { return m_position; }
    void SetPosition(hoxPosition pos) { m_position = pos; }

    // Required for sending with wxPostEvent()
    wxEvent* Clone() const;

private:
    wxString        m_tableId;
    hoxPiece*       m_pPiece;
    hoxPosition     m_oldPosition;    // Old position.
    hoxPosition     m_position;
};


#endif /* __INCLUDED_HOX_PLAYER_EVENT_H_ */
