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
class hoxSite;

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
    hoxTable( hoxSite*          site,
              const wxString&   id,
              hoxIReferee*      referee,
              hoxBoard*         board = NULL );
    
    virtual ~hoxTable();

    /*********************************
     * My MAIN public API
     *********************************/

    const wxString GetId() const { return m_id; }

    /**
     * Assign a player to this Table.
     *
     * @note If the JOIN is successful, other Players in the Table will
     *       be informed about this event.
     *
     * @param player The Player that is requesting to join the Table.
     * @param assignedColor The assigned Color of the Player if 
     *                      the JOIN is allowed.
     */
    hoxResult AssignPlayer( hoxPlayer*     player,
                            hoxPieceColor& assignedColor );

    /**
     * Attempt to assign a player to this Table as a specified role.
     *
     * @note Currently, the Table will NOT inform other Player about this event.
     *
     * @param player The Player that is requesting to join the Table.
     * @param requestColor The requested Color the Player wants to join as.
     */
    hoxResult AssignPlayerAs( hoxPlayer*     player,
                              hoxPieceColor  requestColor );

    /**
     * Unseat a given player from this table.
     *
     * @param player The player to be unseated.
     * @param informer The Player that informs the Table about that face that
     *                 a player wants to be unseated.
     *                 If NULL, then the leaving player is also the informer.
     */
    hoxResult UnassignPlayer( hoxPlayer* player,
                              hoxPlayer* informer = NULL );

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
     * @param player The Player who generates the message.
     * @param message The message that are being sent from the network.
     */
    void OnMessage_FromNetwork( hoxPlayer*       player,
                                const wxString&  message );

    // TODO: We should delete the API.
    void OnMessage_FromNetwork( const wxString&  playerId,
                                const wxString&  message );

    /**
     * Callback function from the NETWORK player to let the Table know that
     * a player who just left the table.
     *
     * @param leavePlayer The Player that just left the table.
     * @param informer The Player that informs the Table about this event.
     */
    void OnLeave_FromNetwork( hoxPlayer* leavePlayer,
                              hoxPlayer* informer );

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

    hoxSite* GetSite() const { return m_site; }

private:
    hoxResult _ParseMoveString( const wxString& moveStr, hoxMove& move );

    /**
     * Post (inform) a player about the fact that this table is 
     * about to be closed.
     */
    void _PostPlayer_CloseEvent( hoxPlayer* player ) const;

    /**
     * Post (inform) a player that a Player just left the table.
     *
     * @param player      The Player to be informed.
     * @param leavePlayer The Player that just left the table.
     */
    void _PostPlayer_LeaveEvent( hoxPlayer*  player,
                                 hoxPlayer*  leavePlayer ) const;

    /**
     * Post (inform) a player that a new Player just joined the table.
     *
     * @param player    The Player to be informed.
     * @param newPlayer The Player that just joined the table.
     * @param newColor  The color (role) that the new Play will have.
     */
    void _PostPlayer_JoinEvent( hoxPlayer*    player,
                                hoxPlayer*    newPlayer,
                                hoxPieceColor newColor ) const;

    /**
     * Post (inform) a player that a new Move has just been made.
     *
     * @param player    The Player to be informed.
     * @param movePlayer The Player that just made the Move.
     * @param moveStr  The string containing the new Move.
     */
    void _PostPlayer_MoveEvent( hoxPlayer*      player,
                                hoxPlayer*      movePlayer,
                                const wxString& moveStr ) const;

    /**
     * Post (inform) a player that a new Message has just been sent.
     *
     * @param player    The Player to be informed.
     * @param msgPlayer The Player that just sent the Message.
     * @param message   The string containing the new Message.
     */
    void _PostPlayer_MessageEvent( hoxPlayer*      player,
                                   hoxPlayer*      msgPlayer,
                                   const wxString& message ) const;

    void _PostBoard_PlayerEvent( wxEventType commandType, 
                                 hoxPlayer*  player,
                                 int         extraCode = wxID_ANY ) const;

    void _PostBoard_MessageEvent( hoxPlayer*      player,
                                  const wxString& message ) const;

    /**
     * Inform other Players that a new Player just joined the Table.
     *
     * @param newPlayer The Player that just joined the table.
     * @param newColor  The color (role) that the new Play will have.
     */
    void _PostAll_JoinEvent( hoxPlayer*    newPlayer,
                             hoxPieceColor newColor ) const;

    /**
     * Inform other Players that a new Move was just made.
     *
     * @param player  The Player that just made the Move.
     * @param moveStr The string containing the new Move.
     */
    void _PostAll_MoveEvent( hoxPlayer*      player,
                             const wxString& moveStr ) const;

    /**
     * Inform other Players that a new Message was just sent.
     *
     * @param player  The Player that just sent the Message.
     * @param message The string containing the new Message.
     * @param fromBoard If true, then the Message is coming from the Board.
     *                  In this case, the Table should inform ALL players
     *                  currently at the Table (instead of skipping the
     *                  sender and skipping the Board-player.)
     */
    void _PostAll_MessageEvent( hoxPlayer*      player,
                                const wxString& message,
                                bool            fromBoard = false ) const;

    /**
     * Inform other Players that a Player just left the Table.
     *
     * @param player  The Player that just left the Table.
     * @param informer The Player that informs the Table about this event.
     *                 If NULL, then the leaving player is also the informer.
     */
    void _PostAll_LeaveEvent( hoxPlayer* player,
                              hoxPlayer* informer = NULL ) const;

    /**
     * The the player who has physically control of the Board.
     */
    hoxPlayer* _GetBoardPlayer() const;

    void       _AddPlayer( hoxPlayer* player, hoxPieceColor role );
    void       _RemovePlayer( hoxPlayer* player );
    hoxPlayer* _FindPlayer( const wxString& playerId );

private:
    hoxSite*         m_site;
    const wxString   m_id;       // The table's ID.

    hoxIReferee*     m_referee;  // The referee

    hoxBoard*        m_board;    // The (OPTIONAL) Board.

    // Players
    hoxPlayerAndRoleList  m_players;
    hoxPlayer*            m_redPlayer;
    hoxPlayer*            m_blackPlayer;
};

#endif /* __INCLUDED_HOX_TABLE_H_ */
