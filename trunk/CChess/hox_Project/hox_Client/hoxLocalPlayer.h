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
// Name:            hoxLocalPlayer.h
// Created:         10/28/2007
//
// Description:     The LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_LOCAL_PLAYER_H_
#define __INCLUDED_HOX_LOCAL_PLAYER_H_

#include <wx/wx.h>
#include "hoxPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/**
 * The abstract LOCAL player.
 */
class hoxLocalPlayer :  public hoxPlayer
{
public:
    hoxLocalPlayer(); // DUMMY default constructor required for RTTI info.
    hoxLocalPlayer( const wxString& name,
                    hoxPlayerType   type,
                    int             score );

    virtual ~hoxLocalPlayer();

public:

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void OnClose_FromTable( wxCommandEvent&  event );

    /*******************************
     * MY-specific Network API
     *******************************/

    virtual hoxResult ConnectToNetworkServer( wxEvtHandler* sender );
    virtual hoxResult DisconnectFromNetworkServer( wxEvtHandler* sender );

    virtual hoxResult QueryForNetworkTables( wxEvtHandler* sender );
    virtual hoxResult JoinNetworkTable( const wxString& tableId,
                                        wxEvtHandler*   sender );
    virtual hoxResult OpenNewNetworkTable( wxEvtHandler*   sender );
    virtual hoxResult LeaveNetworkTable( const wxString& tableId,
                                         wxEvtHandler*   sender );

private:

    DECLARE_ABSTRACT_CLASS(hoxLocalPlayer)
};


#endif /* __INCLUDED_HOX_LOCAL_PLAYER_H_ */
