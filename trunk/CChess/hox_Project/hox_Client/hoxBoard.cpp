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
#include "hoxEnums.h"
#include "hoxUtility.h"
#include "hoxReferee.h"
#include "hoxTypes.h"
#include "hoxTable.h"
#include "hoxPlayer.h"

/* UI-related IDs. */
enum
{
    ID_BOARD_WALL_INPUT = (wxID_HIGHEST+1),

    ID_HISTORY_BEGIN,
    ID_HISTORY_PREV,
    ID_HISTORY_NEXT,
    ID_HISTORY_END,

    ID_ACTION_JOIN
};


/* Define my custom events */
DEFINE_EVENT_TYPE( hoxEVT_BOARD_PLAYER_JOIN )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_PLAYER_LEAVE )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_WALL_OUTPUT )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_NEW_MOVE )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_PLAYER_ACTION )

BEGIN_EVENT_TABLE(hoxBoard, wxPanel)
    EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_PLAYER_JOIN, hoxBoard::OnPlayerJoin)
    EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_PLAYER_LEAVE, hoxBoard::OnPlayerLeave)
    EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_WALL_OUTPUT, hoxBoard::OnWallOutput)
	EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_NEW_MOVE, hoxBoard::OnNewMove)
	EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_PLAYER_ACTION, hoxBoard::OnPlayerAction)

    EVT_TEXT_ENTER(ID_BOARD_WALL_INPUT, hoxBoard::OnWallInputEnter)
    EVT_BUTTON(ID_HISTORY_BEGIN, hoxBoard::OnButtonHistory_BEGIN)
    EVT_BUTTON(ID_HISTORY_PREV, hoxBoard::OnButtonHistory_PREV)
    EVT_BUTTON(ID_HISTORY_NEXT, hoxBoard::OnButtonHistory_NEXT)
    EVT_BUTTON(ID_HISTORY_END, hoxBoard::OnButtonHistory_END)
	EVT_BUTTON(ID_ACTION_JOIN, hoxBoard::OnButtonJoin)

    EVT_TIMER(wxID_ANY, hoxBoard::OnTimer)    
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// hoxBoard
// ----------------------------------------------------------------------------

hoxBoard::hoxBoard( wxWindow*       parent,
                    const wxString& piecesPath,
                    hoxIReferee*    referee,
                    const wxPoint&  pos  /* = wxDefaultPosition */, 
                    const wxSize&   size /* = wxDefaultSize */)
        : wxPanel( parent, 
                   wxID_ANY, 
                   pos, 
                   size,
                   wxFULL_REPAINT_ON_RESIZE )
        , m_coreBoard( NULL )
        , m_referee( referee )
        , m_table( NULL )
        , m_status( hoxGAME_STATUS_OPEN )
		, m_timer( NULL )
{
    wxCHECK_RET( m_referee != NULL, "A Referee must be set." );

    /* Create the core board. */
    m_coreBoard = new hoxCoreBoard( this, m_referee );
    m_coreBoard->SetBoardOwner( this );
    m_coreBoard->SetPiecesPath( piecesPath );

	// *** NOTE: By default, the Board is NOT visible.
    wxPanel::Show( false );  // invoke the parent's API.
}

hoxBoard::~hoxBoard()
{
    if ( m_timer != NULL )
    {
        if ( m_timer->IsRunning() )
            m_timer->Stop();

        delete m_timer;
        m_timer = NULL;
    }

    delete m_coreBoard;
}

void 
hoxBoard::OnBoardMove( const hoxMove& move,
					   hoxGameStatus  status )
{
    _OnValidMove( move );

    /* Inform the Table of the new move. */
    wxCHECK_RET(m_table, "The table is NULL." );
	const hoxTimeInfo playerTime = ( move.piece.color == hoxPIECE_COLOR_RED
		                         ? m_redTime
			    				 : m_blackTime );
    m_table->OnMove_FromBoard( move, 
		                       status,
							   playerTime );
}

void 
hoxBoard::OnBoardMsg( const wxString& message )
{
    const wxString who = "** Board **";  // NOTE: This Board's name.

    _PostToWallOutput( who, message ); 
}

void 
hoxBoard::OnPlayerJoin( wxCommandEvent &event )
{
    const char* FNAME = "hoxBoard::OnPlayerJoin";

    hoxPlayer* player = wx_reinterpret_cast(hoxPlayer*, event.GetEventObject());
    wxCHECK_RET(player, "Player cannot be NULL.");

    hoxPieceColor playerColor = hoxPIECE_COLOR_NONE;

    if ( event.GetInt() == hoxPIECE_COLOR_RED )
    {
        playerColor = hoxPIECE_COLOR_RED;
        _SetRedInfo( player );
    } 
    else if ( event.GetInt() == hoxPIECE_COLOR_BLACK )
    {
        playerColor = hoxPIECE_COLOR_BLACK;
        _SetBlackInfo( player );
    }

    _AddPlayerToList( player->GetName(), player->GetScore() );

    wxCHECK_RET( m_coreBoard != NULL, "The core Board is NULL." );

    /* Update the LOCAL - color on the core Board so that it knows
     * who is allowed to make a Move using the mouse.
     */

    hoxPlayerType playerType = player->GetType();
    if ( playerType == hoxPLAYER_TYPE_LOCAL )
    {
        wxLogDebug("%s: Update the core Board's local-color to [%d].", 
            FNAME, playerColor);
        m_coreBoard->SetLocalColor( playerColor );
    }

    /* Start the game if there are a RED and a BLACK players */

    if (  !m_redId.empty() && !m_blackId.empty()
        && m_status == hoxGAME_STATUS_OPEN )
    {
        m_status = hoxGAME_STATUS_READY;

        _ResetTimerUI();
        _UpdateTimerUI();

        m_coreBoard->StartGame();
    }
}

void 
hoxBoard::OnPlayerLeave( wxCommandEvent &event )
{
    const char* FNAME = "hoxBoard::OnPlayerLeave";

    hoxPlayer* player = wx_reinterpret_cast(hoxPlayer*, event.GetEventObject());
    wxCHECK_RET(player, "Player cannot be NULL.");

    const wxString playerId = player->GetName();

    if ( playerId == m_redId )     // Check RED
    {
        m_redInfo->SetLabel( "*" );
    }
    else if ( playerId == m_blackId ) // Check BLACK
    {
        m_blackInfo->SetLabel( "*" ); // Check Observers
    }
    else
    {
        m_observerIds.remove( playerId );
    }

    _RemovePlayerFromList( playerId );
}

void 
hoxBoard::OnWallOutput( wxCommandEvent &event )
{
    const wxString eventString = event.GetString();
    const wxString who = eventString.BeforeFirst(' ');
    const wxString msg = eventString.AfterFirst(' ');

    _PostToWallOutput( who, msg ); 
}

void 
hoxBoard::OnNewMove( wxCommandEvent &event )
{
	const char* FNAME = "hoxBoard::OnNewMove";
    hoxMove  move;

    const wxString moveStr = event.GetString();
	const bool bSetupMode = (event.GetInt() > 0);

    if ( ! _ParseMoveString( moveStr, move ) ) // failed?
    {
        wxLogError("%s: Failed to parse Move-string [%s].", FNAME, moveStr.c_str());
        return;
    }

	/* Ask the core Board to realize the Move */

    wxCHECK_RET( m_coreBoard, "The core Board is NULL." );

    if ( ! m_coreBoard->DoMove( move ) )  // failed?
        return;

    _OnValidMove( move, bSetupMode );
}

void 
hoxBoard::OnPlayerAction( wxCommandEvent &event )
{
    const char* FNAME = "hoxBoard::OnPlayerAction";

    hoxPlayer* player = wx_reinterpret_cast(hoxPlayer*, event.GetEventObject());
    wxCHECK_RET(player, "Player cannot be NULL.");

    const wxString playerId = player->GetName();
	const hoxActionType action = (hoxActionType) event.GetInt();
	
	switch ( action )
	{
		case hoxACTION_TYPE_RESIGN:
		{
			const wxString message = playerId + " resigned."; 
			this->OnBoardMsg( message );
			m_coreBoard->SetGameOver( true );
			m_status = ( playerId == m_redId ? hoxGAME_STATUS_BLACK_WIN 
				                             : hoxGAME_STATUS_RED_WIN );
			break;
		}
		default:
			wxLogDebug("%s: Unsupported action [%d].", action );
	}
}

void 
hoxBoard::OnWallInputEnter( wxCommandEvent &event )
{
    m_wallInput->Clear();
    m_table->OnMessage_FromBoard( event.GetString() );
}

void 
hoxBoard::OnButtonHistory_BEGIN( wxCommandEvent &event )
{
    if ( m_coreBoard == NULL )
        return;

    m_coreBoard->DoGameReview_BEGIN();
}

void 
hoxBoard::OnButtonHistory_PREV( wxCommandEvent &event )
{
    if ( m_coreBoard == NULL )
        return;

    m_coreBoard->DoGameReview_PREV();
}

void 
hoxBoard::OnButtonHistory_NEXT( wxCommandEvent &event )
{
    if ( m_coreBoard == NULL )
        return;

    m_coreBoard->DoGameReview_NEXT();
}

void 
hoxBoard::OnButtonHistory_END( wxCommandEvent &event )
{
    if ( m_coreBoard == NULL )
        return;

    m_coreBoard->DoGameReview_END();
}

void 
hoxBoard::OnButtonJoin( wxCommandEvent &event )
{
    /* Let the table handle this action. */
    wxCHECK_RET(m_table, "The table is NULL." );
    m_table->OnJoinCommand_FromBoard();
}

void 
hoxBoard::OnTimer(wxTimerEvent& event)
{
    if ( m_status != hoxGAME_STATUS_IN_PROGRESS )
        return;

    hoxPieceColor nextColor = m_referee->GetNextColor();

    if ( nextColor == hoxPIECE_COLOR_BLACK )
    {
		if ( m_blackTime.nGame > 0 ) --m_blackTime.nGame;
		if ( m_blackTime.nMove > 0 ) --m_blackTime.nMove;
    }
    else
    {
		if ( m_redTime.nGame > 0 ) --m_redTime.nGame;
		if ( m_redTime.nMove > 0 ) --m_redTime.nMove;
    }

    _UpdateTimerUI();
}

bool 
hoxBoard::Show(bool show /* = true */)
{
    if ( !this->IsShown() && show ) // hidden -> shown?
    {
        /* Ask the board to display the pieces. */
        m_coreBoard->LoadPieces();

        /* Create the whole panel with player-info + timers */
        _CreateBoardPanel();

        /* Set its background color. */
        wxColour bgPanelCol = wxTheColourDatabase->Find(_T("SKY BLUE"));
        if ( bgPanelCol.Ok() ) {
            this->SetBackgroundColour(bgPanelCol);
        }

		/* Display timer. */
		_UpdateTimerUI();
    }

    return wxPanel::Show( show );  // invoke the parent's API.
}

void 
hoxBoard::_SetRedInfo( const hoxPlayer* player )
{
    m_redId = player->GetName();

    if ( this->IsShown() )
    {
        const wxString info = wxString::Format("%s (%d)", 
                player->GetName().c_str(), player->GetScore());
        m_redInfo->SetLabel( info );
    }
}

void 
hoxBoard::_SetBlackInfo( const hoxPlayer* player )
{
    m_blackId = player->GetName();

    if ( this->IsShown() )
    {
        const wxString info = wxString::Format("%s (%d)", 
                player->GetName().c_str(), player->GetScore());
        m_blackInfo->SetLabel( info );
    }
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
        new wxButton( boardPanel, ID_ACTION_JOIN, "Join", 
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

    m_playerListBox = new wxListBox( boardPanel, wxID_ANY );

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
        1,            // fixed-size vertically
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT, // and make border
        1 );         // set border width

    m_wallSizer->Add(
        m_wallOutput,
        3,            // fixed-size vertically
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
    //m_mainSizer->SetSizeHints( boardPanel );   // set size hints to honour minimum size
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
hoxBoard::SetTable( hoxTable* table ) 
{ 
    m_table = table;

    /* A timer to keep track of the time. */
    m_timer = new wxTimer( this );
    m_timer->Start( hoxTIME_ONE_SECOND_INTERVAL );

    /* Set default game-times. */
    _ResetTimerUI();
}

void 
hoxBoard::ToggleViewSide()
{
    const char* FNAME = "hoxBoard::ToggleViewSide";

    m_coreBoard->ToggleViewSide();

    /* Invert view if the view has been displayed. 
     * Right now, assume that if red-info text has been created,
     * then the view has been displayed.
     */

    if ( m_redInfo == NULL )
    {
        wxLogDebug("%s: View not yet created. Do nothing for info-panels.", FNAME);
        return;
    }

    /* Detach the sizers */

    bool found;

    found = m_boardSizer->Detach( m_redSizer );
    wxASSERT( found );
    found = m_boardSizer->Detach( m_coreBoard );
    wxASSERT( found );
    found = m_boardSizer->Detach( m_blackSizer );
    wxASSERT( found );
    found = m_boardSizer->Detach( m_commandSizer );
    wxASSERT( found );

    /* Invert */

    bool viewInverted = m_coreBoard->IsViewInverted();
    _LayoutBoardPanel( viewInverted );

    // Force the layout update (just to make sure!).
    m_boardSizer->Layout();
}

void 
hoxBoard::_AddPlayerToList( const wxString& playerId,
                            int             playerScore )
{
    const char* FNAME = "hoxBoard::_AddPlayerToList";

    if ( ! this->IsShown() )
    {
        wxLogDebug("%s: Board is not shown. Do nothing.", FNAME);
        return;
    }

	/* Remove the old item, if any. */
	_RemovePlayerFromList( playerId );

    const wxString info = wxString::Format("%s (%d)", playerId.c_str(), playerScore);
    m_playerListBox->Append( info );
}

void 
hoxBoard::_RemovePlayerFromList( const wxString& playerId )
{
    const char* FNAME = "hoxBoard::_RemovePlayerFromList";

    if ( ! this->IsShown() )
    {
        wxLogDebug("%s: Board is not shown. Do nothing.", FNAME);
        return;
    }

    int idCount = m_playerListBox->GetCount();

    for ( int i = 0; i < idCount; ++i )
    {
        if ( m_playerListBox->GetString(i).StartsWith(playerId) )
        {
            m_playerListBox->Delete( i );
            break;
        }
    }
}

void 
hoxBoard::_PostToWallOutput( const wxString& who,
                             const wxString& message )
{
    m_wallOutput->SetDefaultStyle( wxTextAttr(*wxBLACK) );
    m_wallOutput->AppendText( wxString::Format("[%s] ", who.c_str()) );
    m_wallOutput->SetDefaultStyle( wxTextAttr(*wxBLUE) );
    m_wallOutput->AppendText( wxString::Format("%s\n", message.c_str()) );
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

		bool bIsChesscape = (m_initialTime.nMove == 0);

        if ( move.piece.color == hoxPIECE_COLOR_BLACK )
		{
			m_blackTime.nMove = m_initialTime.nMove;
			if ( bIsChesscape ) m_blackTime.nGame += m_initialTime.nFree;
		}
        else
		{
            m_redTime.nMove = m_initialTime.nMove;
			if ( bIsChesscape ) m_redTime.nGame += m_initialTime.nFree;
		}
    }
}

void
hoxBoard::_ResetTimerUI()
{
    /* Set default game-times. */

	if ( m_table != NULL )
	{
		m_initialTime = m_table->GetInitialTime();
		m_blackTime   = m_table->GetBlackTime();
		m_redTime     = m_table->GetRedTime();
	}
	else
	{
		m_initialTime.nGame = hoxTIME_DEFAULT_GAME_TIME;
		m_initialTime.nMove = hoxTIME_DEFAULT_MOVE_TIME;
		m_initialTime.nFree = hoxTIME_DEFAULT_FREE_TIME;

		m_blackTime = m_initialTime;
		m_redTime   = m_initialTime;
	}
}

void 
hoxBoard::_UpdateTimerUI()
{
    // Game times.
	m_blackGameTime->SetLabel( hoxUtility::FormatTime( m_blackTime.nGame ) );
    m_redGameTime->SetLabel(   hoxUtility::FormatTime( m_redTime.nGame ) );

    // Move times.
	m_blackMoveTime->SetLabel( hoxUtility::FormatTime( m_blackTime.nMove ) );
	m_redMoveTime->SetLabel(   hoxUtility::FormatTime( m_redTime.nMove ) );

    // Free times.
	m_blackFreeTime->SetLabel( hoxUtility::FormatTime( m_blackTime.nFree ) );
	m_redFreeTime->SetLabel(   hoxUtility::FormatTime( m_redTime.nFree ) );
}

bool 
hoxBoard::_ParseMoveString( const wxString& moveStr, 
	                        hoxMove&        move ) const
{
    const char* FNAME = "hoxBoard::_ParseMoveString";

    /* NOTE: Move-string has the format of "xyXY" */

    if ( moveStr.size() != 4 )
    {
        return false;   // failure
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
        wxLogDebug("%s: *** WARN *** Failed to locate piece at the position.", FNAME);
        return false;   // failure
    }

    return true; // success
}

/************************* END OF FILE ***************************************/
