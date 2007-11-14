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
// Name:            hoxIReferre.h
// Created:         09/29/2007
//
// Description:     The interface of a Referee.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_IREFEREE_H_
#define __INCLUDED_HOX_IREFEREE_H_

#include "hoxTypes.h"

/**
 * Interface for a referee.
 */
class hoxIReferee
{
  public:
    hoxIReferee() {}
    virtual ~hoxIReferee() {}

    /**
     * Reset the game that this referee is residing over.
     */
    virtual void Reset() = 0;

    /**
     * Validate and record a given move.
     */
    virtual bool ValidateMove(const hoxMove& move) = 0;

    /**
     * Get the current state of the game:
     *   + The info of all 'live' pieces.
     *   + Which side (RED or BLACK) will move next?
     */
    virtual void GetGameState( hoxPieceInfoList& pieceInfoList,
                               hoxPieceColor&    nextColor ) = 0;

    /**
     * Lookup a piece-info at a specified position.
     *
     * @return true if found.
     */
    virtual bool GetPieceAtPosition( const hoxPosition& position, 
                                     hoxPieceInfo&      pieceInfo ) const = 0;
};

#endif /* __INCLUDED_HOX_IREFEREE_H_ */
