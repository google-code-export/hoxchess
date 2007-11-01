/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPlayerMgr.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/06/2007
//
// Description:     The manager that manages ALL the tables in this server.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_PLAYER_MGR_H_
#define __INCLUDED_HOX_PLAYER_MGR_H_

#include "wx/wx.h"
#include "hoxPlayer.h"
#include "hoxHostPlayer.h"
#include "hoxRemotePlayer.h"
#include "hoxHttpPlayer.h"
#include "hoxMyPlayer.h"

/**
 * A singleton class that manages all players in the system.
 */
class hoxPlayerMgr
{
public:
    static hoxPlayerMgr* GetInstance();        

    ~hoxPlayerMgr();

    hoxHostPlayer* CreateHostPlayer( const wxString& name,
                                     int             score = 1500 );

    hoxHttpPlayer* CreateHTTPPlayer( const wxString& name,
                                     int             score = 1500 );

    hoxMyPlayer* CreateMyPlayer( const wxString& name,
                                 int             score = 1500 );

    hoxRemotePlayer* CreateRemotePlayer( const wxString& name,
                                         int             score = 1500 );

    hoxPlayer* CreatePlayer( const wxString& name,
                             hoxPlayerType   type,
                             int             score = 1500 );

    void DeletePlayer( hoxPlayer* player );

    /**
     * @return NULL if not found.
     */
    hoxPlayer* FindPlayer( const wxString& playerId );

private:
    hoxPlayerMgr();

    static hoxPlayerMgr* m_instance;  // The single instance

    hoxPlayerList   m_players;  // The list of all players in the system.
};

#endif /* __INCLUDED_HOX_PLAYER_MGR_H_ */
