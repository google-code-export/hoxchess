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
        , m_gameType( hoxGAME_TYPE_RATED )
{
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER. (%s)", FNAME, m_id.c_str());

	m_blackTime.nGame = hoxTIME_DEFAULT_GAME_TIME;
	m_redTime.nGame   = hoxTIME_DEFAULT_GAME_TIME;
}

hoxTable::~hoxTable()
{
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER. (%s)", FNAME, m_id.c_str());

    _CloseBoard();   // Close GUI Board.
}

hoxResult 
hoxTable::AssignPlayerAs( hoxPlayer* player,
                          hoxColor   requestColor )
{
    const char* FNAME = __FUNCTION__;

    wxCHECK_MSG( player != NULL, hoxRC_ERR, "The player is NULL." );

    bool bRequestOK =
           ( requestColor == hoxCOLOR_RED   && m_redPlayer == NULL )
        || ( requestColor == hoxCOLOR_BLACK && m_blackPlayer == NULL )
        || ( requestColor == hoxCOLOR_NONE );

    if ( ! bRequestOK )
    {
        wxLogDebug("%s: *** WARN *** Failed to handle request-to-join from [%s].", 
            FNAME, player->GetId().c_str());
        return hoxRC_ERR;
    }

    /* Update our player-list */
    _AddPlayer( player, requestColor );

    /* Inform the Board. */
    _PostBoard_PlayerEvent( hoxEVT_BOARD_PLAYER_JOIN, player, requestColor );

    return hoxRC_OK;
}

void 
hoxTable::ViewBoard( hoxBoard* pBoard )
{
	wxCHECK_RET(m_board == NULL, "The Board has already been set.");

    m_board = pBoard;
    m_board->Show( true );  // Display Board's GUI.
}

void 
hoxTable::OnMove_FromBoard( const hoxMove&     move,
						    hoxGameStatus      status,
							const hoxTimeInfo& playerTime )
{
    const char* FNAME = __FUNCTION__;

    if ( m_redPlayer == NULL || m_blackPlayer == NULL )
    {
        const wxString msg = "Not enough players. Ignore Move.";
        wxLogDebug("%s: *** WARNING *** %s", FNAME, msg.c_str());
        _PostBoard_MessageEvent( msg );
        return;
    }

    /* Get the Board Player (or the Board's owner) because he is the
     * one that sent the Move.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");

    /* Inform the Board's owner of the new Move. */
    PostPlayer_MoveEvent( boardPlayer,
                          move.ToString(), 
                          status,
                          playerTime );
}

void
hoxTable::OnMessage_FromBoard( const wxString& message )
{
    /* Get the Board Player (or the Board's owner) because he is the
     * one that sent the Message.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");
    
    /* Post the message on the Wall-Output of the "local" Board. */
    _PostBoard_MessageEvent( message, boardPlayer );

    /* Inform the Board's Onwer. */
	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_MSG ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetId();
	apRequest->parameters["msg"] = message;
    
    boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnJoinCommand_FromBoard( const hoxColor requestColor )
{
    /* Get the Board Player (or the Board's owner) because he is the
     * one that sent the Message.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_JOIN ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetId();
	apRequest->parameters["color"] = hoxUtil::ColorToString( requestColor );
	apRequest->parameters["joined"] = "1";

    boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnOptionsCommand_FromBoard( const bool         bRatedGame,
                                      const hoxTimeInfo& newTimeInfo )
{
    const char* FNAME = __FUNCTION__;

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
            boardPlayer->GetId().c_str());
		return;
	}

    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_UPDATE ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetId();
    apRequest->parameters["rated"] = bRatedGame ? "1" : "0";
    apRequest->parameters["itimes"] = hoxUtil::TimeInfoToString( newTimeInfo );

    boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnResignCommand_FromBoard()
{
    const char* FNAME = __FUNCTION__;

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
		wxLogWarning("The Player [%s] is not playing.", boardPlayer->GetId().c_str());
		return;
	}

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_RESIGN ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetId();

    boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnDrawCommand_FromBoard()
{
    const char* FNAME = __FUNCTION__;

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
		wxLogWarning("The Player [%s] is not playing.", boardPlayer->GetId().c_str());
		return;
	}

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_DRAW ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetId();
	apRequest->parameters["draw_response"] = "";

    boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnResetCommand_FromBoard()
{
    const char* FNAME = __FUNCTION__;

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
		wxLogWarning("The Player [%s] is not playing.", boardPlayer->GetId().c_str());
		return;
	}

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_RESET ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetId();

    boardPlayer->OnRequest_FromTable( apRequest );
}

void 
hoxTable::OnDrawResponse_FromBoard( bool bAcceptDraw )
{
    const char* FNAME = __FUNCTION__;

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
		wxLogWarning("The Player [%s] is not playing.", boardPlayer->GetId().c_str());
		return;
	}

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_DRAW ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetId();
	apRequest->parameters["draw_response"] = (bAcceptDraw ? "1" : "0");

    boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnPlayerInfoRequest_FromBoard( const wxString& sPlayerId )
{
    /* Get the Board Player (or the Board's owner) because he is the
     * one that sent the Request.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");

    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_PLAYER_INFO ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetId();
    apRequest->parameters["info_pid"] = sPlayerId;

    boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnPlayerInviteRequest_FromBoard( const wxString& sPlayerId )
{
    /* Get the Board Player (or the Board's owner) because he is the
     * one that sent the Request.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");

    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_INVITE ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = boardPlayer->GetId();
    apRequest->parameters["invitee"] = sPlayerId;

    boardPlayer->OnRequest_FromTable( apRequest );
}

void 
hoxTable::OnMove_FromNetwork( hoxPlayer*       player,
                              const wxString&  moveStr )
{
    /* Inform the Board about this Move. */
	_PostBoard_MoveEvent( moveStr );
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
hoxTable::OnMessage_FromNetwork( const wxString&  playerId,
                                 const wxString&  message )
{
    const char* FNAME = __FUNCTION__;

    /* Lookup the player */
    hoxPlayer* foundPlayer = _FindPlayer( playerId );
    if ( foundPlayer == NULL )
    {
        wxLogDebug("%s: *** WARN *** Player with Id = [%s] not found.", FNAME, playerId.c_str());
        return;
    }

    /* Post the message to the Board. */
    _PostBoard_MessageEvent( message, foundPlayer );
}

void 
hoxTable::OnSystemMsg_FromNetwork( const wxString&  message )
{
    _PostBoard_SystemMsgEvent( message );
}

void 
hoxTable::OnLeave_FromNetwork( hoxPlayer* leavePlayer )
{
    /* Update our player-list */
    _RemovePlayer( leavePlayer );

    /* Inform the Board. */
    _PostBoard_PlayerEvent( hoxEVT_BOARD_PLAYER_LEAVE, leavePlayer );
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
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER.", FNAME);

    /* Update our player-list */
    _RemovePlayer( player );
}

void
hoxTable::OnUpdate_FromPlayer( hoxPlayer*         player,
                               const hoxGameType  gameType,
                               const hoxTimeInfo& newTimeInfo )
{
    m_gameType = gameType;

    m_initialTime = newTimeInfo;
    m_redTime     = m_initialTime;
    m_blackTime   = m_initialTime;

    _PostBoard_UpdateEvent( player );
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
    const char* FNAME = __FUNCTION__;
    hoxPlayer* player = NULL;

    wxLogDebug("%s: ENTER. [%s].", FNAME, m_id.c_str());

    while ( ! m_players.empty() )
    {
        player = *(m_players.begin());

        player->OnClose_FromTable( m_id );
        _RemovePlayer( player );
    }

    _CloseBoard();   // Close GUI Board.
}

void
hoxTable::PostBoardMessage( const wxString&  message )
{
    _PostBoard_MessageEvent( message );
}

void 
hoxTable::ToggleViewSide()
{
	wxCHECK_RET(m_board, "The Board is not yet set.");
    m_board->ToggleViewSide();
}

hoxColor
hoxTable::GetPlayerRole( const wxString& sPlayerId ) const
{
    hoxPlayer* foundPlayer = _FindPlayer( sPlayerId );
    
    if ( foundPlayer == NULL ) return hoxCOLOR_UNKNOWN;
    if ( foundPlayer == m_redPlayer ) return  hoxCOLOR_RED;
    if ( foundPlayer == m_blackPlayer ) return  hoxCOLOR_BLACK;
    return  hoxCOLOR_NONE;
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
hoxTable::PostPlayer_MoveEvent( hoxPlayer*         player,
                                const wxString&    moveStr,
							    hoxGameStatus      status /* = hoxGAME_STATUS_IN_PROGRESS */,
							    const hoxTimeInfo& playerTime /* = hoxTimeInfo() */ ) const
{
    const wxString statusStr = hoxUtil::GameStatusToString( status );

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_MOVE ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = player->GetId();
	apRequest->parameters["move"] = moveStr;
	apRequest->parameters["status"] = statusStr;
	apRequest->parameters["game_time"] = wxString::Format("%d", playerTime.nGame);

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
hoxTable::_PostBoard_MessageEvent( const wxString& sMessage,
                                   hoxPlayer*      player ) const
{
	if ( m_board == NULL )
		return;

    wxString who;
    wxString eventString;

    who = ( player != NULL ? player->GetId() 
                           : "*Table*" );
    eventString.Printf("%s %s", who.c_str(), sMessage.c_str());

    /* Post the message on the Wall-Output of the Board. */
    wxCommandEvent event( hoxEVT_BOARD_WALL_OUTPUT );
    event.SetString( eventString );
    wxPostEvent( m_board, event );
}

void
hoxTable::_PostBoard_SystemMsgEvent( const wxString& sMessage ) const
{
	if ( m_board == NULL )
		return;

    /* Post the message on the System-Output of the "local" Board. */
    wxCommandEvent event( hoxEVT_BOARD_SYSTEM_OUTPUT );
    event.SetString( sMessage );
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
    event.SetString( fromPlayer->GetId() );
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
hoxTable::_PostBoard_UpdateEvent( hoxPlayer* player ) const
{
	if ( m_board == NULL )
		return;

    wxCommandEvent event( hoxEVT_BOARD_TABLE_UPDATE );
    wxPostEvent( m_board, event );
}

hoxPlayer* 
hoxTable::_GetBoardPlayer() const
{
    for (hoxPlayerSet::const_iterator it = m_players.begin(); 
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
    m_players.insert( player );

    /* "Cache" the RED and BLACK players for easy access. */

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
}

void 
hoxTable::_RemovePlayer( hoxPlayer* player )
{
    const char* FNAME = __FUNCTION__;

    wxCHECK_RET(player != NULL, "Play cannot be NULL.");
    wxLogDebug("%s: ENTER. [%s]", FNAME, player->GetId().c_str());

    m_players.erase( player );

    // Update our "cache" variables.
    if      ( m_redPlayer == player )   m_redPlayer = NULL;
    else if ( m_blackPlayer == player ) m_blackPlayer = NULL;

    /* TODO: A temporary solution to delete AI Players. */
    if ( player->GetType() == hoxPLAYER_TYPE_AI )
    {
        delete player;
    }
}

hoxPlayer* 
hoxTable::_FindPlayer( const wxString& playerId ) const
{
    for (hoxPlayerSet::const_iterator it = m_players.begin(); 
                                      it != m_players.end(); ++it)
    {
        if ( (*it)->GetId() == playerId )
        {
            return (*it);
        }
    }

    return NULL;
}

void
hoxTable::_ResetGame()
{
    m_referee->ResetGame();

    m_redTime   = m_initialTime;
    m_blackTime = m_initialTime;
}


// ----------------------------------------------------------------------------
//
//                    hoxPracticeTable
//
// ----------------------------------------------------------------------------

hoxPracticeTable::hoxPracticeTable( hoxSite*          site,
                                    const wxString&   id,
                                    hoxIReferee_SPtr  referee,
                                    hoxBoard*         board /* = NULL */ )
        : hoxTable( site, id, referee, board )
{
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER. (%s)", FNAME, m_id.c_str());
}
    
hoxPracticeTable::~hoxPracticeTable()
{
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER. (%s)", FNAME, m_id.c_str());
}

void
hoxPracticeTable::OnMove_FromBoard( const hoxMove&     move,
		                            hoxGameStatus      status,
				 	     	        const hoxTimeInfo& playerTime )
{
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER. Move = [%s].", FNAME, move.ToString().c_str());

    /* Get the AI Player. */

    hoxPlayer* aiPlayer = NULL;
    if ( m_redPlayer && m_redPlayer->GetType() == hoxPLAYER_TYPE_AI )
    {
        aiPlayer = m_redPlayer;
    }
    else if ( m_blackPlayer && m_blackPlayer->GetType() == hoxPLAYER_TYPE_AI )
    {
        aiPlayer = m_blackPlayer;
    }
    wxCHECK_RET(aiPlayer, "The AI Player cannot be NULL.");

    /* Inform the AI Player of the new Move. */

    PostPlayer_MoveEvent( aiPlayer,
                          move.ToString(), 
                          status,
                          playerTime );
}

/************************* END OF FILE ***************************************/
