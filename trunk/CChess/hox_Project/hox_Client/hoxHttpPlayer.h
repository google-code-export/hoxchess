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
// Name:            hoxHttpPlayer.h
// Created:         10/28/2007
//
// Description:     The HTTP Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_HTTP_PLAYER_H_
#define __INCLUDED_HOX_HTTP_PLAYER_H_

#include <wx/wx.h>
#include "hoxLocalPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/**
 * The HTTP player.
 */
class hoxHttpPlayer :  public hoxLocalPlayer
{
public:
    hoxHttpPlayer(); // DUMMY default constructor required for event handler.
    hoxHttpPlayer( const wxString& name,
                   hoxPlayerType   type,
                   int             score );

    virtual ~hoxHttpPlayer();

public:
    /*******************************
     * Incoming-data event handlers
     *******************************/

    void OnHTTPResponse_Poll(wxCommandEvent& event);
    void OnHTTPResponse_Connect(wxCommandEvent& event);
    void OnHTTPResponse(wxCommandEvent& event);

    /*******************************
     * Other API
     *******************************/

    void OnTimer( wxTimerEvent& event );

private:
    void _HandleEventFromNetwork( const hoxNetworkEvent& networkEvent );

private:
    wxTimer        m_timer;    // to poll the HTTP server to events.

    DECLARE_DYNAMIC_CLASS(hoxHttpPlayer)
    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_HTTP_PLAYER_H_ */
