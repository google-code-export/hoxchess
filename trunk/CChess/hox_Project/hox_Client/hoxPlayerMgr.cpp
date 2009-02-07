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
// Name:            hoxPlayerMgr.cpp
// Created:         10/06/2007
//
// Description:     The manager that manages ALL the players in this server.
/////////////////////////////////////////////////////////////////////////////

#include "hoxPlayerMgr.h"
#include "hoxMyPlayer.h"
#include "hoxChesscapePlayer.h"


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

hoxLocalPlayer*
hoxPlayerMgr::CreateLocalPlayer( const wxString& name,
                                 int             score /* = 1500 */)
{
    hoxLocalPlayer* player 
        = new hoxLocalPlayer( name, hoxPLAYER_TYPE_LOCAL, score );
    m_players.push_back( player );

    return player;
}

void
hoxPlayerMgr::DeletePlayer( const wxString& playerId )
{
    hoxPlayer* foundPlayer = this->FindPlayer( playerId );
    if ( foundPlayer != NULL )
    {
        m_players.remove( foundPlayer );
        delete foundPlayer;
    }
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

/************************* END OF FILE ***************************************/
