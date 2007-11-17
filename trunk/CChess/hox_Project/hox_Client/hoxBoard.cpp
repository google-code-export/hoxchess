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
// Description:     A "simple" Board with the following features:
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

/* UI-related IDs. */
enum
{
    ID_BOARD_WALL_INPUT = (wxID_HIGHEST+1),

    ID_HISTORY_BEGIN,
    ID_HISTORY_PREV,
    ID_HISTORY_NEXT,
    ID_HISTORY_END
};


/* Define my custom events */
DEFINE_EVENT_TYPE( hoxEVT_BOARD_PLAYER_JOIN )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_PLAYER_LEAVE )
DEFINE_EVENT_TYPE( hoxEVT_BOARD_WALL_OUTPUT )

BEGIN_EVENT_TABLE(hoxSimpleBoard, wxPanel)
    EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_PLAYER_JOIN, hoxSimpleBoard::OnPlayerJoin)
    EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_PLAYER_LEAVE, hoxSimpleBoard::OnPlayerLeave)
    EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_WALL_OUTPUT, hoxSimpleBoard::OnWallOutput)

    EVT_TEXT_ENTER(ID_BOARD_WALL_INPUT, hoxSimpleBoard::OnWallInputEnter)
    EVT_BUTTON(ID_HISTORY_BEGIN, hoxSimpleBoard::OnButtonHistory_BEGIN)
    EVT_BUTTON(ID_HISTORY_PREV, hoxSimpleBoard::OnButtonHistory_PREV)
    EVT_BUTTON(ID_HISTORY_NEXT, hoxSimpleBoard::OnButtonHistory_NEXT)
    EVT_BUTTON(ID_HISTORY_END, hoxSimpleBoard::OnButtonHistory_END)

    EVT_TIMER(wxID_ANY, hoxSimpleBoard::OnTimer)    
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// hoxSimpleBoard
// ----------------------------------------------------------------------------

hoxSimpleBoard::hoxSimpleBoard( wxWindow*       parent,
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
{
    wxCHECK_RET( referee != NULL, "A Referee must be set." );

    /* Create the core board. */
    m_coreBoard = new hoxCoreBoard( this, m_referee );
    m_coreBoard->SetBoardOwner( this );
    m_coreBoard->SetPiecesPath( piecesPath );

    /* A timer to keep track of the time. */
    m_timer = new wxTimer( this );
    m_timer->Start( hoxTIME_ONE_SECOND_INTERVAL );

    /* Set default game-times. */
    _ResetTimerUI();

    // *** NOTE: By default, the Board is NOT visible.
    wxPanel::Show( false );  // invoke the parent's API.
}

hoxSimpleBoard::~hoxSimpleBoard()
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
hoxSimpleBoard::OnBoardMove( const hoxMove& move )
{
    /* Inform the Table of the new move. */
    wxCHECK_RET(m_table, "The table is NULL." );
    m_table->OnMove_FromBoard( move );

    _OnValidMove( move );
}

void 
hoxSimpleBoard::OnBoardMsg( const wxString& message )
{
    const wxString who = "** Board **";  // NOTE: This Board's name.

    _PostToWallOutput( who, message ); 
}

void 
hoxSimpleBoard::OnPlayerJoin( wxCommandEvent &event )
{
    const char* FNAME = "hoxSimpleBoard::OnPlayerJoin";

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
    if (   playerType == hoxPLAYER_TYPE_HOST 
        || playerType == hoxPLAYER_TYPE_LOCAL )
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
hoxSimpleBoard::OnPlayerLeave( wxCommandEvent &event )
{
    const char* FNAME = "hoxSimpleBoard::OnPlayerLeave";

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
hoxSimpleBoard::OnWallOutput( wxCommandEvent &event )
{
    const wxString eventString = event.GetString();
    const wxString who = eventString.BeforeFirst(' ');
    const wxString msg = eventString.AfterFirst(' ');

    _PostToWallOutput( who, msg ); 
}

void 
hoxSimpleBoard::OnWallInputEnter( wxCommandEvent &event )
{
    m_wallInput->Clear();
    m_table->OnMessage_FromBoard( event.GetString() );
}

void 
hoxSimpleBoard::OnButtonHistory_BEGIN( wxCommandEvent &event )
{
    if ( m_coreBoard == NULL )
        return;

    m_coreBoard->DoGameReview_BEGIN();
}

void 
hoxSimpleBoard::OnButtonHistory_PREV( wxCommandEvent &event )
{
    if ( m_coreBoard == NULL )
        return;

    m_coreBoard->DoGameReview_PREV();
}

void 
hoxSimpleBoard::OnButtonHistory_NEXT( wxCommandEvent &event )
{
    if ( m_coreBoard == NULL )
        return;

    m_coreBoard->DoGameReview_NEXT();
}

void 
hoxSimpleBoard::OnButtonHistory_END( wxCommandEvent &event )
{
    if ( m_coreBoard == NULL )
        return;

    m_coreBoard->DoGameReview_END();
}

void 
hoxSimpleBoard::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    if ( m_status != hoxGAME_STATUS_IN_PROGRESS )
        return;

    hoxPieceColor nextColor = m_referee->GetNextColor();

    if ( nextColor == hoxPIECE_COLOR_BLACK )
    {
        --m_nBGameTime;
        --m_nBMoveTime;
    }
    else
    {
        --m_nRGameTime;
        --m_nRMoveTime;
    }

    _UpdateTimerUI();
}

bool 
hoxSimpleBoard::Show(bool show /* = true */)
{
    if ( !this->IsShown() && show ) // hidden -> shown?
    {
        /* Ask the board to display the pieces. */
        m_coreBoard->LoadPieces();

        /* Create the whole panel with player-info + timers */
        _CreateBoardPanel();

        // Set its background color.
        wxColour bgPanelCol = wxTheColourDatabase->Find(_T("SKY BLUE"));
        if ( bgPanelCol.Ok() ) {
            this->SetBackgroundColour(bgPanelCol);
        }
    }

    return wxPanel::Show( show );  // invoke the parent's API.
}

void 
hoxSimpleBoard::_SetRedInfo( const hoxPlayer* player )
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
hoxSimpleBoard::_SetBlackInfo( const hoxPlayer* player )
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
hoxSimpleBoard::_CreateBoardPanel()
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
hoxSimpleBoard::_LayoutBoardPanel( bool viewInverted )
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

    // Add the history-sizer...
    m_boardSizer->Add(
        m_historySizer,
        0,            // fixed-size vertically
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT,  // and make border
        1 );          // set border width
}

void 
hoxSimpleBoard::SetTable( hoxTable* table ) 
{ 
    m_table = table;
}

void 
hoxSimpleBoard::ToggleViewSide()
{
    const char* FNAME = "hoxSimpleBoard::ToggleViewSide";

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
    found = m_boardSizer->Detach( m_historySizer );
    wxASSERT( found );

    /* Invert */

    bool viewInverted = m_coreBoard->IsViewInverted();
    _LayoutBoardPanel( viewInverted );

    // Force the layout update (just to make sure!).
    m_boardSizer->Layout();
}

bool 
hoxSimpleBoard::DoMove( hoxMove& move )
{
    wxCHECK_MSG( m_coreBoard, false, "The core Board is NULL." );

    if ( ! m_coreBoard->DoMove( move ) )
        return false;

    _OnValidMove( move );

    return true;
}

void 
hoxSimpleBoard::_AddPlayerToList( const wxString& playerId,
                                  int             playerScore )
{
    const char* FNAME = "hoxSimpleBoard::_AddPlayerToList";

    if ( ! this->IsShown() )
    {
        wxLogDebug("%s: Board is not shown. Do nothing.", FNAME);
        return;
    }

    const wxString info = wxString::Format("%s (%d)", playerId.c_str(), playerScore);
    m_playerListBox->Append( info );
}

void 
hoxSimpleBoard::_RemovePlayerFromList( const wxString& playerId )
{
    const char* FNAME = "hoxSimpleBoard::_RemovePlayerFromList";

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
hoxSimpleBoard::_PostToWallOutput( const wxString& who,
                                   const wxString& message )
{
    m_wallOutput->SetDefaultStyle( wxTextAttr(*wxBLACK) );
    m_wallOutput->AppendText( wxString::Format("[%s] ", who.c_str()) );
    m_wallOutput->SetDefaultStyle( wxTextAttr(*wxBLUE) );
    m_wallOutput->AppendText( wxString::Format("%s\n", message.c_str()) );
}

void 
hoxSimpleBoard::_OnValidMove( const hoxMove& move )
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
    /* If the game is in progress, reset the Move-time. 
     */
    else if ( m_status == hoxGAME_STATUS_IN_PROGRESS )
    {
        if ( move.piece.color == hoxPIECE_COLOR_BLACK )
            m_nBMoveTime = hoxTIME_DEFAULT_MOVE_TIME;
        else
            m_nRMoveTime = hoxTIME_DEFAULT_MOVE_TIME;
    }
}

void
hoxSimpleBoard::_ResetTimerUI()
{
    /* Set default game-times. */

    m_nBGameTime = hoxTIME_DEFAULT_GAME_TIME;
    m_nRGameTime = m_nBGameTime;

    m_nBMoveTime = hoxTIME_DEFAULT_MOVE_TIME;
    m_nRMoveTime = m_nBMoveTime;

    m_nBFreeTime = hoxTIME_DEFAULT_FREE_TIME;
    m_nRFreeTime = m_nBFreeTime;
}

void 
hoxSimpleBoard::_UpdateTimerUI()
{
    // Game times.
    m_blackGameTime->SetLabel( _FormatTime( m_nBGameTime ) );
    m_redGameTime->SetLabel(   _FormatTime( m_nRGameTime ) );

    // Move times.
    m_blackMoveTime->SetLabel( _FormatTime( m_nBMoveTime ) );
    m_redMoveTime->SetLabel(   _FormatTime( m_nRMoveTime ) );

    // Free times.
    m_blackFreeTime->SetLabel( _FormatTime( m_nBFreeTime ) );
    m_redFreeTime->SetLabel(   _FormatTime( m_nRFreeTime ) );
}

const wxString 
hoxSimpleBoard::_FormatTime( int nTime ) const
{
    return wxString::Format( "%d:%.02d", nTime / 60, nTime % 60 );
}

/************************* END OF FILE ***************************************/
