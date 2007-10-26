/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPlayerEvent.h
// Program's Name:  Huy's Open Xiangqi
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
