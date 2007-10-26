/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPiece.h
// Program's Name:  hox_Board
// Created:         09/28/2007
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_PIECE_H_
#define __INCLUDED_HOX_PIECE_H_

#include "wx/wx.h"

#include "hoxEnums.h"
#include "hoxPosition.h"

// hoxPiece

class hoxPiece: public wxObject
{
public:
    hoxPiece( hoxPieceType       type, 
              hoxPieceColor      color,
              const hoxPosition& position );
    hoxPiece(const hoxPiece& piece);   // Copy constructor
    ~hoxPiece();

//// Operations

  bool Draw(wxDC& dc, const wxPoint& pos, int op = wxCOPY) const;
  bool IsInsidePalace() const;  // the 4-square area where the King resides...

//// Accessors
  bool SetPosition(const hoxPosition& pos);
  const hoxPosition& GetPosition() const;

  wxBitmap& GetBitmap() const { return (wxBitmap&) m_bitmap; }
  void SetBitmap(const wxBitmap& bitmap) { m_bitmap = bitmap; }

  bool IsActive() const { return m_active; }
  void SetActive(bool active) { m_active = active; }

  bool IsShown() const { return m_show; }
  void SetShow(bool show) { m_show = show; }

  hoxPieceType GetType() const { return m_type; }
  void SetType(hoxPieceType type) { m_type = type; }

  hoxPieceColor GetColor() const { return m_color; }
  void SetColor(hoxPieceColor color) { m_color = color; }

  void SetLatest(bool latest) { m_latest = latest; }

private:
  wxBitmap      m_bitmap;

  wxPoint       m_pos;
  bool          m_active;  // Live or already killed?
  bool          m_show;
  hoxPieceType  m_type;   // What type? (Canon, Soldier, ...)
  hoxPieceColor m_color;  // What color? (Black or Red)
  hoxPosition   m_position;
  bool          m_latest;  // The piece that just moved.
};

#endif /* __INCLUDED_HOX_PIECE_H_ */