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
// Name:            hoxPiece.h
// Created:         09/28/2007
//
// Description:     The UI Piece.
/////////////////////////////////////////////////////////////////////////////

#include <wx/wx.h>
#include <wx/image.h>

#include "hoxPiece.h"
#include "hoxEnums.h"
#include "hoxUtil.h"

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

    if ( hoxRC_OK == hoxUtil::LoadPieceImage( m_info.type, 
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
