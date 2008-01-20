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

#include <wx/wx.h>
#include "hoxEnums.h"
#include "hoxTypes.h"

/* Forward declarations */
class hoxSite;
class hoxTable;
class hoxConnection;

/* 
 * New player-event based on wxCommandEvent.
 */
DECLARE_EVENT_TYPE(hoxEVT_PLAYER_JOIN_TABLE, wxID_ANY)
DECLARE_EVENT_TYPE(hoxEVT_PLAYER_DRAW_TABLE, wxID_ANY)
DECLARE_EVENT_TYPE(hoxEVT_PLAYER_NEW_MOVE, wxID_ANY)
DECLARE_EVENT_TYPE(hoxEVT_PLAYER_NEW_JOIN, wxID_ANY)
DECLARE_EVENT_TYPE(hoxEVT_PLAYER_NEW_LEAVE, wxID_ANY)
DECLARE_EVENT_TYPE(hoxEVT_PLAYER_TABLE_CLOSE, wxID_ANY)
DECLARE_EVENT_TYPE(hoxEVT_PLAYER_WALL_MSG, wxID_ANY)
DECLARE_EVENT_TYPE(hoxEVT_PLAYER_SITE_CLOSING, wxID_ANY)

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

	virtual void OnJoinCmd_FromTable( wxCommandEvent&  event );
	virtual void OnDrawCmd_FromTable( wxCommandEvent&  event );
    virtual void OnNewMove_FromTable( wxCommandEvent&  event );
    virtual void OnNewJoin_FromTable( wxCommandEvent&  event );
    virtual void OnNewLeave_FromTable( wxCommandEvent&  event );
    virtual void OnClose_FromTable( wxCommandEvent&  event );
    virtual void OnWallMsg_FromTable( wxCommandEvent&  event );
	virtual void OnClosing_FromSite( wxCommandEvent&  event );

    /***************************
     * Accessor API
     ***************************/

    wxString      GetName() const               { return m_name; }
    void          SetName(const wxString& name) { m_name = name; }

    hoxPlayerType GetType() const { return m_type; }

    const hoxRoleList& GetRoles() const { return m_roles; }
    void               AddRole( hoxRole role );
    void               RemoveRole( hoxRole role );
    bool               RemoveRoleAtTable( const wxString& tableId );
    bool               HasRole( hoxRole role );
	bool               HasRoleAtTable( const wxString& tableId ) const;
	bool               FindRoleAtTable( const wxString& tableId, 
		                                hoxPieceColor&  assignedColor ) const;

    int                GetScore() const    { return m_score; }
    void               SetScore(int score) { m_score = score; }

    wxString GetPassword() const                   { return m_password; }
    void     SetPassword(const wxString& password) { m_password = password; }

    void     SetSite(hoxSite* site) { m_site = site; }
    hoxSite* GetSite() const        { return m_site; }
    
    /***************************
     * Basic action API
     ***************************/

    /**
     * Request to join a Table.
     * Upon returned, the Player will be assigned a role.
     *
     * @param table The Table to join.
     */
    virtual hoxResult JoinTable( hoxTable* table );

    /**
     * Request to join a Table as a specified role.
     *
     * @param table        The Table to join.
     * @param requestColor The request role (color).
     */
    virtual hoxResult JoinTableAs( hoxTable*     table,
                                   hoxPieceColor requestColor );

    virtual hoxResult LeaveTable( hoxTable* table );
    virtual hoxResult LeaveAllTables();

    /**
     * Set the connection to the "outside" world.
     *
     * @return true  - If this connection can be set.
     *         false - If some other existing connection has been set.
     */
    virtual bool SetConnection( hoxConnection* connection );

    /**
     * Return the connection.
     */
    virtual hoxConnection* GetConnection() const { return m_connection; }
  
    /**
     * Reset connection to NULL.    
     */
    virtual bool ResetConnection();

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

    virtual void AddRequestToConnection( hoxRequest* request );

    void DecrementOutstandingRequests();

private:
	void _PostSite_ShutdownReady();

private:
    hoxSite*        m_site;

    hoxConnection*  m_connection;
            /* The connection to "outside" world.
             * For Dummy player, it will be NULL.
             * NOTE: This variable is placed here even though some players
             *       (such as Host and Dummy) do not need it because most
             *       players do. Also, placing it here simplifies the code
             *       and design.
             */

    wxString       m_name;   // The player's name.

    hoxPlayerType  m_type;       
            /* Is it a Local, Network,... player? */

    hoxRoleList    m_roles;
            /* Which tables, which side (color) I am playing (RED or BLACK)?
             */

    int            m_score;  // The player's Score.
	wxString       m_password; // The player's login-password.

    int            m_nOutstandingRequests;

	bool           m_siteClosing;    // The site is being closed?


    DECLARE_DYNAMIC_CLASS(hoxPlayer)
    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_PLAYER_H_ */
