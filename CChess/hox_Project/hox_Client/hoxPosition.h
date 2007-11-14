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
// Name:            hoxPosition.h
// Created:         09/28/2007
//
// Description:     Representing a Position of Piece on Board.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_POSITION_H_
#define __INCLUDED_HOX_POSITION_H_

#include "hoxEnums.h"

// hoxPosition

class hoxPosition
{
public:
    char x;
    char y;

public:
    hoxPosition(char xx = -1, char yy = -1) : x(xx), y(yy) {}
    hoxPosition(const hoxPosition& pos);
    ~hoxPosition();

    hoxPosition& operator=(const hoxPosition& pos);
    bool operator==(const hoxPosition& pos) const;
    bool operator!=(const hoxPosition& pos) const;

    bool IsValid() const;
    bool IsInsidePalace(hoxPieceColor color) const;
    bool IsInsideCountry(hoxPieceColor color) const;
};

#endif /* __INCLUDED_HOX_POSITION_H_ */
