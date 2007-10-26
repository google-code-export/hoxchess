/////////////////////////////////////////////////////////////////////////////
// Name:            hoxBoard.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/05/2007
//
// Description:     The "simple" Board with player-info(s) + timers.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_BOARD_H_
#define __INCLUDED_HOX_BOARD_H_

#include "wx/wx.h"

/* Forward declarations */
class hoxCoreBoard;
class hoxIReferee;
class hoxTable;
class hoxPosition;
class hoxMove;

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
DECLARE_EVENT_TYPE(hoxEVT_BOARD_PLAYER_INFO, wxID_ANY)

class hoxSimpleBoard : public wxPanel
{
public:
    /* Construct an "hidden" Board. */
    hoxSimpleBoard( wxWindow*       parent,
                    const wxString& piecesPath,
                    hoxIReferee*    referee );

    virtual ~hoxSimpleBoard();

    /*********************************
     * My custom event handler.
     *********************************/

    void OnPlayerInfoEvent( wxCommandEvent &event );

    /*********************************
     * Override the parent's API.
     *********************************/

    virtual bool Show(bool show /* = true */);

    /*********************************
     * My MAIN public API
     *********************************/

    void SetRedInfo( const wxString& info );
    void SetBlackInfo( const wxString& info );

    void SetTable( hoxTable* table );

    hoxIReferee* GetReferee() const;

    void ToggleViewSide();

    /**
     * Do a Move on a Board. This is usually invoked indirectly 
     * from a NETWORK Player.
     *
     * @note This move will trigger referee-validation.
     */
    bool DoMove( const hoxMove& move );


private:
    void _CreateBoardPanel();
    void _LayoutBoardPanel( wxSizer* mainSizer, 
                            wxSizer* redSizer, 
                            wxSizer* blackSizer, 
                            bool     viewInverted );

private:
    hoxCoreBoard*     m_coreBoard;  // The "core" board.

    /* Player Info(s) + timers */

    wxStaticText*     m_blackInfo;      // Black's info.
    wxStaticText*     m_redInfo;        // Red's info.
    wxStaticText*     m_blackGameTime;  // Black's game-time.
    wxStaticText*     m_redGameTime;    // Red's game-time.
    wxStaticText*     m_blackMoveTime;  // Black's move-time.
    wxStaticText*     m_redMoveTime;    // Red's move-time.
    wxStaticText*     m_blackFreeTime;  // Black's free-time.
    wxStaticText*     m_redFreeTime;    // Red's free-time.


    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_BOARD_H_ */
