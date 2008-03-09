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
// Name:            hoxNaiveReferee.h
// Created:         09/29/2007
//
// Description:     A simple (naive) Referee.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_NAIVE_REFEREE_H_
#define __INCLUDED_HOX_NAIVE_REFEREE_H_

#include "hoxIReferee.h"
#include "hoxEnums.h"

/**
 * A very simple-minded (naive) Referee.
 */
class hoxNaiveReferee : public hoxIReferee
{
  public:
    hoxNaiveReferee();
    virtual ~hoxNaiveReferee();

    /*********************************
     * Override base class virtuals
     *********************************/

    virtual void Reset();
    virtual bool ValidateMove( hoxMove&       move,
                               hoxGameStatus& status );
    virtual void GetGameState( hoxPieceInfoList& pieceInfoList,
                               hoxColor&    nextColor );
    virtual hoxColor GetNextColor();
    virtual bool GetPieceAtPosition( const hoxPosition& position, 
                                     hoxPieceInfo&      pieceInfo ) const;

  private:
    hoxColor    _nextColor;
        /* Whose's turn (RED or BLACK) is it to move? */
};

#endif /* __INCLUDED_HOX_NAIVE_REFEREE_H_ */
