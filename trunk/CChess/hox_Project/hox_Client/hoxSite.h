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
// Name:            hoxSite.h
// Created:         11/24/2007
//
// Description:     The Site.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_SITE_H_
#define __INCLUDED_HOX_SITE_H_

#include <wx/wx.h>
#include <wx/progdlg.h>
#include <list>
#include "hoxTypes.h"
#include "hoxPlayerMgr.h"
#include "hoxTableMgr.h"
#include "hoxMyPlayer.h"
#include "hoxLocalPlayer.h"

/* Forward declarations */
class hoxSocketServer;
class hoxServer;

/**
 * The Site.
 */
class hoxSite : public wxObject
{
public:
    hoxSite(const hoxServerAddress& address);
    virtual ~hoxSite();

    const hoxServerAddress GetAddress() const { return m_address; }
    
    virtual bool IsLocal() const { return false; }
    virtual const wxString GetName() const { return "_Unknown_"; }
    virtual hoxResult Close() { return hoxRESULT_ERR; }

    virtual hoxResult CreateNewTable(wxString& newTableId) { return hoxRESULT_ERR; }
    virtual hoxResult CreateNewTableAsPlayer(wxString& newTableId, hoxPlayer* player) 
        { return hoxRESULT_ERR; }

    virtual hoxResult CloseTable(hoxTable* table);
    const hoxTableList& GetTables() const { return m_tableMgr.GetTables(); } 
    hoxTable* FindTable( const wxString& tableId ) const
        { return m_tableMgr.FindTable( tableId ); }

    hoxPlayer* FindPlayer( const wxString& playerId ) const
        { return m_playerMgr.FindPlayer( playerId ); }

protected:
    hoxServerAddress   m_address;
    hoxPlayerMgr       m_playerMgr;
    hoxTableMgr        m_tableMgr;

    friend class hoxSocketServer;
    friend class hoxServer;
};

typedef std::list<hoxSite*>  hoxSiteList;

/**
 * The LOCAL Site.
 */
class hoxLocalSite : public hoxSite
{
public:
    hoxLocalSite(const hoxServerAddress& address);
    virtual ~hoxLocalSite();

    virtual bool IsLocal() const { return true; }
    virtual const wxString GetName() const;

    virtual hoxResult OpenServer();
    virtual hoxResult Close();
    bool IsOpened() const { return m_isOpened; }

    virtual hoxResult CreateNewTable(wxString& newTableId);
    virtual hoxResult CreateNewTableAsPlayer(wxString& newTableId, hoxPlayer* player);

private:
    hoxServer*          m_server;
    hoxSocketServer*    m_socketServer;

    bool                m_isOpened;

    hoxPlayer*          m_player;
            /* The player representing the host 
             * Even though the host may particiate in more than one game
             * at the same time, it is assumed to be run by only ONE player.
             */
};

///////////////////////////////////////////////////////////
class hoxRemoteSite;

class hoxResponseHandler : public wxEvtHandler
{
public:
    hoxResponseHandler(hoxRemoteSite* site) : m_remoteSite(site) {}
    virtual ~hoxResponseHandler() {}

    void OnConnectionResponse( wxCommandEvent& event ); 

    hoxRemoteSite* m_remoteSite;

private:
    DECLARE_EVENT_TABLE()
};

///////////////////////////////////////////////////////////

/**
 * The REMOTE Site.
 */
class hoxRemoteSite : public hoxSite
{
public:
    hoxRemoteSite(const hoxServerAddress& address);
    virtual ~hoxRemoteSite();

    virtual const wxString GetName() const;

    virtual hoxResult Connect();
    virtual hoxResult Close();

    bool IsConnected() const;
    virtual hoxResult QueryForNetworkTables();
    virtual hoxResult CreateNewTable(wxString& newTableId);
    virtual hoxResult JoinExistingTable(const hoxNetworkTableInfo& tableInfo);

    //////////////
    void Handle_ConnectionResponse( hoxResponse* pResponse );
    
    hoxPlayer* CreateDummyPlayer( const wxString& playerId )
        { return m_playerMgr.CreateDummyPlayer( playerId ); }
    /////////////

private:
    void _OnResponse_Connect( const wxString& responseStr );
    void _OnResponse_New( const wxString& responseStr );

private:
    hoxLocalPlayer*      m_player;
            /* The player that this Host uses to connect to the server. */

    hoxResponseHandler*  m_responseHandler;
    wxProgressDialog*    m_dlgProgress;
};

#endif /* __INCLUDED_HOX_SITE_H_ */
