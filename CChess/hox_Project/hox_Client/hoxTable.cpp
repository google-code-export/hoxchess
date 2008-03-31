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
// Name:            hoxTable.cpp
// Created:         09/30/2007
//
// Description:     The Table.
/////////////////////////////////////////////////////////////////////////////

#include "hoxTable.h"
#include "hoxReferee.h"
#include "hoxPlayer.h"
#include "hoxBoard.h"
#include "hoxSite.h"
#include "hoxUtil.h"
#include <algorithm>

// ----------------------------------------------------------------------------
// hoxTable
// ----------------------------------------------------------------------------

hoxTable::hoxTable( hoxSite*         site,
                    const wxString&  id,
                    hoxIReferee_SPtr referee,
                    hoxBoard*        board /* = NULL */ )
        : m_site( site )
        , m_id( id )
        , m_referee( referee )
        , m_board( board )
        , m_redPlayer( NULL )
        , m_blackPlayer( NULL )
{
    const char* FNAME = "hoxTable::hoxTable";
    wxLogDebug("%s: ENTER.", FNAME);

	m_blackTime.nGame = hoxTIME_DEFAULT_GAME_TIME;
	m_redTime.nGame   = hoxTIME_DEFAULT_GAME_TIME;
}

hoxTable::~hoxTable()
{
    const char* FNAME = "hoxTable::~hoxTable";
    wxLogDebug("%s: ENTER.", FNAME);

    _CloseBoard();   // Close GUI Board.
}

hoxResult 
hoxTable::AssignPlayer( hoxPlayer*     player,
                        hoxColor& assignedColor,
                        bool           informOthers /* = true */)
{
    hoxResult result = hoxRC_OK;

    wxCHECK_MSG( player != NULL, hoxRC_ERR, "The player is NULL." );

    assignedColor = hoxCOLOR_NONE; // Default: Observer's Role.

    /* Assign to play RED if possible. */
    if ( m_redPlayer == NULL )
    {
        assignedColor = hoxCOLOR_RED;
    }
    /* Assign to play BLACK if possible. */
    else if ( m_blackPlayer == NULL )
    {
        assignedColor = hoxCOLOR_BLACK;
    }
    /* Default: ... The player will join as an Observer. */

    /* Update our player-list */
    _AddPlayer( player, assignedColor );

    /* Inform other players about the new Player */
    if ( informOthers )
    {
        _PostAll_JoinEvent( player, assignedColor );
    }

    return hoxRC_OK;
}

hoxResult 
hoxTable::AssignPlayerAs( hoxPlayer*     player,
                          hoxColor  requestColor )
{
    const char* FNAME = "hoxTable::AssignPlayerAs";

    wxCHECK_MSG( player != NULL, hoxRC_ERR, "The player is NULL." );

    bool bRequestOK =
           ( requestColor == hoxCOLOR_RED   && m_redPlayer == NULL )
        || ( requestColor == hoxCOLOR_BLACK && m_blackPlayer == NULL )
        || ( requestColor == hoxCOLOR_NONE );

    if ( ! bRequestOK )
    {
        wxLogDebug("%s: *** WARN *** Failed to handle request-to-join from [%s].", 
            FNAME, player->GetName().c_str());
        return hoxRC_ERR;
    }

    /* Update our player-list */
    _AddPlayer( player, requestColor );

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * NOTE: Unlike the other call, we will NOT inform other players about 
     * the new Player 
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    return hoxRC_OK;
}

hoxResult 
hoxTable::UnassignPlayer( hoxPlayer* player,
                          hoxPlayer* informer /* = NULL */ )
{
    const char* FNAME = "hoxTable::UnassignPlayer";

    wxLogDebug("%s: ENTER.", FNAME);

    wxCHECK_MSG( player, hoxRC_ERR, "The player is NULL." );

    /* Inform other players about this event. 
     * NOTE: This should be done BEFORE the player is removed
     *       from the internal player-list.
     */
    _PostAll_LeaveEvent( player, informer );

    /* Update our player-list */
    _RemovePlayer( player );

    return hoxRC_OK;
}

void 
hoxTable::ViewBoard( hoxBoard* pBoard )
{
	const char* FNAME = "hoxTable::ViewBoard";

	wxCHECK_RET(m_board == NULL, "The Board has already been set.");

    m_board = pBoard;
    m_board->Show( true );  // Display Board's GUI.
}

void 
hoxTable::OnMove_FromBoard( const hoxMove&     move,
						    hoxGameStatus      status,
							const hoxTimeInfo& playerTime )
{
    const char* FNAME = "hoxTable::OnMove_FromBoard";

    wxLogDebug("%s: Received new Move from Board.", FNAME);
    if ( m_redPlayer == NULL || m_blackPlayer == NULL )
    {
        const wxString msg = "Not enough players. Ignore Move.";
        wxLogDebug("%s: *** WARNING *** %s", FNAME, msg.c_str());
        _PostBoard_MessageEvent( NULL /* System */, msg );
        return;
    }

    /* Get the Board Player (or the Board's owner) because he is the
     * one that sent the Move.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");

    /* Inform all players (including the Board's owner about the new Move */
	_PostAll_MoveEvent( boardPlayer, 
		                move.ToString(), 
						false, /* coming from the Board, not network */
						status,
						playerTime );
}

void
hoxTable::OnMessage_FromBoard( const wxString& message )
{
    const char* FNAME = "hoxTable::OnMessage_FromBoard";
    wxLogDebug("%s: Receive new Message from Board.", FNAME);

    /* Get the Board Player (or the Board's owner) because he is the
     * one that sent the Message.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");
    
    /* Post the message on the Wall-Output of the "local" Board. */
    _PostBoard_MessageEvent( boardPlayer, message );

    /* Post the message to other players in the Table. 
     * NOTE: We also indicate that this Message is coming from the physical
     *       Board so that the Table will send notifications to ALL players
     *       instead of skipping the sender + the Board player.
     */
    _PostAll_MessageEvent( boardPlayer, message, true /* fromBoard */ );
}

void
hoxTable::OnJoinCommand_FromBoard()
{
    const char* FNAME = "hoxTable::OnJoinCommand_FromBoard";
	wxLogDebug("%s: Received a JOIN request from Board's local-player.", FNAME);

    /* Get the Board Player (or the Board's owner) because he is the
     * one that sent the Message.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");

	/* Auto JOIN the table based on the seat availability. */

	hoxColor requestColor = hoxCOLOR_NONE; // Default: observer

	if      ( m_redPlayer   == NULL ) requestColor = hoxCOLOR_RED;
	else if ( m_blackPlayer == NULL ) requestColor = hoxCOLOR_BLACK;
    else                              requestColor = hoxCOLOR_NONE;

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_JOIN ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetName();
	apRequest->parameters["color"] = hoxUtil::ColorToString( requestColor );
	apRequest->parameters["joined"] = "1";

    boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnOptionsCommand_FromBoard( const hoxTimeInfo& newTimeInfo )
{
    const char* FNAME = "hoxTable::OnOptionsCommand_FromBoard";
	wxLogDebug("%s: ENTER.", FNAME);

    /* Get the Board Player (or the Board's owner) because he is the
     * one that issued the command.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");

	/* Make sure the board Player satifies one of the following conditions:
	 *  (1) He is the RED player, or...
     *  (2) He is the BLACK player and there is no RED player.
	 */

	bool bActionAllowed = 
        (     boardPlayer == m_redPlayer 
		  || (boardPlayer == m_blackPlayer && m_redPlayer == NULL) );

    if ( ! bActionAllowed )
	{
		wxLogWarning("Player [%s] is not allowed to change Options.", 
            boardPlayer->GetName().c_str());
		return;
	}

    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_UPDATE ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetName();
    apRequest->parameters["itimes"] = hoxUtil::TimeInfoToString( newTimeInfo );

    boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnResignCommand_FromBoard()
{
    const char* FNAME = "hoxTable::OnResignCommand_FromBoard";
	wxLogDebug("%s: Received a RESIGN request from Board's local-player.", FNAME);

    /* Get the Board Player (or the Board's owner) because he is the
     * one that sent the Message.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");

	/* Make sure the board Player is actually playing. 
	 * If not, ignore the request.
	 */

	if (   boardPlayer != m_redPlayer 
		&& boardPlayer != m_blackPlayer )
	{
		wxLogWarning("The Player [%s] is not playing.", boardPlayer->GetName().c_str());
		return;
	}

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_RESIGN ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetName();

    boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnDrawCommand_FromBoard()
{
    const char* FNAME = "hoxTable::OnDrawCommand_FromBoard";
	wxLogDebug("%s: Received a DRAW request from Board's local-player.", FNAME);

    /* Get the Board Player (or the Board's owner) because he is the
     * one that sent the Message.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");

	/* Make sure the board Player is actually playing. 
	 * If not, ignore the request.
	 */

	if (   boardPlayer != m_redPlayer 
		&& boardPlayer != m_blackPlayer )
	{
		wxLogWarning("The Player [%s] is not playing.", boardPlayer->GetName().c_str());
		return;
	}

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_DRAW ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetName();
	apRequest->parameters["draw_response"] = "";

    boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnResetCommand_FromBoard()
{
    const char* FNAME = "hoxTable::OnResetCommand_FromBoard";
	wxLogDebug("%s: Received a RESET request from Board's local-player.", FNAME);

    /* Get the Board Player (or the Board's owner) because he is the
     * one that sent the Message.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");

	/* Make sure the board Player is actually playing. 
	 * If not, ignore the request.
	 */

	if (   boardPlayer != m_redPlayer 
		&& boardPlayer != m_blackPlayer )
	{
		wxLogWarning("The Player [%s] is not playing.", boardPlayer->GetName().c_str());
		return;
	}

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_RESET ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetName();

    boardPlayer->OnRequest_FromTable( apRequest );
}

void 
hoxTable::OnDrawResponse_FromBoard( bool bAcceptDraw )
{
    const char* FNAME = "hoxTable::OnDrawResponse_FromBoard";
	wxLogDebug("%s: Received a DRAW response [%d] from Board's local-player.", FNAME, bAcceptDraw);

    /* Get the Board Player (or the Board's owner) because he is the
     * one that sent the Message.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");

	/* Make sure the board Player is actually playing. 
	 * If not, ignore the request.
	 */

	if (   boardPlayer != m_redPlayer 
		&& boardPlayer != m_blackPlayer )
	{
		wxLogWarning("The Player [%s] is not playing.", boardPlayer->GetName().c_str());
		return;
	}

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_DRAW ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetName();
	apRequest->parameters["draw_response"] = (bAcceptDraw ? "1" : "0");

    boardPlayer->OnRequest_FromTable( apRequest );
}

void 
hoxTable::OnMove_FromNetwork( hoxPlayer*       player,
                              const wxString&  moveStr )
{
    /* Inform the Board about this Move. */
	_PostBoard_MoveEvent( moveStr );

    /* Inform other players about the new Player */
	_PostAll_MoveEvent( player, 
						moveStr,
						true /* coming from the network, not from Board */ );
}

void
hoxTable::OnPastMoves_FromNetwork( hoxPlayer*           player,
                                   const hoxStringList& moves )
{
    /* Inform the Board about this event. */
    for ( hoxStringList::const_iterator it = moves.begin();
                                        it != moves.end(); ++it )
    {
        _PostBoard_MoveEvent( (*it), true /* in SETUP-MODE */ );
    }
}

void 
hoxTable::OnMessage_FromNetwork( hoxPlayer*       player,
                                 const wxString&  message )
{
    const char* FNAME = "hoxTable::OnMessage_FromNetwork";

    wxLogDebug("%s: ENTER.", FNAME);

    /* Post the message (from the player) to the Board. */
    _PostBoard_MessageEvent( player, message );

    /* Post the message to other players in the Table. */
    _PostAll_MessageEvent( player, message );
}

void 
hoxTable::OnMessage_FromNetwork( const wxString&  playerId,
                                 const wxString&  message )
{
    const char* FNAME = "hoxTable::OnMessage_FromNetwork";

    wxLogDebug("%s: ENTER (by playerId).", FNAME);

    /* Lookup the player */
    hoxPlayer* foundPlayer = _FindPlayer( playerId );
    if ( foundPlayer == NULL )
    {
        wxLogDebug("%s: *** WARN *** Player with Id = [%s] not found.", FNAME, playerId.c_str());
        return;
    }

    this->OnMessage_FromNetwork( foundPlayer, message );
}

void 
hoxTable::OnLeave_FromNetwork( hoxPlayer* leavePlayer,
                               hoxPlayer* informer )
{
    const char* FNAME = "hoxTable::OnLeave_FromNetwork";

    wxLogDebug("%s: [%s] informing that [%s] just left the table.", 
        FNAME, informer->GetName().c_str(), leavePlayer->GetName().c_str());

    /* This step is not actually necessary but it is nice 
     * to the debugger. 
     */
    hoxPlayer* actualInformer = 
        ( leavePlayer == informer ? NULL : informer ); 

    this->UnassignPlayer( leavePlayer, actualInformer );
}

void 
hoxTable::OnDrawRequest_FromNetwork( hoxPlayer* fromPlayer )
{
    /* Find out to whom this request is targeting to.
     * If this player is the Board's owner, then popup the request.
     */
    hoxPlayer* toPlayer = ( fromPlayer == m_redPlayer
                           ? m_blackPlayer
                           : m_redPlayer );
    wxCHECK_RET(toPlayer, "There is no TO-player for Draw-request");

    bool bPopupRequest = ( toPlayer->GetType() == hoxPLAYER_TYPE_LOCAL );

    /* Inform the Board about this request. */
	_PostBoard_DrawRequestEvent( fromPlayer, bPopupRequest );
}

void 
hoxTable::OnGameOver_FromNetwork( hoxPlayer*    player,
                                  hoxGameStatus gameStatus )
{
    /* Inform the Board about the status. */
	_PostBoard_GameOverEvent( /*player,*/ gameStatus );
}

void 
hoxTable::OnGameReset_FromNetwork()
{
    const char* FNAME = "hoxTable::OnGameReset_FromNetwork";
    wxLogDebug("%s: ENTER.", FNAME);

    _ResetGame();

    /* Inform the Board about the status. */
	_PostBoard_GameResetEvent();
}

void 
hoxTable::OnLeave_FromPlayer( hoxPlayer* player )
{
    const char* FNAME = "hoxTable::OnLeave_FromPlayer";
    wxLogDebug("%s: ENTER.", FNAME);

    this->UnassignPlayer( player );
}

void
hoxTable::OnUpdate_FromPlayer( hoxPlayer*         player,
                               const hoxTimeInfo& newTimeInfo )
{
    m_initialTime = newTimeInfo;
    m_redTime     = m_initialTime;
    m_blackTime   = m_initialTime;

    _PostBoard_UpdateEvent( player, m_initialTime );
}

void
hoxTable::OnScore_FromNetwork( hoxPlayer* player )
{
    const char* FNAME = "hoxTable::OnScore_FromNetwork";
    wxLogDebug("%s: ENTER.", FNAME);

    _PostBoard_ScoreEvent( player );
}

void 
hoxTable::OnClose_FromSystem()
{
    const char* FNAME = "hoxTable::OnClose_FromSystem";
    hoxPlayer* player = NULL;

    wxLogDebug("%s: ENTER. [%s].", FNAME, m_id.c_str());

    while ( ! m_players.empty() )
    {
        player = m_players.front();

        _PostPlayer_CloseEvent( player );
        _RemovePlayer( player );
    }

    _CloseBoard();   // Close GUI Board.
}

void
hoxTable::PostSystemMessage( const wxString&  message )
{
    _PostBoard_MessageEvent( NULL /* System */, message );
}

void 
hoxTable::ToggleViewSide()
{
	wxCHECK_RET(m_board, "The Board is not yet set.");
    m_board->ToggleViewSide();
}

void
hoxTable::_CloseBoard()
{
    const char* FNAME = "hoxTable::_CloseBoard";

    if ( m_board != NULL )
    {
        wxLogDebug("%s: ENTER. Table-Id = [%s].", FNAME, m_id.c_str());
        
        m_board->Close();
            /* NOTE: This has to be used instead of "delete" or "Destroy()"
             *       function to avoid memory leaks.
             *       For example, "LEAVE" event would not be processed...
             *
             * See http://docs.wxwidgets.org/stable/wx_windowdeletionoverview.html#windowdeletionoverview
             */
        
        m_board = NULL;
    }
}

void 
hoxTable::_PostPlayer_CloseEvent( hoxPlayer* player ) const
{
    const char* FNAME = "hoxTable::_PostPlayer_CloseEvent";

    wxCHECK_RET( player, "The player is NULL." );

    wxLogDebug("%s: Informing player [%s] about the Table [%s] being closed...", 
        FNAME, player->GetName().c_str(), m_id.c_str());
    player->OnClose_FromTable( m_id );
}

void 
hoxTable::_PostPlayer_LeaveEvent( hoxPlayer* player,
                                  hoxPlayer* leavePlayer ) const
{
    const char* FNAME = "hoxTable::_PostPlayer_LeaveEvent";

    wxCHECK_RET( player, "The player is NULL." );
    wxCHECK_RET( leavePlayer, "The 'leave' player is NULL." );

    wxLogDebug("%s: Informing player [%s] about [%s] just left...", 
        FNAME, player->GetName().c_str(), leavePlayer->GetName().c_str());

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_LEAVE ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = leavePlayer->GetName();

    player->OnRequest_FromTable( apRequest );
}

void 
hoxTable::_PostPlayer_JoinEvent( hoxPlayer*    player,
                                 hoxPlayer*    newPlayer,
                                 hoxColor newColor ) const
{
    const char* FNAME = "hoxTable::_PostPlayer_JoinEvent";

    wxCHECK_RET( player, "The Player is NULL." );
    wxCHECK_RET( newPlayer, "The new Player is NULL." );

    wxLogDebug("%s: Informing player [%s] that a new Player [%s] just joined as [%d]...", 
        FNAME, player->GetName().c_str(), newPlayer->GetName().c_str(), newColor);

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_E_JOIN ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = newPlayer->GetName();
	apRequest->parameters["color"] = wxString::Format("%d", newColor);

    player->OnRequest_FromTable( apRequest );
}

void 
hoxTable::_PostPlayer_MoveEvent( hoxPlayer*         player,
                                 hoxPlayer*         movePlayer,
                                 const wxString&    moveStr,
								 hoxGameStatus      status /* = hoxGAME_STATUS_IN_PROGRESS */,
								 const hoxTimeInfo& playerTime /* = hoxTimeInfo() */ ) const
{
    const char* FNAME = "hoxTable::_PostPlayer_MoveEvent";

    wxCHECK_RET( player, "The Player is NULL." );
    wxCHECK_RET( movePlayer, "The 'move' Player is NULL." );

	/* Convert game's status into a string. */
    const wxString statusStr = hoxUtil::GameStatusToString( status );

    wxLogDebug("%s: Informing player [%s] that [%s] just made a new Move [%s]...", 
        FNAME, player->GetName().c_str(), movePlayer->GetName().c_str(), moveStr.c_str());
	
	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_MOVE ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = movePlayer->GetName();
	apRequest->parameters["move"] = moveStr;
	apRequest->parameters["status"] = statusStr;
	apRequest->parameters["game_time"] = wxString::Format("%d", playerTime.nGame);

    player->OnRequest_FromTable( apRequest );
}

void 
hoxTable::_PostPlayer_MessageEvent( hoxPlayer*      player,
                                    hoxPlayer*      msgPlayer,
                                    const wxString& message ) const
{
    const char* FNAME = "hoxTable::_PostPlayer_MessageEvent";

    wxCHECK_RET( player, "The Player is NULL." );
    wxCHECK_RET( msgPlayer, "The 'message' Player is NULL." );

    wxLogDebug("%s: Informing player [%s] that [%s] just sent a Message [%s]...", 
        FNAME, player->GetName().c_str(), msgPlayer->GetName().c_str(), message.c_str());

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_MSG ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = msgPlayer->GetName();
	apRequest->parameters["msg"] = message;
    
    player->OnRequest_FromTable( apRequest );
}

void 
hoxTable::_PostBoard_PlayerEvent( wxEventType commandType,
                                  hoxPlayer*  player,
                                  int         extraCode /* = wxID_ANY */ ) const
{
	if ( m_board == NULL )
		return;

    wxCommandEvent event( commandType );
    hoxPlayerInfo_APtr apPlayerInfo( player->GetPlayerInfo() );
    event.SetEventObject( apPlayerInfo.release() );
    event.SetInt( extraCode );
    wxPostEvent( m_board , event );
}

void 
hoxTable::_PostBoard_MessageEvent( hoxPlayer*      player,
                                   const wxString& message ) const
{
	if ( m_board == NULL )
		return;

    wxString who;
    wxString eventString;

    who = (   player != NULL 
            ? player->GetName() 
            : "*Table*" );
    eventString.Printf("%s %s", who.c_str(), message.c_str());

    /* Post the message on the Wall-Output of the "local" Board. */
    wxCommandEvent event( hoxEVT_BOARD_WALL_OUTPUT );
    event.SetString( eventString );
    wxPostEvent( m_board, event );
}

void 
hoxTable::_PostBoard_MoveEvent( const wxString& moveStr,
							    bool            bSetupMode /* = false */ ) const
{
	if ( m_board == NULL )
		return;

    wxCommandEvent event( hoxEVT_BOARD_NEW_MOVE );
    event.SetString( moveStr );
	event.SetInt( bSetupMode ? 1 : 0 );
    wxPostEvent( m_board, event );
}

void 
hoxTable::_PostBoard_DrawRequestEvent( hoxPlayer* fromPlayer,
                                       bool       bPopupRequest) const
{
	if ( m_board == NULL )
		return;

    wxCommandEvent event( hoxEVT_BOARD_DRAW_REQUEST );
    event.SetString( fromPlayer->GetName() );
    event.SetInt( (int) bPopupRequest );
    wxPostEvent( m_board, event );
}

void 
hoxTable::_PostBoard_GameOverEvent( const hoxGameStatus gameStatus ) const
{
	if ( m_board == NULL )
		return;

    wxCommandEvent event( hoxEVT_BOARD_GAME_OVER );
    event.SetInt( (int) gameStatus );
    wxPostEvent( m_board, event );
}

void 
hoxTable::_PostBoard_GameResetEvent() const
{
	if ( m_board == NULL )
		return;

    wxCommandEvent event( hoxEVT_BOARD_GAME_RESET );
    wxPostEvent( m_board, event );
}

void 
hoxTable::_PostBoard_ScoreEvent( hoxPlayer*  player ) const
{
	if ( m_board == NULL )
		return;

    wxCommandEvent event( hoxEVT_BOARD_PLAYER_SCORE );
    hoxPlayerInfo_APtr apPlayerInfo( player->GetPlayerInfo() );
    event.SetEventObject( apPlayerInfo.release() );
    wxPostEvent( m_board, event );
}

void
hoxTable::_PostBoard_UpdateEvent( hoxPlayer*         player,
                                  const hoxTimeInfo& newTimeInfo ) const
{
	if ( m_board == NULL )
		return;

    wxCommandEvent event( hoxEVT_BOARD_TABLE_UPDATE );
    event.SetString( hoxUtil::TimeInfoToString( newTimeInfo ) );
    wxPostEvent( m_board, event );
}

void 
hoxTable::_PostAll_JoinEvent( hoxPlayer*    newPlayer,
                              hoxColor newColor ) const
{
    const char* FNAME = "hoxTable::_PostAll_JoinEvent";

    wxLogDebug("%s: ENTER.", FNAME);

    for (hoxPlayerList::const_iterator it = m_players.begin(); 
                                       it != m_players.end(); ++it)
    {
        if ( (*it) == newPlayer )
        {
            wxLogDebug("%s: Skip this Player since he is the new Player.", FNAME);
            continue;
        }

        _PostPlayer_JoinEvent( (*it), newPlayer, newColor );
    }
}

void 
hoxTable::_PostAll_MoveEvent( hoxPlayer*         player,
                              const wxString&    moveStr,
							  bool               fromNetwork,
							  hoxGameStatus      status /* = hoxGAME_STATUS_IN_PROGRESS */,
							  const hoxTimeInfo& playerTime /* = hoxTimeInfo() */ ) const
{
    const char* FNAME = "hoxTable::_PostAll_MoveEvent";

    /* Get the Board Player (or the Board's owner).
     * Note: Currently, the Board player can be NULL because this Table
     *       may be hosted on a remote server and the two main players
     *       are all 'remote' players.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();

    for (hoxPlayerList::const_iterator it = m_players.begin(); 
                                       it != m_players.end(); ++it)
    {
		/* Skip the sender and the Board-player if the Move is coming 
		 * from the network. 
		 */
		if ( fromNetwork )
		{
			if ( (*it) == player )
			{
				//wxLogDebug("%s: Skip this Player [%s] since he just made this new Move.", 
				//    FNAME, player->GetName().c_str());
				continue;
			}

			if (   boardPlayer != NULL 
				&& (*it) == boardPlayer )
			{
				//wxLogDebug("%s: Skip this Player [%s] since he is Board player.", 
				//    FNAME, boardPlayer->GetName().c_str() );
				continue;
			}
		}

		/* Skip dummy player.
		 * Although this is not necesary but I do it here to avoid
		 * seeing a lot of debug messages.
		 */
		if ( (*it)->GetType() == hoxPLAYER_TYPE_DUMMY )
		{
            //wxLogDebug("%s: Skip this Player [%s] since he is a DUMMY player.", 
            //    FNAME, it->player->GetName().c_str() );
			continue;
		}

        _PostPlayer_MoveEvent( (*it), player, moveStr, status, playerTime );
    }
}

void 
hoxTable::_PostAll_MessageEvent( hoxPlayer*      player,
                                 const wxString& message,
                                 bool            fromBoard /* =  false */) const
{
    const char* FNAME = "hoxTable::_PostAll_MessageEvent";

    /* Get the Board Player (or the Board's owner).
     * Note: Currently, the Board player can be NULL because this Table
     *       may be hosted on a remote server and the two main players
     *       are all 'remote' players.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    
    if ( fromBoard ) // Message coming from the physical Board?
    {
        wxCHECK_RET( boardPlayer != NULL && boardPlayer == player,
                     "The sender should be the Board player.");
    }

    for (hoxPlayerList::const_iterator it = m_players.begin(); 
                                       it != m_players.end(); ++it)
    {
        if ( ! fromBoard )
        {
            if ( (*it) == player )
            {
                wxLogDebug("%s: Skip this Player [%s] since he just sent this new Message.", 
                    FNAME, player->GetName().c_str());
                continue;
            }

            if (   boardPlayer != NULL 
                && (*it) == boardPlayer )
            {
                wxLogDebug("%s: Skip this Player [%s] since he is Board player.", 
                    FNAME, boardPlayer->GetName().c_str() );
                continue;
            }
        }

        _PostPlayer_MessageEvent( (*it), player, message );
    }
}

void 
hoxTable::_PostAll_LeaveEvent( hoxPlayer* player,
                               hoxPlayer* informer /* = NULL */ ) const
{
    const char* FNAME = "hoxTable::_PostAll_LeaveEvent";

    for (hoxPlayerList::const_iterator it = m_players.begin(); 
                                       it != m_players.end(); ++it)
    {
        if ( (*it) == player )
        {
            wxLogDebug("%s: Skip this Player [%s] since he already left the Table.", 
                FNAME, player->GetName().c_str());
            continue;
        }

        if (   informer != NULL 
            && (*it) == informer )
        {
            wxLogDebug("%s: Skip this Player [%s] since he is the event's informer.", 
                FNAME, informer->GetName().c_str() );
            continue;
        }

        _PostPlayer_LeaveEvent( (*it), player );
    }
}

hoxPlayer* 
hoxTable::_GetBoardPlayer() const
{
    for (hoxPlayerList::const_iterator it = m_players.begin(); 
                                       it != m_players.end(); ++it)
    {
        if ( (*it)->GetType() == hoxPLAYER_TYPE_LOCAL )
        {
            return (*it);
        }
    }

	return NULL;
}

void 
hoxTable::_AddPlayer( hoxPlayer* player, 
                      hoxColor   role )
{
    hoxPlayerList::iterator found_it = 
        std::find( m_players.begin(), m_players.end(), player );

    if ( found_it == m_players.end() )  // not found?
	{
		m_players.push_back( player );
	}

    // "Cache" the RED and BLACK players for easy access.
    if ( role == hoxCOLOR_RED )
    {
        m_redPlayer = player;
        if ( m_blackPlayer == player ) m_blackPlayer = NULL;
    }
    else if ( role == hoxCOLOR_BLACK )
    {
        m_blackPlayer = player;
        if ( m_redPlayer == player ) m_redPlayer = NULL;
    }
    else
    {
        if ( m_redPlayer   == player ) m_redPlayer = NULL;
        if ( m_blackPlayer == player ) m_blackPlayer = NULL;
    }

    // Inform the Board.
    _PostBoard_PlayerEvent( hoxEVT_BOARD_PLAYER_JOIN, player, role );
}

void 
hoxTable::_RemovePlayer( hoxPlayer* player )
{
    const char* FNAME = "hoxTable::_RemovePlayer";
    bool bFound = false;   // For debugging purpose only.

    wxCHECK_RET(player != NULL, "Play cannot be NULL.");
    wxLogDebug("%s: ENTER. [%s]", FNAME, player->GetName().c_str());

    hoxPlayer* foundItem = NULL;
    for (hoxPlayerList::iterator it = m_players.begin(); 
                                 it != m_players.end(); ++it)
    {
        if ( (*it) == player )
        {
            foundItem = *it;
            bFound = true;
            break;
        }
    }

    m_players.remove( foundItem );

    wxLogDebug("%s: Player found or not = [%d].", FNAME, bFound);
    wxLogDebug("%s: # of remaining players = [%d].", FNAME, (int) m_players.size());

    // Update our "cache" variables.
    if ( m_redPlayer == player )
    {
        m_redPlayer = NULL;
    }
    else if ( m_blackPlayer == player )
    {
        m_blackPlayer = NULL;
    }

    // Inform the Board.
    _PostBoard_PlayerEvent( hoxEVT_BOARD_PLAYER_LEAVE, player );
}

hoxPlayer* 
hoxTable::_FindPlayer( const wxString& playerId )
{
    for (hoxPlayerList::iterator it = m_players.begin(); 
                                 it != m_players.end(); ++it)
    {
        if ( (*it)->GetName() == playerId )
        {
            return (*it);
        }
    }

    return NULL;
}

void
hoxTable::_ResetGame()
{
    m_redTime   = m_initialTime;
    m_blackTime = m_initialTime;
}
/************************* END OF FILE ***************************************/
