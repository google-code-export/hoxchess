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
// Name:            hoxTSITOPlayer.cpp
// Created:         09/07/2008
//
// Description:     The AI Player based on the open-source Xiangqi Engine
//                  called TSITO 
//                     http://xiangqi-engine.sourceforge.net/tsito.html
/////////////////////////////////////////////////////////////////////////////

#include "hoxTSITOPlayer.h"
#include "hoxUtil.h"
#include "hoxReferee.h"

#include "tsito/Move.h"
#include "tsito/Board.h"
#include "tsito/Lawyer.h"
#include "tsito/Engine.h"

//-----------------------------------------------------------------------------
// hoxTSITOPlayer::TSITO_Engine
//-----------------------------------------------------------------------------

class hoxTSITOPlayer::TSITO_Engine
{
public:
    TSITO_Engine( hoxIReferee_SPtr referee )
            : m_referee( referee )
    {
        m_board.reset( new Board() );
        m_lawyer.reset( new Lawyer( m_board.get() ) );
        m_engine.reset( new Engine( m_board.get(),
                                    m_lawyer.get() ) );
    }

    void _translate( const hoxMove& hMove,
                     Move&          tMove)
    {
        char fromX = hMove.piece.position.x;
        char fromY = hMove.piece.position.y;
        char toX = hMove.newPosition.x;
        char toY = hMove.newPosition.y;

        tMove.origin(     9 * fromY + fromX );
        tMove.destination( 9 * toY + toX );
    }

    void _translate( Move&    tMove,
                     hoxMove& hMove )
    {
        wxString sMove;
        sMove.Printf("%d%d%d%d", 
            tMove.origin() % 9,
            tMove.origin() / 9,
            tMove.destination() % 9,
            tMove.destination() / 9 );

        hMove = m_referee->StringToMove( sMove );
        wxASSERT( hMove.IsValid() );
    }

    void OnHumanMove( const hoxMove& move )
    {
        Move tMove;
        _translate( move, tMove);
        m_board->makeMove( tMove);
    }

    hoxMove generateMove()
    {
        hoxMove  hMove;
        Move     move;

        m_engine->think();

        if ( m_engine->doneThinking() )
        {
          move = m_engine->getMove();
          Move x = Move();
          if (!(move == x))
            {
              // HPHAN: Timer::timerForColor(board->sideToMove())->stopTimer();
              // HPHAN: Timer::timerForColor(board->sideToMove())->moveMade();
              m_board->makeMove(move);
              // HPHAN: interfce->printBoard(*board);
              // HPHAN: interfce->printMove(move);
              // HPHAN: interfce->printPrompt();
              // HPHAN: Timer::timerForColor(board->sideToMove())->startTimer();

              _translate( move, hMove );
            }
        }

        return hMove;
    }

private:
    hoxIReferee_SPtr    m_referee;

    typedef std::auto_ptr<Board>  TSITO_Board_APtr;
    typedef std::auto_ptr<Lawyer> TSITO_Lawyer_APtr;
    typedef std::auto_ptr<Engine> TSITO_Engine_APtr;

    TSITO_Board_APtr    m_board;
    TSITO_Lawyer_APtr   m_lawyer;
    TSITO_Engine_APtr   m_engine;
};

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_CLASS(hoxTSITOPlayer, hoxAIPlayer)

BEGIN_EVENT_TABLE(hoxTSITOPlayer, hoxAIPlayer)
    // Empty
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxTSITOPlayer
//-----------------------------------------------------------------------------

hoxTSITOPlayer::hoxTSITOPlayer( const wxString& name,
                                hoxPlayerType   type,
                                int             score )
            : hoxAIPlayer( name, type, score )
            , m_tsito_engine( new TSITO_Engine( m_referee ) )
{ 
}

hoxTSITOPlayer::~hoxTSITOPlayer()
{
    delete m_tsito_engine;
}

void
hoxTSITOPlayer::OnOpponentMove( const hoxMove& move )
{
    m_tsito_engine->OnHumanMove( move );
}

hoxMove
hoxTSITOPlayer::generateNextMove()
{
    return m_tsito_engine->generateMove();
}

/************************* END OF FILE ***************************************/
