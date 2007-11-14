/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPiece.h
// Program's Name:  hox_Board
// Created:         09/28/2007
//
// Description:     The UI Piece.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_PIECE_H_
#define __INCLUDED_HOX_PIECE_H_

#include <wx/wx.h>

#include "hoxEnums.h"
#include "hoxTypes.h"
#include "hoxPosition.h"

// hoxPiece

class hoxPiece: public wxObject
{
public:
    hoxPiece( const hoxPieceInfo& info );
    ~hoxPiece();

    /* Main API */

    bool IsInsidePalace() const;  // the 4-square area where the King resides...

    /* Other API */

    const hoxPieceInfo GetInfo() const { return m_info; }

    bool SetPosition(const hoxPosition& pos);
    const hoxPosition GetPosition() const { return m_info.position; }

    const wxBitmap& GetBitmap() const { return m_bitmap; }
    void SetBitmap(const wxBitmap& bitmap) { m_bitmap = bitmap; }

    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }

    bool IsShown() const { return m_show; }
    void SetShow(bool show) { m_show = show; }

    hoxPieceType GetType() const { return m_info.type; }
    void SetType(hoxPieceType type) { m_info.type = type; }

    hoxPieceColor GetColor() const { return m_info.color; }
    void SetColor(hoxPieceColor color) { m_info.color = color; }

    bool IsLatest() const { return m_latest; }
    void SetLatest(bool latest) { m_latest = latest; }

private:
    wxBitmap      m_bitmap;
    
    hoxPieceInfo  m_info;
    bool          m_active;  // Live or already killed?
    bool          m_show;

    bool          m_latest;  // The piece that just moved.
};

#endif /* __INCLUDED_HOX_PIECE_H_ */
