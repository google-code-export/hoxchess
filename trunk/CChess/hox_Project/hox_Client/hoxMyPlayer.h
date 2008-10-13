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
// Description:     The LOCAL Player specialized to login to
//                  my "games.PlayXiangqi.com" server
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_MY_PLAYER_H__
#define __INCLUDED_HOX_MY_PLAYER_H__

#include "hoxLocalPlayer.h"
#include "hoxTypes.h"

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
    void _HandleResponseEvent_LOGIN( const wxString&         sCode,
                                     const wxString&         sContent,
                                     const hoxResponse_APtr& apResponse );

    void _HandleResponseEvent_LOGOUT( const wxString&         sContent,
                                      const hoxResponse_APtr& apResponse );

    void _ParsePlayerLoginEvent( const wxString& sContent,
                                 wxString&       playerId,
                                 int&            nPlayerScore );

    hoxResult _ParseNetworkTables( const wxString&          responseStr,
                                   hoxNetworkTableInfoList& tableList );

    hoxResult _ParsePlayerLeaveEvent( const wxString& sContent,
                                      hoxTable_SPtr&  pTable,
                                      hoxPlayer*&     player );

    hoxResult _ParseTableUpdateEvent( const wxString& sContent,
                                      hoxTable_SPtr&  pTable,
                                      hoxPlayer*&     player,
                                      bool&           bRatedGame,
                                      hoxTimeInfo&    newTimeInfo );

    hoxResult _ParsePlayerJoinEvent( const wxString& sContent,
                                     wxString&       tableId,
                                     wxString&       playerId,
                                     int&            nPlayerScore,
                                     hoxColor&       color);

    hoxResult _ParsePlayerMsgEvent( const wxString& sContent,
                                    hoxTable_SPtr&  pTable,
                                    wxString&       playerId,
                                    wxString&       message );

    hoxResult _ParsePlayerMoveEvent( const wxString& sContent,
                                     hoxTable_SPtr&  pTable,
                                     hoxPlayer*&     player,
                                     wxString&       sMove );

    hoxResult _ParsePlayerDrawEvent( const wxString& sContent,
                                     hoxTable_SPtr&  pTable,
                                     hoxPlayer*&     player );

    hoxResult _ParsePlayerEndEvent( const wxString& sContent,
                                    hoxTable_SPtr&  pTable,
                                    hoxGameStatus&  gameStatus,
                                    wxString&       sReason );

    hoxResult _ParsePlayerResetEvent( const wxString& sContent,
                                      hoxTable_SPtr&  pTable );

    hoxResult _ParsePlayerScoreEvent( const wxString& sContent,
                                      hoxTable_SPtr&  pTable,
                                      hoxPlayer*&     player,
                                      int&            nScore );

    hoxResult _ParsePastMovesEvent( const wxString& sContent,
                                    hoxTable_SPtr&  pTable,
                                    hoxStringList&  moves );

    hoxResult _ParsePlayerInfoEvent( const wxString& sContent,
                                     hoxPlayerStats& playerStats );

    hoxResult _ParseMovesString( const wxString& sMoves,
                                 hoxStringList&  moves );

private:
    bool      m_bLoginSuccess;  /* Has this Player logged in successfully? */ 


    DECLARE_DYNAMIC_CLASS(hoxMyPlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_MY_PLAYER_H__ */
