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
// Name:            hoxAIPlayer.h
// Created:         05/04/2008
//
// Description:     The Artificial Intelligent (AI) Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_AI_PLAYER_H_
#define __INCLUDED_HOX_AI_PLAYER_H_

#include "hoxPlayer.h"
#include "hoxTypes.h"

/**
 * The AI player.
 */
class hoxAIPlayer :  public hoxPlayer
{
public:
    hoxAIPlayer(); // DUMMY default constructor required for event handler.
    hoxAIPlayer( const wxString& name,
                 hoxPlayerType   type,
                 int             score );

    virtual ~hoxAIPlayer();

	/*******************************
     * Table-event handlers
     *******************************/

    virtual void OnRequest_FromTable( hoxRequest_APtr apRequest );

private:
    hoxMove _generateNextMove();

private:
    hoxIReferee_SPtr  m_referee;
        /* Currently, this is a very simple-minded AI Player
         * who only selects Moves randomly from the list
         * of current valid Moves.
         * This Referee helps to return the list of valid Moves.
         */

    DECLARE_DYNAMIC_CLASS(hoxAIPlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_AI_PLAYER_H_ */
