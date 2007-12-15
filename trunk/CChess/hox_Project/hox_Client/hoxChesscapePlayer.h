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
// Name:            hoxChesscapePlayer.h
// Created:         12/12/2007
//
// Description:     The Chesscape LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_CHESSCAPE_PLAYER_H_
#define __INCLUDED_HOX_CHESSCAPE_PLAYER_H_

#include <wx/wx.h>
#include "hoxLocalPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/**
 * The Chesscape player.
 */
class hoxChesscapePlayer :  public hoxLocalPlayer
{
public:
    hoxChesscapePlayer(); // DUMMY default constructor required for event handler.
    hoxChesscapePlayer( const wxString& name,
						hoxPlayerType   type,
						int             score );

    virtual ~hoxChesscapePlayer();

    /*******************************
     * Override the parent's API
     *******************************/

	virtual hoxResult ConnectToNetworkServer( wxEvtHandler* sender );
	virtual hoxResult DisconnectFromNetworkServer( wxEvtHandler* sender );
	virtual hoxResult QueryForNetworkTables( wxEvtHandler* sender );
    virtual hoxResult JoinNetworkTable( const wxString& tableId,
                                        wxEvtHandler*   sender );

    /*******************************
     * Socket-event handlers
     *******************************/

    void OnIncomingNetworkData( wxSocketEvent& event );
    
    void OnConnectionResponse_PlayerData( wxCommandEvent& event ); 
    void OnConnectionResponse( wxCommandEvent& event ); 

private:
	bool _ParseTableInfoString( const wxString&      tableStr,
		                        hoxNetworkTableInfo& tableInfo ) const;
	bool _AddTableToList( const wxString& tableStr ) const;
	bool _RemoveTableFromList( const wxString& tableId ) const;
	bool _UpdateTableInList( const wxString& tableStr ) const;

private:
	/* Chesscape server sends a list of tables upon login.
	 * After that, it only sends updates for each tables.
	 * Thus, this player needs to maintain a "cache" list of tables.
	 */
	mutable hoxNetworkTableInfoList  m_networkTables;

    DECLARE_DYNAMIC_CLASS(hoxChesscapePlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_CHESSCAPE_PLAYER_H_ */
