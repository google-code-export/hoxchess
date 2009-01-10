/***************************************************************************
 *  Copyright 2007, 2008, 2009 Huy Phan  <huyphan@playxiangqi.com>         *
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
// Name:            hoxReferee.h
// Created:         09/30/2007
//
// Description:     Implementing the standard Xiangqi referee.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_REFEREE_H_
#define __INCLUDED_HOX_REFEREE_H_

#include "hoxIReferee.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

namespace BoardInfoAPI
{
   class Board;
}

/**
 * The main Referee of this Application.
 */
class hoxReferee : public hoxIReferee
{
public:
    hoxReferee();
    virtual ~hoxReferee();

    /*********************************
     * Override base class virtuals
     *********************************/

    virtual void ResetGame();
    virtual bool ValidateMove( hoxMove&       move,
                               hoxGameStatus& status );
    virtual void GetGameState( hoxPieceInfoList& pieceInfoList,
                               hoxColor&         nextColor );
    virtual hoxColor GetNextColor();
    virtual hoxMove StringToMove( const wxString& sMove ) const;
    virtual void GetAvailableNextMoves( hoxMoveVector& moves ) const;

private:
    BoardInfoAPI::Board*  m_board;  // Board-Info.
};

#endif /* __INCLUDED_HOX_REFEREE_H_ */
