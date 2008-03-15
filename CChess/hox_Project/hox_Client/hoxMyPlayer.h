/***************************************************************************
 *  Copyright 2007, 2008 Huy Phan  <huyphan@playxiangqi.com>               *
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
// Description:     The new "advanced" LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_MY_PLAYER_H_
#define __INCLUDED_HOX_MY_PLAYER_H_

#include <wx/wx.h>
#include "hoxLocalPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/**
 * The MY player.
 */
class hoxMyPlayer :  public hoxLocalPlayer
{
public:
    hoxMyPlayer(); // DUMMY default constructor required for event handler.
    hoxMyPlayer( const wxString& name,
                 hoxPlayerType   type,
                 int             score );

    virtual ~hoxMyPlayer();

public:

    /*******************************
     * Socket-event handlers
     *******************************/

    void OnIncomingNetworkData( wxSocketEvent& event );
    
    void OnConnectionResponse_PlayerData( wxCommandEvent& event ); 
    void OnConnectionResponse( wxCommandEvent& event ); 

private:
    hoxResult _HandleResponseEvent_LOGIN( const wxString& sContent );

    hoxResult _ParsePlayerLoginEvent( const wxString& sContent,
                                      wxString&       playerId,
                                      int&            nPlayerScore );

    hoxResult _ParseNetworkTables( const wxString&          responseStr,
                                   hoxNetworkTableInfoList& tableList );

    hoxResult _ParsePlayerLeaveEvent( const wxString& sContent,
                                      hoxTable*&      table,
                                      hoxPlayer*&     player );

    hoxResult _ParsePlayerJoinEvent( const wxString& sContent,
                                     wxString&       tableId,
                                     wxString&       playerId,
                                     int&            nPlayerScore,
                                     hoxColor&       color);

    hoxResult _ParsePlayerMsgEvent( const wxString& sContent,
                                    hoxTable*&      table,
                                    wxString&       playerId,
                                    wxString&       message );

    hoxResult _ParsePlayerMoveEvent( const wxString& sContent,
                                     hoxTable*&      table,
                                     hoxPlayer*&     player,
                                     wxString&       sMove );

    hoxResult _ParsePlayerDrawEvent( const wxString& sContent,
                                     hoxTable*&      table,
                                     hoxPlayer*&     player );

    hoxResult _ParsePlayerEndEvent( const wxString& sContent,
                                    hoxTable*&      table,
                                    hoxGameStatus&  gameStatus,
                                    wxString&       sReason );

private:

    DECLARE_DYNAMIC_CLASS(hoxMyPlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_MY_PLAYER_H_ */
