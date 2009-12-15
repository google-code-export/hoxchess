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
// Name:            hoxLocalPlayer.h
// Created:         10/28/2007
//
// Description:     The LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_LOCAL_PLAYER_H__
#define __INCLUDED_HOX_LOCAL_PLAYER_H__

#include "hoxPlayer.h"
#include "hoxTypes.h"

class hoxChatWindow;

/**
 * The LOCAL player.
 */
class hoxLocalPlayer :  public hoxPlayer
{
public:
    hoxLocalPlayer() {}
    hoxLocalPlayer( const wxString& name,
                    hoxPlayerType   type,
                    int             score );

    virtual ~hoxLocalPlayer();

    virtual void Start();

public:

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void OnClose_FromTable( const wxString& tableId );

    /*******************************
     * MY-specific Network API
     *******************************/

    virtual void ConnectToServer();
    virtual void DisconnectFromServer();

    virtual hoxResult QueryForNetworkTables();
    virtual hoxResult JoinNetworkTable( const wxString& tableId );
    virtual hoxResult OpenNewNetworkTable();
    virtual hoxResult LeaveNetworkTable( const wxString& tableId );

    virtual hoxResult QueryPlayerInfo( const wxString& sInfoId );
    virtual hoxResult InvitePlayer( const wxString& sInviteeId );
    virtual hoxResult SendPrivateMessage( const wxString& sOtherId,
                                          const wxString& message );

    virtual void CreatePrivateChatWith( const wxString& sOtherId );
    virtual void OnPrivateMessageReceived( const wxString& senderId,
                                           const wxString& message );
    virtual void OnPrivateChatWindowClosed();

private:
	bool            m_bRequestingLogout;
			/* Whether this Player is LOGOUT-ing from the server.
             * This state is needed to avoid sending LOGOUT twice.
             */

    hoxChatWindow*  m_chatWindow;

    DECLARE_DYNAMIC_CLASS(hoxLocalPlayer)
	DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_LOCAL_PLAYER_H__ */
