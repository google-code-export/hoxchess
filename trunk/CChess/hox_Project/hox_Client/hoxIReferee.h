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
// Name:            hoxIReferee.h
// Created:         09/29/2007
//
// Description:     The interface of a Referee.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_IREFEREE_H__
#define __INCLUDED_HOX_IREFEREE_H__

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
    virtual void ResetGame() = 0;

    /**
     * Validate and record a given Move.
     *
     * @note The Referee will fill in the information about
     *       which Piece, if any, is captured as a result of the Move.
     */
    virtual bool ValidateMove( hoxMove&       move,
                               hoxGameStatus& status ) = 0;

    /**
     * Get the current state of the game:
     *   + The info of all 'live' pieces.
     *   + Which side (RED or BLACK) should move next.
     *   + Whether the game has ended (in-progress, red-win, black-win).
     */
    virtual void GetGameState( hoxGameState& gameState ) const = 0;

    /**
     * Get the list of (past) Moves made so far.
     */
    virtual void GetHistoryMoves( hoxMoveList& moveList ) const = 0;

    /**
     * Get the NEXT color, which specifies who (RED or BLACK) should
     * move next.
     */
    virtual hoxColor GetNextColor() const = 0;

    /**
     * Convert a string into a Move.
     * @return an invalid Move if the string is "invalid".
     */
    virtual hoxMove StringToMove( const wxString& sMove ) const = 0;

    /**
     * Get all available Moves of the NEXT color.
     * @param moves The [OUT] returned vector containing the list of
     *              valid 'next' Moves.
     * @note The returned vector is empty if the game is over.
     */
    virtual void GetAvailableNextMoves( hoxMoveVector& moves ) const = 0;


public: /* STATIC API */

    /**
     * Check if a given game-status is a "game-over" status.
     */
    static bool IsGameOverStatus( const hoxGameStatus status )
    {
        return (   status == hoxGAME_STATUS_RED_WIN
                || status == hoxGAME_STATUS_BLACK_WIN
                || status == hoxGAME_STATUS_DRAWN );
    }
};

#endif /* __INCLUDED_HOX_IREFEREE_H__ */
