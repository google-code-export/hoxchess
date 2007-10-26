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
}

hoxTable::~hoxTable()
{
    const char* FNAME = "hoxTable::~hoxTable";

    wxLogDebug(wxString::Format("%s: ENTER.",FNAME));
    //this->UnassignAllPlayers();
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
hoxTable::AssignPlayer( hoxPlayer* player )
{
    hoxResult result = hoxRESULT_OK;

    wxASSERT( player != NULL );

    /* Assign to play RED if possible. */
    if ( m_redPlayer == NULL )
    {
        player->AddRole( hoxRole( m_id, hoxPIECE_COLOR_RED ) );
        m_redPlayer = player;
        
        // Inform the Board about the new player.
        wxCommandEvent playerEvent( hoxEVT_BOARD_PLAYER_INFO );
        playerEvent.SetEventObject( player );
        playerEvent.SetInt( hoxPIECE_COLOR_RED );
        ::wxPostEvent( m_board , playerEvent );
        return hoxRESULT_OK;
    }

    /* Assign to play BLACK if possible. */
    if ( m_blackPlayer == NULL )
    {
        player->AddRole( hoxRole( m_id, hoxPIECE_COLOR_BLACK ) );
        m_blackPlayer = player;

        // Inform the Board about the new player.
        wxCommandEvent playerEvent( hoxEVT_BOARD_PLAYER_INFO );
        playerEvent.SetEventObject( player );
        playerEvent.SetInt( hoxPIECE_COLOR_BLACK );
        ::wxPostEvent( m_board , playerEvent );
        return hoxRESULT_OK;
    }

    // TODO: Not yet support "observers" !!!

    return hoxRESULT_ERR;
}

hoxResult 
hoxTable::UnassignPlayer( hoxPlayer* player )
{
    hoxResult result = hoxRESULT_OK;

    wxASSERT( player != NULL );

    /* Check RED. */
    if ( m_redPlayer == player )
    {
        player->RemoveRole( hoxRole( m_id, hoxPIECE_COLOR_RED ) );
        m_redPlayer = NULL;
        m_board->SetRedInfo( wxString::Format(_("*")) );
        return hoxRESULT_OK;
    }

    /* Check BLACK. */
    if ( m_blackPlayer == player )
    {
        player->RemoveRole( hoxRole( m_id, hoxPIECE_COLOR_BLACK ) );
        m_blackPlayer = NULL;
        m_board->SetBlackInfo( wxString::Format(_("*")) );
        return hoxRESULT_OK;
    }

    // TODO: Not yet support "observers" !!!

    return hoxRESULT_ERR;
}

hoxResult 
hoxTable::UnassignAllPlayers()
{
    this->UnassignPlayer( m_redPlayer );
    this->UnassignPlayer( m_blackPlayer );

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

    wxLogDebug(wxString::Format(_("%s: Receive new Move from Board."), FNAME));
    if ( m_redPlayer == NULL || m_blackPlayer == NULL )
    {
        wxLogError(wxString::Format(_("%s: Not enough players. Ignore Move."), FNAME));
        return;
    }

    hoxPlayerList players;
    players.push_back( m_redPlayer );
    players.push_back( m_blackPlayer );

    for ( hoxPlayerList::iterator it = players.begin();
                                  it != players.end(); ++it )
    {
        wxLogDebug(wxString::Format(_("%s: Inform player [%s] about the Board Move..."), 
                            FNAME, (*it)->GetName()));
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

    wxLogDebug(wxString::Format(_("%s: Receive new Move from Network."), FNAME));
    wxASSERT( player != NULL );

    // Look up Move based on "fromPosition".    

    hoxMove move;
    move.newPosition = toPosition;

    if ( ! m_referee->GetPieceAtPosition( fromPosition, move.piece ) )
    {
        wxLogDebug(wxString::Format(_("%s: Failed to locate piece at the position."), FNAME));
        return;
    }

    wxLogDebug(wxString::Format(_("%s: Ask the Board to do this Move."), FNAME));
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
            wxLogDebug(wxString::Format(_("%s: Skip this Player since he is the Sender."), FNAME));
            continue;
        }

        switch( (*it)->GetType() )
        {
        case hoxPLAYER_TYPE_HOST:
            wxLogDebug(wxString::Format(_("%s: Ignore this Move since this is a HOST player."), FNAME));
            break;

        case hoxPLAYER_TYPE_LOCAL:
            wxLogDebug(wxString::Format(_("%s: Ignore this Move since this is a LOCAL player."), FNAME));
            break;

        case hoxPLAYER_TYPE_NETWORK:
            //(*it)->OnMove_FromTable( m_id, move );
            break;

        case hoxPLAYER_TYPE_DUMMY:
            /* fall through */
        default: 
            wxLogDebug(wxString::Format(_("%s: Ignore this Move since this is a DUMMY player."), FNAME));
            break;
        }
    }
}

void 
hoxTable::OnEvent_FromWWWNetwork( hoxPlayer*             player,
                                  const hoxNetworkEvent& networkEvent )
{
    const char* FNAME = "hoxTable::OnEvent_FromWWWNetwork";

    wxLogDebug(wxString::Format(_("%s: Receive new Event from Network."), FNAME));
    wxASSERT( player != NULL );

    /* Determine which color is affected. */

    switch ( networkEvent.type )
    {
        case hoxNETWORK_EVENT_TYPE_NEW_PLAYER_RED:    // NEW PLAYER (RED)
        {
            wxString otherPlayerId = networkEvent.content;
            if ( this->m_redPlayer == NULL && !otherPlayerId.empty() )
            {
                wxLogDebug(wxString::Format(_("%s: A player [%s] just joined table as RED."), 
                                FNAME, otherPlayerId));
                hoxPlayer* newPlayer 
                    = hoxPlayerMgr::GetInstance()->CreatePlayer( otherPlayerId,
                                                                 hoxPLAYER_TYPE_DUMMY );
                this->AssignPlayer( newPlayer );
            }
            break;
        }
        case hoxNETWORK_EVENT_TYPE_NEW_PLAYER_BLACK:    // NEW PLAYER (BLACK)
        {
            wxString otherPlayerId = networkEvent.content;
            if ( this->m_blackPlayer == NULL && !otherPlayerId.empty() )
            {
                wxLogDebug(wxString::Format(_("%s: A player [%s] just joined table as BLACK."), 
                                FNAME, otherPlayerId));
                hoxPlayer* newPlayer 
                    = hoxPlayerMgr::GetInstance()->CreatePlayer( otherPlayerId,
                                                                 hoxPLAYER_TYPE_DUMMY );
                this->AssignPlayer( newPlayer );
            }
            break;
        }
        case hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_RED:    // LEAVE PLAYER (RED)
            /* fall through */
        case hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_BLACK:    // LEAVE PLAYER (BLACK)
        {
            wxString otherPlayerId = networkEvent.content;
            if ( !otherPlayerId.empty() )
            {
                wxLogDebug(wxString::Format(_("%s: A player [%s] just left the table."), 
                                FNAME, otherPlayerId));
                hoxPlayer* foundPlayer 
                    = hoxPlayerMgr::GetInstance()->FindPlayer( otherPlayerId );
                if ( foundPlayer == NULL ) {
                    wxLogError(wxString::Format(_("%s: Player [%s] not found in the system."), 
                                    FNAME, otherPlayerId));
                } else {
                    this->UnassignPlayer( foundPlayer );
                }
            }
            break;
        }
        case hoxNETWORK_EVENT_TYPE_NEW_MOVE:    // NEW MOVE
        {
            wxString moveStr = networkEvent.content;
            hoxMove  move;

            hoxResult result = _ParseMoveString( moveStr, move );
            if ( result != hoxRESULT_OK ) // failed?
            {
                wxLogError(wxString::Format(_("%s: Parse Move from string [%s] failed."), 
                                FNAME, moveStr));
                return;
            }

            wxLogDebug(wxString::Format(_("%s: Ask the Board to do this Move."), FNAME));
            m_board->DoMove( move );

            break;
        }
        default:
            wxLogDebug(wxString::Format(_("%s: Unknown event type = [%d]."), FNAME, networkEvent.type));
            break;
    }
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
        wxLogDebug(wxString::Format(_("%s: Failed to locate piece at the position."), FNAME));
        return hoxRESULT_ERR;
    }

    return hoxRESULT_OK;
}


/************************* END OF FILE ***************************************/
