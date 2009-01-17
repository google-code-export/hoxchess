/***************************************************************************
 *  Copyright 2007, 2008, 2009 Huy Phan  <huyphan@playxiangqi.com>         *
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

#ifndef __INCLUDED_HOX_CHESSCAPE_PLAYER_H__
#define __INCLUDED_HOX_CHESSCAPE_PLAYER_H__

#include "hoxLocalPlayer.h"
#include "hoxTypes.h"

/**
 * The Chesscape player.
 */
class hoxChesscapePlayer :  public hoxLocalPlayer
{
public:
    hoxChesscapePlayer() {} // DUMMY constructor required for event handler.
    hoxChesscapePlayer( const wxString& name,
						hoxPlayerType   type,
						int             score );

    virtual ~hoxChesscapePlayer() {}

    virtual void Start();

    /*******************************
     * Override the parent's API
     *******************************/

    virtual hoxResult ConnectToServer();
	virtual hoxResult QueryForNetworkTables();
    virtual hoxResult JoinNetworkTable( const wxString& tableId );
	virtual hoxResult OpenNewNetworkTable();

	/*******************************
     * Table-event handlers
     *******************************/

    virtual void OnRequest_FromTable( hoxRequest_APtr apRequest );

    /*******************************
     * Socket-event handlers
     *******************************/

    void OnConnectionResponse_PlayerData( wxCommandEvent& event ); 
    void OnConnectionResponse( wxCommandEvent& event ); 

private:
	void _ParseTableInfoString( const wxString&      tableStr,
		                        hoxNetworkTableInfo& tableInfo ) const;
	bool _ParseTablePlayersString( const wxString&      playersInfoStr,
		                           hoxNetworkTableInfo& tableInfo ) const;
	void _ParsePlayerStatsString( const wxString&  sStatsStr,
		                          hoxPlayerStats&  playerStats ) const;

    bool _DoesTableExist( const wxString& tableId ) const;
	bool _FindTableById( const wxString&      tableId,
		                 hoxNetworkTableInfo*& pTableInfo ) const;
	bool _AddTableToList( const wxString& tableStr ) const;
	bool _RemoveTableFromList( const wxString& tableId ) const;
	bool _UpdateTableInList( const wxString& tableStr,
		                     hoxNetworkTableInfo* pTableInfo = NULL );

	void _ParseLoginInfoString( const wxString& cmdStr,
                                wxString&       name,
                                wxString&       score,
                                wxString&       role ) const;

	bool _ParseIncomingCommand(const wxMemoryBuffer& data,
		                       wxString&       command,
							   wxString&       paramsStr) const;
	void _HandleCmd_Login(const hoxResponse_APtr& response,
                          const wxString&         cmdStr);
	void _HandleCmd_Code(const hoxResponse_APtr& response,
                         const wxString&         cmdStr);
	void _HandleCmd_Logout(const wxString& cmdStr);
    void _HandleCmd_Clients(const wxString& cmdStr);
	void _HandleCmd_Show(const wxString& cmdStr);
	void _HandleCmd_Unshow(const wxString& cmdStr);
	void _HandleCmd_Update(const wxString& cmdStr,
		                   hoxNetworkTableInfo* pTableInfo = NULL);
	void _HandleCmd_UpdateRating(const wxString& cmdStr);
    void _HandleCmd_UpdateStatus(const wxString& cmdStr);
	void _HandleCmd_PlayerInfo(const wxString& cmdStr);

	void _HandleTableCmd(const wxString& cmdStr);
	void _HandleTableCmd_Settings(const wxString& cmdStr);
    void _HandleTableCmd_Invite(const wxString& cmdStr);
	void _HandleTableCmd_PastMoves(hoxTable_SPtr   pTable,
		                           const wxString& cmdStr);
	void _HandleTableCmd_Move(hoxTable_SPtr   pTable,
		                      const wxString& cmdStr);
	void _HandleTableCmd_GameOver(hoxTable_SPtr   pTable,
		                          const wxString& cmdStr);
	void _HandleTableCmd_OfferDraw(hoxTable_SPtr pTable);
	void _HandleTableCmd_Clients(hoxTable_SPtr   pTable,
		                         const wxString& cmdStr);
	void _HandleTableCmd_Unjoin(hoxTable_SPtr   pTable,
		                        const wxString& cmdStr);
    void _HandleMsg(const wxString& cmdStr,
                    bool            bPublic);

	/* Private event-handlers */

	void _OnTableUpdated( const hoxNetworkTableInfo& tableInfo );

    /* Other private helpers */

    hoxGameType     _StringToGameType( const wxString& sInput ) const;
    hoxPlayerStatus _StringToPlayerStatus( const wxString& sInput ) const;

    hoxTable_SPtr _GetMyTable() const; // Chesscape only supports 1 Table.

private:
	/* Chesscape server sends a list of tables upon login.
	 * After that, it only sends updates for each tables.
	 * Thus, this player needs to maintain a "cache" list of tables.
	 */
	mutable hoxNetworkTableInfoList  m_networkTables;

	/* NOTE: Chesscape server sends a list of players upon login.
	 *       After that, it only sends updates for each player
     *       regarding score, status, ...
	 *       Thus, this player needs to maintain a "cache" list of players.
	 */

	wxString                         m_pendingJoinTableId;
			/* The Id of the table that this player is requesting to join. */

	wxString                         m_pendingRequestSeat;
			/* The Seat (RED/BLACK) this player is requesting to play as. */

	bool                             m_bRequestingLogin;
			/* Whether this Player is LOGIN-ing the server. */

	bool                             m_bRequestingNewTable;
			/* Whether this Player is requesting for a NEW Table. */

    hoxPlayerStatus                  m_playerStatus;
            /* Playing, Observing, Solo playing */

    DECLARE_DYNAMIC_CLASS(hoxChesscapePlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_CHESSCAPE_PLAYER_H__ */
