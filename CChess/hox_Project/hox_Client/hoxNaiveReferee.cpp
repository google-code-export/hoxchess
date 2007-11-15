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
// Name:            hoxIReferre.cpp
// Created:         09/29/2007
//
// Description:     A simple (naive) Referee.
/////////////////////////////////////////////////////////////////////////////

#include "hoxNaiveReferee.h"
#include "hoxPiece.h"

hoxNaiveReferee::hoxNaiveReferee()
{
    this->Reset();
}

hoxNaiveReferee::~hoxNaiveReferee()
{
}

void
hoxNaiveReferee::Reset()
{
    _nextColor = hoxPIECE_COLOR_RED;
}

bool 
hoxNaiveReferee::ValidateMove( hoxMove& move )
{
    /**
     * NOTE: Only check for 'turns'. Otherwise, allow everything.
     */

    /* Check for 'turn' */

    if ( move.piece.color != _nextColor )
    {
        return false; // Error! Wrong turn.
    }

    /* Set the next-turn. */

    _nextColor = ( _nextColor == hoxPIECE_COLOR_RED 
                   ? hoxPIECE_COLOR_BLACK
                   : hoxPIECE_COLOR_RED);

    return true;
}

void 
hoxNaiveReferee::GetGameState( hoxPieceInfoList& pieceInfoList,
                               hoxPieceColor&    nextColor )
{
    wxLogError(_("Not yet implemented."));
}

bool 
hoxNaiveReferee::GetPieceAtPosition( const hoxPosition& position, 
                                     hoxPieceInfo&      pieceInfo ) const
{
    wxLogError(_("Not yet implemented."));
    return false;
}

/************************* END OF FILE ***************************************/
