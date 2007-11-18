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
// Name:            hoxTable.h
// Created:         09/30/2007
//
// Description:     The Table that controls a single table with the following:
//                    + A referee
//                    + Players
//                    + A Board (maybe optional).
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_TABLE_H_
#define __INCLUDED_HOX_TABLE_H_

#include "wx/wx.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/* Forward declarations */
class hoxIReferee;
class hoxPlayer;
class hoxBoard;

/**
 * The Table with a board, a referee, and players.
 * Specifically, it contains the following components:
 *      - One hoxBoard (with all Pieces + One hoxIReferee).
 *      - One RED hoxPlayer.
 *      - One BLACK hoxPlayer.
 */
class hoxTable
{
public:
    hoxTable( const wxString&   id,
              hoxIReferee*      referee,
              hoxBoard*         board = NULL );
    
    virtual ~hoxTable();

    /*********************************
     * My MAIN public API
     *********************************/

    const wxString GetId() const { return m_id; }

    /**
     * Assign a player to this Table.
     */
    hoxResult AssignPlayer( hoxPlayer*     player,
                            hoxPieceColor& assignedColor );
    /* TODO: Why this API is different from AssignPlayer() */
    hoxResult RequestJoinFromPlayer( hoxPlayer*     player,
                                     hoxPieceColor  requestColor );

    hoxResult UnassignPlayer( hoxPlayer* player );

    hoxPlayer* GetRedPlayer() const { return m_redPlayer; }
    hoxPlayer* GetBlackPlayer() const { return m_blackPlayer; }

    /**
     * Set the Board (the Table's GUI).
     */
    void SetBoard( hoxBoard* board );

    hoxBoard* GetBoard() const { return m_board; }


    /**
     * Callback function from the Board to let this Table know about
     * physical (Board) Moves.
     *
     * @param move The current design assumes that the Board has contacted
     *             the referee to validate the Move.
     */
    void OnMove_FromBoard( const hoxMove& move );

    /**
     * Callback function from the Board to let this Table know about
     * new Wall-input messages.
     */
    void OnMessage_FromBoard( const wxString& message );

    /**
     * Callback function from the NETWORK Player to let this Table know about
     * the newly-received "remote" Moves.
     *
     * @param player The parameter is passed in so that the Table does not have
     *               to inform that player about this move (to avoid getting 
     *               into an endless loop).
     * @param fromPosition The current position of the Piece.
     * @param toPosition The new position of the Piece.
     */
    void OnMove_FromNetwork( hoxPlayer*         player,
                             const hoxPosition& fromPosition,
                             const hoxPosition& toPosition );
    void OnMove_FromNetwork( hoxPlayer*         player,
                             const wxString&    moveStr );

    /**
     * Callback function from the NETWORK Player to let this Table know about
     * the newly-received Wall-Message(s).
     *
     * @param playerId The Id of the player who generates the message.
     * @param message The message that are being sent from the network.
     */
    void OnMessage_FromNetwork( const wxString&  playerId,
                                const wxString&  message );

    /**
     * Callback function from a player who is leaving the table.
     */
    void OnLeave_FromPlayer( hoxPlayer* player );

    /**
     * Callback function from the (local) system to force this table
     * to close.
     */
    void OnClose_FromSystem();


    void ToggleViewSide();

private:
    hoxResult _ParseMoveString( const wxString& moveStr, hoxMove& move );

    /**
     * Post (inform) a player about the fact that this table is 
     * about to be closed.
     */
    void _PostPlayer_CloseEvent( hoxPlayer* player );

    void _PostBoard_PlayerEvent( wxEventType commandType, 
                                 hoxPlayer*  player,
                                 int         extraCode = wxID_ANY );

    void _PostBoard_MessageEvent( hoxPlayer*      player,
                                  const wxString& message );

    /**
     * The the player who has physically control of the Board.
     */
    hoxPlayer* _GetBoardPlayer();

    void       _AddPlayer( hoxPlayer* player, hoxPieceColor role );
    void       _RemovePlayer( hoxPlayer* player );
    hoxPlayer* _FindPlayer( const wxString& playerId );

private:
    const wxString   m_id;       // The table's ID.

    hoxIReferee*     m_referee;  // The referee

    hoxBoard*        m_board;    // The (OPTIONAL) Board.

    // Players
    hoxPlayerAndRoleList  m_players;
    hoxPlayer*            m_redPlayer;
    hoxPlayer*            m_blackPlayer;
    
};

#endif /* __INCLUDED_HOX_TABLE_H_ */
