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
// Name:            hoxTable.h
// Created:         09/30/2007
//
// Description:     The Table that controls a single table with the following:
//                    + A referee
//                    + Players
//                    + A Board (maybe optional).
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_TABLE_H__
#define __INCLUDED_HOX_TABLE_H__

#include "hoxTypes.h"

/* Forward declarations */
class hoxPlayer;
class hoxLocalPlayer;
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
              hoxIReferee_SPtr  referee,
              hoxBoard*         board = NULL );
    
    virtual ~hoxTable();

    /*********************************
     * My MAIN public API
     *********************************/

	/**
	 * Return the Table's Id.
	 */
    const wxString GetId() const { return m_id; }

    hoxIReferee_SPtr GetReferee() const { return m_referee; }

	/**
	 * Return the (optional) Board UI object.
     * If the Table does NOT yet have a UI, then the returned pointer is NULL.
	 */
    hoxBoard* GetBoardUI() const { return m_board; }

    /**
     * Attempt to assign a player to this Table as a specified role.
     *
     * @param player The Player that is requesting to join the Table.
     * @param requestColor The requested Color the Player wants to join as.
     */
    hoxResult AssignPlayerAs( hoxPlayer* player,
                              hoxColor   requestColor );

    hoxPlayer* GetRedPlayer() const { return m_redPlayer; }
    hoxPlayer* GetBlackPlayer() const { return m_blackPlayer; }

    /**
     * View the table's Board (the Table's GUI) by creating a new
	 * Board using the given 'tableWindow' parameter as the parent Window.
	 *
     */
    void ViewBoard( hoxBoard* pBoard );

	void SetGameType(const hoxGameType gameType) { m_gameType = gameType; }
	hoxGameType GetGameType() const { return m_gameType; }

	void SetInitialTime(const hoxTimeInfo& timeInfo) { m_initialTime = timeInfo; }
	const hoxTimeInfo GetInitialTime() const { return m_initialTime; }

	void SetBlackTime(const hoxTimeInfo& timeInfo) { m_blackTime = timeInfo; }
	const hoxTimeInfo GetBlackTime() const { return m_blackTime; }

	void SetRedTime(const hoxTimeInfo& timeInfo) { m_redTime = timeInfo; }
	const hoxTimeInfo GetRedTime() const { return m_redTime; }

    /**
     * Callback function from the Board to let this Table know about
     * physical (Board) Moves.
     *
     * @param move The current design assumes that the Board has contacted
     *             the referee to validate the Move.
	 * @param status The game's status.
	 * @param playerTime The time of the Player that made the Move.
     */
    virtual void OnMove_FromBoard( const hoxMove&     move,
		                           hoxGameStatus      status,
					     	       const hoxTimeInfo& playerTime );

    /**
     * Callback function from the Board to let this Table know about
     * new Wall-input messages.
     */
    void OnMessage_FromBoard( const wxString& message );

    /**
     * Callback function from the Board to let this Table know about
     * JOIN-command button that has been pressed by the "local" player.
     *
     * @param requestColor The requested Color (or Role).
     */
    void OnJoinCommand_FromBoard( const hoxColor requestColor );

    /**
     * Callback function from the Board to let this Table know about
     * OPTIONS-command button that has been pressed by the "local" player.
     */
    void OnOptionsCommand_FromBoard( const bool         bRatedGame,
                                     const hoxTimeInfo& newTimeInfo );

    /**
     * Callback function from the Board to let this Table know about
     * RESIGN-command button that has been pressed by the "local" player.
     */
    void OnResignCommand_FromBoard();

    /**
     * Callback function from the Board to let this Table know about
     * DRAW-command button that has been pressed by the "local" player.
     */
    void OnDrawCommand_FromBoard();

    /**
     * Callback function from the Board to let this Table know about
     * RESET-command button that has been pressed by the "local" player.
     */
    void OnResetCommand_FromBoard();

    /**
     * Callback function from the Board to let this Table know about
     * the response to a DRAW-request.
     */
    void OnDrawResponse_FromBoard( bool bAcceptDraw );

    /**
     * Callback function from the Board to let this Table know about
     * the Player-Info Request.
     */
    void OnPlayerInfoRequest_FromBoard( const wxString& sPlayerId );

    /**
     * Callback function from the Board to let this Table know about
     * the Player-Invite Request.
     */
    void OnPlayerInviteRequest_FromBoard( const wxString& sPlayerId );

    /**
     * Callback function from the NETWORK Player to let this Table know about
     * the newly-received "remote" Moves.
     *
     * @param player Currently, this player can be the one that made the Move
	 *               OR can be the Player that informs about the Move.
	 *               TODO: We need to look at this 'dual-role' issue.
	 *
     * @param moveStr The string containing the Move.
     */
    void OnMove_FromNetwork( hoxPlayer*       player,
                             const wxString&  moveStr );

    /**
     * Callback function from the NETWORK Player to let this Table know about
     * the "past/history" Moves.
     *
     * @param player The player who informs about the Move.
     * @param moves The list of past Moves.
     */
    void OnPastMoves_FromNetwork( hoxPlayer*           player,
                                  const hoxStringList& moves );

    /**
     * Callback function from the NETWORK Player to let this Table know about
     * the newly-received Wall-Message(s).
     *
     * @param sSenderId The Id of the Player who generates the message.
     * @param message The message that are being sent from the network.
     * @param bPublic Whether this is a public message
     *                .. or a private/instant one.
     */
    void OnMessage_FromNetwork( const wxString&  sSenderId,
                                const wxString&  message,
                                bool             bPublic = true );

    /**
     * Callback function from the NETWORK Player to let this Table know about
     * the newly-received System-Message(s).
     *
     * @param message The message that are being sent from the network.
     */
    void OnSystemMsg_FromNetwork( const wxString&  message );

    /**
     * Callback function from the NETWORK player to let the Table know that
     * a player who just left the table.
     *
     * @param leavePlayer The Player that just left the table.
     */
    void OnLeave_FromNetwork( hoxPlayer* leavePlayer );

    /**
     * Callback function from the NETWORK Player to let this Table know about
     * the newly-received Draw request.
     *
     * @param fromPlayer The Player who generated the request.
     */
    void OnDrawRequest_FromNetwork( hoxPlayer* fromPlayer );

    /**
     * Callback function from the NETWORK Player to let this Table know about
     * the GameOver event.
     *
     * @param player The Player who informs the event.
     * @param gameStatus The game's status.
     */
    void OnGameOver_FromNetwork( hoxPlayer*    player,
                                 hoxGameStatus gameStatus );

    /**
     * Callback function from the NETWORK Player to let this Table know about
     * the newly-received Reset request.
     */
    void OnGameReset_FromNetwork();

    /**
     * Callback function from the NETWORK Player to let this Table know about
     * a player's new Score.
     *
     * @param player The Player who has the new score.
     */
    void OnScore_FromNetwork( hoxPlayer* player );

    /**
     * Callback function from a player who is leaving the table.
     */
    void OnLeave_FromPlayer( hoxPlayer* player );

    /**
     * Callback function from a player who just updated the Options (Table).
     *
     * @param player The Player who just updated the Options.
     * @param gameType The Game-Type option.
     * @param newTimeInfo The new initial Time-Info.
     */
    void OnUpdate_FromPlayer( hoxPlayer*         player,
                              const hoxGameType  gameType,
                              const hoxTimeInfo& newTimeInfo );

    /**
     * Callback function from the (local) system to force this table
     * to close.
     */
    void OnClose_FromSystem();

    /**
     * Post a Board message (without an Owner).
     *
     * @param message The message that are being sent.
     */
    void PostBoardMessage( const wxString&  message );

    void ToggleViewSide();

    hoxSite* GetSite() const { return m_site; }

    hoxColor GetPlayerRole( const wxString& sPlayerId ) const;

protected:
    /**
     * Post (inform) a player that a new Move has just been made.
     *
     * @param player    The Player to be informed.
     * @param moveStr  The string containing the new Move.
	 * @param status  The game's status.
     * @param playerTime The remaining Player's Time.
     */
    void PostPlayer_MoveEvent( hoxPlayer*         player,
                               const wxString&    moveStr,
							   hoxGameStatus      status = hoxGAME_STATUS_IN_PROGRESS,
							   const hoxTimeInfo& playerTime = hoxTimeInfo() ) const;

private:
    /**
     * Close the GUI Board.
     *
     * @note This function is required instead of calling "delete" to
     *       avoid memory leak issues.
     */
    void _CloseBoard();

    void _PostBoard_PlayerEvent( wxEventType commandType, 
                                 hoxPlayer*  player,
                                 int         extraCode = wxID_ANY ) const;

    /**
     * Post a message to the Wall (or Chat) Area.
     *
     * @param sMessage The string containing the new Message.
     * @param sSenderId The sender ID.
     * @param bPublic Whether this is a public message
     *                .. or a private/instant one.
     */
    void _PostBoard_MessageEvent( const wxString& sMessage,
                                  const wxString& sSenderId = _T("*Table*"),
                                  bool            bPublic = true ) const;

    void _PostBoard_SystemMsgEvent( const wxString& sMessage ) const;

    void _PostBoard_MoveEvent( const wxString& moveStr,
		                       bool            bSetupMode = false ) const;

    void _PostBoard_DrawRequestEvent( hoxPlayer* fromPlayer,
                                      bool       bPopupRequest ) const;

    void _PostBoard_GameOverEvent( const hoxGameStatus gameStatus ) const;

    void _PostBoard_GameResetEvent() const;

    void _PostBoard_ScoreEvent( hoxPlayer*  player ) const;

    void _PostBoard_UpdateEvent( hoxPlayer* player ) const;

    void       _AddPlayer( hoxPlayer* player, hoxColor role );
    void       _RemovePlayer( hoxPlayer* player );
    hoxPlayer* _FindPlayer( const wxString& playerId ) const;

    void       _ResetGame();

protected:
    hoxSite*         m_site;
    const wxString   m_id;       // The table's ID.

    hoxIReferee_SPtr m_referee;  // The referee

    hoxBoard*        m_board;    // The (OPTIONAL) Board.

    // Players
    hoxPlayerSet     m_players;  // Players + Observers
    hoxPlayer*       m_redPlayer;
    hoxPlayer*       m_blackPlayer;

    hoxLocalPlayer*  m_boardPlayer;
            /* The player who has physical control of the Board. */

    // Rated / Unrated / Solo / Practice
    hoxGameType      m_gameType;

	// Timers
	hoxTimeInfo      m_initialTime; // *** Initial time.
    hoxTimeInfo      m_blackTime;   // Black's time.
	hoxTimeInfo      m_redTime;     // Red's time.
};


/**
 * A PRACTICE Table in which the local player is playing with his computer.
 *
 */
class hoxPracticeTable : public hoxTable
{
public:
    hoxPracticeTable( hoxSite*          site,
                      const wxString&   id,
                      hoxIReferee_SPtr  referee,
                      hoxBoard*         board = NULL );
    
    virtual ~hoxPracticeTable();

    /**
     * Callback function from the Board to let this Table know about
     * physical (Board) Moves.
     *
     * @param move The current design assumes that the Board has contacted
     *             the referee to validate the Move.
	 * @param status The game's status.
	 * @param playerTime The time of the Player that made the Move.
     */
    virtual void OnMove_FromBoard( const hoxMove&     move,
		                           hoxGameStatus      status,
					     	       const hoxTimeInfo& playerTime );
};

#endif /* __INCLUDED_HOX_TABLE_H__ */
