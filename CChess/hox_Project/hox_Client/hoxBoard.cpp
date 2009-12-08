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
#include "hoxUtil.h"
#include "hoxReferee.h"
#include "hoxTypes.h"
#include "hoxTable.h"
#include "hoxOptionDialog.h"

/* UI-related IDs. */
enum
{
    ID_BOARD_WALL_INPUT = hoxUI_ID_RANGE_BOARD,
    ID_BOARD_INPUT_BUTTON,

    ID_HISTORY_BEGIN,
    ID_HISTORY_PREV,
    ID_HISTORY_NEXT,
    ID_HISTORY_END,

    ID_ACTION_RED,    // Play as RED
    ID_ACTION_BLACK,  // Play as BLACK
    ID_ACTION_NONE,   // Play as NONE (i.e., Unsit/Standup)

    ID_ACTION_REVERSE,
    ID_ACTION_OPTIONS,
    ID_ACTION_RESIGN,
	ID_ACTION_DRAW,
	ID_ACTION_RESET
};


BEGIN_EVENT_TABLE(hoxBoard, wxPanel)
    EVT_TEXT_ENTER(ID_BOARD_WALL_INPUT, hoxBoard::OnWallInputEnter)
    EVT_BUTTON(ID_BOARD_INPUT_BUTTON, hoxBoard::OnWallInputEnter)
    EVT_BUTTON(ID_HISTORY_BEGIN, hoxBoard::OnButtonHistory_BEGIN)
    EVT_BUTTON(ID_HISTORY_PREV, hoxBoard::OnButtonHistory_PREV)
    EVT_BUTTON(ID_HISTORY_NEXT, hoxBoard::OnButtonHistory_NEXT)
    EVT_BUTTON(ID_HISTORY_END, hoxBoard::OnButtonHistory_END)
    EVT_BUTTON(ID_ACTION_REVERSE, hoxBoard::OnButtonReverse)
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
// hoxWallOutput
// ----------------------------------------------------------------------------

class hoxWallOutput : public wxPanel
{
public:
    hoxWallOutput( wxWindow *parent, wxWindowID id,
                   const wxString& sCaption  )
        : wxPanel( parent, id )
        , m_sCaption( sCaption )
        , m_wall( NULL )
    {
        m_wall = new wxTextCtrl( this, wxID_ANY, _T(""),
                                  wxDefaultPosition, wxDefaultSize,
                                  wxTE_MULTILINE | wxRAISED_BORDER | wxTE_READONLY 
                                    | wxTE_RICH2 /* Windows only */ );
        _CreateUI();
    }

    void AppendMessage( const wxString& who,
                        const wxString& sMessage,
                        bool            bPublic = true );

protected:
    void OnClearButton( wxCommandEvent& event );

private:
    void _CreateUI();

private:
    const wxString   m_sCaption;
    wxTextCtrl*      m_wall;

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(hoxWallOutput, wxPanel)
    EVT_BUTTON(wxID_ANY, hoxWallOutput::OnClearButton)
END_EVENT_TABLE()

void
hoxWallOutput::_CreateUI()
{
    /* Reference: Color constants from:
     *       http://www.colorschemer.com/online.html
     */

    wxBoxSizer* mainSizer = new wxBoxSizer( wxVERTICAL );
    this->SetSizer( mainSizer );    

    wxBoxSizer* headerSizer = new wxBoxSizer( wxHORIZONTAL );
    wxStaticText* captionText = new wxStaticText( this, wxID_ANY, m_sCaption );
    captionText->SetBackgroundColour( wxColor(87,87,87) ) ;
    captionText->SetForegroundColour( wxColor(*wxWHITE) ) ;
    headerSizer->Add( captionText,
        wxSizerFlags(1).Expand().Align(wxALIGN_CENTER_VERTICAL) );
    wxBitmapButton* clearButton = new wxBitmapButton( this, wxID_ANY,
                                        hoxUtil::LoadImage("edit-clear.png"));
    clearButton->SetToolTip( _("Clear All") );
    headerSizer->Add( clearButton );

    mainSizer->Add( headerSizer,
        wxSizerFlags().Border(wxTOP|wxLEFT|wxRIGHT,1) );
    mainSizer->Add( m_wall,
        wxSizerFlags(1).Expand().Border(wxRIGHT|wxLEFT|wxBOTTOM,1) );
}

void 
hoxWallOutput::AppendMessage( const wxString& who,
                              const wxString& sMessage,
                              bool            bPublic /* = true */ )
{
    if ( !who.empty() )
    {
        m_wall->SetDefaultStyle( wxTextAttr(*wxBLACK) );
        m_wall->AppendText( wxString::Format("[%s] ", who.c_str()) );
    }
    m_wall->SetDefaultStyle( wxTextAttr( bPublic ? *wxBLUE : *wxRED) );
    const wxString displayMsg = wxString::Format("%s\n", sMessage.c_str());

    /* NOTE:
     *    Make sure that the last line is at the bottom of the wxTextCtrl
     *    so that new messages are visiable to the Player.
     *    This technique was learned from the following site:
     *        http://wiki.wxwidgets.org/WxTextCtrl#Scrolling
     *
     * HACK: Under Windows (using wxTE_RICH2) we have trouble ensuring that the last
     * entered line is really at the bottom of the screen. We jump through some
     * hoops to get this working.
     */
 
    // Count number of newlines (i.e lines)
    int lines = 0;
    for ( wxString::const_iterator it = displayMsg.begin();
                                   it != displayMsg.end(); ++it )
    {
        const wchar_t ch = *it;
        if( ch == '\n' ) ++lines;
    }

    m_wall->Freeze();                 // Freeze the window to prevent scrollbar jumping
    m_wall->AppendText( displayMsg ); // Add the text
    m_wall->ScrollLines( lines + 1 ); // Scroll down correct number of lines + one (the extra line is important for some cases!)
    m_wall->ShowPosition( m_wall->GetLastPosition() ); // Ensure the last line is shown at the very bottom of the window
    m_wall->Thaw();                   // Allow the window to redraw

}

void
hoxWallOutput::OnClearButton( wxCommandEvent& event )
{
    m_wall->Clear();
}

// ----------------------------------------------------------------------------
// hoxAISettings
// ----------------------------------------------------------------------------

class hoxAISettings : public wxPanel
{
public:
    hoxAISettings( wxWindow *parent, wxWindowID id,
                   const wxString& sCaption,
                   const int nAILevel )
        : wxPanel( parent, id )
        , m_sCaption( sCaption )
        , m_nAILevel( nAILevel )
        , m_aiInfoText( NULL )
        , m_playWithSelfCtrl( NULL )
    {
        _CreateUI();
    }

    void SetAIInfo( const wxString& sAIInfo )
    {
        if ( m_aiInfoText ) m_aiInfoText->SetLabel( sAIInfo );
    }

    bool IsPlayWithSelf() const
    {
        return ( m_playWithSelfCtrl && m_playWithSelfCtrl->IsChecked() );
    }

protected:
    void OnAISliderUpdate( wxScrollEvent& event );

private:
    void _CreateUI();

private:
    const wxString m_sCaption;
    int            m_nAILevel;
    wxStaticText*  m_aiInfoText; 
    wxCheckBox*    m_playWithSelfCtrl;

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(hoxAISettings, wxPanel)
    EVT_COMMAND_SCROLL(wxID_ANY, hoxAISettings::OnAISliderUpdate)
END_EVENT_TABLE()

void
hoxAISettings::_CreateUI()
{
    wxBoxSizer* mainSizer = new wxBoxSizer( wxVERTICAL );
    this->SetSizer( mainSizer );    

    // Header.
    wxBoxSizer* headerSizer = new wxBoxSizer( wxHORIZONTAL );
    wxStaticText* captionText = new wxStaticText( this, wxID_ANY, m_sCaption,
                                                  wxDefaultPosition, wxSize(-1,18) );
    captionText->SetBackgroundColour( wxColor(87,87,87) ) ;
    captionText->SetForegroundColour( wxColor(*wxWHITE) ) ;
    headerSizer->Add( captionText,
        wxSizerFlags(1).Expand().Align(wxALIGN_CENTER_VERTICAL) );
    mainSizer->Add( headerSizer, wxSizerFlags().Border(wxALL,1) );

    // AI settings. 
    wxStaticBoxSizer* aiSizer = new wxStaticBoxSizer( wxVERTICAL, this, "" );
    aiSizer->Add( new wxStaticText( this, wxID_ANY, _("Difficulty level:") ),
                  wxSizerFlags().Border(wxALL,5) );
    wxSlider* aiSlider = new wxSlider( this, wxID_ANY,
                                       m_nAILevel /*value*/, 1 /*min*/, 10 /*max*/,
                                       wxDefaultPosition, wxDefaultSize,
                                       wxSL_AUTOTICKS | wxSL_LABELS);
    aiSizer->Add( aiSlider, wxSizerFlags().Expand().Border(wxALL,5) );

    // AI info.
    wxBoxSizer* infoSizer = new wxBoxSizer( wxHORIZONTAL );
    infoSizer->Add( new wxStaticText( this, wxID_ANY, _("Author:") ) );
    infoSizer->AddSpacer(5);
    m_aiInfoText = new wxStaticText( this, wxID_ANY, "" );
    infoSizer->Add( m_aiInfoText );
    aiSizer->AddSpacer(10);
    aiSizer->Add( infoSizer, wxSizerFlags().Expand().Border(wxALL,5) );

    mainSizer->Add( aiSizer, wxSizerFlags(1).Expand().Border(wxALL,1) );

    // ---
    m_playWithSelfCtrl = new wxCheckBox(this, wxID_ANY, _("Play with yourself"));
    m_playWithSelfCtrl->SetValue( false );

    mainSizer->Add( m_playWithSelfCtrl, wxSizerFlags(1).Expand().Border(wxALL,5) );
    mainSizer->AddStretchSpacer();
}

void
hoxAISettings::OnAISliderUpdate( wxScrollEvent& event )
{
    if ( m_nAILevel == event.GetInt() ) return;
    m_nAILevel = event.GetInt();
    event.Skip(); // Let the search for the event handler continue.
}

// ----------------------------------------------------------------------------
// hoxInputTextCtrl
// ----------------------------------------------------------------------------

class hoxInputTextCtrl : public wxTextCtrl
{
public:
    hoxInputTextCtrl(wxWindow *parent, wxWindowID id,
                     const wxString& value = wxEmptyString )
        : wxTextCtrl( parent, id, value,
                      wxDefaultPosition, wxDefaultSize,
                      wxTE_PROCESS_ENTER | wxSUNKEN_BORDER
                        | wxTE_RICH2 /* Windows only */ )
        , m_bFirstEnter( true )
    {
        const wxTextAttr defaultStyle = GetDefaultStyle();
        SetDefaultStyle(wxTextAttr(wxNullColour, *wxLIGHT_GREY));
        AppendText( _("[Type your message here]") );
        SetDefaultStyle(defaultStyle);
    }

private:
    void OnMouseEvent( wxMouseEvent& event );

    bool  m_bFirstEnter;

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(hoxInputTextCtrl, wxTextCtrl)
    EVT_MOUSE_EVENTS( hoxInputTextCtrl::OnMouseEvent  )
END_EVENT_TABLE()

void
hoxInputTextCtrl::OnMouseEvent( wxMouseEvent& event )
{
    event.Skip();
    if ( event.LeftUp() )
    {
        if ( m_bFirstEnter )
        {
            m_bFirstEnter = false;
            Clear();
        }
        else if ( ! IsEmpty() )
        {
            SelectAll();
        }
    }
}

// ----------------------------------------------------------------------------
// hoxBoard
// ----------------------------------------------------------------------------

hoxBoard::hoxBoard( wxWindow*        parent,
                    const wxString&  piecesPath,
                    hoxIReferee_SPtr referee,
                    hoxTable_SPtr    pTable,
                    const wxString&  ownerId,
                    const wxString&  sBgImage,
                    wxColor          bgColor,
                    wxColor          fgColor,
                    const wxPoint&   pos  /* = wxDefaultPosition */, 
                    const wxSize&    size /* = wxDefaultSize */,
                    unsigned int     featureFlags /* = hoxBOARD_FEATURE_ALL */ )
        : wxPanel( parent, wxID_ANY, pos, size, wxFULL_REPAINT_ON_RESIZE )
        , m_coreBoard( NULL )
        , m_referee( referee )
        , m_pTable( pTable )
        , m_status( hoxGAME_STATUS_OPEN )
        , m_localColor( hoxCOLOR_NONE )
        , m_ownerId( ownerId )
        , m_featureFlags( featureFlags )
        , m_bRated( true )
		, m_timer( NULL )
        , m_bUICreated( false )
        , m_playerListBox( NULL )
        , m_systemOutput( NULL )
        , m_wallOutput( NULL )
        , m_wallInput( NULL )
        , m_bSoundEnabled( true )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
    
    wxCHECK_RET( m_referee, "A Referee must be set" );
    wxCHECK_RET( m_pTable, "A Table must be set" );
    wxCHECK_RET( !m_ownerId.empty(), "An Owner must be set" );

    /* Create the core board. */
    m_coreBoard = new hoxCoreBoard( this, m_referee,
                                    sBgImage, piecesPath, bgColor, fgColor );
    m_coreBoard->SetBoardOwner( this );

    /* Sync Info (Rated/Non-Rated + Timers) with Table's. */
    _SyncInfoWithTable();

    /* A timer to keep track of the time. */
    m_timer = new wxTimer( this );
    m_timer->Start( hoxTIME_ONE_SECOND_INTERVAL );

    /* Prepare sounds. */
    const wxString soundFile( hoxUtil::GetPath(hoxRT_SOUND) + "move.wav" );
    m_soundMove.Create( soundFile );
    wxASSERT_MSG(m_soundMove.IsOk(), "Failed to load sound file " + soundFile);

    /* Finally, show the UI. */
    // ShowUI();
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
    this->OnValidMove( move );

    /* Inform the Table of the new move. */

	const hoxTimeInfo playerTime = ( move.piece.color == hoxCOLOR_RED
                                    ? m_redTime : m_blackTime );
    m_pTable->OnMove_FromBoard( move, status, playerTime );
}

void 
hoxBoard::OnBoardMsg( const wxString& message )
{
    const wxString who = "***";  // NOTE: This Board's name.
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

    /* Check for the "next" Color.
     * Also, only the physical player can move the Piece.
     */
    if (   pieceInfo.color != m_referee->GetNextColor()
        || pieceInfo.color != m_localColor )
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
            m_pTable->OnPlayerInfoRequest_FromBoard( sPlayerId );
            break;
        }
        case hoxPlayersUI::EVENT_TYPE_INVITE:
        {
            m_pTable->OnPlayerInviteRequest_FromBoard( sPlayerId );
            break;
        }
        case hoxPlayersUI::EVENT_TYPE_MSG:
        {
            m_pTable->OnPrivateMessageRequest_FromBoard( sPlayerId );
            break;
        }
        default:
            wxLogWarning("%s: Unsupported eventType [%d].", __FUNCTION__, eventType);
            break;
    }
}

void 
hoxBoard::OnPlayerJoin( const hoxPlayerInfo playerInfo,
                        const hoxColor      playerColor )
{
    const wxString playerId = playerInfo.id;
    const int      nScore   = playerInfo.score;

    if ( playerColor == hoxCOLOR_RED )
    {
        _SetRedInfo( playerId, nScore );
        if ( playerId == m_blackId ) _SetBlackInfo( "" );
    } 
    else if ( playerColor == hoxCOLOR_BLACK )
    {
        _SetBlackInfo( playerId, nScore );
        if ( playerId == m_redId ) _SetRedInfo( "" );
    }
    else
    {
        if ( playerId == m_redId )   _SetRedInfo( "" );
        if ( playerId == m_blackId ) _SetBlackInfo( "" );
    }

    /* Toggle the view if necessary. There two cases:
     *  (1) The owner just joined as RED, make sure the view is "normal".
     *  (2) The owner just joined as BLACK, make sure the view is "inverted".
     */
    if (   playerId == m_ownerId 
        && (   (playerColor == hoxCOLOR_RED && m_coreBoard->IsViewInverted())
             ||(playerColor == hoxCOLOR_BLACK && !m_coreBoard->IsViewInverted()) ) )
    {
        _ReverseView();
    }

    const bool bNewlyJoined = _AddPlayerToList( playerId, nScore );
    if ( bNewlyJoined )
    {
        const wxString sMessage = wxString::Format("%s joined", playerId.c_str());
        _PostToSystemOutput( sMessage );
    }

    /* Update the LOCAL - color on the core Board so that it knows
     * who is allowed to make a Move using the mouse.
     */
    if ( playerInfo.type == hoxPLAYER_TYPE_LOCAL )
    {
        m_localColor = playerColor;
    }

    _UpdateStatus(); // Update the game-status.
}

void 
hoxBoard::OnPlayerLeave( const wxString& playerId )
{
    if      ( playerId == m_redId   ) _SetRedInfo( "" );
    else if ( playerId == m_blackId ) _SetBlackInfo( "" );

    const bool bRemoved = _RemovePlayerFromList( playerId );
    if ( bRemoved )
    {
        const wxString sMessage = wxString::Format("%s left", playerId.c_str());
        _PostToSystemOutput( sMessage );
    }

    _UpdateStatus(); // Update the game-status.
}

void 
hoxBoard::OnPlayerScore( const wxString& playerId,
                         const int       nScore )
{
    if      ( playerId == m_redId )   _SetRedInfo( playerId, nScore );
    else if ( playerId == m_blackId ) _SetBlackInfo( playerId, nScore );

    _UpdatePlayerScore( playerId, nScore );
}

void 
hoxBoard::OnSystemOutput( const wxString& sMessage )
{
    _PostToSystemOutput( sMessage );
}

void 
hoxBoard::OnWallOutput( const wxString& sMessage,
                        const wxString& sSenderId,
                        const bool      bPublic )
{
    _PostToWallOutput( sSenderId, sMessage, bPublic ); 
}

void 
hoxBoard::OnNewMove( const wxString& sMove )
{
    hoxMove move = m_referee->StringToMove( sMove );
    if ( ! move.IsValid() )
    {
        wxLogError("%s: Failed to parse Move-string [%s].", __FUNCTION__, sMove.c_str());
        return;
    }

	/* Ask the core Board to realize the Move */

    if ( ! m_coreBoard->DoMove( move ) )  // failed?
        return;

    this->OnValidMove( move );
}

void
hoxBoard::OnPastMoves( const hoxStringList& moves )
{
    for ( hoxStringList::const_iterator it = moves.begin();
                                        it != moves.end(); ++it )
    {
        hoxMove move = m_referee->StringToMove( *it );
        if ( ! move.IsValid() )
        {
            wxLogError("%s: Failed to parse Move-string [%s].", __FUNCTION__, it->c_str());
            return;
        }

	    /* Ask the core Board to realize the Move */
        if ( ! m_coreBoard->DoMove( move, false /* bRefresh */ ) ) // failed?
            return;

        this->OnValidMove( move, true /* bSetupMode */ );
    }

    m_coreBoard->Refresh(); // Now, refresh the Board UI.
}

void 
hoxBoard::OnDrawRequest( const wxString& playerId,
                         const bool      bPopupRequest )
{
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
hoxBoard::OnGameOver( const hoxGameStatus gameStatus,
                      const wxString&     sReason )
{
    wxString boardMessage = _GetGameOverMessage( gameStatus );
    if ( !sReason.empty() ) boardMessage += " " + sReason;

	m_status = gameStatus;
	this->OnBoardMsg( boardMessage );
	m_coreBoard->SetGameOver( true );
}

void 
hoxBoard::OnGameReset()
{
    m_coreBoard->ResetBoard();
    _SyncInfoWithTable();

    m_status = hoxGAME_STATUS_OPEN;
	_UpdateStatus();

	/* Display the "reset" message. */
    const wxString boardMessage = _("Game Reset"); 
	this->OnBoardMsg( boardMessage );
}

void 
hoxBoard::OnTableUpdate()
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

	/* Display a notification message regarding the new Timer. */
    boardMessage =
        "Timer changed to " + hoxUtil::TimeInfoToString( m_initialTime ); 
	this->OnBoardMsg( boardMessage );
}

void 
hoxBoard::OnWallInputEnter( wxCommandEvent &event )
{
    const wxString sText = m_wallInput->GetValue();
    if ( ! sText.empty() )
    {
        m_pTable->OnMessage_FromBoard( sText );
        m_wallInput->Clear();
    }
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
hoxBoard::OnButtonReverse( wxCommandEvent &event )
{
    _ReverseView();
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

    hoxOptionDialog optionDlg( this, wxID_ANY, _("Table settings"),
                               m_pTable, optionDlgFlags );
    optionDlg.Center();
    const int nCommandId = optionDlg.ShowModal();
    if ( nCommandId == wxID_OK )
    {
        const bool        bRatedGame  = optionDlg.IsRatedGame();
        const hoxTimeInfo newTimeInfo = optionDlg.GetNewTimeInfo();
        m_pTable->OnOptionsCommand_FromBoard( bRatedGame, newTimeInfo );
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
                        && _IsOwnerSeated()
                        && (m_featureFlags & hoxBOARD_FEATURE_UNSIT) );
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
    hoxTimeInfo* pNextTime = ( nextColor == hoxCOLOR_RED ? &m_redTime
                                                         : &m_blackTime );
	if ( pNextTime->nGame > 0 ) --pNextTime->nGame;
	if ( pNextTime->nMove > 0 ) --pNextTime->nMove;
    if ( pNextTime->nGame == 0
      && pNextTime->nFree > 0 ) --pNextTime->nFree;

    _OnTimerUpdated();
}

void
hoxBoard::ShowUI()
{
    /* Ask the board to display the pieces. */
    m_coreBoard->LoadPiecesAndStatus();

    /* Create the whole panel with player-info + timers */
    _CreateBoardUI();

    /* Indicate that all UI elements have been created. */
    m_bUICreated = true;

    /* Set the background color. */
    const wxColour bgPanelColor = wxTheColourDatabase->Find("SKY BLUE");
    if ( bgPanelColor.Ok() ) this->SetBackgroundColour(bgPanelColor);

    /* NOTE: Force Timer Updated event!!! */
    _OnTimerUpdated();
}

void
hoxBoard::_SetRedInfo( const wxString& playerId,
                       const int       nScore /* = 0 */ )
{
    if ( ! m_bUICreated ) return;

    m_redId = playerId;

    wxString info = "*";
    if ( ! m_redId.empty() ) info.Printf("%s (%d)", m_redId.c_str(), nScore);
    m_redInfo->SetLabel( info );
}

void
hoxBoard::_SetBlackInfo( const wxString& playerId,
                         const int       nScore /* = 0 */ )
{
    if ( ! m_bUICreated ) return;

    m_blackId = playerId;

    wxString info = "*";
    if ( ! m_blackId.empty() ) info.Printf("%s (%d)", m_blackId.c_str(), nScore);
    m_blackInfo->SetLabel( info );
}

void
hoxBoard::_CreateBoardUI()
{
    m_mainSizer  = new wxBoxSizer( wxHORIZONTAL );
    m_boardSizer = new wxBoxSizer( wxVERTICAL );
    m_wallSizer  = new wxBoxSizer( wxVERTICAL );

    // Create the Board and the Wall panels.

    _CreateBoardPanel();
    _LayoutBoardPanel( m_coreBoard->IsViewInverted() );

    this->CreateAndLayoutWallPanel();

    // Setup the main sizer.

    m_mainSizer->Add( m_boardSizer, wxSizerFlags().Expand().Border(wxRIGHT,1) );
    m_mainSizer->Add( m_wallSizer,  wxSizerFlags(1).Expand() );

    this->SetSizer( m_mainSizer );
    m_mainSizer->SetSizeHints( this ); // Set size hints to honor minimum size
}

void
hoxBoard::_CreateBoardPanel()
{
    /************************************
     * Create Player-Info + Game-Timers 
     ************************************/

    m_btnPlayRed = new wxButton( this, ID_ACTION_RED, _("Play RED"), 
                                 wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    m_btnPlayBlack = new wxButton( this, ID_ACTION_BLACK, _("Play BLACK"), 
                                   wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );

    // Player-Info.
    m_blackInfo = new wxStaticText( this, wxID_ANY, "*", 
        wxDefaultPosition, wxSize(200,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);
    m_redInfo = new wxStaticText( this, wxID_ANY, "*", 
        wxDefaultPosition, wxSize(200,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);

    // Game-Timers.
    m_blackGameTime = new wxStaticText( this, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);
    m_redGameTime = new wxStaticText( this, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);

    m_blackMoveTime = new wxStaticText( this, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);
    m_redMoveTime = new wxStaticText( this, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);

    m_blackFreeTime = new wxStaticText( this, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);
    m_redFreeTime = new wxStaticText( this, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);

    /****************************************
     * Arrange Player-Info + Game-Timers 
     ****************************************/
   
    m_blackSizer = new wxBoxSizer( wxHORIZONTAL );
    m_redSizer   = new wxBoxSizer( wxHORIZONTAL );

    // BLACK side.
    m_blackSizer->Add( m_btnPlayBlack,
        wxSizerFlags(1).Expand().Border(wxRIGHT|wxLEFT|wxTOP,1) );
    m_blackSizer->Add( m_blackInfo,
        wxSizerFlags(1).Expand().Border(wxRIGHT|wxLEFT|wxTOP,1) );
    m_blackSizer->Add( m_blackGameTime,
        wxSizerFlags().Expand().Border(wxRIGHT|wxLEFT|wxTOP,1) );
    m_blackSizer->Add( m_blackMoveTime,
        wxSizerFlags().Expand().Border(wxRIGHT|wxLEFT|wxTOP,1) );
    m_blackSizer->Add( m_blackFreeTime,
        wxSizerFlags().Expand().Border(wxRIGHT|wxLEFT|wxTOP,1) );

    // RED side.
    m_redSizer->Add( m_btnPlayRed,
        wxSizerFlags(1).Expand().Border(wxRIGHT|wxLEFT|wxTOP,1) );
    m_redSizer->Add( m_redInfo,
        wxSizerFlags(1).Expand().Border(wxRIGHT|wxLEFT|wxTOP,1) );
    m_redSizer->Add( m_redGameTime,
        wxSizerFlags().Expand().Border(wxRIGHT|wxLEFT|wxTOP,1) );
    m_redSizer->Add( m_redMoveTime,
        wxSizerFlags().Expand().Border(wxRIGHT|wxLEFT|wxTOP,1) );
    m_redSizer->Add( m_redFreeTime,
        wxSizerFlags().Expand().Border(wxRIGHT|wxLEFT|wxTOP,1) );

    /*********************************
     * Create MOVE-History's buttons.
     *********************************/

    m_historySizer = new wxBoxSizer( wxHORIZONTAL );

    m_historySizer->Add( 
        new wxButton( this, ID_HISTORY_BEGIN, "|<", 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        wxSizerFlags().Center().FixedMinSize() );
    m_historySizer->Add( 
        new wxButton( this, ID_HISTORY_PREV, "<",
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        wxSizerFlags().Center().FixedMinSize() );
    m_historySizer->Add( 
        new wxButton( this, ID_HISTORY_NEXT, ">", 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        wxSizerFlags().Center().FixedMinSize() );
    m_historySizer->Add( 
        new wxButton( this, ID_HISTORY_END, ">|", 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        wxSizerFlags().Center().FixedMinSize() );

    /*********************************
     * Create Action's buttons.
     *********************************/

    m_actionSizer = new wxBoxSizer( wxHORIZONTAL );
    wxBitmapButton* aButton = NULL;

    aButton = new wxBitmapButton( this, ID_ACTION_REVERSE,
                                  hoxUtil::LoadImage("reverse.png"));
    aButton->SetToolTip( _("Reverse view") );
    m_actionSizer->Add( aButton, wxSizerFlags().Center() );

    aButton = new wxBitmapButton( this, ID_ACTION_OPTIONS,
                                  hoxUtil::LoadImage("settings.png"));
    aButton->SetToolTip( _("Table settings") );
    m_actionSizer->Add( aButton, wxSizerFlags().Center() );

    aButton = new wxBitmapButton( this, ID_ACTION_RESET,
                                  hoxUtil::LoadImage("reset.png"));
    aButton->SetToolTip( _("Reset table") );
    m_actionSizer->Add( aButton, wxSizerFlags().Center() );

    m_actionSizer->AddSpacer( 40 );  // Add some spaces in between.

    m_actionSizer->Add( 
        new wxButton( this, ID_ACTION_RESIGN, _("Resign"), 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        wxSizerFlags().Left().FixedMinSize().Center() );
    m_actionSizer->Add( 
        new wxButton( this, ID_ACTION_DRAW, _("Draw"), 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        wxSizerFlags().Left().FixedMinSize().Center() );
    m_actionSizer->Add( 
        new wxButton( this, ID_ACTION_NONE, _("Unsit"), 
                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
        wxSizerFlags().Left().FixedMinSize().Center() );

    /*********************************
     * Arrange Command's buttons (History + Action).
     *********************************/

    m_commandSizer = new wxBoxSizer( wxHORIZONTAL );
    m_commandSizer->Add( m_historySizer, wxSizerFlags().Border(1) );
	m_commandSizer->AddStretchSpacer();
    m_commandSizer->Add( m_actionSizer, wxSizerFlags().Border(1) );
}

void 
hoxBoard::_LayoutBoardPanel( bool bViewInverted )
{
    wxSizer* topSizer    = m_blackSizer;
    wxSizer* bottomSizer = m_redSizer;

    if ( bViewInverted )  // inverted view?
    {
        topSizer    = m_redSizer;
        bottomSizer = m_blackSizer;
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // HACK: Adjust the "core" Board's height!!!
    const int bestHeightAdjust = (m_btnPlayRed->GetSize().GetHeight()+2) * 3;
    m_coreBoard->SetBestHeightAdjustment( bestHeightAdjust );
    wxLogDebug("%s: ADJUST Core Board's Height: (%d)", __FUNCTION__, bestHeightAdjust);
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // Add the top-sizer, the main board, and the bottom size.
    m_boardSizer->Add( topSizer,    wxSizerFlags().Expand() );
    m_boardSizer->Add( m_coreBoard, wxSizerFlags(1).Expand() );
    m_boardSizer->Add( bottomSizer, wxSizerFlags().Expand() );

    // Add the command-sizer (History + Action)...
    m_boardSizer->Add( m_commandSizer, wxSizerFlags().Expand() );
}

void
hoxBoard::CreateAndLayoutWallPanel()
{
    /* Create the Wall's content. */

    m_playerListBox = new hoxPlayersUI( this, hoxPlayersUI::UI_TYPE_TABLE );
    m_playerListBox->SetOwner( this );

    m_systemOutput = new hoxWallOutput( this, wxID_ANY, _("Activities") );
    m_wallOutput   = new hoxWallOutput( this, wxID_ANY, _("Messages") );

    wxBoxSizer* inputSizer  = new wxBoxSizer( wxHORIZONTAL );
    m_wallInput  = new hoxInputTextCtrl( this, ID_BOARD_WALL_INPUT );
    inputSizer->Add( m_wallInput, 1, wxEXPAND|wxALL, 0 );
    wxBitmapButton* inputBtn = new wxBitmapButton( this, ID_BOARD_INPUT_BUTTON,
                                                   hoxUtil::LoadImage("go-jump.png") );
    inputSizer->Add( inputBtn, 0, wxEXPAND|wxALL, 0 );

    /* Arrange the Wall. */

    m_wallSizer->Add( m_playerListBox, wxSizerFlags(1).Expand() );
    m_wallSizer->Add( m_systemOutput,  wxSizerFlags(1).Expand() );
    m_wallSizer->Add( m_wallOutput,    wxSizerFlags(3).Expand() );
    m_wallSizer->Add( inputSizer,      wxSizerFlags().Expand() );
}

void 
hoxBoard::_ReverseView()
{
    if ( ! m_bUICreated ) return;

    /* Invert the "core" board. */
    const bool bViewInverted = m_coreBoard->ReverseView();

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
hoxBoard::SetBackgroundImage( const wxString& sImage )
{
    m_coreBoard->SetBackgroundImage( sImage );
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

void
hoxBoard::SetPiecesPath( const wxString& piecesPath )
{
    m_coreBoard->SetPiecesPath( piecesPath );
}

void
hoxBoard::Repaint()
{
    m_coreBoard->Repaint();
}

bool 
hoxBoard::_AddPlayerToList( const wxString& playerId,
                            const int       nScore )
{
    if ( ! m_bUICreated ) return false;

    return m_playerListBox->AddPlayer( playerId, nScore );
}

void
hoxBoard::_UpdatePlayerScore( const wxString& playerId,
                              const int       nScore )
{
    if ( ! m_bUICreated ) return;

    if ( m_playerListBox->HasPlayer( playerId ) )
    {
        m_playerListBox->UpdateScore( playerId, nScore );
    }
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
    m_systemOutput->AppendMessage( "" /* who */, sMessage );
}

void 
hoxBoard::_PostToWallOutput( const wxString& who,
                             const wxString& sMessage,
                             bool            bPublic /* = true */ )
{
    hoxWallOutput* outputUI  = ( m_wallOutput ? m_wallOutput
                                              : m_systemOutput );
    outputUI->AppendMessage( who, sMessage, bPublic );
}

void 
hoxBoard::OnValidMove( const hoxMove& move,
                       bool           bSetupMode /* = false */ )
{
    /* For the 1st move of BLACK, change the game-status to 'in-progress'. 
     */
    if ( m_status == hoxGAME_STATUS_READY
      && move.piece.color == hoxCOLOR_BLACK )
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

		const bool bIsChesscape = (m_initialTime.nMove == 0);

        hoxTimeInfo* pCurrTime = &m_blackTime;
        hoxTimeInfo* pNextTime = &m_redTime;
        if ( move.piece.color == hoxCOLOR_RED )
        {
            pCurrTime = &m_redTime;
            pNextTime = &m_blackTime;
        }

        pNextTime->nMove = m_initialTime.nMove;
        pNextTime->nFree = m_initialTime.nFree;
		if ( bIsChesscape ) pCurrTime->nGame += m_initialTime.nFree;
    }

    /* Play the sound for the MOVE. */
    if ( m_bSoundEnabled && !bSetupMode )
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
hoxBoard::_GetGameOverMessage( const hoxGameStatus gameStatus ) const
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


// ----------------------------------------------------------------------------
//
//                    hoxPracticeBoard
//
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(hoxPracticeBoard, hoxBoard)
    EVT_COMMAND_SCROLL(wxID_ANY, hoxPracticeBoard::OnAISliderUpdate)
END_EVENT_TABLE()

bool
hoxPracticeBoard::OnBoardAskMovePermission( const hoxPieceInfo& pieceInfo )
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

    if (  !m_aiSettings->IsPlayWithSelf()
        && pieceInfo.color != m_localColor )
    {
        return false;
    }

    return true;  // OK. The Piece can be moved.
}

void
hoxPracticeBoard::CreateAndLayoutWallPanel()
{
    /* Create the Wall's content. */

    m_playerListBox = new hoxPlayersUI( this, hoxPlayersUI::UI_TYPE_TABLE );
    m_playerListBox->SetOwner( this );

    m_systemOutput = new hoxWallOutput( this, wxID_ANY, _("Activities") );
    m_aiSettings   = new hoxAISettings( this, wxID_ANY, _("AI Settings"), 5 /* AI Level */ );

    /* Arrange the Wall. */

    m_wallSizer->Add( m_playerListBox, wxSizerFlags(1).Expand() );
    m_wallSizer->Add( m_systemOutput,  wxSizerFlags(1).Expand() );
    m_wallSizer->Add( m_aiSettings,    wxSizerFlags(3).Expand() );
}

void 
hoxPracticeBoard::OnBoardMove( const hoxMove& move,
                               hoxGameStatus  status )
{
    this->OnValidMove( move );

    /* If not in Play-with-self mode, inform the Table of the new move. */

    if ( ! m_aiSettings->IsPlayWithSelf() )
    {
        const hoxTimeInfo playerTime = ( move.piece.color == hoxCOLOR_RED
                                        ? m_redTime : m_blackTime );
        m_pTable->OnMove_FromBoard( move, status, playerTime );
    }
}

void 
hoxPracticeBoard::OnPlayerJoin( const hoxPlayerInfo playerInfo,
                                const hoxColor      playerColor )
{
    this->hoxBoard::OnPlayerJoin( playerInfo, playerColor );

    if ( playerInfo.type == hoxPLAYER_TYPE_AI )
    {
        wxString sInfo;
        hoxPracticeTable* practiceTable( wxDynamicCast(m_pTable.get(), hoxPracticeTable) );
        if ( practiceTable )
        {
            sInfo = practiceTable->GetAIInfo();
            m_aiSettings->SetAIInfo( sInfo );
        }
    }
}

void
hoxPracticeBoard::OnAISliderUpdate( wxScrollEvent& event )
{
    const int nAILevel = event.GetInt();
    wxLogDebug("%s: AI Level updated to [%d].", __FUNCTION__, nAILevel);

    hoxPracticeTable* practiceTable( wxDynamicCast(m_pTable.get(), hoxPracticeTable) );
    if ( practiceTable )
    {
        practiceTable->OnAILevelUpdate( nAILevel );
    }
}

/************************* END OF FILE ***************************************/
