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
// Description:     The "simple" Board with player-info(s) + timers.
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
class hoxPosition;
class hoxPlayer;

/**
 *   Board layout:
 *
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
 *
 */

/** 
 * Boarded-related player events (players entering / leaving tables.)
 */
DECLARE_EVENT_TYPE(hoxEVT_BOARD_PLAYER_JOIN, wxID_ANY)
DECLARE_EVENT_TYPE(hoxEVT_BOARD_PLAYER_LEAVE, wxID_ANY)
DECLARE_EVENT_TYPE(hoxEVT_BOARD_WALL_OUTPUT, wxID_ANY)


class hoxSimpleBoard : public wxPanel
                     , public hoxCoreBoard::BoardOwner
{
public:
    /* Construct an "hidden" Board. */
    hoxSimpleBoard( wxWindow*       parent,
                    const wxString& piecesPath,
                    hoxIReferee*    referee,
                    const wxPoint&  pos = wxDefaultPosition, 
                    const wxSize&   size = wxDefaultSize );

    virtual ~hoxSimpleBoard();

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
     * a message, which may need the end-user's attention, occurs.
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

    /****************************************
     * Override the parent (wxPanel) 's API.
     ****************************************/

    virtual bool Show(bool show /* = true */);

    /*********************************
     * My MAIN public API
     *********************************/

    void SetRedInfo( const hoxPlayer* player );
    void SetBlackInfo( const hoxPlayer* player );

    void SetTable( hoxTable* table );

    hoxIReferee* GetReferee() const;

    void ToggleViewSide();

    /**
     * Do a Move on a Board. This is usually invoked indirectly 
     * from a NETWORK Player.
     *
     * @note This move will trigger referee-validation.
     */
    bool DoMove( hoxMove& move );


private:
    void _CreateBoardPanel();
    void _LayoutBoardPanel( bool viewInverted );
    
    void _AddPlayerToList( const wxString& playerId, int playerScore );
    void _RemovePlayerFromList( const wxString& playerId );

    void _PostToWallOutput( const wxString& who,
                            const wxString& message );

private:
    hoxCoreBoard*     m_coreBoard;  // The "core" board.
    hoxTable*         m_table;  // The table to which this board belongs.

    /* Players */

    typedef std::list<wxString> StringList;

    wxString          m_redId;
    wxString          m_blackId;
    StringList        m_observerIds;

    /* Player Info(s) + timers */

    wxStaticText*     m_blackInfo;      // Black's info.
    wxStaticText*     m_redInfo;        // Red's info.
    wxStaticText*     m_blackGameTime;  // Black's game-time.
    wxStaticText*     m_redGameTime;    // Red's game-time.
    wxStaticText*     m_blackMoveTime;  // Black's move-time.
    wxStaticText*     m_redMoveTime;    // Red's move-time.
    wxStaticText*     m_blackFreeTime;  // Black's free-time.
    wxStaticText*     m_redFreeTime;    // Red's free-time.

    /* Controls on for our Wall */

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
