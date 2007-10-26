/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPiece.h
// Program's Name:  Huy's Open Xiangqi
// Created:         09/28/2007
/////////////////////////////////////////////////////////////////////////////

#include "wx/wx.h"

// Under Windows, change this to 1 to use wxGenericDragImage
#define wxUSE_GENERIC_DRAGIMAGE 1

#if wxUSE_GENERIC_DRAGIMAGE
  #include "wx/generic/dragimgg.h"
  #define wxDragImage wxGenericDragImage
#else
  #include "wx/dragimag.h"
#endif

#include "wx/image.h"

#include "hoxEnums.h"
#include "hoxUtility.h"
#include "hoxPosition.h"
#include "hoxPiece.h"

// -----------------------------------------------------------------------
// hoxPiece
// -----------------------------------------------------------------------

hoxPiece::hoxPiece( hoxPieceType       type, 
                    hoxPieceColor      color,
                    const hoxPosition& position )
            : m_type( type )
            , m_color( color )
            , m_position( position ) 
            , m_active( true )
            , m_show( true )
            , m_latest( false )
{
    // Load bitmap based on piece-type.
    wxImage image;

    if ( hoxUtility::LoadPieceImage(m_type, m_color, image) == hoxRESULT_OK )
    {
        m_bitmap = wxBitmap(image);
    }
}

// Copy constructor
hoxPiece::hoxPiece(const hoxPiece& piece)
{
    m_pos = piece.m_pos;
    // *** No need to copy bitmap;
    m_active = piece.m_active;
    m_show = piece.m_show;
    m_type = piece.m_type;
    m_color = piece.m_color;
    m_position = piece.m_position;
}

hoxPiece::~hoxPiece()
{
  // ... Doing nothing for now.
}

bool hoxPiece::Draw(wxDC& dc, const wxPoint& pos, int op) const
{
  if (m_bitmap.Ok())
  {
    wxMemoryDC memDC;
    memDC.SelectObject( const_cast<hoxPiece*>(this)->m_bitmap);

    dc.Blit( pos.x, pos.y, 
             m_bitmap.GetWidth(), m_bitmap.GetHeight(),
             &memDC, 0, 0, op, true);

    // Highlight the piece, if required.
    if ( m_latest )
    {
      dc.SetPen(*wxCYAN);
      dc.DrawLine( pos.x, pos.y, pos.x + m_bitmap.GetWidth(), pos.y);
      dc.DrawLine( pos.x + m_bitmap.GetWidth(), pos.y, pos.x + m_bitmap.GetWidth(), pos.y + m_bitmap.GetHeight());
      dc.DrawLine( pos.x + m_bitmap.GetWidth(), pos.y + m_bitmap.GetHeight(), pos.x, pos.y + m_bitmap.GetHeight());
      dc.DrawLine( pos.x, pos.y + m_bitmap.GetHeight(), pos.x, pos.y);
    }

    return true;
  }

  return false;
}

bool hoxPiece::SetPosition(const hoxPosition& pos) 
{ 
  if ( pos.IsValid() )
  {
    m_position = pos;
    return true;
  }
  return false;
}


const hoxPosition& hoxPiece::GetPosition() const 
{ 
  return m_position;
}

bool 
hoxPiece::IsInsidePalace() const 
{ 
  return m_position.IsInsidePalace(m_color);
}
