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
// Name:            AIPlayer.h
// Created:         10/04/2008
//
// Description:     This is an AI Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_AI_PLAYER_H__
#define __INCLUDED_AI_PLAYER_H__

#include <string>
#include "TcpLib.h"  // Socket
#include "hoxCommon.h"

/* Forward declarations. */
class hoxCommand;

/**
 * The AI Player
 */
class AIPlayer
{
public:
    AIPlayer( const std::string& id,
              const std::string& password );
    virtual ~AIPlayer();

    void Connect();
    void Disconnect();
    void Login();
    void Logout();
    void OpenNewTable( hoxTableInfo& tableInfo );
    void LeaveCurrentTable();

    void ReadIncomingCommand( hoxCommand& inCommand );
    void HandleIncoming_MOVE( const std::string& sInContent );
    void HandleIncoming_DRAW( const std::string& sInContent );

protected:
    void        OnOpponentMove( const std::string& sMove );
    std::string GenerateNextMove();

private:
    void _SendCommand( hoxCommand& command );
    void _SendMove( const std::string& sMove );
    void _SendDraw();
    void _ResetAIEngine();

private:
    HOX::Socket         m_sock;
    const std::string   m_id;
    const std::string   m_password;

    std::string         m_sTableId; // THE table this Player is playing.

    class FOLIUM_Engine;  // Forward declaration.
    FOLIUM_Engine*      m_engine;
};

#endif /* __INCLUDED_AI_PLAYER_H__ */
