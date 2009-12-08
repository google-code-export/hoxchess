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
// Name:            hoxBoard.h
// Created:         10/05/2007
//
// Description:     A "simple" Board with the following features:
//                     + Player's information (such as Name, Score).
//                     + Timers (including Game, Move, and Free times).
//                     + Game History (forward/backward 'past' Moves).
//                     + Chat feature (Text Input + Wall Output).
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_BOARD_H__
#define __INCLUDED_HOX_BOARD_H__

#include "hoxTypes.h"
#include "hoxCoreBoard.h"
#include "hoxPlayersUI.h"
#include <wx/sound.h>

/* Forward declarations. */
class hoxWallOutput;
class hoxAISettings;

/** 
 * A full-featured Board acting as the Table's UI.
 * It has the following features:
 *    - Player's information (such as Name, Score).
 *    - Timers (including Game, Move, and Free times).
 *    - Game History (forward/backward 'past' Moves).
 *    - Chat feature (Text Input + Wall Output).
 *
 *  Board layout:
 * <pre>
 *  +------------------+
 *  | Player-Info |    |
 *  +------------------+
 *  |                  |
 *  |                  |
 *  |    Core Board    |
 *  |                  |
 *  +------------------+
 *  | Player-Info |    |
 *  +------------------+
 * </pre>
 */
class hoxBoard : public wxPanel
               , public hoxCoreBoard::BoardOwner
               , public hoxPlayersUI::UIOwner
{
public:
    /**
     * The Board's Feature flags.
     */
    enum FeatureEnum
    {
	    /* NOTE: The numeric values are of the 32-bitmap ones. */

        hoxBOARD_FEATURE_RESET        = ( (unsigned int) 1 ),
                /* RESET action */

        hoxBOARD_FEATURE_UNSIT        = ( (unsigned int) 2 ),
                /* UNSIT action */

        hoxBOARD_FEATURE_ALL = 0xffffffff
                /* Special flag: ALL FEATURES enabled. */
    };

public:
    /* Construct an "hidden" Board. */
    hoxBoard( wxWindow*        parent,
              const wxString&  piecesPath,
              hoxIReferee_SPtr referee,
              hoxTable_SPtr    pTable,
              const wxString&  ownerId,
              const wxString&  sBgImage,
              wxColor          bgColor,
              wxColor          fgColor,
              const wxPoint&   pos = wxDefaultPosition, 
              const wxSize&    size = wxDefaultSize,
              unsigned int     featureFlags = hoxBOARD_FEATURE_ALL );

    virtual ~hoxBoard();

    /************************************
     * Implement BoardOwner's interface.
     ************************************/

    /**
     * A callback function invoked by the core Board when
     * a physical Move (or, Board-Move) is made on the Board.
     *
     * @note This Move has been validated by the Referee.
     */
    virtual void OnBoardMove( const hoxMove& move,
		                      hoxGameStatus  status );

    /**
     * A callback function invoked by the core Board when
     * a message, which may require the end-user's attention, occurs.
     */
    virtual void OnBoardMsg( const wxString& message );

    /**
     * A callback function invoked by the core Board when
     * it wants to know whether a Piece can move next.
     */
    virtual bool OnBoardAskMovePermission( const hoxPieceInfo& pieceInfo );

    /*************************************************
     * Implement hoxPlayersUI::UIOwner 's interface.
     *************************************************/

    virtual void OnPlayersUIEvent( hoxPlayersUI::EventType eventType,
                                   const wxString&         sPlayerId );

    /*********************************
     * My custom event handler.
     *********************************/

    virtual void OnPlayerJoin( const hoxPlayerInfo playerInfo,
                               const hoxColor      playerColor );
    void OnPlayerLeave( const wxString& playerId );
    void OnPlayerScore( const wxString& playerId,
                        const int       nScore );
    void OnSystemOutput( const wxString& sMessage );
    void OnWallOutput( const wxString& sMessage,
                       const wxString& sSenderId,
                       const bool      bPublic );
	void OnNewMove( const wxString& sMove );
    void OnPastMoves( const hoxStringList& moves );
	void OnDrawRequest( const wxString& playerId,
                        const bool      bPopupRequest );
	void OnGameOver( const hoxGameStatus gameStatus,
                     const wxString&     sReason );
    void OnGameReset();
    void OnTableUpdate();

    void OnWallInputEnter( wxCommandEvent& event );

    void OnButtonHistory_BEGIN( wxCommandEvent& event );
    void OnButtonHistory_PREV( wxCommandEvent& event );
    void OnButtonHistory_NEXT( wxCommandEvent& event );
    void OnButtonHistory_END( wxCommandEvent& event );

    void OnButtonReverse( wxCommandEvent& event );
    void OnButtonOptions( wxCommandEvent& event );
    void OnButtonResign( wxCommandEvent& event );
    void OnButtonDraw( wxCommandEvent& event );
    void OnButtonReset( wxCommandEvent& event );
    void OnButtonRed( wxCommandEvent& event );
    void OnButtonBlack( wxCommandEvent& event );
    void OnButtonNone( wxCommandEvent& event );

    void OnUpdateUI_ActionRed( wxUpdateUIEvent& event );
    void OnUpdateUI_ActionBlack( wxUpdateUIEvent& event );
    void OnUpdateUI_ActionNone( wxUpdateUIEvent& event );
    void OnUpdateUI_ActionResign( wxUpdateUIEvent& event );
    void OnUpdateUI_ActionDraw( wxUpdateUIEvent& event );
    void OnUpdateUI_ActionReset( wxUpdateUIEvent& event );

    void OnTimer( wxTimerEvent& event );

    /*********************************
     * My MAIN public API
     *********************************/

    void EnableSound( bool bEnabled ) { m_bSoundEnabled = bEnabled; }
    void SetBackgroundImage( const wxString& sImage );
    void SetBgColor( wxColor color ); // Set Board's Background Color
    void SetFgColor( wxColor color ); // Set Board's Foreground Color
    void SetPiecesPath( const wxString& piecesPath );
    void Repaint(); // Paint again using the current settings.

    void ShowUI();

protected:
    virtual void CreateAndLayoutWallPanel();

    /**
     * A helper to do various task whenever a valid Move has been made.
     */
    void OnValidMove( const hoxMove& move,
		              bool           bSetupMode = false );

private:
    void _SetRedInfo( const wxString& playerId,
                      const int       nScore = 0 );
    void _SetBlackInfo( const wxString& playerId,
                        const int       nScore = 0 );
    void _CreateBoardUI();
    void _CreateBoardPanel();
    void _LayoutBoardPanel( bool bViewInverted );

    void _ReverseView();

    /**
     * Add a Player's Id to the Player-List GUI.
     * @return true if the Player is NEWLY inserted.
     */
    bool _AddPlayerToList( const wxString& playerId,
                           const int       nScore );

    /**
     * Update a Player's Score in the Player-List GUI.
     * Do nothing if the Player is not found.
     */
    void _UpdatePlayerScore( const wxString& playerId,
                             const int       nScore );

    /**
     * Remove a Player's Id from the Player-List GUI.
     * @return true if the Player was actually there and removed.
     */
    bool _RemovePlayerFromList( const wxString& playerId );

    /**
     * Post a System-message (i.e, triggered by JOINED/LEFT events,...) 
     * to the System-Output GUI.
     */
    void _PostToSystemOutput( const wxString& sMessage );

    /**
     * Post a Wall-message (i.e, CHAT message,...) to the Wall-Output GUI.
     */
    void _PostToWallOutput( const wxString& who,
                            const wxString& sMessage,
                            bool            bPublic = true );

    void _UpdateStatus();

    void _SyncInfoWithTable();
    void _OnTimerUpdated();

    wxString _GetGameOverMessage( const hoxGameStatus gameStatus ) const;

    /**
     * Check if the Owner is currently seated 
     * (e.g. having role of RED or BLACK).
     */
    bool _IsOwnerSeated() const;

protected:
    hoxCoreBoard*     m_coreBoard;  // The "core" board.
    hoxIReferee_SPtr  m_referee;    // The Referee.
    hoxTable_SPtr     m_pTable;     // The Table to which this Board belongs.

    hoxGameStatus     m_status;     // The game's status.
    hoxColor          m_localColor; // The color of the "local" player.

    /* Players */

    const wxString    m_ownerId;
            /* The Id of the Board-Owner.
             * NOTE: The concept of ownership is required to trigger
             *       ACTIONs (draw, resign,...) on the Board.
             */

    wxString          m_redId;
    wxString          m_blackId;

    /* Features */
    unsigned int      m_featureFlags;

    /* Rated/Non-Rated Game option. */
    bool              m_bRated;

    /* Timers */

    wxTimer*          m_timer;       // To keep track of time.
	hoxTimeInfo       m_initialTime; // *** Initial time.
	hoxTimeInfo       m_blackTime;   // Black's time.
	hoxTimeInfo       m_redTime;     // Red's time.

    /* The flag to indicate whether all UI elements have been created.
     * NOTE: This flag is needed because we do not automatically create
     *       those elements in the constructor.
     */
    bool              m_bUICreated;

    /* Player Info(s) + timers UI */

    wxButton*         m_btnPlayRed;
    wxButton*         m_btnPlayBlack;

    wxStaticText*     m_blackInfo;      // Black's info UI.
    wxStaticText*     m_redInfo;        // Red's info UI.
    wxStaticText*     m_blackGameTime;  // Black's Game-time UI.
    wxStaticText*     m_redGameTime;    // Red's Game-time UI.
    wxStaticText*     m_blackMoveTime;  // Black's Move-time UI.
    wxStaticText*     m_redMoveTime;    // Red's Move-time UI.
    wxStaticText*     m_blackFreeTime;  // Black's Free-time UI.
    wxStaticText*     m_redFreeTime;    // Red's Free-time UI.

    /* UI-Controls for non-board parts. */

    hoxPlayersUI*     m_playerListBox;

    hoxWallOutput*    m_systemOutput;
    hoxWallOutput*    m_wallOutput;
    wxTextCtrl*       m_wallInput;

    /* UI-Layout variables. */

    wxBoxSizer*       m_mainSizer;
    wxBoxSizer*       m_boardSizer;
    wxBoxSizer*       m_wallSizer;
    
    wxBoxSizer*       m_redSizer;
    wxBoxSizer*       m_blackSizer;

	wxBoxSizer*       m_commandSizer; // (Move-History + Action) sizer.
    wxBoxSizer*       m_historySizer; // Move-History sizer.
	wxBoxSizer*       m_actionSizer;  // Action (JOIN,...) sizer.

    /* Sound variables. */

    bool              m_bSoundEnabled; // Sound is enabled?
    wxSound           m_soundMove;     // Sound of a MOVE

    DECLARE_EVENT_TABLE()
};

/**
 * A PRACTICE Board in which the local player is playing with his computer.
 */
class hoxPracticeBoard : public hoxBoard
{
public:
    hoxPracticeBoard( wxWindow*        parent,
                      const wxString&  piecesPath,
                      hoxIReferee_SPtr referee,
                      hoxTable_SPtr    pTable,
                      const wxString&  ownerId,
                      const wxString&  sBgImage,
                      wxColor          bgColor,
                      wxColor          fgColor,
                      const wxPoint&   pos = wxDefaultPosition, 
                      const wxSize&    size = wxDefaultSize,
                      unsigned int     featureFlags = hoxBOARD_FEATURE_ALL )
        : hoxBoard( parent, piecesPath, referee, pTable, ownerId,
                    sBgImage, bgColor, fgColor, pos, size, featureFlags )
        , m_aiSettings( NULL )
        { /* empty */ }

    virtual ~hoxPracticeBoard() {}

protected:
    /* Override the parent' s API */
    virtual bool OnBoardAskMovePermission( const hoxPieceInfo& pieceInfo );
    virtual void CreateAndLayoutWallPanel();
    virtual void OnBoardMove( const hoxMove& move,
		                      hoxGameStatus  status );
    virtual void OnPlayerJoin( const hoxPlayerInfo playerInfo,
                               const hoxColor      playerColor );
    /* My own API */
    void OnAISliderUpdate( wxScrollEvent& event );

private:
    hoxAISettings*    m_aiSettings;

    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_BOARD_H__ */
