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
#include "hoxSite.h"

// ----------------------------------------------------------------------------
// hoxTable
// ----------------------------------------------------------------------------

hoxTable::hoxTable( hoxSite*         site,
                    const wxString&  id,
                    hoxIReferee*     referee,
                    hoxBoard*        board /* = NULL */ )
        : m_site( site )
        , m_id( id )
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

    /* NOTE: The order of deletion is important.
     * Board should be deleted AFTER all the players.
     */
    delete m_board;

    delete m_referee;
}

hoxResult 
hoxTable::AssignPlayer( hoxPlayer*     player,
                        hoxPieceColor& assignedColor,
                        bool           informOthers /* = true */)
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

    /* Inform other players about the new Player */
    if ( informOthers )
    {
        _PostAll_JoinEvent( player, assignedColor );
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxTable::AssignPlayerAs( hoxPlayer*     player,
                          hoxPieceColor  requestColor )
{
    const char* FNAME = "hoxTable::AssignPlayerAs";

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

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * NOTE: Unlike the other call, we will NOT inform other players about 
     * the new Player 
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    return hoxRESULT_OK;
}

hoxResult 
hoxTable::UnassignPlayer( hoxPlayer* player,
                          hoxPlayer* informer /* = NULL */ )
{
    const char* FNAME = "hoxTable::UnassignPlayer";

    wxLogDebug("%s: ENTER.", FNAME);

    wxCHECK_MSG( player, hoxRESULT_ERR, "The player is NULL." );

    /* Inform other players about this event. 
     * NOTE: This should be done BEFORE the player is removed
     *       from the internal player-list.
     */
    _PostAll_LeaveEvent( player, informer );

    /* Update our player-list */
    _RemovePlayer( player );

    return hoxRESULT_OK;
}

void 
hoxTable::SetBoard( hoxBoard* board )
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
        _PostBoard_MessageEvent( NULL /* System */, msg );
        return;
    }

    /* Get the Board Player (or the Board's owner) because he is the
     * one that sent the Move.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();
    wxCHECK_RET(boardPlayer, "The Board Player cannot be NULL.");

    /* Inform all players (including the Board's owner about the new Move */

    wxString moveStr = move.ToString();

    for ( hoxPlayerAndRoleList::const_iterator it = m_players.begin(); 
                                               it != m_players.end(); ++it )
    {
        wxLogDebug("%s: Inform player [%s] about the Board Move...", 
            FNAME, it->player->GetName().c_str());
        _PostPlayer_MoveEvent( it->player, boardPlayer, moveStr );
    }
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

    /* Inform other players about the new Player */
    _PostAll_MoveEvent( player, moveStr );
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
        wxLogWarning("%s: Player with Id = [%s] not found.", FNAME, playerId.c_str());
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
    hoxPlayer* player = NULL;

    wxLogDebug("%s: ENTER. [%s].", FNAME, m_id.c_str());

    while ( ! m_players.empty() )
    {
        player = m_players.front().player;

        _PostPlayer_CloseEvent( player );
        _RemovePlayer( player );
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
hoxTable::_PostPlayer_CloseEvent( hoxPlayer* player ) const
{
    const char* FNAME = "hoxTable::_PostPlayer_CloseEvent";

    wxCHECK_RET( player, "The player is NULL." );

    wxLogDebug("%s: Informing player [%s] about the Table [%s] being closed...", 
        FNAME, player->GetName().c_str(), m_id.c_str());
    wxCommandEvent event( hoxEVT_PLAYER_TABLE_CLOSE );
    event.SetString( m_id );
    wxPostEvent( player, event );
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
    wxString commandStr;
    commandStr.Printf("tid=%s&pid=%s", 
        m_id.c_str(), leavePlayer->GetName().c_str());
    wxCommandEvent event( hoxEVT_PLAYER_NEW_LEAVE );
    event.SetString( commandStr );
    wxPostEvent( player, event );
}

void 
hoxTable::_PostPlayer_JoinEvent( hoxPlayer*    player,
                                 hoxPlayer*    newPlayer,
                                 hoxPieceColor newColor ) const
{
    const char* FNAME = "hoxTable::_PostPlayer_JoinEvent";

    wxCHECK_RET( player, "The Player is NULL." );
    wxCHECK_RET( newPlayer, "The new Player is NULL." );

    wxLogDebug("%s: Informing player [%s] that a new Player [%s] just joined as [%d]...", 
        FNAME, player->GetName().c_str(), newPlayer->GetName().c_str(), newColor);
    wxString commandStr;
    commandStr.Printf("tid=%s&pid=%s&color=%d", 
        m_id.c_str(), newPlayer->GetName().c_str(), newColor);
    wxCommandEvent event( hoxEVT_PLAYER_NEW_JOIN );
    event.SetString( commandStr );
    wxPostEvent( player, event );
}

void 
hoxTable::_PostPlayer_MoveEvent( hoxPlayer*      player,
                                 hoxPlayer*      movePlayer,
                                 const wxString& moveStr ) const
{
    const char* FNAME = "hoxTable::_PostPlayer_MoveEvent";

    wxCHECK_RET( player, "The Player is NULL." );
    wxCHECK_RET( movePlayer, "The 'move' Player is NULL." );

    wxLogDebug("%s: Informing player [%s] that [%s] just made a new Move [%s]...", 
        FNAME, player->GetName().c_str(), movePlayer->GetName().c_str(), moveStr.c_str());
    wxString commandStr;
    commandStr.Printf("tid=%s&pid=%s&move=%s", 
        m_id.c_str(), movePlayer->GetName().c_str(), moveStr.c_str());
    wxCommandEvent event( hoxEVT_PLAYER_NEW_MOVE );
    event.SetString( commandStr );
    wxPostEvent( player, event );
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
    wxString commandStr;
    commandStr.Printf("tid=%s&pid=%s&msg=%s", 
        m_id.c_str(), msgPlayer->GetName().c_str(), message.c_str());
    wxCommandEvent event( hoxEVT_PLAYER_WALL_MSG );
    event.SetEventObject( msgPlayer );
    event.SetString( commandStr );
    wxPostEvent( player, event );
}

void 
hoxTable::_PostBoard_PlayerEvent( wxEventType commandType,
                                  hoxPlayer*  player,
                                  int         extraCode /* = wxID_ANY */ ) const
{
    wxCommandEvent playerEvent( commandType );
    playerEvent.SetEventObject( player );
    playerEvent.SetInt( extraCode );
    wxPostEvent( m_board , playerEvent );
}

void 
hoxTable::_PostBoard_MessageEvent( hoxPlayer*      player,
                                   const wxString& message ) const
{
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
hoxTable::_PostAll_JoinEvent( hoxPlayer*    newPlayer,
                              hoxPieceColor newColor ) const
{
    const char* FNAME = "hoxTable::_PostAll_JoinEvent";

    wxLogDebug("%s: ENTER.", FNAME);

    for (hoxPlayerAndRoleList::const_iterator it = m_players.begin(); 
                                              it != m_players.end(); ++it)
    {
        if ( it->player == newPlayer )
        {
            wxASSERT_MSG(it->role == newColor, "The colors are not matched."); 
            wxLogDebug("%s: Skip this Player since he is the new Player.", FNAME);
            continue;
        }

        _PostPlayer_JoinEvent( it->player, newPlayer, newColor );
    }
}

void 
hoxTable::_PostAll_MoveEvent( hoxPlayer*      player,
                              const wxString& moveStr ) const
{
    const char* FNAME = "hoxTable::_PostAll_MoveEvent";

    /* Get the Board Player (or the Board's owner).
     * Note: Currently, the Board player can be NULL because this Table
     *       may be hosted on a remote server and the two main players
     *       are all 'remote' players.
     */
    hoxPlayer* boardPlayer = _GetBoardPlayer();

    for (hoxPlayerAndRoleList::const_iterator it = m_players.begin(); 
                                              it != m_players.end(); ++it)
    {
        if ( it->player == player )
        {
            wxLogDebug("%s: Skip this Player [%s] since he just made this new Move.", 
                FNAME, player->GetName().c_str());
            continue;
        }

        if (   boardPlayer != NULL 
            && it->player == boardPlayer )
        {
            wxLogDebug("%s: Skip this Player [%s] since he is Board player.", 
                FNAME, boardPlayer->GetName().c_str() );
            continue;
        }

        _PostPlayer_MoveEvent( it->player, player, moveStr );
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

    for (hoxPlayerAndRoleList::const_iterator it = m_players.begin(); 
                                              it != m_players.end(); ++it)
    {
        if ( ! fromBoard )
        {
            if ( it->player == player )
            {
                wxLogDebug("%s: Skip this Player [%s] since he just sent this new Message.", 
                    FNAME, player->GetName().c_str());
                continue;
            }

            if (   boardPlayer != NULL 
                && it->player == boardPlayer )
            {
                wxLogDebug("%s: Skip this Player [%s] since he is Board player.", 
                    FNAME, boardPlayer->GetName().c_str() );
                continue;
            }
        }

        _PostPlayer_MessageEvent( it->player, player, message );
    }
}

void 
hoxTable::_PostAll_LeaveEvent( hoxPlayer* player,
                               hoxPlayer* informer /* = NULL */ ) const
{
    const char* FNAME = "hoxTable::_PostAll_LeaveEvent";

    for (hoxPlayerAndRoleList::const_iterator it = m_players.begin(); 
                                              it != m_players.end(); ++it)
    {
        if ( it->player == player )
        {
            wxLogDebug("%s: Skip this Player [%s] since he already left the Table.", 
                FNAME, player->GetName().c_str());
            continue;
        }

        if (   informer != NULL 
            && it->player == informer )
        {
            wxLogDebug("%s: Skip this Player [%s] since he is the event's informer.", 
                FNAME, informer->GetName().c_str() );
            continue;
        }

        _PostPlayer_LeaveEvent( it->player, player );
    }
}

hoxPlayer* 
hoxTable::_GetBoardPlayer() const
{
    hoxPlayerType playerType;

    hoxPlayerList players;
    players.push_back( m_redPlayer );
    players.push_back( m_blackPlayer );

    for ( hoxPlayerList::iterator it = players.begin();
                                  it != players.end(); ++it )
    {
        playerType = (*it)->GetType();
        if ( playerType == hoxPLAYER_TYPE_LOCAL )
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
    const char* FNAME = "hoxTable::_RemovePlayer";
    bool bFound = false;   // For debugging purpose only.

    wxCHECK_RET(player != NULL, "Play cannot be NULL.");
    wxLogDebug("%s: ENTER. [%s]", FNAME, player->GetName().c_str());

    hoxPlayerAndRole foundItem;
    for (hoxPlayerAndRoleList::iterator it = m_players.begin(); 
                                        it != m_players.end(); ++it)
    {
        if ( it->player == player )
        {
            foundItem = *it;
            //hoxPlayerAndRoleList::iterator foundIt = m_players.erase( it );
            //wxASSERT( foundIt != m_players.end() );
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
    for (hoxPlayerAndRoleList::iterator it = m_players.begin(); 
                                        it != m_players.end(); ++it)
    {
        if ( it->player->GetName() == playerId )
        {
            return it->player;
        }
    }

    return NULL;
}

/************************* END OF FILE ***************************************/
