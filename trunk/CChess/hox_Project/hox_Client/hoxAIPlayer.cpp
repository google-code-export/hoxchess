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
// Name:            hoxAIPlayer.cpp
// Created:         05/04/2008
//
// Description:     The Artificial Intelligent (AI) Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxAIPlayer.h"
#include "hoxUtil.h"
#include "hoxReferee.h"
#include "hoxTable.h"

IMPLEMENT_DYNAMIC_CLASS(hoxAIPlayer, hoxPlayer)

BEGIN_EVENT_TABLE(hoxAIPlayer, hoxPlayer)
    // Empty
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxAIPlayer
//-----------------------------------------------------------------------------

hoxAIPlayer::hoxAIPlayer()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxAIPlayer::hoxAIPlayer( const wxString& name,
                          hoxPlayerType   type,
                          int             score )
            : hoxPlayer( name, type, score )
            , m_referee( new hoxReferee() )
{ 
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxAIPlayer::~hoxAIPlayer() 
{
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER.", FNAME);
}

void 
hoxAIPlayer::OnRequest_FromTable( hoxRequest_APtr apRequest )
{
    const char* FNAME = __FUNCTION__;

    switch ( apRequest->type )
    {
        case hoxREQUEST_MOVE:
        {
            const wxString sMove = apRequest->parameters["move"];
            wxLogDebug("%s: Received Move [%s].", FNAME, sMove.c_str());
            hoxMove move = m_referee->StringToMove( sMove );
            wxASSERT( move.IsValid() );

            hoxGameStatus gameStatus = hoxGAME_STATUS_UNKNOWN;
            bool bValid = m_referee->ValidateMove( move, gameStatus );
            wxASSERT( bValid );

            this->OnOpponentMove( move );

            if ( gameStatus == hoxGAME_STATUS_IN_PROGRESS )
            {
                hoxMove myNextMove = generateNextMove();
                wxLogDebug("%s: Generated next Move = [%s].", FNAME, myNextMove.ToString().c_str());

                bValid = m_referee->ValidateMove( myNextMove, gameStatus );
                wxASSERT( bValid );

                hoxTable_SPtr pTable = this->GetFrontTable();
                wxASSERT( pTable.get() != NULL );
                pTable->OnMove_FromNetwork( this, myNextMove.ToString() );
            }
            break;
        }
        default:
        {
            wxLogDebug("%s: Do nothing for other requests [%s].",
                FNAME, hoxUtil::RequestTypeToString(apRequest->type).c_str());
            break; // Do nothing for other requests.
        }
    }
}

void
hoxAIPlayer::OnOpponentMove( const hoxMove& move )
{
    // Do nothing.
}

hoxMove
hoxAIPlayer::generateNextMove()
{
    const hoxMove invalidMove;  // Returned if no Move can be generated.

    /* -----------------------------------------------------------
     * Here is a naive algorithm to generate the next Move:
     *
     *  (1) Get the list of all available 'next' Moves.
     *  (2) Randomly pick a Move from the 'available' list.
     *
     * ----------------------------------------------------------- */

    hoxMoveVector availableMoves;

    m_referee->GetAvailableNextMoves( availableMoves );
    if ( availableMoves.empty() )
    {
        return invalidMove;
    }

    ::srand( ::time(NULL) );
    int someNumber = ::rand();
    someNumber %= availableMoves.size();
    
    const hoxMove nextMove = availableMoves[someNumber];
    return nextMove;
}

/************************* END OF FILE ***************************************/
