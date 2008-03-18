/***************************************************************************
 *  Copyright 2007, 2008 Huy Phan  <huyphan@playxiangqi.com>               *
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
// Name:            hoxNaiveReferee.cpp
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
    _nextColor = hoxCOLOR_RED;
}

bool 
hoxNaiveReferee::ValidateMove( hoxMove&       move,
                               hoxGameStatus& status )
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

    _nextColor = ( _nextColor == hoxCOLOR_RED 
                   ? hoxCOLOR_BLACK
                   : hoxCOLOR_RED);

    status = hoxGAME_STATUS_IN_PROGRESS;

    return true;
}

void 
hoxNaiveReferee::GetGameState( hoxPieceInfoList& pieceInfoList,
                               hoxColor&    nextColor )
{
    wxLogError(_("Not yet implemented."));
}

hoxColor
hoxNaiveReferee::GetNextColor()
{
    return _nextColor;
}

hoxMove
hoxNaiveReferee::StringToMove( const wxString& sMove ) const
{
    wxLogError(_("Not yet implemented."));
    return hoxMove();  // Return an "invalid" Move.
}

/************************* END OF FILE ***************************************/
