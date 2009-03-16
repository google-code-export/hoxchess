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
        , m_boardPlayer( NULL )
        , m_gameType( hoxGAME_TYPE_RATED )
{
    wxLogDebug("%s: ENTER. (%s)", __FUNCTION__, m_id.c_str());

	m_blackTime.nGame = hoxTIME_DEFAULT_GAME_TIME;
	m_redTime.nGame   = hoxTIME_DEFAULT_GAME_TIME;
}

hoxTable::~hoxTable()
{
    wxLogDebug("%s: ENTER. (%s)", __FUNCTION__, m_id.c_str());

    _CloseBoard();   // Close GUI Board.
}

hoxResult 
hoxTable::AssignPlayerAs( hoxPlayer* player,
                          hoxColor   requestColor )
{
    wxCHECK_MSG( player != NULL, hoxRC_ERR, "The player is NULL." );

    bool bRequestOK =
           ( requestColor == hoxCOLOR_RED   && m_redPlayer == NULL )
        || ( requestColor == hoxCOLOR_BLACK && m_blackPlayer == NULL )
        || ( requestColor == hoxCOLOR_NONE );

    if ( ! bRequestOK )
    {
        wxLogDebug("%s: *** WARN *** Failed to handle request-to-join from [%s].", 
            __FUNCTION__, player->GetId().c_str());
        return hoxRC_ERR;
    }

    /* Update our player-list */
    _AddPlayer( player, requestColor );

    /* Inform the Board. */
    _PostBoard_PlayerEvent( hoxEVT_BOARD_PLAYER_JOIN, player, requestColor );

    return hoxRC_OK;
}

void 
hoxTable::SetBoard( hoxBoard* pBoard )
{
	wxCHECK_RET(m_board == NULL, "The Board has already been set.");
    m_board = pBoard;
}

void 
hoxTable::OnMove_FromBoard( const hoxMove&     move,
						    hoxGameStatus      status,
							const hoxTimeInfo& playerTime )
{
    if ( m_redPlayer == NULL || m_blackPlayer == NULL )
    {
        const wxString msg = "Not enough players. Ignore Move.";
        wxLogDebug("%s: *WARN* %s", __FUNCTION__, msg.c_str());
        _PostBoard_MessageEvent( msg );
        return;
    }

    /* Inform the Board's owner of the new Move. */
    PostPlayer_MoveEvent( m_boardPlayer,
                          move.ToString(), 
                          status,
                          playerTime );
}

void
hoxTable::OnMessage_FromBoard( const wxString& message )
{
    wxCHECK_RET(m_boardPlayer, "The Board Player cannot be NULL.");

    /* Post the message on the Wall-Output of the "local" Board. */
    _PostBoard_MessageEvent( message, m_boardPlayer->GetId() );

    /* Inform the Board's Onwer. */
	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_MSG ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = m_boardPlayer->GetId();
	apRequest->parameters["msg"] = message;
    
    m_boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnJoinCommand_FromBoard( const hoxColor requestColor )
{
    wxCHECK_RET(m_boardPlayer, "The Board Player cannot be NULL.");

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_JOIN ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = m_boardPlayer->GetId();
	apRequest->parameters["color"] = hoxUtil::ColorToString( requestColor );
	apRequest->parameters["joined"] = "1";

    m_boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnOptionsCommand_FromBoard( const bool         bRatedGame,
                                      const hoxTimeInfo& newTimeInfo )
{
    wxCHECK_RET(m_boardPlayer, "The Board Player cannot be NULL.");

	/* Make sure the board Player satifies one of the following conditions:
	 *  (1) He is the RED player, or...
     *  (2) He is the BLACK player and there is no RED player.
	 */

	bool bActionAllowed = 
        (     m_boardPlayer == m_redPlayer 
		  || (m_boardPlayer == m_blackPlayer && m_redPlayer == NULL) );

    if ( ! bActionAllowed )
	{
		wxLogWarning("Player [%s] is not allowed to change Options.", 
            m_boardPlayer->GetId().c_str());
		return;
	}

    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_UPDATE ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = m_boardPlayer->GetId();
    apRequest->parameters["rated"] = bRatedGame ? "1" : "0";
    apRequest->parameters["itimes"] = hoxUtil::TimeInfoToString( newTimeInfo );

    m_boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnResignCommand_FromBoard()
{
    wxCHECK_RET(m_boardPlayer, "The Board Player cannot be NULL.");

	/* Make sure the board Player is actually playing. 
	 * If not, ignore the request.
	 */

	if (   m_boardPlayer != m_redPlayer 
		&& m_boardPlayer != m_blackPlayer )
	{
		wxLogWarning("The Player [%s] is not playing.", m_boardPlayer->GetId().c_str());
		return;
	}

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_RESIGN ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = m_boardPlayer->GetId();

    m_boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnDrawCommand_FromBoard()
{
    wxCHECK_RET(m_boardPlayer, "The Board Player cannot be NULL.");

	/* Make sure the board Player is actually playing. 
	 * If not, ignore the request.
	 */

	if (   m_boardPlayer != m_redPlayer 
		&& m_boardPlayer != m_blackPlayer )
	{
		wxLogWarning("The Player [%s] is not playing.", m_boardPlayer->GetId().c_str());
		return;
	}

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_DRAW ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = m_boardPlayer->GetId();
	apRequest->parameters["draw_response"] = "";

    m_boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnResetCommand_FromBoard()
{
    wxCHECK_RET(m_boardPlayer, "The Board Player cannot be NULL.");

	/* Make sure the board Player is actually playing. 
	 * If not, ignore the request.
	 */

	if (   m_boardPlayer != m_redPlayer 
		&& m_boardPlayer != m_blackPlayer )
	{
		wxLogWarning("The Player [%s] is not playing.", m_boardPlayer->GetId().c_str());
		return;
	}

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_RESET ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = m_boardPlayer->GetId();

    m_boardPlayer->OnRequest_FromTable( apRequest );
}

void 
hoxTable::OnDrawResponse_FromBoard( bool bAcceptDraw )
{
    wxCHECK_RET(m_boardPlayer, "The Board Player cannot be NULL.");

	/* Make sure the board Player is actually playing. 
	 * If not, ignore the request.
	 */

	if (   m_boardPlayer != m_redPlayer 
		&& m_boardPlayer != m_blackPlayer )
	{
		wxLogWarning("The Player [%s] is not playing.", m_boardPlayer->GetId().c_str());
		return;
	}

	hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_DRAW ) );
	apRequest->parameters["tid"] = m_id;
	apRequest->parameters["pid"] = m_boardPlayer->GetId();
	apRequest->parameters["draw_response"] = (bAcceptDraw ? "1" : "0");

    m_boardPlayer->OnRequest_FromTable( apRequest );
}

void
hoxTable::OnPlayerInfoRequest_FromBoard( const wxString& sPlayerId )
{
    wxCHECK_RET(m_boardPlayer, "The Board Player cannot be NULL.");

    m_boardPlayer->SetActiveTableId( m_id );
        /* Which table will display the INFO... */

    m_boardPlayer->QueryPlayerInfo( sPlayerId );
}

void
hoxTable::OnPlayerInviteRequest_FromBoard( const wxString& sPlayerId )
{
    wxCHECK_RET(m_boardPlayer, "The Board Player cannot be NULL.");
    m_boardPlayer->InvitePlayer( sPlayerId );
}

void
hoxTable::OnPrivateMessageRequest_FromBoard( const wxString& sPlayerId )
{
    wxCHECK_RET(m_boardPlayer, "The Board Player cannot be NULL.");
    m_boardPlayer->SendPrivateMessage( sPlayerId );
}

void 
hoxTable::OnMove_FromNetwork( const wxString&  moveStr )
{
	_PostBoard_MoveEvent( moveStr );
}

void
hoxTable::OnPastMoves_FromNetwork( const hoxStringList& moves )
{
    for ( hoxStringList::const_iterator it = moves.begin();
                                        it != moves.end(); ++it )
    {
        _PostBoard_MoveEvent( (*it), true /* in SETUP-MODE */ );
    }
}

void 
hoxTable::OnMessage_FromNetwork( const wxString&  sSenderId,
                                 const wxString&  message,
                                 bool             bPublic /* = true */ )
{
    _PostBoard_MessageEvent( message, sSenderId, bPublic );
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
hoxTable::OnGameOver_FromNetwork( const hoxGameStatus gameStatus )
{
	_PostBoard_GameOverEvent( gameStatus );
}

void 
hoxTable::OnGameReset_FromNetwork()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    _ResetGame();

    /* Inform the Board about the status. */
	_PostBoard_GameResetEvent();
}

void 
hoxTable::OnLeave_FromPlayer( hoxPlayer* player )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

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
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    _PostBoard_ScoreEvent( player );
}

void 
hoxTable::OnClose_FromSystem()
{
    hoxPlayer* player = NULL;

    wxLogDebug("%s: ENTER. [%s].", __FUNCTION__, m_id.c_str());

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
    if ( m_board != NULL )
    {
        wxLogDebug("%s: ENTER. Table-Id = [%s].", __FUNCTION__, m_id.c_str());
        
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
                                   const wxString& sSenderId /* = wxEmptyString */,
                                   bool            bPublic /* = true */ ) const
{
	if ( m_board == NULL )
		return;

    const wxString eventString =
        wxString::Format("%s %s", sSenderId.c_str(), sMessage.c_str());

    /* Post the message on the Wall-Output of the Board. */
    wxCommandEvent event( hoxEVT_BOARD_WALL_OUTPUT );
    event.SetString( eventString );
    event.SetInt( bPublic ? 1 : 0 );
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

    /* "Cache" the BOARD player for easy access as well. */
    if ( player->GetType() == hoxPLAYER_TYPE_LOCAL )
    {
        m_boardPlayer = wxDynamicCast(player, hoxLocalPlayer);
        wxCHECK_RET(m_boardPlayer, "Fail to cast to LOCAL player");
    }
}

void 
hoxTable::_RemovePlayer( hoxPlayer* player )
{
    wxCHECK_RET(player != NULL, "Play cannot be NULL.");
    wxLogDebug("%s: ENTER. [%s]", __FUNCTION__, player->GetId().c_str());

    m_players.erase( player );

    // Update our "cache" variables.
    if      ( m_redPlayer == player )   m_redPlayer = NULL;
    else if ( m_blackPlayer == player ) m_blackPlayer = NULL;
    else if ( m_boardPlayer == player ) m_boardPlayer = NULL;

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
    wxLogDebug("%s: ENTER. (%s)", __FUNCTION__, m_id.c_str());
}
    
hoxPracticeTable::~hoxPracticeTable()
{
    wxLogDebug("%s: ENTER. (%s)", __FUNCTION__, m_id.c_str());
}

void
hoxPracticeTable::OnMove_FromBoard( const hoxMove&     move,
		                            hoxGameStatus      status,
				 	     	        const hoxTimeInfo& playerTime )
{
    wxLogDebug("%s: ENTER. Move = [%s].", __FUNCTION__, move.ToString().c_str());

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

void
hoxPracticeTable::OnResignCommand_FromBoard()
{
    const hoxGameStatus gameStatus = (   m_boardPlayer == m_redPlayer
                                       ? hoxGAME_STATUS_BLACK_WIN 
                                       : hoxGAME_STATUS_RED_WIN );
    this->OnGameOver_FromNetwork( gameStatus );
}

void
hoxPracticeTable::OnDrawCommand_FromBoard()
{
    this->OnGameOver_FromNetwork( hoxGAME_STATUS_DRAWN );
}

/************************* END OF FILE ***************************************/
