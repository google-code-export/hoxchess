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

DECLARE_EVENT_TYPE(hoxEVT_SITE_PLAYER_DISCONNECT, wxID_ANY)
DECLARE_EVENT_TYPE(hoxEVT_SITE_PLAYER_SHUTDOWN_READY, wxID_ANY)

/**
 * The Site's Actions that are enabled at a given time.
 */
enum hoxSiteAction
{
	/* NOTE: The numeric values are of the 32-bitmap ones. */

	hoxSITE_ACTION_CONNECT        = ( (unsigned int) 1 ),
	hoxSITE_ACTION_DISCONNECT     = ( (unsigned int) 1 << 1 ),

	hoxSITE_ACTION_LIST           = ( (unsigned int) 1 << 2 ),
	hoxSITE_ACTION_NEW            = ( (unsigned int) 1 << 3 ),
    hoxSITE_ACTION_JOIN           = ( (unsigned int) 1 << 4 ),
	hoxSITE_ACTION_CLOSE          = ( (unsigned int) 1 << 5 )
};

/* Forward declarations */
class hoxServer;
class hoxSite;

class hoxResponseHandler : public wxEvtHandler
{
public:
    hoxResponseHandler(hoxSite* site) : m_site(site) {}
    virtual ~hoxResponseHandler() {}

	void OnDisconnect_FromPlayer( wxCommandEvent& event ); 
	void OnShutdownReady_FromPlayer( wxCommandEvent& event ); 
    void OnConnectionResponse( wxCommandEvent& event ); 

    hoxSite*   m_site;

private:
    DECLARE_EVENT_TABLE()
};

/**
 * The Site.
 */
class hoxSite : public wxObject
{
public:
    hoxSite( hoxSiteType             type, 
             const hoxServerAddress& address );
    virtual ~hoxSite();

    hoxSiteType GetType() const { return m_type; }
    hoxServerAddress GetAddress() const { return m_address; }

    virtual const wxString GetName() const { return "_Unknown_"; }
    virtual hoxResult Close() = 0;

    virtual hoxResult CreateNewTable(wxString& newTableId) { return hoxRESULT_ERR; }
    virtual hoxResult CreateNewTableAsPlayer(wxString&          newTableId, 
		                                     hoxPlayer*         player,
											 const hoxTimeInfo& initialTime) 
        { return hoxRESULT_ERR; }

    virtual hoxResult CloseTable(hoxTable* table);
    const hoxTableList& GetTables() const { return m_tableMgr.GetTables(); } 
    hoxTable* FindTable( const wxString& tableId ) const
        { return m_tableMgr.FindTable( tableId ); }

    hoxPlayer* FindPlayer( const wxString& playerId ) const
        { return m_playerMgr.FindPlayer( playerId ); }

    hoxPlayer* CreateDummyPlayer( const wxString& playerId,
		                          int             score = 1500 )
        { return m_playerMgr.CreateDummyPlayer( playerId, score ); }

    virtual void DeletePlayer( hoxPlayer* player );

    hoxResponseHandler*  GetResponseHandler() const 
        { return m_responseHandler; }

	virtual void Handle_DisconnectFromPlayer( hoxPlayer* player );
	virtual void Handle_ShutdownReadyFromPlayer( const wxString& playerId );

	virtual unsigned int GetCurrentActionFlags() const { return 0; }

protected:
    const hoxSiteType  m_type;

    hoxServerAddress   m_address;
    hoxPlayerMgr       m_playerMgr;
    hoxTableMgr        m_tableMgr;

    hoxResponseHandler*  m_responseHandler;
    wxProgressDialog*    m_dlgProgress;

	bool               m_siteClosing;    // The Site is being closed?

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

    virtual const wxString GetName() const;

    virtual hoxResult OpenServer();
    virtual hoxResult Close();
    bool IsOpened() const { return m_isOpened; }

    virtual hoxResult CreateNewTableAsPlayer(wxString&          newTableId, 
		                                     hoxPlayer*         player,
		                                     const hoxTimeInfo& initialTime);
	
	virtual void Handle_DisconnectFromPlayer( hoxPlayer* player );
	virtual void Handle_ShutdownReadyFromPlayer( const wxString& playerId );

private:
	void           _DoCloseSite();
	const wxString _GenerateTableId();

private:
    hoxServer*   m_server;
    bool         m_isOpened;

	int          m_nNextTableId; // To generate new Table-Id.
};

/**
 * The REMOTE Site.
 */
class hoxRemoteSite : public hoxSite
{
public:
    hoxRemoteSite(const hoxServerAddress& address,
                  hoxSiteType             type = hoxSITE_TYPE_REMOTE);
    virtual ~hoxRemoteSite();

    virtual const wxString GetName() const;

    virtual hoxResult Connect();
    virtual hoxResult Close();

    virtual bool IsConnected() const;
    virtual hoxResult QueryForNetworkTables();
    virtual hoxResult CreateNewTable(wxString& newTableId);

	// Handle event in which a Player just joins an EXISTING Table.
	virtual hoxResult OnPlayerJoined(const wxString&     tableId,
		                             const wxString&     playerId,
                                     const int           playerScore,
									 const hoxColor requestColor);

	virtual hoxResult JoinNewTable(const hoxNetworkTableInfo& tableInfo);
    virtual hoxResult JoinExistingTable(const hoxNetworkTableInfo& tableInfo);
    virtual hoxResult DisplayListOfTables(const hoxNetworkTableInfoList& tableList);

	virtual void DeletePlayer( hoxPlayer* player );

	virtual void Handle_ShutdownReadyFromPlayer( const wxString& playerId );

	/* TODO: Need to review this API... */
    virtual hoxLocalPlayer* CreateLocalPlayer(const wxString& playerName);

	virtual unsigned int GetCurrentActionFlags() const;

    virtual hoxTable* CreateNewTableWithGUI(const hoxNetworkTableInfo& tableInfo);

protected:
    virtual void Handle_ConnectionResponse( hoxResponse_AutoPtr response );
    
    virtual void OnResponse_Connect( const hoxResponse_AutoPtr& response );
	virtual void OnResponse_Disconnect( const hoxResponse_AutoPtr& response );
    virtual void OnResponse_List( const hoxResponse_AutoPtr& response );

protected:
    hoxLocalPlayer*      m_player;
            /* The player that this Host uses to connect to the server. */

    friend class hoxResponseHandler;
};

/**
 * The HTTP (remote) Site.
 */
class hoxHTTPSite : public hoxRemoteSite
{
public:
    hoxHTTPSite(const hoxServerAddress& address);
    virtual ~hoxHTTPSite();

	/* TODO: Need to review this API... */
    virtual hoxLocalPlayer* CreateLocalPlayer(const wxString& playerName);
};

/**
 * The Chesscape (remote) Site.
 */
class hoxChesscapeSite : public hoxRemoteSite
{
public:
    hoxChesscapeSite(const hoxServerAddress& address);
    virtual ~hoxChesscapeSite();

	/* TODO: Need to review this API... */
    virtual hoxLocalPlayer* CreateLocalPlayer(const wxString& playerName);

	virtual unsigned int GetCurrentActionFlags() const;

protected:
    virtual void OnResponse_Connect( const hoxResponse_AutoPtr& response );
};

///////////////////////////////////////////////////////////////////////////////

/**
 * The Site-Manager.
 * This is implemented as a singleton since we only need one instance.
 */
class hoxSiteManager
{
public:
	static hoxSiteManager* GetInstance();
    ~hoxSiteManager();

	hoxSite* CreateSite( hoxSiteType             siteType, 
		                 const hoxServerAddress& address,
				         const wxString&         userName,
						 const wxString&         password );

	hoxRemoteSite* FindRemoteSite( const hoxServerAddress& address ) const;
	
	int GetNumberOfSites() const { return (int) m_sites.size(); }

	void DeleteSite( hoxSite* site );

	void Close();

	const hoxSiteList& GetSites() const { return m_sites; }
	hoxLocalSite* GetLocalSite() const { return m_localSite; }

private:
    hoxSiteManager();
	static hoxSiteManager* m_instance;

private:
	hoxSiteList     m_sites;
	
	hoxLocalSite*   m_localSite;  
		/* The "cache" variable, pointing to the LOCAL site, for easy access. 
		 * NOTE: Currently, we only allow to have AT MOST one local site.
		 */
};

#endif /* __INCLUDED_HOX_SITE_H_ */
