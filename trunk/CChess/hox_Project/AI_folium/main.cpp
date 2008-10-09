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
// Name:            main.cpp
// Created:         10/04/2008
//
// Description:     The Entry-Point of the Artificial Intelligent (AI)
//                  robot that is based on Folilum project:
//                         http://folium.googlecode.com
//
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdexcept>
#include "hoxCommon.h"
#include "TcpLib.h"
#include "AIPlayer.h"
#include "hoxCommand.h"

using namespace std;

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
    while ( !bDone )
    {
        hoxCommand  inCommand;  // Incoming command from the server.
        aiPlayer.ReadIncomingCommand( inCommand );
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
main()
{
    printf("%s: Folium AI starting...\n", __FUNCTION__);

    if ( 0 != HOX::tcp_initialize() ) /* Initialize TCP. */
        return -1;

    try
    {
        AIPlayer  aiPlayer( "AI_folium", "AI_folium" ); // (pid, password)

        aiPlayer.Connect();
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

    /* -------- */
    /* Cleanup. */
    /* -------- */
    HOX::tcp_deinitialize(); /* De-initialize TCP. */

    printf("%s: Folium AI stopping...\n", __FUNCTION__);
    return 0;
}

/************************* END OF FILE ***************************************/
