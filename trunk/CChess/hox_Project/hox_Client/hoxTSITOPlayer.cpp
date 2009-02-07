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
// Name:            hoxTSITOPlayer.cpp
// Created:         09/07/2008
//
// Description:     The AI Player based on the open-source Xiangqi Engine
//                  called TSITO written by Noah Roberts
//                     http://xiangqi-engine.sourceforge.net/tsito.html
/////////////////////////////////////////////////////////////////////////////

#include "hoxTSITOPlayer.h"

#include "../tsito/Move.h"
#include "../tsito/Board.h"
#include "../tsito/Lawyer.h"
#include "../tsito/tsiEngine.h"

//-----------------------------------------------------------------------------
// hoxTSITOEngine::TSITO_Engine
//-----------------------------------------------------------------------------

class hoxTSITOEngine::TSITO_Engine
{
public:
    TSITO_Engine()
    {
        m_board.reset( new Board() );
        m_lawyer.reset( new Lawyer( m_board.get() ) );
        m_engine.reset( new tsiEngine( m_board.get(),
                                    m_lawyer.get() ) );
    }

    Move _translateStringToMove( const wxString& sMove )
    {
        wxASSERT( sMove.size() == 4 ); // "xyXY"
        char fromX = sMove[0] - '0';
        char fromY = sMove[1] - '0';
        char toX   = sMove[2] - '0';
        char toY   = sMove[3] - '0';

        Move tMove;
        tMove.origin(     9 * fromY + fromX );
        tMove.destination( 9 * toY + toX );
        return tMove;
    }

    wxString _translateMoveToString( Move& tMove )
    {
        wxString sMove;
        sMove.Printf("%d%d%d%d", tMove.origin() % 9,
                                 tMove.origin() / 9,
                                 tMove.destination() % 9,
                                 tMove.destination() / 9 );
        return sMove;
    }

    void OnHumanMove( const wxString& sMove )
    {
        Move tMove = _translateStringToMove( sMove );
        m_board->makeMove( tMove);
    }

    wxString GenerateMove()
    {
        wxString sNextMove;

        m_engine->think();

        if ( m_engine->doneThinking() )
        {
          Move move = m_engine->getMove();
          Move x = Move();
          if (!(move == x))
            {
              m_board->makeMove( move );
              sNextMove = _translateMoveToString( move );
            }
        }

        return sNextMove;
    }

private:
    typedef std::auto_ptr<Board>  TSITO_Board_APtr;
    typedef std::auto_ptr<Lawyer> TSITO_Lawyer_APtr;
    typedef std::auto_ptr<tsiEngine> TSITO_Engine_APtr;

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
{ 
}

void 
hoxTSITOPlayer::Start()
{
    hoxConnection_APtr connection( new hoxTSITOConnection( this ) );
    this->SetConnection( connection );
    this->GetConnection()->Start();
}


// ----------------------------------------------------------------------------
// hoxTSITOEngine
// ----------------------------------------------------------------------------

hoxTSITOEngine::hoxTSITOEngine( wxEvtHandler* player )
        : hoxAIEngine( player )
        , m_tsito_engine( new TSITO_Engine() )
{
}

hoxTSITOEngine::~hoxTSITOEngine()
{
    delete m_tsito_engine;
}

void
hoxTSITOEngine::OnOpponentMove( const wxString& sMove )
{
    m_tsito_engine->OnHumanMove( sMove );
}

wxString
hoxTSITOEngine::GenerateNextMove()
{
    return m_tsito_engine->GenerateMove();
}


// ----------------------------------------------------------------------------
// hoxTSITOConnection
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(hoxTSITOConnection, hoxAIConnection)

hoxTSITOConnection::hoxTSITOConnection( wxEvtHandler* player )
        : hoxAIConnection( player )
{
}

void
hoxTSITOConnection::CreateAIEngine()
{
    m_aiEngine.reset( new hoxTSITOEngine( this->GetPlayer() ) );
}

/************************* END OF FILE ***************************************/
