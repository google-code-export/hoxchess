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
// Name:            hoxPlayer.h
// Created:         10/06/2007
//
// Description:     The Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_PLAYER_H_
#define __INCLUDED_HOX_PLAYER_H_

#include "hoxTypes.h"

/* Forward declarations */
class hoxSite;
class hoxTable;

// ----------------------------------------------------------------------------
// The Player class
// ----------------------------------------------------------------------------

// NOTE **** According to wxWidgets documentation regarding wxEvtHandler:
//
//         When using multiple inheritance it is imperative that 
//         the wxEvtHandler(-derived) class be the first class inherited 
//         such that the "this" pointer for the overall object will be 
//         identical to the "this" pointer for the wxEvtHandler portion.
//

/**
 * An interface for a Player.
 * Other more advanced Players will derive from this class.
 */
class hoxPlayer : public wxEvtHandler
{
public:
    hoxPlayer(); // DUMMY default constructor required for event handler.
    hoxPlayer( const wxString& name,
               hoxPlayerType   type,
               int             score = 1500);

    virtual ~hoxPlayer();

    /***************************
     * Event-handle API
     ***************************/

	virtual void OnRequest_FromTable( hoxRequest_APtr apRequest );

    virtual void OnClose_FromTable( const wxString& tableId );
	virtual void OnClosing_FromSite();

    /***************************
     * Accessor API
     ***************************/

    wxString      GetName() const { return m_info.id; }
    hoxPlayerType GetType() const { return m_info.type; }

    const hoxRoleList& GetRoles() const { return m_roles; }
    void               AddRole( hoxRole role );
    void               RemoveRole( hoxRole role );
    bool               RemoveRoleAtTable( const wxString& tableId );
    bool               HasRole( hoxRole role );
	bool               HasRoleAtTable( const wxString& tableId ) const;
	bool               FindRoleAtTable( const wxString& tableId, 
		                                hoxColor&  assignedColor ) const;

    int                GetScore() const    { return m_info.score; }
    void               SetScore(int score) { m_info.score = score; }

    wxString GetPassword() const                   { return m_password; }
    void     SetPassword(const wxString& password) { m_password = password; }

    void     SetSite(hoxSite* site) { m_site = site; }
    hoxSite* GetSite() const        { return m_site; }
    
    hoxPlayerInfo_APtr GetPlayerInfo() const 
        { return hoxPlayerInfo_APtr( new hoxPlayerInfo(m_info) ); }

    /**
     * Set the connection to the "outside" world.
     *
     * @return true  - If this connection can be set.
     *         false - If some other existing connection has been set.
     */
    bool SetConnection( hoxConnection_APtr connection );

    /**
     * Return the connection.
     */
    hoxConnection* GetConnection() const { return m_connection.get(); }
  
    /**
     * Reset connection to NULL.    
     */
    void ResetConnection();

    /***************************
     * Basic action API
     ***************************/

    /**
     * Request to join a Table.
     * Upon returned, the Player will be assigned a role.
     *
     * @param pTable The Table to join.
     */
    virtual hoxResult JoinTable( hoxTable_SPtr pTable );

    /**
     * Request to join a Table as a specified role.
     *
     * @param pTable       The Table to join.
     * @param requestColor The request role (color).
     */
    virtual hoxResult JoinTableAs( hoxTable_SPtr pTable,
                                   hoxColor      requestColor );

    virtual hoxResult LeaveTable( hoxTable_SPtr pTable );
    virtual hoxResult LeaveAllTables();

protected:
    /**
     * Handle the incoming data from the connection.
     */
    virtual hoxResult HandleIncomingData( const wxString& commandStr );
	virtual hoxResult HandleIncomingData_Disconnect( hoxCommand& command );
    virtual hoxResult HandleIncomingData_Move( hoxCommand& command, wxString& response );
    virtual hoxResult HandleIncomingData_Leave( hoxCommand& command, wxString& response );
    virtual hoxResult HandleIncomingData_WallMsg( hoxCommand& command, wxString& response );
    virtual hoxResult HandleIncomingData_List( hoxCommand& command, wxString& response );
    virtual hoxResult HandleIncomingData_Join( hoxCommand& command, wxString& response );
    virtual hoxResult HandleIncomingData_NewJoin( hoxCommand& command, wxString& response );
    virtual hoxResult HandleIncomingData_New( hoxCommand& command, wxString& response );

    virtual void StartConnection();
    virtual void ShutdownConnection();

    virtual void AddRequestToConnection( hoxRequest_APtr apRequest );

private:
	void _PostSite_ShutdownReady();

private:
    hoxPlayerInfo   m_info;   
            /* Basic player's info (ID, Type, Score,...) */

    hoxSite*        m_site;

    hoxConnection_APtr  m_connection;
            /* The connection to "outside" world.
             * For Dummy player, it will be NULL.
             * NOTE: This variable is placed here even though some players
             *       (such as Host and Dummy) do not need it because most
             *       players do. Also, placing it here simplifies the code
             *       and design.
             */

    hoxRoleList    m_roles;
            /* Which tables, which side (color) I am playing (RED or BLACK)?
             */

	wxString       m_password; // The player's login-password.

	bool           m_siteClosing;    // The site is being closed?


    DECLARE_DYNAMIC_CLASS(hoxPlayer)
    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_PLAYER_H_ */
