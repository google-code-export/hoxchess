/////////////////////////////////////////////////////////////////////////////
// Name:            hoxTable.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         09/30/2007
/////////////////////////////////////////////////////////////////////////////

#include "hoxTable.h"
#include "hoxReferee.h"
#include "hoxPlayerMgr.h"
#include "hoxPlayer.h"
#include "hoxBoard.h"

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

    assignedColor = hoxPIECE_COLOR_NONE;

    /* Assign to play RED if possible. */
    if ( m_redPlayer == NULL )
    {
        assignedColor = hoxPIECE_COLOR_RED;
        m_redPlayer = player;
    }
    /* Assign to play BLACK if possible. */
    else if ( m_blackPlayer == NULL )
    {
        assignedColor = hoxPIECE_COLOR_BLACK;
        m_blackPlayer = player;
    }

    if ( assignedColor != hoxPIECE_COLOR_NONE )
    {
        // Inform the Board about the new player.
        _PostBoard_PlayerEvent( hoxEVT_BOARD_PLAYER_JOIN, player, assignedColor );
        return hoxRESULT_OK;
    }

    // TODO: Not yet support "observers" !!!

    return hoxRESULT_ERR;
}

hoxResult 
hoxTable::RequestJoinFromPlayer( hoxPlayer*     player,
                                 hoxPieceColor  requestColor )
{
    hoxResult result = hoxRESULT_OK;

    wxCHECK_MSG( player != NULL, hoxRESULT_ERR, "The player is NULL." );

    hoxPieceColor assignedColor = hoxPIECE_COLOR_NONE;

    /* Assign to play RED if possible. */
    if ( requestColor == hoxPIECE_COLOR_RED && m_redPlayer == NULL )
    {
        assignedColor = hoxPIECE_COLOR_RED;
        m_redPlayer = player;
    }
    /* Assign to play BLACK if possible. */
    else if ( requestColor == hoxPIECE_COLOR_BLACK && m_blackPlayer == NULL )
    {
        assignedColor = hoxPIECE_COLOR_BLACK;
        m_blackPlayer = player;
    }

    if ( assignedColor != hoxPIECE_COLOR_NONE )
    {
        // Inform the Board about the new player.
        _PostBoard_PlayerEvent( hoxEVT_BOARD_PLAYER_JOIN, player, assignedColor );
        return hoxRESULT_OK;
    }

    // TODO: Not yet support "observers" !!!

    return hoxRESULT_ERR;
}

hoxResult 
hoxTable::UnassignPlayer( hoxPlayer* player )
{
    hoxResult result = hoxRESULT_OK;

    wxCHECK_MSG( player, hoxRESULT_ERR, "The player is NULL." );

    /* Check RED. */
    if ( m_redPlayer == player )
    {
        m_redPlayer = NULL;
        //m_board->SetRedInfo( "*" );
        _PostBoard_PlayerEvent( hoxEVT_BOARD_PLAYER_LEAVE, player );
        return hoxRESULT_OK;
    }

    /* Check BLACK. */
    if ( m_blackPlayer == player )
    {
        m_blackPlayer = NULL;
        //m_board->SetBlackInfo( "*" );
        _PostBoard_PlayerEvent( hoxEVT_BOARD_PLAYER_LEAVE, player );
        return hoxRESULT_OK;
    }

    // TODO: Not yet support "observers" !!!

    return hoxRESULT_ERR;
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
        wxLogError("%s: Not enough players. Ignore Move.", FNAME);
        return;
    }

    hoxPlayerList players;
    players.push_back( m_redPlayer );
    players.push_back( m_blackPlayer );

    for ( hoxPlayerList::iterator it = players.begin();
                                  it != players.end(); ++it )
    {
        wxLogDebug("%s: Inform player [%s] about the Board Move...", 
            FNAME, (*it)->GetName());
        hoxPlayerEvent event(hoxEVT_PLAYER_NEW_MOVE);
        event.SetTableId(m_id);
        event.SetOldPosition(move.piece.position);   // last move's old-piece.
        event.SetPosition(move.newPosition);         // last move's new-position.
        wxPostEvent( (*it), event);
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
        wxLogError("%s: Parse Move from string [%s] failed.", FNAME, moveStr);
        return;
    }

    wxLogDebug("%s: Ask the Board to do this Move.", FNAME);
    m_board->DoMove( move );
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
        FNAME, player->GetName(), m_id);
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
    ::wxPostEvent( m_board , playerEvent );

}

/************************* END OF FILE ***************************************/
