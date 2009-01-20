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
// Name:            hoxBoard.cpp
// Created:         10/05/2007
//
// Description:     A full-featured Board with the following features:
//                     + Player's information (such as Name, Score).
//                     + Timers (including Game, Move, and Free times).
//                     + Game History (forward/backward 'past' Moves).
//                     + Chat feature (Text Input + Wall Output).
/////////////////////////////////////////////////////////////////////////////

#include "hoxBoard.h"
#include "hoxCoreBoard.h"
#include "hoxUtil.h"
#include "hoxReferee.h"
#include "hoxTypes.h"
#include "hoxTable.h"
#include "hoxOptionDialog.h"

/* UI-related IDs. */
enum
{
    ID_BOARD_WALL_INPUT = hoxUI_ID_RANGE_BOARD,

    ID_HISTORY_BEGIN,
    ID_HISTORY_PREV,
    ID_HISTORY_NEXT,
    ID_HISTORY_END,

    ID_ACTION_RED,    // Play as RED
    ID_ACTION_BLACK,  // Play as BLACK
    ID_ACTION_NONE,   // Play as NONE (i.e., Unsit/Standup)

    ID_ACTION_OPTIONS,
    ID_ACTION_RESIGN,
	ID_ACTION_DRAW,
	ID_ACTION_RESET
};


/* Define my custom events */
DEFINE_EVENT_TYPE( hoxEVT_BOARD_PLAYER_JOIN )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_PLAYER_LEAVE )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_PLAYER_SCORE )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_SYSTEM_OUTPUT )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_WALL_OUTPUT )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_NEW_MOVE )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_DRAW_REQUEST )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_GAME_OVER )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_GAME_RESET )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_TABLE_UPDATE )

BEGIN_EVENT_TABLE(hoxBoard, wxPanel)
    EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_PLAYER_JOIN, hoxBoard::OnPlayerJoin)
    EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_PLAYER_LEAVE, hoxBoard::OnPlayerLeave)
    EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_PLAYER_SCORE, hoxBoard::OnPlayerScore)
    EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_SYSTEM_OUTPUT, hoxBoard::OnSystemOutput)
    EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_WALL_OUTPUT, hoxBoard::OnWallOutput)
	EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_NEW_MOVE, hoxBoard::OnNewMove)
	EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_DRAW_REQUEST, hoxBoard::OnDrawRequest)
	EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_GAME_OVER, hoxBoard::OnGameOver)
    EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_GAME_RESET, hoxBoard::OnGameReset)
    EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_TABLE_UPDATE, hoxBoard::OnTableUpdate)

    EVT_TEXT_ENTER(ID_BOARD_WALL_INPUT, hoxBoard::OnWallInputEnter)
    EVT_BUTTON(ID_HISTORY_BEGIN, hoxBoard::OnButtonHistory_BEGIN)
    EVT_BUTTON(ID_HISTORY_PREV, hoxBoard::OnButtonHistory_PREV)
    EVT_BUTTON(ID_HISTORY_NEXT, hoxBoard::OnButtonHistory_NEXT)
    EVT_BUTTON(ID_HISTORY_END, hoxBoard::OnButtonHistory_END)
    EVT_BUTTON(ID_ACTION_OPTIONS, hoxBoard::OnButtonOptions)
    EVT_BUTTON(ID_ACTION_RESIGN, hoxBoard::OnButtonResign)
	EVT_BUTTON(ID_ACTION_DRAW, hoxBoard::OnButtonDraw)
    EVT_BUTTON(ID_ACTION_RESET, hoxBoard::OnButtonReset)
    EVT_BUTTON(ID_ACTION_RED, hoxBoard::OnButtonRed)
    EVT_BUTTON(ID_ACTION_BLACK, hoxBoard::OnButtonBlack)
	EVT_BUTTON(ID_ACTION_NONE, hoxBoard::OnButtonNone)

    EVT_UPDATE_UI(ID_ACTION_RED, hoxBoard::OnUpdateUI_ActionRed)
    EVT_UPDATE_UI(ID_ACTION_BLACK, hoxBoard::OnUpdateUI_ActionBlack)
    EVT_UPDATE_UI(ID_ACTION_NONE, hoxBoard::OnUpdateUI_ActionNone)
    EVT_UPDATE_UI(ID_ACTION_RESIGN, hoxBoard::OnUpdateUI_ActionResign)
    EVT_UPDATE_UI(ID_ACTION_DRAW, hoxBoard::OnUpdateUI_ActionDraw)
    EVT_UPDATE_UI(ID_ACTION_RESET, hoxBoard::OnUpdateUI_ActionReset)

    EVT_TIMER(wxID_ANY, hoxBoard::OnTimer)    
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// hoxBoard
// ----------------------------------------------------------------------------

hoxBoard::hoxBoard( wxWindow*        parent,
                    const wxString&  piecesPath,
                    hoxIReferee_SPtr referee,
                    hoxTable_SPtr    pTable,
                    const wxString&  ownerId,
                    wxColor          bgColor,
                    wxColor          fgColor,
                    const wxPoint&   pos  /* = wxDefaultPosition */, 
                    const wxSize&    size /* = wxDefaultSize */,
                    unsigned int     featureFlags /* = hoxBOARD_FEATURE_ALL */ )
        : wxPanel( parent, 
                   wxID_ANY, 
                   pos, 
                   size,
                   wxFULL_REPAINT_ON_RESIZE )
        , m_coreBoard( NULL )
        , m_referee( referee )
        , m_pTable( pTable )
        , m_status( hoxGAME_STATUS_OPEN )
        , m_ownerId( ownerId )
        , m_featureFlags( featureFlags )
        , m_bRated( true )
		, m_timer( NULL )
        , m_bUICreated( false )
        , m_bSoundEnabled( true )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
    
    wxCHECK_RET( m_referee.get() != NULL, "A Referee must be set" );
    wxCHECK_RET( m_pTable.get() != NULL, "A Table must be set" );
    wxCHECK_RET( !m_ownerId.empty(), "An Owner must be set" );

    /* Create the core board. */
    m_coreBoard = new hoxCoreBoard( this, m_referee, bgColor, fgColor );
    m_coreBoard->SetBoardOwner( this );
    m_coreBoard->SetPiecesPath( piecesPath );

    /* Sync Info (Rated/Non-Rated + Timers) with Table's. */
    _SyncInfoWithTable();

    /* A timer to keep track of the time. */
    m_timer = new wxTimer( this );
    m_timer->Start( hoxTIME_ONE_SECOND_INTERVAL );

    /* Prepare sounds. */
    const wxString soundFile( wxString(SOUNDS_PATH) + "/" + "move.wav" );
    m_soundMove.Create( soundFile );
    wxASSERT_MSG(m_soundMove.IsOk(), "Failed to load sound file " + soundFile);

	// *** NOTE: By default, the Board is NOT visible.
    wxPanel::Show( false );  // invoke the parent's API.
}

hoxBoard::~hoxBoard()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    if ( m_timer != NULL )
    {
        if ( m_timer->IsRunning() )
            m_timer->Stop();

        delete m_timer;
        m_timer = NULL;
    }

    if ( m_coreBoard != NULL )
    {
        m_coreBoard->Close();
            /* NOTE: This has to be used instead of "delete" or "Destroy()"
             *       function to process pending events.
             *
             * See http://docs.wxwidgets.org/stable/wx_windowdeletionoverview.html#windowdeletionoverview
             */

        m_coreBoard = NULL;
    }
}

void 
hoxBoard::OnBoardMove( const hoxMove& move,
					   hoxGameStatus  status )
{
    _OnValidMove( move );

    /* Inform the Table of the new move. */

	const hoxTimeInfo playerTime = ( move.piece.color == hoxCOLOR_RED
		                            ? m_redTime
			    				    : m_blackTime );
    m_pTable->OnMove_FromBoard( move, 
		                        status,
			 				    playerTime );
}

void 
hoxBoard::OnBoardMsg( const wxString& message )
{
    const wxString who = "** Board **";  // NOTE: This Board's name.

    _PostToWallOutput( who, message ); 
}

bool
hoxBoard::OnBoardAskMovePermission( const hoxPieceInfo& pieceInfo )
{
    /* Check for Game-Status. */
    if (    m_status != hoxGAME_STATUS_READY 
         && m_status != hoxGAME_STATUS_IN_PROGRESS )
    {
        return false;
    }

    /* Check for the "next" Color. */
    if ( pieceInfo.color != m_referee->GetNextColor() )
    {
        return false;
    }

    return true;  // OK. The Piece can be moved.
}

void
hoxBoard::OnPlayersUIEvent( hoxPlayersUI::EventType eventType,
                            const wxString&         sPlayerId )
{
    switch ( eventType )
    {
        case hoxPlayersUI::EVENT_TYPE_INFO:
        {
            wxLogDebug("%s: Request Info for Player [%s]...", __FUNCTION__, sPlayerId.c_str());
            m_pTable->OnPlayerInfoRequest_FromBoard( sPlayerId );
            break;
        }
        case hoxPlayersUI::EVENT_TYPE_INVITE:
        {
            wxLogDebug("%s: Invite Player [%s]...", __FUNCTION__, sPlayerId.c_str());
            m_pTable->OnPlayerInviteRequest_FromBoard( sPlayerId );
            break;
        }
        default:
            wxLogDebug("%s: Unsupported eventType [%d].", __FUNCTION__, eventType);
            break;
    }
}

void 
hoxBoard::OnPlayerJoin( wxCommandEvent &event )
{
    hoxPlayerInfo_APtr apPlayerInfo( wxDynamicCast(event.GetEventObject(), hoxPlayerInfo) );
    wxCHECK_RET(apPlayerInfo.get(), "Player cannot be NULL.");

    const wxString playerId    = apPlayerInfo->id;
    const int      nPlayerRole = event.GetInt();
    hoxColor       playerColor = hoxCOLOR_UNKNOWN;

    if ( nPlayerRole == hoxCOLOR_RED )
    {
        playerColor = hoxCOLOR_RED;
        _SetRedInfo( apPlayerInfo.get() );
        if ( playerId == m_blackId ) _SetBlackInfo( NULL );
    } 
    else if ( nPlayerRole == hoxCOLOR_BLACK )
    {
        playerColor = hoxCOLOR_BLACK;
        _SetBlackInfo( apPlayerInfo.get() );
        if ( playerId == m_redId ) _SetRedInfo( NULL );
    }
    else
    {
        playerColor = hoxCOLOR_NONE;
        if ( playerId == m_redId )   _SetRedInfo( NULL );
        if ( playerId == m_blackId ) _SetBlackInfo( NULL );
    }

    /* Toggle the view if necessary. There two cases:
     *  (1) The owner just joined as RED, make sure the view is "normal".
     *  (2) The owner just joined as BLACK, make sure the view is "inverted".
     */
    if (   playerId == m_ownerId 
        && (   (playerColor == hoxCOLOR_RED && m_coreBoard->IsViewInverted())
             ||(playerColor == hoxCOLOR_BLACK && !m_coreBoard->IsViewInverted()) ) )
    {
        this->ToggleViewSide();
    }

    bool bNewlyJoined = _AddPlayerToList( playerId, apPlayerInfo->score );
    if ( bNewlyJoined )
    {
        const wxString sMessage = wxString::Format("%s joined", playerId.c_str());
        _PostToSystemOutput( sMessage );
    }

    /* Update the LOCAL - color on the core Board so that it knows
     * who is allowed to make a Move using the mouse.
     */

    hoxPlayerType playerType = apPlayerInfo->type;
    if ( playerType == hoxPLAYER_TYPE_LOCAL )
    {
        wxLogDebug("%s: Update the core Board's local-color to [%s].", 
            __FUNCTION__, hoxUtil::ColorToString(playerColor).c_str());
        m_coreBoard->SetLocalColor( playerColor );
    }

    _UpdateStatus(); // Update the game-status.
}

void 
hoxBoard::OnPlayerLeave( wxCommandEvent &event )
{
    hoxPlayerInfo_APtr apPlayerInfo( wxDynamicCast(event.GetEventObject(), hoxPlayerInfo) );
    wxCHECK_RET(apPlayerInfo.get(), "Player cannot be NULL.");

    const wxString playerId = apPlayerInfo->id;

    if ( playerId == m_redId )     // Check RED
    {
        _SetRedInfo( NULL );
    }
    else if ( playerId == m_blackId ) // Check BLACK
    {
        _SetBlackInfo( NULL );
    }

    bool bRemoved = _RemovePlayerFromList( playerId );
    if ( bRemoved )
    {
        const wxString sMessage = wxString::Format("%s left", playerId.c_str());
        _PostToSystemOutput( sMessage );
    }

    _UpdateStatus(); // Update the game-status.
}

void 
hoxBoard::OnPlayerScore( wxCommandEvent &event )
{
    hoxPlayerInfo_APtr apPlayerInfo( wxDynamicCast(event.GetEventObject(), hoxPlayerInfo) );
    wxCHECK_RET(apPlayerInfo.get(), "Player cannot be NULL.");

    const wxString playerId = apPlayerInfo->id;

    if ( playerId == m_redId )
    {
        _SetRedInfo( apPlayerInfo.get() );
    } 
    else if ( playerId == m_blackId )
    {
        _SetBlackInfo( apPlayerInfo.get() );
    } 

    /* NOTE: This action "add" can be used as an "update" action. */
    _AddPlayerToList( playerId, apPlayerInfo->score );
}

void 
hoxBoard::OnSystemOutput( wxCommandEvent &event )
{
    const wxString sMessage = event.GetString();
    _PostToSystemOutput( sMessage );
}

void 
hoxBoard::OnWallOutput( wxCommandEvent &event )
{
    const wxString eventString = event.GetString();
    const bool bPublic = (event.GetInt() > 0);
    const wxString who = eventString.BeforeFirst(' ');
    const wxString msg = eventString.AfterFirst(' ');

    _PostToWallOutput( who, msg, bPublic ); 
}

void 
hoxBoard::OnNewMove( wxCommandEvent &event )
{
    hoxMove  move;

    const wxString moveStr = event.GetString();
	const bool bSetupMode = (event.GetInt() > 0);

    move = m_referee->StringToMove( moveStr );
    if ( ! move.IsValid() )
    {
        wxLogError("%s: Failed to parse Move-string [%s].", __FUNCTION__, moveStr.c_str());
        return;
    }

	/* Ask the core Board to realize the Move */

    if ( ! m_coreBoard->DoMove( move ) )  // failed?
        return;

    _OnValidMove( move, bSetupMode );
}

void 
hoxBoard::OnDrawRequest( wxCommandEvent &event )
{
    const wxString playerId = event.GetString();
	const int bPopupRequest = event.GetInt(); // NOTE: force to boolean!

    const wxString boardMessage = playerId + " is offering a DRAW."; 
    this->OnBoardMsg( boardMessage );

    /* For observers, display the above Board message is enough. */
    if ( ! bPopupRequest )
        return;

    /* For the other player, popup the request... */

    const wxString confirmMessage = boardMessage + "\n" 
	                              + "Do you want to accept a Draw?";
    int answer = ::wxMessageBox(confirmMessage, "Confirmation",
							    wxYES_NO | wxCANCEL, this);
    if ( answer == wxYES )
    {
	    /* Inform the Table. */
	    m_pTable->OnDrawResponse_FromBoard( true );

	    /* Set Game's status to DRAW */
	    this->OnBoardMsg( "Accepted Draw request. Game drawn." ); 
	    m_coreBoard->SetGameOver( true );
    }
}

void 
hoxBoard::OnGameOver( wxCommandEvent &event )
{
	const int gameStatus = event.GetInt();
	
    const wxString boardMessage = _GetGameOverMessage( gameStatus );

	/* Display the status */
	m_status = (hoxGameStatus) gameStatus; // TODO: Force it!!!
	this->OnBoardMsg( boardMessage );
	m_coreBoard->SetGameOver( true );
}

void 
hoxBoard::OnGameReset( wxCommandEvent &event )
{
    m_coreBoard->ResetBoard();
    
    _SyncInfoWithTable();

    m_status = hoxGAME_STATUS_OPEN;
	_UpdateStatus();

	/* Display the "reset" message. */
    const wxString boardMessage = "Game Reset"; 
	this->OnBoardMsg( boardMessage );
}

void 
hoxBoard::OnTableUpdate( wxCommandEvent &event )
{
    /* Sync Info (Rated/Non-Rated + Timers) with Table's. */
    _SyncInfoWithTable();

	/* Display a notification message regarding the 
     * new Rated/Non-Rated Game option.
     */
    const hoxGameType gameType = ( m_bRated ? hoxGAME_TYPE_RATED 
                                            : hoxGAME_TYPE_NONRATED );
    wxString boardMessage =
        "Game-Type changed to " + hoxUtil::GameTypeToString( gameType ); 
	this->OnBoardMsg( boardMessage );

	/* Display a notification message regarding 
     * the new Timer.
     */
    boardMessage =
        "Timer changed to " + hoxUtil::TimeInfoToString( m_initialTime ); 
	this->OnBoardMsg( boardMessage );
}

void 
hoxBoard::OnWallInputEnter( wxCommandEvent &event )
{
    m_wallInput->Clear();
    m_pTable->OnMessage_FromBoard( event.GetString() );
}

void 
hoxBoard::OnButtonHistory_BEGIN( wxCommandEvent &event )
{
    m_coreBoard->DoGameReview_BEGIN();
}

void 
hoxBoard::OnButtonHistory_PREV( wxCommandEvent &event )
{
    m_coreBoard->DoGameReview_PREV();
}

void 
hoxBoard::OnButtonHistory_NEXT( wxCommandEvent &event )
{
    m_coreBoard->DoGameReview_NEXT();
}

void 
hoxBoard::OnButtonHistory_END( wxCommandEvent &event )
{
    m_coreBoard->DoGameReview_END();
}

void 
hoxBoard::OnButtonOptions( wxCommandEvent &event )
{
    unsigned int optionDlgFlags = 0;

    /* Player-Checking: Make sure the board Player satifies one of the 
     *                  following conditions:
	 *  (1) He is the RED player, or...
     *  (2) He is the BLACK player and there is no RED player.
	 */

    bool bChangeAllowed = (    m_ownerId == m_redId 
		                   || (m_ownerId == m_blackId && m_redId.empty()) );

    bool bGameNotStarted = (    m_status == hoxGAME_STATUS_OPEN 
                             || m_status == hoxGAME_STATUS_READY );


    if ( !bChangeAllowed || !bGameNotStarted )
    {
        optionDlgFlags |= hoxOptionDialog::hoxOPTION_READONLY_FLAG;
    }

    hoxOptionDialog optionDlg( this, wxID_ANY, "Table Options",
                               m_pTable, optionDlgFlags );
    optionDlg.ShowModal();

    hoxOptionDialog::CommandId selectedCommand = optionDlg.GetSelectedCommand();

    switch( selectedCommand )
    {
        case hoxOptionDialog::COMMAND_ID_SAVE:
        {
            const bool        bRatedGame  = optionDlg.IsRatedGame();
            const hoxTimeInfo newTimeInfo = optionDlg.GetNewTimeInfo();
            
            m_pTable->OnOptionsCommand_FromBoard( bRatedGame, 
                                                  newTimeInfo );
            break;
        }
        default:
            // No command is selected. Fine.
            break;
    }
}

void 
hoxBoard::OnButtonResign( wxCommandEvent &event )
{
    m_pTable->OnResignCommand_FromBoard();
}

void 
hoxBoard::OnButtonDraw( wxCommandEvent &event )
{
    m_pTable->OnDrawCommand_FromBoard();
}

void 
hoxBoard::OnButtonReset( wxCommandEvent &event )
{
    m_pTable->OnResetCommand_FromBoard();
}

void 
hoxBoard::OnButtonRed( wxCommandEvent &event )
{
    m_pTable->OnJoinCommand_FromBoard( hoxCOLOR_RED );
}

void 
hoxBoard::OnButtonBlack( wxCommandEvent &event )
{
    m_pTable->OnJoinCommand_FromBoard( hoxCOLOR_BLACK );
}

void 
hoxBoard::OnButtonNone( wxCommandEvent &event )
{
    m_pTable->OnJoinCommand_FromBoard( hoxCOLOR_NONE );
}

void
hoxBoard::OnUpdateUI_ActionRed( wxUpdateUIEvent& event )
{
	bool bMenuEnabled = (  m_status != hoxGAME_STATUS_IN_PROGRESS
                        && !_IsOwnerSeated()
                        && m_redId.empty() );
    event.Enable( bMenuEnabled );
}

void
hoxBoard::OnUpdateUI_ActionBlack( wxUpdateUIEvent& event )
{
	bool bMenuEnabled = (  m_status != hoxGAME_STATUS_IN_PROGRESS
                        && !_IsOwnerSeated()
                        && m_blackId.empty() );
    event.Enable( bMenuEnabled );
}

void
hoxBoard::OnUpdateUI_ActionNone( wxUpdateUIEvent& event )
{
	bool bMenuEnabled = ( m_status != hoxGAME_STATUS_IN_PROGRESS
                        && _IsOwnerSeated() );
    event.Enable( bMenuEnabled );
}

void
hoxBoard::OnUpdateUI_ActionResign( wxUpdateUIEvent& event )
{
	bool bMenuEnabled = ( m_status == hoxGAME_STATUS_IN_PROGRESS
                        && _IsOwnerSeated() );
    event.Enable( bMenuEnabled );
}

void
hoxBoard::OnUpdateUI_ActionDraw( wxUpdateUIEvent& event )
{
	bool bMenuEnabled = ( m_status == hoxGAME_STATUS_IN_PROGRESS
                        && _IsOwnerSeated() );
    event.Enable( bMenuEnabled );
}

void
hoxBoard::OnUpdateUI_ActionReset( wxUpdateUIEvent& event )
{
	bool bMenuEnabled = ( m_status != hoxGAME_STATUS_IN_PROGRESS
                        && _IsOwnerSeated()
                        && (m_featureFlags & hoxBOARD_FEATURE_RESET) );
    event.Enable( bMenuEnabled );
}

void 
hoxBoard::OnTimer( wxTimerEvent& event )
{
    if ( m_status != hoxGAME_STATUS_IN_PROGRESS )
        return;

    const hoxColor nextColor = m_referee->GetNextColor();

    if ( nextColor == hoxCOLOR_BLACK )
    {
		if ( m_blackTime.nGame > 0 ) --m_blackTime.nGame;
		if ( m_blackTime.nMove > 0 ) --m_blackTime.nMove;
    }
    else
    {
		if ( m_redTime.nGame > 0 ) --m_redTime.nGame;
		if ( m_redTime.nMove > 0 ) --m_redTime.nMove;
    }

    _OnTimerUpdated();
}

bool 
hoxBoard::Show( bool show /* = true */ )
{
    if ( !this->IsShown() && show ) // hidden -> shown?
    {
        /* Ask the board to display the pieces. */
        m_coreBoard->LoadPieces();

        /* Create the whole panel with player-info + timers */
        _CreateBoardPanel();

        /* Indicate that all UI elements have been created. */
        m_bUICreated = true;

        /* Set its background color. */
        wxColour bgPanelCol = wxTheColourDatabase->Find(_T("SKY BLUE"));
        if ( bgPanelCol.Ok() ) {
            this->SetBackgroundColour(bgPanelCol);
        }

        /* NOTE: Force Timer Updated event!!! */
        _OnTimerUpdated();
    }

    return wxPanel::Show( show );  // invoke the parent's API.
}

void 
hoxBoard::_SetRedInfo( const hoxPlayerInfo* playerInfo )
{
    if ( ! m_bUICreated ) return;

    wxString info;

    if ( playerInfo != NULL )
    {
        m_redId = playerInfo->id;
        info = wxString::Format("%s (%d)", m_redId.c_str(), playerInfo->score);
    }
    else
    {
        m_redId = "";
        info = "*";
    }

    m_redInfo->SetLabel( info );
}

void 
hoxBoard::_SetBlackInfo( const hoxPlayerInfo* playerInfo )
{
    if ( ! m_bUICreated ) return;

    wxString info;

    if ( playerInfo != NULL )
    {
        m_blackId = playerInfo->id;
        info = wxString::Format("%s (%d)", m_blackId.c_str(), playerInfo->score);
    }
    else
    {
        m_blackId = "";
        info = "*";
    }

    m_blackInfo->SetLabel( info );
}

/*
 * Create panel with the core board + player-info(s) + timers
 */
void
hoxBoard::_CreateBoardPanel()
{
    wxPanel* boardPanel = this;

    /*********************************
     * Create players' info + timers 
     *********************************/

    m_btnPlayRed = new wxButton( boardPanel, ID_ACTION_RED, "Play RED", 
                                 wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    m_btnPlayBlack = new wxButton( boardPanel, ID_ACTION_BLACK, "Play BLACK", 
                                   wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );

    // Create players' info.
    m_blackInfo = new wxStaticText( boardPanel, wxID_ANY, "*", 
        wxDefaultPosition, wxSize(200,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);
    m_redInfo = new wxStaticText( boardPanel, wxID_ANY, "*", 
        wxDefaultPosition, wxSize(200,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);

    // Create players' game-time.
    m_blackGameTime = new wxStaticText( boardPanel, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);
    m_redGameTime = new wxStaticText( boardPanel, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);

    m_blackMoveTime = new wxStaticText( boardPanel, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);
    m_redMoveTime = new wxStaticText( boardPanel, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);

    m_blackFreeTime = new wxStaticText( boardPanel, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);
    m_redFreeTime = new wxStaticText( boardPanel, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);

    /*********************************
     * Create History's buttons.
     *********************************/

    m_historySizer = new wxBoxSizer( wxHORIZONTAL );

    m_historySizer->Add( 
        new wxButton( boardPanel, ID_HISTORY_BEGIN, "|<", 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        0,    // Unstretchable
        wxALIGN_CENTER | wxFIXED_MINSIZE );

    m_historySizer->Add( 
        new wxButton( boardPanel, ID_HISTORY_PREV, "<",
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        0,    // Unstretchable
        wxALIGN_CENTER | wxFIXED_MINSIZE );

    m_historySizer->Add( 
        new wxButton( boardPanel, ID_HISTORY_NEXT, ">", 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        0,    // Unstretchable
        wxALIGN_CENTER | wxFIXED_MINSIZE );

    m_historySizer->Add( 
        new wxButton( boardPanel, ID_HISTORY_END, ">|", 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        0,    // Unstretchable
        wxALIGN_CENTER | wxFIXED_MINSIZE );

    /*********************************
     * Create Action's buttons.
     *********************************/

    m_actionSizer = new wxBoxSizer( wxHORIZONTAL );

    m_actionSizer->Add( 
        new wxButton( boardPanel, ID_ACTION_OPTIONS, "Options", 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        0,    // Unstretchable
        wxALIGN_LEFT | wxFIXED_MINSIZE );

    m_actionSizer->AddSpacer( 20 );  // Add some spaces in between.

    m_actionSizer->Add( 
        new wxButton( boardPanel, ID_ACTION_RESIGN, "Resign", 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        0,    // Unstretchable
        wxALIGN_LEFT | wxFIXED_MINSIZE );

    m_actionSizer->Add( 
        new wxButton( boardPanel, ID_ACTION_DRAW, "Draw", 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        0,    // Unstretchable
        wxALIGN_LEFT | wxFIXED_MINSIZE );

    m_actionSizer->Add( 
        new wxButton( boardPanel, ID_ACTION_RESET, "Reset", 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        0,    // Unstretchable
        wxALIGN_LEFT | wxFIXED_MINSIZE );

    m_actionSizer->Add( 
        new wxButton( boardPanel, ID_ACTION_NONE, "Unsit", 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        0,    // Unstretchable
        wxALIGN_LEFT | wxFIXED_MINSIZE );

    /*********************************
     * Create Command's buttons (History + Action).
     *********************************/

    m_commandSizer = new wxBoxSizer( wxHORIZONTAL );
    m_commandSizer->Add( m_historySizer, wxSizerFlags().Border(1) );
	m_commandSizer->AddStretchSpacer();
    m_commandSizer->Add( m_actionSizer, wxSizerFlags().Border(1) );

    /*********************************
     * Create Wall's contents.
     *********************************/

	m_playerListBox = new hoxPlayersUI( this );
    m_playerListBox->SetOwner( this );

    m_systemOutput = new wxTextCtrl( boardPanel, wxID_ANY, _T(""),
                                     wxDefaultPosition, wxDefaultSize,
                                     wxTE_MULTILINE | wxRAISED_BORDER | wxTE_READONLY 
                                     | wxHSCROLL | wxTE_RICH /* needed for Windows */ );

    m_wallOutput = new wxTextCtrl( boardPanel, wxID_ANY, _T(""),
                                   wxDefaultPosition, wxDefaultSize,
                                   wxTE_MULTILINE | wxRAISED_BORDER | wxTE_READONLY 
                                   | wxHSCROLL | wxTE_RICH /* needed for Windows */ );

    m_wallInput  = new wxTextCtrl( boardPanel, ID_BOARD_WALL_INPUT, _T(""),
                                   wxDefaultPosition, wxDefaultSize,
                                   wxTE_PROCESS_ENTER | wxSUNKEN_BORDER );

    /****************************************
     * Arrange the players' info + timers 
     ****************************************/

    // Sizers
    m_mainSizer  = new wxBoxSizer( wxHORIZONTAL );
    m_boardSizer = new wxBoxSizer( wxVERTICAL );
    m_wallSizer  = new wxBoxSizer( wxVERTICAL );
    
    m_blackSizer = new wxBoxSizer( wxHORIZONTAL );
    m_redSizer = new wxBoxSizer( wxHORIZONTAL );

    // Add Black player-info
    m_blackSizer->Add(
        m_btnPlayBlack,
        1,            // make vertically stretchable
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    m_blackSizer->Add(
        m_blackInfo,
        1,            // make vertically stretchable
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    m_blackSizer->Add(
        m_blackGameTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    m_blackSizer->Add(
        m_blackMoveTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    m_blackSizer->Add(
        m_blackFreeTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    // Add Red player-info

    m_redSizer->Add(
        m_btnPlayRed,
        1,            // make vertically stretchable
        wxEXPAND |    // make horizontally stretchable
        wxBOTTOM|wxRIGHT|wxLEFT,  //   and make border
        1 );         // set border width

    m_redSizer->Add(
        m_redInfo,
        1,            // make vertically stretchable
        wxEXPAND |    // make horizontally stretchable
        wxBOTTOM|wxRIGHT|wxLEFT,  //   and make border
        1 );         // set border width

    m_redSizer->Add(
        m_redGameTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    m_redSizer->Add(
        m_redMoveTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    m_redSizer->Add(
        m_redFreeTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    // Invert view if required.

    bool viewInverted = m_coreBoard->IsViewInverted();
    _LayoutBoardPanel( viewInverted);

    // Setup the Wall.

    m_wallSizer->Add(
        m_playerListBox,
        1,            // Proportion
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT, // and make border
        1 );         // set border width

    m_wallSizer->Add(
        m_systemOutput,
        1,            // Proportion
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT, // and make border
        1 );         // set border width

    m_wallSizer->Add(
        m_wallOutput,
        3,            // Proportion
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT, // and make border
        1 );         // set border width

    m_wallSizer->Add(
        m_wallInput,
        0,            // fixed-size vertically
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT, // and make border
        1 );         // set border width

    // Setup main sizer.

    m_mainSizer->Add(
        m_boardSizer,
        0,            // fixed size
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    m_mainSizer->Add(
        m_wallSizer,
        1,            // proportion
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    /* Setup the main size */
    boardPanel->SetSizer( m_mainSizer );      // use the sizer for layout
}

void 
hoxBoard::_LayoutBoardPanel( bool viewInverted )
{
    wxSizer* topSizer = NULL;
    wxSizer* bottomSizer = NULL;

    if ( ! viewInverted ) // normal view?
    {
        topSizer = m_blackSizer;
        bottomSizer = m_redSizer;
    }
    else                  // inverted view?
    {
        topSizer = m_redSizer;
        bottomSizer = m_blackSizer;
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // HACK: Adjust the "core" Board's height!!!
    const int bestHeightAdjust = (m_btnPlayRed->GetSize().GetHeight()+2) * 3;
    m_coreBoard->SetBestHeightAdjustment( bestHeightAdjust );
    wxLogDebug("%s: ADJUST Core Board's Height: (%d)", __FUNCTION__, bestHeightAdjust);
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // Add the top-sizer...
    m_boardSizer->Add(
        topSizer,
        0,            // fixed-size vertically
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT, // and make border
        1 );         

    // Add the main board...
    m_boardSizer->Add(
        m_coreBoard,
        1,            // make vertically stretchable
        wxEXPAND |    // make horizontally stretchable
        wxALL,        // and make border all around
        1 );          // set border width

    // Add the bottom-sizer...
    m_boardSizer->Add(
        bottomSizer,
        0,            // fixed-size vertically
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT,  // and make border
        1 );          // set border width

    // Add the command-sizer (History + Action)...
    m_boardSizer->Add(
        m_commandSizer,
        0,            // fixed-size vertically
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT,  // and make border
        1 );          // set border width
}

void 
hoxBoard::ToggleViewSide()
{
    /* If the view has NOT been created, do nothing.
     * Otherwise, invert it. 
     */

    if ( ! m_bUICreated ) return;

    /* Invert the "core" board. */
    bool bViewInverted = m_coreBoard->ToggleViewSide();

    /* Detach the sizers */

    m_boardSizer->Detach( m_redSizer );
    m_boardSizer->Detach( m_coreBoard );
    m_boardSizer->Detach( m_blackSizer );
    m_boardSizer->Detach( m_commandSizer );

    /* Invert the Board's Panel. */

    _LayoutBoardPanel( bViewInverted );

    /* Force the layout update (just to make sure!). */
    m_boardSizer->Layout();
}

void
hoxBoard::SetBgColor( wxColor color )
{
    m_coreBoard->SetBgColor( color );
}

void
hoxBoard::SetFgColor( wxColor color )
{
    m_coreBoard->SetFgColor( color );
}

bool 
hoxBoard::_AddPlayerToList( const wxString& playerId,
                            const int       playerScore )
{
    if ( ! m_bUICreated ) return false;

    return m_playerListBox->AddPlayer( playerId, playerScore );
}

bool 
hoxBoard::_RemovePlayerFromList( const wxString& playerId )
{
    if ( ! m_bUICreated ) return false;

    return m_playerListBox->RemovePlayer( playerId );
}

void 
hoxBoard::_PostToSystemOutput( const wxString& sMessage )
{
    m_systemOutput->SetDefaultStyle( wxTextAttr(*wxBLUE) );
    m_systemOutput->AppendText( wxString::Format("*%s\n", sMessage.c_str()) );

    /* NOTE:
     *    Make sure that the last line is at the bottom of the wxTextCtrl
     *    so that new messages are visiable to the Player.
     *    This technique was learned from the following site:
     *        http://wiki.wxwidgets.org/WxTextCtrl#Scrolling
     */
    m_systemOutput->ScrollLines(1);
}

void 
hoxBoard::_PostToWallOutput( const wxString& who,
                             const wxString& sMessage,
                             bool            bPublic /* = true */ )
{
    m_wallOutput->SetDefaultStyle( wxTextAttr(*wxBLACK) );
    m_wallOutput->AppendText( wxString::Format("[%s] ", who.c_str()) );
    m_wallOutput->SetDefaultStyle( wxTextAttr( bPublic ? *wxBLUE : *wxRED) );
    m_wallOutput->AppendText( wxString::Format("%s\n", sMessage.c_str()) );

    /* NOTE:
     *    Make sure that the last line is at the bottom of the wxTextCtrl
     *    so that new messages are visiable to the Player.
     *    This technique was learned from the following site:
     *        http://wiki.wxwidgets.org/WxTextCtrl#Scrolling
     */
    m_wallOutput->ScrollLines(1);
}

void 
hoxBoard::_OnValidMove( const hoxMove& move,
					    bool           bSetupMode /* = false */ )
{
    /* For the 1st move, change the game-status to 'in-progress'. 
     */
    if ( m_status == hoxGAME_STATUS_READY )
    {
        m_status = hoxGAME_STATUS_IN_PROGRESS;
        /* NOTE: The above action is enough to trigger the timer which will
         *       update the timer-related UI.
         */
    }
    /* If the game is in progress, reset the Move-time after each Move.
     */
    else if ( m_status == hoxGAME_STATUS_IN_PROGRESS )
    {
		if ( bSetupMode )
		{
			return;
		}

		// NOTE: For Chesscape server, the Free time is rewarded to the Player after
		//       each Move.  Also, there is such thing as Move time in Chesscape.
		//       Thus, we can detect whether Move time == 0 to indicate that this is
		//       a Chesscape server.  If so, the Free time is added to the Game time.

        // TODO: Need to handle the case in which the Game time is
        //       less than Move time or is zero.

		bool bIsChesscape = (m_initialTime.nMove == 0);

        if ( move.piece.color == hoxCOLOR_BLACK )
		{
            m_redTime.nMove = m_initialTime.nMove;
			if ( bIsChesscape ) m_blackTime.nGame += m_initialTime.nFree;
		}
        else
		{
            m_blackTime.nMove = m_initialTime.nMove;
			if ( bIsChesscape ) m_redTime.nGame += m_initialTime.nFree;
		}
    }

    /* Play the sound for the MOVE. */
    if ( m_bSoundEnabled )
    {
        m_soundMove.Play();   
    }
}

void
hoxBoard::_UpdateStatus()
{
    /* Update Table-Status based on Player availability. */

    if ( m_status == hoxGAME_STATUS_OPEN )
    {
        if ( !m_redId.empty() && !m_blackId.empty() )
        {
            m_status = hoxGAME_STATUS_READY;
        }
    }
    else if ( m_status == hoxGAME_STATUS_READY )
    {
        if ( m_redId.empty() || m_blackId.empty() )
        {
            m_status = hoxGAME_STATUS_OPEN;
        }
    }
}

void
hoxBoard::_SyncInfoWithTable()
{
    /* Sync Rated/Non-Rated Game option with Table's. */
    
    m_bRated = ( m_pTable->GetGameType() == hoxGAME_TYPE_RATED );

    /* Sync timer with Table's. */

	m_initialTime = m_pTable->GetInitialTime();
	m_blackTime   = m_pTable->GetBlackTime();
	m_redTime     = m_pTable->GetRedTime();

    _OnTimerUpdated();  // Update UI.
}

void 
hoxBoard::_OnTimerUpdated()
{
    if ( ! m_bUICreated ) return;

    // Game times.
	m_blackGameTime->SetLabel( hoxUtil::FormatTime( m_blackTime.nGame ) );
    m_redGameTime->SetLabel(   hoxUtil::FormatTime( m_redTime.nGame ) );

    // Move times.
	m_blackMoveTime->SetLabel( hoxUtil::FormatTime( m_blackTime.nMove ) );
	m_redMoveTime->SetLabel(   hoxUtil::FormatTime( m_redTime.nMove ) );

    // Free times.
	m_blackFreeTime->SetLabel( hoxUtil::FormatTime( m_blackTime.nFree ) );
	m_redFreeTime->SetLabel(   hoxUtil::FormatTime( m_redTime.nFree ) );
}

wxString
hoxBoard::_GetGameOverMessage( const int gameStatus ) const
{
	wxString boardMessage; 

	switch ( gameStatus )
	{
		case hoxGAME_STATUS_RED_WIN:
		{
			boardMessage = "Game Over. " + m_redId + " won."; 
			break;
		}
		case hoxGAME_STATUS_BLACK_WIN:
		{
			boardMessage = "Game Over. " + m_blackId + " won."; 
			break;
		}
		case hoxGAME_STATUS_DRAWN:
		{
			boardMessage = "Game drawn."; 
			break;
		}
		default:
        {
			wxLogDebug("%s: Unsupported game-status [%d].", __FUNCTION__, gameStatus );
            boardMessage = "Game over: !!! UNKNOWN !!!";
        }
	}

    return boardMessage;
}

bool
hoxBoard::_IsOwnerSeated() const
{
    return (    m_ownerId == m_redId 
             || m_ownerId == m_blackId );
}

/************************* END OF FILE ***************************************/
