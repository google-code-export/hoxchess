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

#ifndef __INCLUDED_HOX_PIECE_H__
#define __INCLUDED_HOX_PIECE_H__

#include "hoxTypes.h"
#include "hoxUtil.h"

class hoxPiece : public wxObject
{
public:
    hoxPiece( const hoxPieceInfo& info,
              const wxString&     bitmapPath )
            : m_info( info )
            , m_active( true )
            , m_show( true )
            , m_latest( false )
    {
        this->LoadBitmap( bitmapPath );
    }

    void LoadBitmap( const wxString& bitmapPath )
    {
        wxImage image;
        if ( hoxRC_OK == hoxUtil::LoadPieceImage( bitmapPath,
                                                  m_info.type, m_info.color,
                                                  image ) )
        {
            m_bitmap = wxBitmap(image);
        }
    }
    const wxBitmap& GetBitmap() const { return m_bitmap; }

    bool IsInsidePalace() const // the 4-square area where the King resides...
        { return m_info.position.IsInsidePalace( m_info.color); }

    const hoxPieceInfo GetInfo() const { return m_info; }

    void SetPosition(const hoxPosition& pos) { m_info.position = pos; }
    const hoxPosition GetPosition() const { return m_info.position; }

    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }

    bool IsShown() const { return m_show; }
    void SetShow(bool show) { m_show = show; }

    hoxPieceType GetType() const { return m_info.type; }
    hoxColor GetColor() const { return m_info.color; }

    bool IsLatest() const { return m_latest; }
    void SetLatest(bool latest) { m_latest = latest; }

private:
    wxBitmap      m_bitmap;
    
    hoxPieceInfo  m_info;
    bool          m_active;  // Live or already killed?
    bool          m_show;

    bool          m_latest;  // The piece that just moved.
};

#endif /* __INCLUDED_HOX_PIECE_H__ */
