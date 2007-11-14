/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPiece.h
// Program's Name:  Huy's Open Xiangqi
// Created:         09/28/2007
//
// Description:     The UI Piece.
/////////////////////////////////////////////////////////////////////////////

#include <wx/wx.h>
#if 0
// Under Windows, change this to 1 to use wxGenericDragImage
//#define wxUSE_GENERIC_DRAGIMAGE 1

#if wxUSE_GENERIC_DRAGIMAGE
  #include "wx/generic/dragimgg.h"
  #define wxDragImage wxGenericDragImage
#else
  #include "wx/dragimag.h"
#endif
#endif
#include <wx/image.h>

#include "hoxPiece.h"
#include "hoxEnums.h"
#include "hoxUtility.h"
#include "hoxPosition.h"

// -----------------------------------------------------------------------
// hoxPiece
// -----------------------------------------------------------------------

hoxPiece::hoxPiece( const hoxPieceInfo& info )
            : m_info( info )
            , m_active( true )
            , m_show( true )
            , m_latest( false )
{
    wxImage image;

    if ( hoxRESULT_OK == hoxUtility::LoadPieceImage( m_info.type, 
                                                     m_info.color, 
                                                     image ) )
    {
        m_bitmap = wxBitmap(image);
    }
}

hoxPiece::~hoxPiece()
{
  // ... Doing nothing for now.
}

bool 
hoxPiece::SetPosition(const hoxPosition& pos) 
{ 
    if ( pos.IsValid() )
    {
        m_info.position = pos;
        return true;
    }
    return false;
}

bool 
hoxPiece::IsInsidePalace() const 
{ 
    return m_info.position.IsInsidePalace( m_info.color);
}
