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

    virtual hoxResult ConnectToNetworkServer();
	virtual hoxResult QueryForNetworkTables();
    virtual hoxResult JoinNetworkTable( const wxString& tableId );
	virtual hoxResult OpenNewNetworkTable();

	/*******************************
     * Table-event handlers
     *******************************/

    virtual void OnRequest_FromTable( hoxRequest* request );

    /*******************************
     * Socket-event handlers
     *******************************/

    void OnIncomingNetworkData( wxSocketEvent& event );
    
    void OnConnectionResponse_PlayerData( wxCommandEvent& event ); 
    void OnConnectionResponse( wxCommandEvent& event ); 

private:
	bool _ParseTableInfoString( const wxString&      tableStr,
		                        hoxNetworkTableInfo& tableInfo ) const;
	bool _ParsePlayersInfoString( const wxString&      playersInfoStr,
		                          hoxNetworkTableInfo& tableInfo ) const;
	bool _DoesTableExist( const wxString& tableId ) const;
	bool _FindTableById( const wxString&      tableId,
		                 hoxNetworkTableInfo*& pTableInfo ) const;
	bool _AddTableToList( const wxString& tableStr ) const;
	bool _RemoveTableFromList( const wxString& tableId ) const;
	bool _UpdateTableInList( const wxString& tableStr,
		                     hoxNetworkTableInfo* pTableInfo = NULL );

	bool _ParseIncomingCommand(const wxString& contentStr,
		                       wxString&       command,
							   wxString&       paramsStr) const;
	bool _HandleCmd_Login(const wxString& cmdStr,
                          wxString&       name,
                          wxString&       score,
                          wxString&       role) const;

	bool _HandleCmd_Show(const wxString& cmdStr);
	bool _HandleCmd_Unshow(const wxString& cmdStr);
	bool _HandleCmd_Update(const wxString& cmdStr,
		                   hoxNetworkTableInfo* pTableInfo = NULL);
	bool _HandleCmd_UpdateRating(const wxString& cmdStr);

	bool _HandleTableCmd(const wxString& cmdStr);
	bool _HandleTableCmd_Settings(const wxString& cmdStr);
	bool _HandleTableCmd_PastMoves(hoxTable*       table,
		                           const wxString& cmdStr);
	bool _HandleTableCmd_Move(hoxTable*       table,
		                      const wxString& cmdStr);
	bool _HandleTableCmd_GameOver(hoxTable*       table,
		                          const wxString& cmdStr);
	bool _HandleTableCmd_OfferDraw(hoxTable* table);
	bool _HandleTableMsg(const wxString& cmdStr);

	/* Private event-handlers */

	void _OnTableUpdated( const hoxNetworkTableInfo& tableInfo );


private:
	/* Chesscape server sends a list of tables upon login.
	 * After that, it only sends updates for each tables.
	 * Thus, this player needs to maintain a "cache" list of tables.
	 */
	mutable hoxNetworkTableInfoList  m_networkTables;

	wxString                         m_pendingJoinTableId;
			/* The Id of the table that this player is requesting to join. */

	wxString                         m_pendingRequestSeat;
			/* The Seat (RED/BLACK) this player is requesting to play as. */

	bool                             m_bRequestingLogin;
			/* Whether this Player is LOGIN-ing the server. */

	bool                             m_bRequestingNewTable;
			/* Whether this Player is requesting for a NEW Table. */

	bool                             m_bSentMyFirstMove;
			/* Whether this Player has sent his first Move since JOINing the Table. */

    DECLARE_DYNAMIC_CLASS(hoxChesscapePlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_CHESSCAPE_PLAYER_H_ */
