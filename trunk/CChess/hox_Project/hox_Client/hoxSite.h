/***************************************************************************
 *  Copyright 2007-2009 Huy Phan  <huyphan@playxiangqi.com>                *
 *                      Bharatendra Boddu (bharathendra at yahoo dot com)  *
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

#ifndef __INCLUDED_HOX_SITE_H__
#define __INCLUDED_HOX_SITE_H__

#include <wx/wx.h>
#include "hoxTypes.h"
#include "hoxPlayerMgr.h"
#include "hoxTableMgr.h"
#include "hoxLocalPlayer.h"
#include "hoxPlayersUI.h"

/* Forward declarations. */
class hoxProgressDialog;
class hoxTablesDialog;

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
	hoxSITE_ACTION_CLOSE          = ( (unsigned int) 1 << 5 ),
    hoxSITE_ACTION_PRACTICE       = ( (unsigned int) 1 << 6 ),
    hoxSITE_ACTION_OPEN		      = ( (unsigned int) 1 << 7 )
};

/**
 * The Site.
 */
class hoxSite : public wxObject
              , public hoxPlayersUI::UIOwner
{
public:
    hoxSite( hoxSiteType             type, 
             const hoxServerAddress& address );
    virtual ~hoxSite();

    hoxSiteType GetType() const { return m_type; }
    hoxServerAddress GetAddress() const { return m_address; }

    virtual const wxString GetName() const
        { return wxString( m_address.c_str() ); }

    virtual void Connect() {}
    virtual void Disconnect() {}

    virtual bool IsConnected() const = 0;

    virtual void QueryForTables() {}

	virtual void OnPlayerJoined(const wxString& tableId,
		                        const wxString& playerId,
                                const int       playerScore,
                                const hoxColor  requestColor) {}

    virtual void JoinLocalPlayerToTable(const hoxNetworkTableInfo& tableInfo) {}

    virtual void OnResponse_LOGIN( const hoxResponse_APtr& response ) {}

    hoxResult CloseTable(hoxTable_SPtr pTable);
    
    const hoxTableList& GetTables() const { return m_tableMgr.GetTables(); } 
    
    hoxTable_SPtr FindTable( const wxString& tableId ) const
        { return m_tableMgr.FindTable( tableId ); }

    /**
     * Find the Player by ID first.
     * If not found, then return a NULL pointer.
     */
    hoxPlayer* FindPlayer( const wxString& playerId ) const
        { return m_playerMgr.FindPlayer( playerId ); }

    /**
     * Find the Player by ID first.
     * If not found, then create a new DUMMY player with the given score.
     */
    hoxPlayer* GetPlayerById( const wxString& sPlayerId,
                              const int       nScore );

    virtual void OnShutdownReadyFromLocalPlayer() {}

    virtual hoxLocalPlayer* CreateLocalPlayer(const wxString& playerName) = 0;

	virtual unsigned int GetCurrentActionFlags() const = 0;

    virtual void OnPlayerLoggedIn( const wxString&       sPlayerId,
                                   const int             nPlayerScore,
                                   const hoxPlayerStatus playerStatus = hoxPLAYER_STATUS_UNKNOWN );
    virtual void OnPlayerLoggedOut( const wxString& sPlayerId );

    virtual void OnListOfTablesReceived(const hoxNetworkTableInfoList& tableList) {}

    /**
     * Update the score of an ONLINE Player.
     * If the Player is not found, then add the Player to the online list.
     */
    virtual void UpdateScoreOfOnlinePlayer( const wxString& sPlayerId,
                                            const int       nPlayerScore );

    /**
     * Update the player-status of an ONLINE Player.
     * If the Player is not found, then add the Player to the online list.
     */
    virtual void UpdateStatusOfOnlinePlayer( const wxString&       sPlayerId,
                                             const hoxPlayerStatus playerStatus );

    /**
     * Get the score of an ONLINE Player.
     * @return hoxSCORE_UNKNOWN - if the Player is not found.
     */
    virtual int GetScoreOfOnlinePlayer( const wxString& sPlayerId ) const;

    hoxPlayersUI* GetPlayersUI() const { return m_playersUI; }

    /**
     * On the LOCAL player's request to join a Table.
     */
    virtual void OnLocalRequest_JOIN( const wxString& sTableId ) = 0;

    /**
     * On the LOCAL player's request to create a new Table.
     */
    virtual void OnLocalRequest_NEW() = 0;

    /**
     * On the LOCAL player's request to create a new PRACTICE Table.
     */
    virtual void OnLocalRequest_PRACTICE( const wxString& sSavedFile = "" ) {}


	/*************************************************
     * Implement hoxPlayersUI::UIOwner 's interface.
     *************************************************/

    virtual void OnPlayersUIEvent( hoxPlayersUI::EventType eventType,
                                   const wxString&         sPlayerId ) {}

protected:
    virtual unsigned int GetBoardFeatureFlags() const;

    void ShowProgressDialog( bool bShow = true );

    hoxTable_SPtr CreateNewTableWithGUI( const hoxNetworkTableInfo& tableInfo,
                                         hoxIReferee_SPtr&          pReferee );

protected:
    const hoxSiteType  m_type;

    hoxServerAddress   m_address;
    hoxPlayerMgr       m_playerMgr;
    hoxTableMgr        m_tableMgr;

    hoxProgressDialog* m_dlgProgress;

	bool               m_siteDisconnecting; // The Site is being disconnected?

    hoxLocalPlayer*    m_player;
            /* The player that this Host uses to connect to the server. */

	/* The "cache" containing the list of ONLINE players.
     * TODO: These players are different from the ones managed
     *       by 'm_playerMgr'. It is kind of confusing now!
     *       I will take care of this issue later.
	 */
	hoxPlayerInfoMap   m_onlinePlayers;

    hoxPlayersUI*      m_playersUI;
};

typedef std::list<hoxSite*>  hoxSiteList;

/**
 * The LOCAL Site.
 */
class hoxLocalSite : public hoxSite
{
public:
    hoxLocalSite(const hoxServerAddress& address);
    virtual ~hoxLocalSite() {}

    virtual const wxString GetName() const
        { return _("My Computer"); }

    virtual bool IsConnected() const { return true; }

    virtual hoxLocalPlayer* CreateLocalPlayer(const wxString& playerName);

	virtual unsigned int GetCurrentActionFlags() const;

    virtual void OnLocalRequest_JOIN( const wxString& sTableId )
        {}
    virtual void OnLocalRequest_NEW()
        {}
    virtual void OnLocalRequest_PRACTICE( const wxString& sSavedFile = "" );

protected:
    virtual unsigned int GetBoardFeatureFlags() const;
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

    virtual void Connect();
    virtual void Disconnect();

    virtual bool IsConnected() const;

    virtual void QueryForTables();

	virtual void OnPlayerJoined(const wxString& tableId,
		                        const wxString& playerId,
                                const int       playerScore,
                                const hoxColor  requestColor);

	virtual void JoinLocalPlayerToTable(const hoxNetworkTableInfo& tableInfo);

	virtual void OnShutdownReadyFromLocalPlayer();

    virtual hoxLocalPlayer* CreateLocalPlayer(const wxString& playerName);

	virtual unsigned int GetCurrentActionFlags() const;

    virtual void OnResponse_LOGIN( const hoxResponse_APtr& response );

    virtual void OnListOfTablesReceived(const hoxNetworkTableInfoList& tableList);

    /*************************************************
     * Implement hoxPlayersUI::UIOwner 's interface.
     *************************************************/

    virtual void OnPlayersUIEvent( hoxPlayersUI::EventType eventType,
                                   const wxString&         sPlayerId );

    virtual void OnLocalRequest_JOIN( const wxString& sTableId );
    virtual void OnLocalRequest_NEW();

protected:
    virtual void DisplayListOfTables(const hoxNetworkTableInfoList& tableList);

private:
    /* This pointer exists solely for the purpose of handling the case
     * in which the Site is deleted while the list-of-tables dialog
     * is being displayed.
     */
    hoxTablesDialog*    m_pTablesDlg;
};

/**
 * The Chesscape (remote) Site.
 */
class hoxChesscapeSite : public hoxRemoteSite
{
public:
    hoxChesscapeSite(const hoxServerAddress& address);
    virtual ~hoxChesscapeSite();

    virtual hoxLocalPlayer* CreateLocalPlayer(const wxString& playerName);

	virtual unsigned int GetCurrentActionFlags() const;

    virtual void OnLocalRequest_JOIN( const wxString& sTableId );
    virtual void OnLocalRequest_NEW();

    virtual void OnListOfTablesReceived(const hoxNetworkTableInfoList& tableList);

protected:
    virtual unsigned int GetBoardFeatureFlags() const;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Forward declaration
 */
class hoxSitesUI;

/**
 * The Site-Manager.
 * This is implemented as a singleton since we only need one instance.
 */
class hoxSiteManager
{
public:
	static hoxSiteManager* GetInstance();
    static void            DeleteInstance();
    ~hoxSiteManager() {}

	hoxSite* CreateSite( hoxSiteType             siteType, 
		                 const hoxServerAddress& address,
				         const wxString&         userName,
						 const wxString&         password );

    void CreateLocalSite();
    hoxSite* GetLocalSite() const { return m_localSite; }

	hoxSite* FindSite( const hoxServerAddress& address ) const;
	
	int GetNumberOfSites() const { return (int) m_sites.size(); }

	void DeleteSite( hoxSite* site );
    void DeleteLocalSite();

	void Close();

    void SetUI( hoxSitesUI* sitesUI ) { m_sitesUI = sitesUI; }
    void OnTableUICreated( hoxSite*      site,
                           hoxTable_SPtr pTable );
    void OnTableUIRemoved( hoxSite*      site,
                           hoxTable_SPtr pTable );

private:
    hoxSiteManager();
	static hoxSiteManager* m_instance;

private:
	hoxSiteList     m_sites;
    hoxSite*        m_localSite;
    hoxSitesUI*     m_sitesUI;
};

#endif /* __INCLUDED_HOX_SITE_H__ */
