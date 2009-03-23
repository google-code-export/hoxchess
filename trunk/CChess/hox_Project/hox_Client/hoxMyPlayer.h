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
// Name:            hoxMyPlayer.h
// Created:         10/28/2007
//
// Description:     The LOCAL Player specialized to login to
//                  my "games.PlayXiangqi.com" server
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_MY_PLAYER_H__
#define __INCLUDED_HOX_MY_PLAYER_H__

#include "hoxLocalPlayer.h"

/**
 * The MY player.
 */
class hoxMyPlayer :  public hoxLocalPlayer
{
public:
    hoxMyPlayer() {} // DUMMY constructor required for event handler.
    hoxMyPlayer( const wxString& name,
                 hoxPlayerType   type,
                 int             score );

    virtual ~hoxMyPlayer() {}

    virtual void Start();

public:

    /*******************************
     * Socket-event handlers
     *******************************/

    void OnConnectionResponse_PlayerData( wxCommandEvent& event ); 
    void OnConnectionResponse( wxCommandEvent& event ); 

private:
    hoxResult _ParseCommand( const wxMemoryBuffer& data, 
                             hoxCommand&           command ) const;

    void _HandleEvent_LOGIN( const wxString&         sCode,
                             const wxString&         sContent,
                             const hoxResponse_APtr& apResponse );
    void _HandleEvent_LOGOUT( const wxString& sContent );
    void _HandleEvent_LIST( const wxString& sContent );
    void _HandleEvent_I_PLAYERS( const wxString& sContent );
    void _HandleEvent_I_TABLE( const wxString& sContent );
    void _HandleEvent_LEAVE( const wxString& sContent );
    void _HandleEvent_UPDATE( const wxString& sContent );
    void _HandleEvent_E_JOIN( const wxString& sContent );
    void _HandleEvent_MSG( const wxString&      sTableId,
                           const wxString&      sContent,
                           const hoxTable_SPtr& pTable );
    void _HandleEvent_MOVE( const wxString& sContent );
    void _HandleEvent_DRAW( const wxString& sContent );
    void _HandleEvent_RESET( const wxString& sContent );
    void _HandleEvent_E_END( const wxString& sContent );
    void _HandleEvent_E_SCORE( const wxString& sContent );
    void _HandleEvent_I_MOVES( const wxString& sContent );
    void _HandleEvent_INVITE( const wxString& sTableId,
                              const wxString& sContent );
    void _HandleEvent_PLAYER_INFO( const wxString& sContent );

	/* Private event-handlers */

    void _OnLoginFailure( const hoxResponse_APtr& apResponse );

    /* Private parsers */

    void _ParseNetworkTable( const wxString&      sTable,
                             hoxNetworkTableInfo& tableInfo,
                             hoxStringList*       pObservers = NULL );

    hoxResult _ParseMovesString( const wxString& sMoves,
                                 hoxStringList&  moves );

private:
    bool      m_bLoginSuccess;  /* Has this Player logged in successfully? */ 


    DECLARE_DYNAMIC_CLASS(hoxMyPlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_MY_PLAYER_H__ */
