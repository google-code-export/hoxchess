/////////////////////////////////////////////////////////////////////////////
// Name:            hoxTable.h
// Program's Name:  Huy's Open Xiangqi
// Created:         09/30/2007
//
// Description:     The Table that controls a single table with the following:
//                    + A referee
//                    + Players
//                    + A Board (maybe optional).
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_TABLE_H_
#define __INCLUDED_HOX_TABLE_H_

#include "wx/wx.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/* Forward declarations */
class hoxIReferee;
class hoxPlayer;
class hoxSimpleBoard;

/**
 * Representing a logical Table consisting of the following:
 *      + One Board (with all pieces + One Referee).
 *      + One RED Player.
 *      + One BLACK Player.
 */
class hoxTable
{
  public:
    hoxTable( const wxString&   id,
              hoxIReferee*      referee,
              hoxSimpleBoard*   board = NULL );
    
    virtual ~hoxTable();

    /*********************************
     * My MAIN public API
     *********************************/

    const wxString GetId() const { return m_id; }

    /**
     * Assign a player to this Table.
     *
     * @note Side affects:
     *    (1) The Role of the player is changed.
     *        The assigned role can be RED, BLACK, or NONE (aka. observer).
     *        The caller can query the new role by calling player->GetColor()
     *
     *    (2) If successful, this Table will take over the memory management
     *        of the player.
     */
    hoxResult AssignPlayer( hoxPlayer* player );

    hoxResult UnassignPlayer( hoxPlayer* player );
    hoxResult UnassignAllPlayers();  // used when closing a table.

    hoxPlayer* GetRedPlayer() const { return m_redPlayer; }
    hoxPlayer* GetBlackPlayer() const { return m_blackPlayer; }

    /**
     * Set the Board (the Table's GUI).
     */
    void SetBoard( hoxSimpleBoard* board );

    hoxSimpleBoard* GetBoard() const { return m_board; }


    /**
     * Callback function from the Board to let this Table know about
     * physical (Board) Moves.
     *
     * @param move The current design assumes that the Board has contacted
     *             the referee to validate the Move.
     */
    void OnMove_FromBoard( const hoxMove& move );

    /**
     * Callback function from the NETWORK Player to let this Table know about
     * the newly-received "remote" Moves.
     *
     * @param player The parameter is passed in so that the Table does not have
     *               to inform that player about this move (to avoid getting 
     *               into an endless loop).
     */
    void OnMove_FromNetwork( hoxPlayer*         player,
                             const hoxPosition& fromPosition,
                             const hoxPosition& toPosition );

    /**
     * Callback function from the WWW NETWORK Player to let this Table know about
     * the newly-received network event.
     */
    void OnEvent_FromWWWNetwork( hoxPlayer*         player,
                                 const hoxNetworkEvent& networkEvent );

    void ToggleViewSide();

  private:
    hoxResult _ParseMoveString( const wxString& moveStr, hoxMove& move );


  private:
    const wxString   m_id;       // The table's ID.

    hoxIReferee*     m_referee;  // The referee

    hoxSimpleBoard*  m_board;    // The (OPTIONAL) Board.

    // Players
    hoxPlayer*       m_redPlayer;
    hoxPlayer*       m_blackPlayer;
    
};

#endif /* __INCLUDED_HOX_TABLE_H_ */
