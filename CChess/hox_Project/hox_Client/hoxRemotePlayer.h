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
// Name:            hoxRemotePlayer.h
// Created:         11/01/2007
//
// Description:     The REMOTE Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_REMOTE_PLAYER_H_
#define __INCLUDED_HOX_REMOTE_PLAYER_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include "hoxPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/**
 * The REMOTE player.
 */
class hoxRemotePlayer :  public hoxPlayer
{
  public:
    hoxRemotePlayer(); // DUMMY default constructor required for event handler.
    hoxRemotePlayer( const wxString& name,
                     hoxPlayerType   type,
                     int             score );

    virtual ~hoxRemotePlayer();

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/


    /***************************
     * Network API
     ***************************/

    void OnIncomingNetworkData( wxSocketEvent& event );
    void OnConnectionResponse_PlayerData( wxCommandEvent& event ); 
    void OnServerResponse( wxCommandEvent& event ); 

private:

    DECLARE_DYNAMIC_CLASS(hoxRemotePlayer)
    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_REMOTE_PLAYER_H_ */
