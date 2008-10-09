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
// Name:            AIPlayer.cpp
// Created:         10/04/2008
//
// Description:     This is an AI Player.
/////////////////////////////////////////////////////////////////////////////

#include "AIPlayer.h"
#include "hoxCommand.h"
#include "../folium/folEngine.h"

#include <stdexcept>   // std::runtime_error
#include <sstream>     // ostringstream

//-----------------------------------------------------------------------------
//
//                                FOLIUM_Engine
//
//-----------------------------------------------------------------------------

class AIPlayer::FOLIUM_Engine
{
public:
    FOLIUM_Engine()
    {
        // FEN starting position of the Board.
        const std::string fenStartPosition = 
            "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1";

        /* TODO:
         *    Wangmao, I think we should hide this '21' magic number.
         *    For a normal developer (who is not familiar with Xiangqi AI),
         *    we have no idea what this 'hash' value is.
         */
        _engine.reset( new folEngine(XQ(fenStartPosition), 21) );
    }

    unsigned int _hox2folium( const std::string& sMove )
    {
        unsigned int sx = sMove[0] - '0';
        unsigned int sy = sMove[1] - '0';
        unsigned int dx   = sMove[2] - '0';
        unsigned int dy   = sMove[3] - '0';
	    unsigned int src = 89 - (sx + sy * 9);
	    unsigned int dst = 89 - (dx + dy * 9);
	    return src | (dst << 7);
    }

    std::string _folium2hox( unsigned int move )
    {
	    unsigned int src = 89 - (move & 0x7f);
	    unsigned int dst = 89 - ((move >> 7) & 0x7f);
	    unsigned int sx = src % 9;
	    unsigned int sy = src / 9;
	    unsigned int dx = dst % 9;
	    unsigned int dy = dst / 9;
        std::ostringstream ostr;
        ostr << sx << sy << dx << dy;
	    return ostr.str();
    }

    std::string GenerateMove()
    {
        std::set<unsigned int> ban;
        const int searchDepth = 3 /* 7 */;
        unsigned int move = _engine->search( searchDepth, ban );
        std::string sNextMove;
        if (move)
        {
            sNextMove = _folium2hox( move );
            _engine->make_move(move);
        }
        return sNextMove;
    }

    void OnHumanMove( const std::string& sMove )
    {
        unsigned int move = _hox2folium(sMove);
        _engine->make_move(move);
    }

private:
    typedef std::auto_ptr<folEngine> folEngine_APtr;
    folEngine_APtr   _engine;
};


//-----------------------------------------------------------------------------
//
//                                  AIPlayer
//
//-----------------------------------------------------------------------------

AIPlayer::AIPlayer( const std::string& id,
                    const std::string& password )
        : m_id( id )
        , m_password( password )
        , m_engine( NULL )
{
}

AIPlayer::~AIPlayer()
{
    delete m_engine;
}

void
AIPlayer::Connect()
{
    const std::string        sHost = "games.playxiangqi.com";
    const unsigned short int nPort = 8000;

    if ( 0 != m_sock.Connect( sHost, nPort ) )
    {
        throw std::runtime_error("Failed to connect to server: " + sHost);
    }
}

void
AIPlayer::Disconnect()
{
    if ( 0 != m_sock.Close() )
    {
        throw std::runtime_error("Failed to close socket connection");
    }
}

void
AIPlayer::Login()
{
    hoxCommand outCommand("LOGIN");
    outCommand["password"] = m_password;
    _SendCommand( outCommand );

    hoxCommand  inCommand;  // Incoming command from the server.
    this->ReadIncomingCommand( inCommand );
}

void
AIPlayer::Logout()
{
    hoxCommand outCommand("LOGOUT");
    _SendCommand( outCommand );

    hoxCommand  inCommand;  // Incoming command from the server.
    this->ReadIncomingCommand( inCommand );
}

void
AIPlayer::OpenNewTable( hoxTableInfo& tableInfo )
{
    hoxCommand outCommand("NEW");
    outCommand["itimes"]   = "1500/300/20";
    outCommand["color"]    = "Black";
    _SendCommand( outCommand );

    /* Wait until reading the "right" response. */
    for (;;)
    {
        hoxCommand  inCommand;  // Incoming command from the server.
        this->ReadIncomingCommand( inCommand );
        if ( inCommand.m_type == "I_TABLE" )
        {
            hoxTableInfo::String_To_Table( inCommand["content"],
                                           tableInfo );
            if ( tableInfo.blackId == m_id ) // Is this my Table?
            {
                break;
            }
        }
    }

    m_sTableId = tableInfo.id;  // Remember that this is my table.
    _ResetAIEngine();
}

void
AIPlayer::_SendMove( const std::string& sMove )
{
    hoxCommand outCommand("MOVE");
    outCommand["tid"]     = m_sTableId;
    outCommand["move"]    = sMove;
    outCommand["status"]  = "in_progress";
    _SendCommand( outCommand );
}

void
AIPlayer::_SendDraw()
{
    hoxCommand outCommand("DRAW");
    outCommand["tid"]     = m_sTableId;
    outCommand["draw_response"]  = "1";  // Accept the DRAW request.
    _SendCommand( outCommand );
}

void
AIPlayer::LeaveCurrentTable()
{
    hoxCommand outCommand("LEAVE");
    outCommand["tid"]     = m_sTableId;
    _SendCommand( outCommand );
}

void
AIPlayer::ReadIncomingCommand( hoxCommand& inCommand )
{
    inCommand.Clear();

    std::string sResponse;
    if ( 0 >= m_sock.ReadUntilAll( "\n\n", sResponse ) )
    {
        throw std::runtime_error("Failed to read response");
    }

    hoxCommand::String_To_Command( sResponse, inCommand );
    if ( inCommand["code"] != "0" )
    {
        throw std::runtime_error("Failed to convert String to Command");
    }
    printf("%s: Received command [%s: %s].\n",
        __FUNCTION__, inCommand.m_type.c_str(), inCommand["content"].c_str());
}

void
AIPlayer::HandleIncoming_MOVE( const std::string& sInContent )
{
    std::string   tableId, playerId, sMove;
    hoxGameStatus gameStatus = hoxGAME_STATUS_UNKNOWN;

    hoxCommand::Parse_InCommand_MOVE( sInContent,
        tableId, playerId, sMove, gameStatus );
    printf("%s: Received [MOVE: %s %s].\n", __FUNCTION__,
        playerId.c_str(), sMove.c_str());

    this->OnOpponentMove( sMove );
    if ( gameStatus == hoxGAME_STATUS_IN_PROGRESS )
    {
        const std::string sNextMove = this->GenerateNextMove();
        printf("%s: Generated next Move = [%s].", __FUNCTION__, sNextMove.c_str());

        _SendMove( sNextMove );
    }
}

void
AIPlayer::HandleIncoming_DRAW( const std::string& sInContent )
{
    std::string   tableId, playerId;

    hoxCommand::Parse_InCommand_DRAW( sInContent,
        tableId, playerId );
    printf("%s: Received [DRAW: %s %s].\n", __FUNCTION__,
        tableId.c_str(), playerId.c_str());

    _SendDraw();
}

void
AIPlayer::OnOpponentMove( const std::string& sMove )
{
    m_engine->OnHumanMove( sMove );
}

std::string
AIPlayer::GenerateNextMove()
{
    return m_engine->GenerateMove();
}

void
AIPlayer::_SendCommand( hoxCommand& command )
{
    /* Make sure THIS player-ID is sent along. */
    command["pid"] = m_id;

    const std::string sCommand = command.ToString() + "\n";
    if ( 0 != m_sock.SendData( sCommand ) )
    {
        throw std::runtime_error("Failed to send command");
    }
}

void
AIPlayer::_ResetAIEngine()
{
    delete m_engine;
    m_engine = new FOLIUM_Engine();
}

/************************* END OF FILE ***************************************/
