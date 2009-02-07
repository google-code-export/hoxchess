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
    this->Disconnect();
}

void
AIPlayer::Connect( const std::string&       sHost,
                   const unsigned short int nPort,
                   int nReadTimeout /* = HOX_NO_SOCKET_TIMEOUT */ )
{
    if ( 0 != m_sock.Connect( sHost, nPort ) )
    {
        throw std::runtime_error("Failed to connect to server: " + sHost );
    }

    if ( nReadTimeout != HOX_NO_SOCKET_TIMEOUT )
    {
        if ( 0 != m_sock.SetReadTimeout( nReadTimeout ) )
        {
            throw std::runtime_error("Failed to set socket's read timeout");
        }
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
AIPlayer::SendKeepAlive()
{
    hoxCommand outCommand("PING");
    _SendCommand( outCommand );

    hoxCommand  inCommand;  // Incoming command from the server.
    this->ReadIncomingCommand( inCommand );
}

void
AIPlayer::ReadIncomingCommand( hoxCommand& inCommand )
{
    inCommand.Clear();

    std::string sResponse;
    const int nRet = m_sock.ReadUntilAll( "\n\n", sResponse );
    if ( nRet == HOX_ERR_SOCKET_TIMEOUT )
    {
        throw TimeoutException("Socket read timeout");
    }
    else if ( nRet <= 0 )
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
