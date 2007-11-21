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
// Name:            hoxPlayerMgr.cpp
// Created:         10/06/2007
//
// Description:     The manager that manages ALL the players in this server.
/////////////////////////////////////////////////////////////////////////////

#include "hoxPlayerMgr.h"
#include "MyApp.h"          // wxGetApp()
#include <algorithm>   // std::find


// Declare the single instance.
hoxPlayerMgr* hoxPlayerMgr::m_instance = NULL;

hoxPlayerMgr*
hoxPlayerMgr::GetInstance()
{
    if ( m_instance == NULL )
    {
        m_instance = new hoxPlayerMgr();
    }
        
    return m_instance;
}

hoxPlayerMgr::hoxPlayerMgr()
{
}

hoxPlayerMgr::~hoxPlayerMgr()
{
    for ( hoxPlayerList::iterator it = m_players.begin();
                                  it != m_players.end(); ++it )
    {
        delete (*it);
    }
}

hoxHostPlayer*
hoxPlayerMgr::CreateHostPlayer( const wxString& name,
                                int             score /* = 1500 */)
{
    hoxHostPlayer* player 
        = new hoxHostPlayer( name, hoxPLAYER_TYPE_HOST, score );
    m_players.push_back( player );

    return player;
}

hoxHttpPlayer*
hoxPlayerMgr::CreateHTTPPlayer( const wxString& name,
                                int             score /* = 1500 */)
{
    hoxHttpPlayer* player 
        = new hoxHttpPlayer( name, hoxPLAYER_TYPE_LOCAL, score );
    m_players.push_back( player );

    return player;
}

hoxMyPlayer*
hoxPlayerMgr::CreateMyPlayer( const wxString& name,
                              int             score /* = 1500 */)
{
    hoxMyPlayer* player 
        = new hoxMyPlayer( name, hoxPLAYER_TYPE_LOCAL, score );
    m_players.push_back( player );

    return player;
}

hoxRemotePlayer*
hoxPlayerMgr::CreateRemotePlayer( const wxString& name,
                                  int             score /* = 1500 */)
{
    hoxRemotePlayer* player 
        = new hoxRemotePlayer( name, hoxPLAYER_TYPE_REMOTE, score );
    m_players.push_back( player );

    return player;
}

hoxPlayer*
hoxPlayerMgr::CreatePlayer( const wxString& name,
                            hoxPlayerType   type,
                            int             score /* = 1500 */)
{
    const char* FNAME = "hoxPlayerMgr::CreatePlayer";
    wxLogDebug("%s: Creating player [%s]...", FNAME, name.c_str());

    hoxPlayer* player = new hoxPlayer( name, type, score );
    m_players.push_back( player );

    return player;
}

void 
hoxPlayerMgr::DeletePlayer( hoxPlayer* player )
{
    const char* FNAME = "hoxPlayerMgr::DeletePlayer";

    wxCHECK_RET( player != NULL, "The player should not be NULL." );
    
    wxLogDebug("%s: Deleting player [%s]...", FNAME, player->GetName().c_str());

    delete player;
    m_players.remove( player );
}

int
hoxPlayerMgr::RemovePlayer( hoxPlayer* player )
{
    const char* FNAME = "hoxPlayerMgr::RemovePlayer";
    int playerFound = 0;

    wxCHECK_MSG( player != NULL, 0, "The player should not be NULL." );
    
    wxLogDebug("%s: Deleting player [%s]...", FNAME, player->GetName().c_str());

    hoxPlayerList::iterator found = std::find( m_players.begin(), m_players.end(), player );
    if ( found != m_players.end() )
        playerFound = 1;

    //delete player;
    m_players.remove( player );

    return playerFound;
}

hoxPlayer* 
hoxPlayerMgr::FindPlayer( const wxString& playerId )
{
    for ( hoxPlayerList::iterator it = m_players.begin();
                                  it != m_players.end(); ++it )
    {
        if ( playerId == (*it)->GetName() )
        {
            return (*it);
        }
    }
    
    return NULL;
}

void
hoxPlayerMgr::OnSystemShutdown()
{
    const char* FNAME = "hoxPlayerMgr::OnSystemShutdown";

    wxLogDebug("%s: ENTER.", FNAME);

    /* Inform all players about the SHUTDOWN. */
    for ( hoxPlayerList::iterator it = m_players.begin();
                                  it != m_players.end(); ++it )
    {
        wxCommandEvent event( hoxEVT_PLAYER_APP_SHUTDOWN );
        event.SetString( "System being shutdowned" );
        event.SetEventObject( &(wxGetApp()) );
        wxPostEvent( (*it) , event );
    }
}

/************************* END OF FILE ***************************************/
