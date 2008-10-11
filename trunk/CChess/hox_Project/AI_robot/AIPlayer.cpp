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
#include "../xqwlight/XQWLight.h"

#include <stdexcept>   // std::runtime_error

//-----------------------------------------------------------------------------
//
//                                  Constants
//
//-----------------------------------------------------------------------------

#define  DEFAULT_HOX_SERVER    "games.playxiangqi.com"
#define  DEFAULT_HOX_PORT      8000


//-----------------------------------------------------------------------------
//
//                                  AIPlayer
//
//-----------------------------------------------------------------------------

AIPlayer::AIPlayer( const std::string& id,
                    const std::string& password )
        : m_id( id )
        , m_password( password )
{
}

AIPlayer::~AIPlayer()
{
}

void
AIPlayer::Connect()
{
    if ( 0 != m_sock.Connect( DEFAULT_HOX_SERVER, DEFAULT_HOX_PORT ) )
    {
        throw std::runtime_error("Failed to connect to server: "
                                 + std::string(DEFAULT_HOX_SERVER));
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
    XQWLight::on_human_move( sMove );
}

std::string
AIPlayer::GenerateNextMove()
{
    return XQWLight::generate_move();
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
    XQWLight::initialize();
}

/************************* END OF FILE ***************************************/
