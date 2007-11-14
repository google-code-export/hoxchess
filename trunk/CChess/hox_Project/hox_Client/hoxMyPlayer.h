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

/* Forward declarations */
class hoxSocketConnection;

/** 
 * Connection event-type for responses.
 */
DECLARE_EVENT_TYPE(hoxEVT_CONNECTION_RESPONSE, wxID_ANY)

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

    virtual hoxResult JoinTable( hoxTable* table );

    /*******************************
     * Socket-event handlers
     *******************************/

    void OnIncomingNetworkData( wxSocketEvent& event );
    
    void OnConnectionResponse_PlayerData( wxCommandEvent& event ); 
    void OnConnectionResponse( wxCommandEvent& event ); 

private:

    DECLARE_DYNAMIC_CLASS(hoxMyPlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_MY_PLAYER_H_ */
