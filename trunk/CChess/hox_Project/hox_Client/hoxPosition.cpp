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
// Name:            hoxPosition.cpp
// Created:         09/28/2007
//
// Description:     Representing a Position on Board.
/////////////////////////////////////////////////////////////////////////////


#include "hoxPosition.h"

//
// hoxPosition
//

hoxPosition::hoxPosition(const hoxPosition& pos)
{
    if ( &pos != this )
    {
        x = pos.x;
        y = pos.y;
    }
}

hoxPosition::~hoxPosition()
{
    // Doing nothing
}

hoxPosition& 
hoxPosition::operator=(const hoxPosition& pos)
{
    x = pos.x;
    y = pos.y;
    return *this;
}

bool
hoxPosition::operator==(const hoxPosition& pos) const
{
    return (x == pos.x && y == pos.y);
}

bool
hoxPosition::operator!=(const hoxPosition& pos) const
{
    return (x != pos.x || y != pos.y);
}

bool 
hoxPosition::IsValid() const 
{ 
    return (x >= 0 && x <= 8 && y >= 0 && y <= 9); 
}

bool 
hoxPosition::IsInsidePalace(hoxPieceColor color) const 
{ 
    if (color == hoxPIECE_COLOR_BLACK)
    {
        return (x >= 3 && x <= 5 && y >= 0 && y <= 2); 
    }
    else  // Red?
    {
        return (x >= 3 && x <= 5 && y >= 7 && y <= 9); 
    }
}

// Is inside one's country (not yet cross the river)?
bool 
hoxPosition::IsInsideCountry(hoxPieceColor color) const 
{ 
    if (color == hoxPIECE_COLOR_BLACK)
    {
        return (y >= 0 && y <= 4);
    }
    else  // Red?
    {
        return (y >= 5 && y <= 9);
    }
}

/************************* END OF FILE ***************************************/
