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
// Name:            main.cpp
// Created:         10/04/2008
//
// Description:     The Entry-Point of the Artificial Intelligent (AI) robot.
//
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdexcept>
#include <ctime>
#include "hoxCommon.h"
#include "TcpLib.h"
#include "AIPlayer.h"
#include "hoxCommand.h"

using namespace std;

//-----------------------------------------------------------------------------
//
//                                  Constants
//
//-----------------------------------------------------------------------------

#define  DEFAULT_HOX_SERVER    "games.playxiangqi.com"
#define  DEFAULT_HOX_PORT      80
#define  SOCKET_READ_TIMEOUT   (5 * 60) /* in seconds */

// ----------------------------------------------------------------------------
// Run the AI engine.
// ----------------------------------------------------------------------------
static void
_run_AI_engine( AIPlayer& aiPlayer )
{
    hoxTableInfo tableInfo;
    aiPlayer.OpenNewTable( tableInfo );

    const std::string sMyTableId = tableInfo.id;  // THE table.

    bool bDone = false;
    time_t lastKeepAliveTS = ::time(NULL);  // Last time to send Keep-Alive.
    while ( !bDone )
    {
        hoxCommand  inCommand;  // Incoming command from the server.
        try
        {
            time_t now = ::time(NULL);
            if ( now - lastKeepAliveTS > SOCKET_READ_TIMEOUT )
            {
                aiPlayer.SendKeepAlive();
                lastKeepAliveTS = now;
            }
            aiPlayer.ReadIncomingCommand( inCommand );
        }
        catch ( const TimeoutException& ex )
        {
            printf("%s: Timeout occurred [%s].\n", __FUNCTION__, ex.what());
            continue;
        }
        printf("%s: Received command [%s: %s].\n",
            __FUNCTION__, inCommand.m_type.c_str(), inCommand["content"].c_str());

        const std::string sInType    = inCommand.m_type;
        const std::string sInContent = inCommand["content"];

        if      ( sInType == "E_JOIN" )
        {
            std::string   tableId, playerId;
            int           nScore;
            hoxColor      color;

            hoxCommand::Parse_InCommand_E_JOIN( sInContent,
                tableId, playerId, nScore, color );
            printf("%s: Received [%s: %s (%d) %s].\n", __FUNCTION__, sInType.c_str(),
                playerId.c_str(), nScore, hoxUtil::ColorToString(color).c_str());
        }
        else if ( sInType == "LEAVE" )
        {
        }
        else if ( sInType == "MOVE" )
        {
            aiPlayer.HandleIncoming_MOVE( sInContent );
        }
        else if ( sInType == "DRAW" )
        {
            aiPlayer.HandleIncoming_DRAW( sInContent );
        }
        else if ( sInType == "E_END" )
        {
            std::string    tableId;
            hoxGameStatus  gameStatus;
            std::string    sReason;

            hoxCommand::Parse_InCommand_E_END( sInContent,
                tableId, gameStatus, sReason );
            printf("%s: Received [%s: %s %s].\n", __FUNCTION__, sInType.c_str(),
                tableId.c_str(), hoxUtil::GameStatusToString(gameStatus).c_str());

            if ( tableId == sMyTableId )
            {
                bDone = true;
                break;
            }
        }
    } // while ( ... )

    aiPlayer.LeaveCurrentTable();
}

// ----------------------------------------------------------------------------
// The main function.
// ----------------------------------------------------------------------------
int
main( int argc, char *argv[] )
{
    printf("%s: AI Robot starting...\n", __FUNCTION__);

    std::string ai_pid       = "Your_AI_PID"; // Player ID
    std::string ai_password  = "YOur_AI_Password"; // Player Password
    const int   nReadTimeout = SOCKET_READ_TIMEOUT; // Socket's read timeout (in seconds)

    if ( argc == 3 ) // pid / password from command-line?
    {
        ai_pid = argv[1];
        ai_password = argv[2];
    }

    if ( 0 != HOX::tcp_initialize() ) /* Initialize TCP. */
        return -1;

    for (;;)
    {
        try
        {
            printf("%s: Running AI [%s].\n", __FUNCTION__, ai_pid.c_str());
            AIPlayer  aiPlayer( ai_pid, ai_password ); // (pid, password)

            aiPlayer.Connect( DEFAULT_HOX_SERVER, DEFAULT_HOX_PORT,
                              nReadTimeout );
            aiPlayer.Login();

            for (;;)
            {
                _run_AI_engine( aiPlayer );
            }

            aiPlayer.Logout();
            aiPlayer.Disconnect();
        }
        catch (std::runtime_error ex)
        {
            printf("%s: Caught runtime exception [%s]\n", __FUNCTION__, ex.what());
        }
    }

    /* -------- */
    /* Cleanup. */
    /* -------- */
    HOX::tcp_deinitialize(); /* De-initialize TCP. */

    printf("%s: AI Robot stopping...\n", __FUNCTION__);
    return 0;
}

/************************* END OF FILE ***************************************/
