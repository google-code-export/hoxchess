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
// Name:            hoxPlayerMgr.cpp
// Created:         10/06/2007
//
// Description:     The manager that manages ALL the players in this server.
/////////////////////////////////////////////////////////////////////////////

#include "hoxPlayerMgr.h"
#include "MyApp.h"          // wxGetApp()
#include "hoxMyPlayer.h"
#include "hoxChesscapePlayer.h"
#include "hoxAIPlayer.h"
#include <algorithm>   // std::find


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

hoxChesscapePlayer*
hoxPlayerMgr::CreateChesscapePlayer( const wxString& name,
                                     int             score /* = 1500 */)
{
    hoxChesscapePlayer* player 
        = new hoxChesscapePlayer( name, hoxPLAYER_TYPE_LOCAL, score );
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

hoxPlayer*
hoxPlayerMgr::CreateDummyPlayer( const wxString& name,
                                 int             score /* = 1500 */)
{
    hoxPlayer* player 
        = new hoxPlayer( name, hoxPLAYER_TYPE_DUMMY, score );
    m_players.push_back( player );

    return player;
}

hoxAIPlayer*
hoxPlayerMgr::CreateAIPlayer( const wxString& name,
                              int             score /*= 1500 */ )
{
    hoxAIPlayer* player =
        new hoxAIPlayer( name, hoxPLAYER_TYPE_AI, score );
    m_players.push_back( player );

    return player;
}

hoxPlayer* 
hoxPlayerMgr::FindPlayer( const wxString& playerId ) const
{
    for ( hoxPlayerList::const_iterator it = m_players.begin();
                                        it != m_players.end(); ++it )
    {
        if ( playerId == (*it)->GetId() )
        {
            return (*it);
        }
    }
    
    return NULL;
}

void
hoxPlayerMgr::OnSiteClosing()
{
    const char* FNAME = "hoxPlayerMgr::OnSiteClosing";

    wxLogDebug("%s: ENTER.", FNAME);

    /* Inform all players about the CLOSING. */
    for ( hoxPlayerList::iterator it = m_players.begin();
                                  it != m_players.end(); ++it )
    {
        (*it)->OnClosing_FromSite();
    }
}

/************************* END OF FILE ***************************************/
