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

/* 
 * Connection event-type for responses.
 */
DECLARE_EVENT_TYPE(hoxEVT_CONNECTION_RESPONSE, wxID_ANY)

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

    /***************************
     * Accessor API
     ***************************/

    wxString      GetId()   const { return m_info.id; }
    hoxPlayerType GetType() const { return m_info.type; }

    /**
     * Get the 'front' table.
     *
     * @note This API is needed because most online Sites allow a Player to
     *       join AT MOST one table at any time.
     * @return An 'empty' pointer if the Player has not joined any Table.
     */
    hoxTable_SPtr GetFrontTable() const;

    /**
     * Get the 'front' role.
     *
     * @note This API is needed because most online Sites allow a Player to
     *       join AT MOST one table at any time.
     * @param sTableId [OUT] The Table-Id if there is such a Table.
     * @return hoxCOLOR_UNKNOWN if the Player has not joined any Table.
     */
    hoxColor GetFrontRole( wxString& sTableId ) const;

    /**
     * Find a Table by a Table-Id.
     *
     * @return An 'empty' pointer if such a Table was not found.
     */
    hoxTable_SPtr FindTable( const wxString& sTableId ) const;

    /**
     * Specific a given table to be the "active" or default table
     * to handle incoming notifications (such as a Player's INFO.)
     */
    void SetActiveTableId( const wxString& sTableId )
        { m_activeTableId = sTableId; }

    /**
     * Get the "active" table to handle incoming notifications.
     * If no table was explicitly set, the return the 1st table (if any).
     */
    hoxTable_SPtr GetActiveTable() const;

    int  GetScore() const    { return m_info.score; }
    void SetScore(int score) { m_info.score = score; }

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
     * Request to join a Table as a specified role.
     *
     * @param pTable       The Table to join.
     * @param requestColor The request role (color).
     */
    virtual hoxResult JoinTableAs( hoxTable_SPtr pTable,
                                   hoxColor      requestColor );

    virtual hoxResult LeaveTable( hoxTable_SPtr pTable );
    virtual hoxResult LeaveAllTables();

public:
    /**
     * Start a Connection background thread to connect to a remote server
     * or an AI Engine.
     */
    virtual void Start() {}

protected:
    virtual void StartConnection();

    virtual void AddRequestToConnection( hoxRequest_APtr apRequest );

protected:
    hoxSite*        m_site;

private:
    hoxPlayerInfo   m_info;   
            /* Basic player's info (ID, Type, Score,...) */

    hoxConnection_APtr  m_connection;
            /* The connection to "outside" world.
             * For Dummy player, it will be NULL.
             * NOTE: This variable is placed here even though some players
             *       (such as 'Dummy') do not need it because most
             *       players do. Also, placing it here simplifies the code
             *       and design.
             */

    hoxTableSet    m_tables;
            /* Which tables this Player has joined. */

    wxString       m_activeTableId;
            /* The table that will receive notifications
             * such as a Player's INFO.
             */

	wxString       m_password; // The player's login-password.

    DECLARE_DYNAMIC_CLASS(hoxPlayer)
    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_PLAYER_H_ */
