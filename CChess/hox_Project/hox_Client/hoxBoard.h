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
// Name:            hoxBoard.h
// Created:         10/05/2007
//
// Description:     A "simple" Board with the following features:
//                     + Player's information (such as Name, Score).
//                     + Timers (including Game, Move, and Free times).
//                     + Game History (forward/backward 'past' Moves).
//                     + Chat feature (Text Input + Wall Output).
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_BOARD_H_
#define __INCLUDED_HOX_BOARD_H_

#include <wx/wx.h>
#include <list>
#include "hoxTypes.h"
#include "hoxCoreBoard.h"

/* Forward declarations */
class hoxIReferee;
class hoxTable;
class hoxPlayer;

DECLARE_EVENT_TYPE(hoxEVT_BOARD_PLAYER_JOIN, wxID_ANY)
DECLARE_EVENT_TYPE(hoxEVT_BOARD_PLAYER_LEAVE, wxID_ANY)
DECLARE_EVENT_TYPE(hoxEVT_BOARD_WALL_OUTPUT, wxID_ANY)


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
{
public:
    /* Construct an "hidden" Board. */
    hoxBoard( wxWindow*       parent,
              const wxString& piecesPath,
              hoxIReferee*    referee,
              const wxPoint&  pos = wxDefaultPosition, 
              const wxSize&   size = wxDefaultSize );

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
    virtual void OnBoardMove( const hoxMove& move );

    /**
     * A callback function invoked by the core Board when
     * a message, which may require the end-user's attention, occurs.
     */
    virtual void OnBoardMsg( const wxString& message );

    /*********************************
     * My custom event handler.
     *********************************/

    void OnPlayerJoin( wxCommandEvent &event );
    void OnPlayerLeave( wxCommandEvent &event );
    void OnWallOutput( wxCommandEvent &event );

    void OnWallInputEnter( wxCommandEvent &event );

    void OnButtonHistory_BEGIN( wxCommandEvent &event );
    void OnButtonHistory_PREV( wxCommandEvent &event );
    void OnButtonHistory_NEXT( wxCommandEvent &event );
    void OnButtonHistory_END( wxCommandEvent &event );

    void OnTimer( wxTimerEvent& event );

    /****************************************
     * Override the parent (wxPanel) 's API.
     ****************************************/

    virtual bool Show(bool show /* = true */);

    /*********************************
     * My MAIN public API
     *********************************/

    void SetTable( hoxTable* table );

    void ToggleViewSide();

    /**
     * Do a Move on a Board. This is usually invoked indirectly 
     * from a NETWORK Player.
     *
     * @note This Move will trigger Referee-validation.
     */
    bool DoMove( hoxMove& move );

private:
    void _SetRedInfo( const hoxPlayer* player );
    void _SetBlackInfo( const hoxPlayer* player );

    void _CreateBoardPanel();
    void _LayoutBoardPanel( bool viewInverted );
    
    void _AddPlayerToList( const wxString& playerId, int playerScore );
    void _RemovePlayerFromList( const wxString& playerId );

    void _PostToWallOutput( const wxString& who,
                            const wxString& message );

    /**
     * A helper to do various task whenever a valid Move has been made.
     */
    void _OnValidMove( const hoxMove& move );

    void           _ResetTimerUI();  // Reset times to start a new game.
    void           _UpdateTimerUI();
    const wxString _FormatTime( int nTime ) const;

private:
    hoxCoreBoard*     m_coreBoard;  // The "core" board.
    hoxIReferee*      m_referee;    // The Referee.
    hoxTable*         m_table;      // The Table to which this Board belongs.

    hoxGameStatus     m_status;     // The game's status.

    /* Players */

    typedef std::list<wxString> StringList;

    wxString          m_redId;
    wxString          m_blackId;
    StringList        m_observerIds;

    /* Timers */

    wxTimer*          m_timer;       // To keep track of time.
    int               m_nBGameTime;  // Black's Game-time.
    int               m_nRGameTime;  // Red's Game-time.
    int               m_nBMoveTime;  // Black's Move-time.
    int               m_nRMoveTime;  // Red's Move-time.
    int               m_nBFreeTime;  // Black's Free-time.
    int               m_nRFreeTime;  // Red's Free-time.

    /* Player Info(s) + timers UI */

    wxStaticText*     m_blackInfo;      // Black's info UI.
    wxStaticText*     m_redInfo;        // Red's info UI.
    wxStaticText*     m_blackGameTime;  // Black's Game-time UI.
    wxStaticText*     m_redGameTime;    // Red's Game-time UI.
    wxStaticText*     m_blackMoveTime;  // Black's Move-time UI.
    wxStaticText*     m_redMoveTime;    // Red's Move-time UI.
    wxStaticText*     m_blackFreeTime;  // Black's Free-time UI.
    wxStaticText*     m_redFreeTime;    // Red's Free-time UI.

    /* Controls for our Wall */

    wxListBox*        m_playerListBox;
    wxTextCtrl*       m_wallOutput;
    wxTextCtrl*       m_wallInput;

    /* Convenient variables. */

    wxBoxSizer*       m_mainSizer;
    wxBoxSizer*       m_boardSizer;
    wxBoxSizer*       m_wallSizer;
    
    wxBoxSizer*       m_redSizer;
    wxBoxSizer*       m_blackSizer;

    wxBoxSizer*       m_historySizer; // Move-History sizer.

    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_BOARD_H_ */
