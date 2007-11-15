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
// Name:            hoxTable.cpp
// Created:         09/30/2007
//
// Description:     The Table.
/////////////////////////////////////////////////////////////////////////////

#include "hoxTable.h"
#include "hoxReferee.h"
#include "hoxPlayerMgr.h"
#include "hoxPlayer.h"
#include "hoxBoard.h"
#include "MyApp.h"     // wxGetApp()

// ----------------------------------------------------------------------------
// hoxTable
// ----------------------------------------------------------------------------

hoxTable::hoxTable( const wxString&  id,
                    hoxIReferee*     referee,
                    hoxSimpleBoard*  board /* = NULL */ )
        : m_id( id )
        , m_referee( referee )
        , m_board( board )
        , m_redPlayer( NULL )
        , m_blackPlayer( NULL )
{
    /* Validate inputs.
     *
     * NOTE: Currently, we only log errors.
     *       This is not the best way to handle errors within a contructor
     *       because an Table object has already been constructed.
     *       Throwing an exception would be a better solution.
     *       However, as of now (Sep 30, 2007), wxWidgets 2.8.5 does not
     *       officially support C++ exceptions.
     */

    //if ( m_board == NULL )
    //{
    //    wxLogError(_T("Unexpected NULL pointer for Board."));
    //}

    const char* FNAME = "hoxTable::hoxTable";

    wxLogDebug("%s: ENTER.", FNAME);

}

hoxTable::~hoxTable()
{
    const char* FNAME = "hoxTable::~hoxTable";

    wxLogDebug("%s: ENTER.", FNAME);

    if ( m_redPlayer != NULL )
    {
        if ( m_redPlayer->GetRoles().size() == 0 )
            hoxPlayerMgr::GetInstance()->DeletePlayer( m_redPlayer );
    }

    if ( m_blackPlayer != NULL )
    {
        if ( m_blackPlayer->GetRoles().size() == 0 )
            hoxPlayerMgr::GetInstance()->DeletePlayer( m_blackPlayer );
    }

    /* NOTE: The order of deletion is important.
     * Board should be deleted AFTER all the players.
     */
    delete m_board;

    delete m_referee;
}

hoxResult 
hoxTable::AssignPlayer( hoxPlayer*     player,
                        hoxPieceColor& assignedColor )
{
    hoxResult result = hoxRESULT_OK;

    wxCHECK_MSG( player != NULL, hoxRESULT_ERR, "The player is NULL." );

    assignedColor = hoxPIECE_COLOR_NONE; // Default: Observer's Role.

    /* Assign to play RED if possible. */
    if ( m_redPlayer == NULL )
    {
        assignedColor = hoxPIECE_COLOR_RED;
    }
    /* Assign to play BLACK if possible. */
    else if ( m_blackPlayer == NULL )
    {
        assignedColor = hoxPIECE_COLOR_BLACK;
    }
    /* Default: ... The player will join as an Observer. */

    /* Update our player-list */
    _AddPlayer( player, assignedColor );

    return hoxRESULT_OK;
}

hoxResult 
hoxTable::RequestJoinFromPlayer( hoxPlayer*     player,
                                 hoxPieceColor  requestColor )
{
    const char* FNAME = "hoxTable::RequestJoinFromPlayer";

    wxCHECK_MSG( player != NULL, hoxRESULT_ERR, "The player is NULL." );

    hoxPieceColor assignedColor = hoxPIECE_COLOR_NONE;

    /* Assign to play RED if possible. */
    if ( requestColor == hoxPIECE_COLOR_RED && m_redPlayer == NULL )
    {
        assignedColor = hoxPIECE_COLOR_RED;
    }
    /* Assign to play BLACK if possible. */
    else if ( requestColor == hoxPIECE_COLOR_BLACK && m_blackPlayer == NULL )
    {
        assignedColor = hoxPIECE_COLOR_BLACK;
    }
    /* Assign to be an observer */
    else if ( requestColor == hoxPIECE_COLOR_NONE )
    {
        assignedColor = hoxPIECE_COLOR_NONE;
    }
    else
    {
        wxLogWarning("%s: Failed to fullfil request-to-join from player.", FNAME);
        return hoxRESULT_ERR;
    }

    /* Update our player-list */
    _AddPlayer( player, assignedColor );

    return hoxRESULT_OK;
}

hoxResult 
hoxTable::UnassignPlayer( hoxPlayer* player )
{
    wxCHECK_MSG( player, hoxRESULT_ERR, "The player is NULL." );

    /* Update our player-list */
    _RemovePlayer( player );

    return hoxRESULT_OK;
}

void 
hoxTable::SetBoard( hoxSimpleBoard* board )
{
    wxASSERT_MSG( m_board == NULL, "The Board has already been set." );

    m_board = board;

    /* Assuming the this API can be used to "unset" the table.
     * Thus, the input argument 'board' can be NULL.
     * In the case it is not NULL, then also attach the Table
     * to the Board so that the Board can inform (call back) 
     * the Table about the new Board Moves.
     */
    if ( m_board != NULL )
    {
        m_board->SetTable( this );
        m_board->Show( true );  // Display Board's GUI.
    }
}

void 
hoxTable::OnMove_FromBoard( const hoxMove& move )
{
    const char* FNAME = "hoxTable::OnMove_FromBoard";

    wxLogDebug("%s: Receive new Move from Board.", FNAME);
    if ( m_redPlayer == NULL || m_blackPlayer == NULL )
    {
        const wxString msg = "Not enough players. Ignore Move.";
        wxLogDebug("%s: *** WARNING *** %s", FNAME, msg.c_str());
        _PostBoard_MessageEvent( wxGetApp().GetHostPlayer(), msg );
        return;
    }

    hoxPlayer* player = NULL;  // A player holder.

    for (hoxPlayerAndRoleList::iterator it = m_players.begin(); 
                                        it != m_players.end(); ++it)
    {
        player = it->player;
        wxLogDebug("%s: Inform player [%s] about the Board Move...", 
            FNAME, player->GetName().c_str());
        hoxPlayerEvent event(hoxEVT_PLAYER_NEW_MOVE);
        event.SetTableId(m_id);
        event.SetOldPosition(move.piece.position);   // last move's old-piece.
        event.SetPosition(move.newPosition);         // last move's new-position.
        wxPostEvent( player, event );
    }
}

void
hoxTable::OnMessage_FromBoard( const wxString& message )
{
    const char* FNAME = "hoxTable::OnMessage_FromBoard";
    wxLogDebug("%s: Receive new Message from Board.", FNAME);

    /* Get the Board Player (or the Board's owner) */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");
    
    /* Post the message on the Wall-Output of the "local" Board. */
    _PostBoard_MessageEvent( boardPlayer, message );

    /* Post the message to other players */

    hoxPlayer* player = NULL;  // A player holder.    

    for (hoxPlayerAndRoleList::iterator it = m_players.begin(); 
                                        it != m_players.end(); ++it)
    {
        player = it->player;

        //if ( player == boardPlayer )
        //{
        //    wxLogDebug("%s: Skip this Player since he is Board player.", FNAME);
        //    continue;
        //}

        switch( player->GetType() )
        {
        case hoxPLAYER_TYPE_HOST:
            wxLogDebug("%s: Ignore this Message since this is a HOST player.", FNAME);
            break;

        case hoxPLAYER_TYPE_LOCAL:
            /* fall through */
        case hoxPLAYER_TYPE_REMOTE:
        {
            wxString commandStr;
            commandStr.Printf("tid=%s&pid=%s&msg=%s", m_id.c_str(), boardPlayer->GetName().c_str(), message.c_str()); 
            wxCommandEvent event( hoxEVT_PLAYER_WALL_MSG );
            event.SetEventObject( boardPlayer );
            event.SetString( commandStr );
            wxPostEvent( player,event );
            break;
        }

        case hoxPLAYER_TYPE_DUMMY:
            /* fall through */
        default: 
            wxLogDebug("%s: Ignore this Message since this is a DUMMY player.", FNAME);
            break;
        }
    }

}

void 
hoxTable::OnMove_FromNetwork( hoxPlayer*         player,
                              const hoxPosition& fromPosition,
                              const hoxPosition& toPosition )
{
    const char* FNAME = "hoxTable::OnMove_FromNetwork";

    wxLogDebug("%s: Receive new Move from Network.", FNAME);
    wxASSERT( player != NULL );

    // Look up Move based on "fromPosition".    

    hoxMove move;
    move.newPosition = toPosition;

    if ( ! m_referee->GetPieceAtPosition( fromPosition, move.piece ) )
    {
        wxLogDebug("%s: Failed to locate piece at the position.", FNAME);
        return;
    }

    wxLogDebug("%s: Ask the Board to do this Move.", FNAME);
    m_board->DoMove( move );

    /* Inform all players at this table about this move. 
     * Skip the Move's sender since he is the one that informed me.
     */

    hoxPlayerList players;
    players.push_back( m_redPlayer );
    players.push_back( m_blackPlayer );

    for ( hoxPlayerList::iterator it = players.begin();
                                  it != players.end(); ++it )
    {
        if ( (*it) == player )
        {
            wxLogDebug("%s: Skip this Player since he is the Sender.", FNAME);
            continue;
        }

        switch( (*it)->GetType() )
        {
        case hoxPLAYER_TYPE_HOST:
            wxLogDebug("%s: Ignore this Move since this is a HOST player.", FNAME);
            break;

        case hoxPLAYER_TYPE_LOCAL:
            wxLogDebug("%s: Ignore this Move since this is a LOCAL player.", FNAME);
            break;

        case hoxPLAYER_TYPE_REMOTE:
            //(*it)->OnMove_FromTable( m_id, move );
            break;

        case hoxPLAYER_TYPE_DUMMY:
            /* fall through */
        default: 
            wxLogDebug("%s: Ignore this Move since this is a DUMMY player.", FNAME);
            break;
        }
    }
}

void 
hoxTable::OnMove_FromNetwork( hoxPlayer*         player,
                              const wxString&    moveStr )
{
    const char* FNAME = "hoxTable::OnMove_FromNetwork(moveStr)";
    hoxMove  move;

    hoxResult result = _ParseMoveString( moveStr, move );
    if ( result != hoxRESULT_OK ) // failed?
    {
        wxLogError("%s: Parse Move from string [%s] failed.", FNAME, moveStr.c_str());
        return;
    }

    wxLogDebug("%s: Ask the Board to do this Move.", FNAME);
    m_board->DoMove( move );
}

void 
hoxTable::OnMessage_FromNetwork( const wxString&  playerId,
                                 const wxString&  message )
{
    const char* FNAME = "hoxTable::OnMessage_FromNetwork";

    wxLogDebug("%s: ENTER.", FNAME);

    /* Lookup the player */
    hoxPlayer* foundPlayer = _FindPlayer( playerId );
    if ( foundPlayer == NULL )
    {
        wxLogWarning("%s: Player with Id = [%s] not found.", FNAME, playerId.c_str());
        return;
    }

    /* Post the message (from the player) to the Board. */
    _PostBoard_MessageEvent( foundPlayer, message );
}

void 
hoxTable::OnLeave_FromPlayer( hoxPlayer* player )
{
    const char* FNAME = "hoxTable::OnLeave_FromPlayer";

    wxLogDebug("%s: ENTER.", FNAME);

    this->UnassignPlayer( player );
}

void 
hoxTable::OnClose_FromSystem()
{
    const char* FNAME = "hoxTable::OnClose_FromSystem";
    hoxPlayerList players;

    wxLogDebug("%s: ENTER.", FNAME);

    if ( m_redPlayer != NULL )   players.push_back( m_redPlayer );
    if ( m_blackPlayer != NULL ) players.push_back( m_blackPlayer );

    for ( hoxPlayerList::iterator it = players.begin();
                                  it != players.end(); ++it )
    {
        _PostPlayer_CloseEvent( *it );
        this->UnassignPlayer( *it );
    }

    delete m_board;
    m_board = NULL;

    delete m_referee;
    m_referee = NULL;
}

void 
hoxTable::ToggleViewSide()
{
    m_board->ToggleViewSide();
}

hoxResult 
hoxTable::_ParseMoveString( const wxString& moveStr, 
                            hoxMove&        move )
{
    const char* FNAME = "hoxTable::_ParseMoveString";

    /* NOTE: Move-string has the format of "xyXY" */

    if ( moveStr.size() != 4 )
    {
        return hoxRESULT_ERR;
    }

    hoxPosition fromPosition;
    hoxPosition toPosition;

    fromPosition.x = moveStr[0] - '0';
    fromPosition.y = moveStr[1] - '0';
    toPosition.x = moveStr[2] - '0';
    toPosition.y = moveStr[3] - '0';

    // Look up Move based on "fromPosition".    

    move.newPosition = toPosition;

    if ( ! m_referee->GetPieceAtPosition( fromPosition, move.piece ) )
    {
        wxLogDebug("%s: Failed to locate piece at the position.", FNAME);
        return hoxRESULT_ERR;
    }

    return hoxRESULT_OK;
}

void 
hoxTable::_PostPlayer_CloseEvent( hoxPlayer* player )
{
    const char* FNAME = "hoxTable::_PostPlayer_CloseEvent";

    wxCHECK_RET( player, "The player is NULL." );

    wxLogDebug("%s: Informing player [%s] about the Table [%s] being closed...", 
        FNAME, player->GetName().c_str(), m_id.c_str());
    hoxPlayerEvent event( hoxEVT_PLAYER_TABLE_CLOSED );
    event.SetTableId( m_id );
    wxPostEvent( player, event );
}

void 
hoxTable::_PostBoard_PlayerEvent( wxEventType commandType,
                                  hoxPlayer*  player,
                                  int         extraCode /* = wxID_ANY */ )
{
    wxCommandEvent playerEvent( commandType );
    playerEvent.SetEventObject( player );
    playerEvent.SetInt( extraCode );
    wxPostEvent( m_board , playerEvent );
}

void 
hoxTable::_PostBoard_MessageEvent( hoxPlayer*      player,
                                   const wxString& message )
{
    wxCHECK_RET(player, "The player cannot be NULL.");
    
    wxString eventString;
    eventString.Printf("%s %s", player->GetName().c_str(), message.c_str());

    /* Post the message on the Wall-Output of the "local" Board. */
    wxCommandEvent event( hoxEVT_BOARD_WALL_OUTPUT );
    event.SetEventObject( player );
    event.SetString( eventString );
    wxPostEvent( m_board, event );
}

hoxPlayer* 
hoxTable::_GetBoardPlayer()
{
    hoxPlayerType playerType;

    hoxPlayerList players;
    players.push_back( m_redPlayer );
    players.push_back( m_blackPlayer );

    for ( hoxPlayerList::iterator it = players.begin();
                                  it != players.end(); ++it )
    {
        playerType = (*it)->GetType();
        if (   playerType == hoxPLAYER_TYPE_HOST 
            || playerType == hoxPLAYER_TYPE_LOCAL )
        {
            return (*it);
        }
    }

    return NULL;
}

void 
hoxTable::_AddPlayer( hoxPlayer*    player, 
                      hoxPieceColor role )
{
    m_players.push_back( hoxPlayerAndRole(player, role) );

    // "Cache" the RED and BLACK players for easy access.
    if ( role == hoxPIECE_COLOR_RED )
    {
        m_redPlayer = player;
    }
    else if ( role == hoxPIECE_COLOR_BLACK )
    {
        m_blackPlayer = player;
    }

    // Inform the Board.
    _PostBoard_PlayerEvent( hoxEVT_BOARD_PLAYER_JOIN, player, role );
}

void 
hoxTable::_RemovePlayer( hoxPlayer* player )
{
    for (hoxPlayerAndRoleList::iterator it = m_players.begin(); 
                                        it != m_players.end(); ++it)
    {
        if ( it->player == player )
        {
            m_players.erase( it );
            break;
        }
    }

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
    for (hoxPlayerAndRoleList::iterator it = m_players.begin(); 
                                        it != m_players.end(); ++it)
    {
        if (  it->player->GetName() == playerId )
        {
            return it->player;
        }
    }

    return NULL;
}

/************************* END OF FILE ***************************************/
