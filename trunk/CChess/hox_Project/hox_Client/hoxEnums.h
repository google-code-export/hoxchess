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
// Name:            hoxEnums.h
// Created:         09/28/2007
//
// Description:     Containing basic constants commonly used through out 
//                  the project.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_ENUMS_H_
#define __INCLUDED_HOX_ENUMS_H_

/**
 * @file hoxEnums.h
 *
 * The file contains all basic constants commonly used through out 
 * the project.
 */

/**
 * The default path of all the pieces' images.
 */
#define PIECES_PATH "../pieces_5"

/**
 * The default HTTP server (hostname and port).
 */
#define HOX_HTTP_SERVER_HOSTNAME  "www.playxiangqi.com"
#define HOX_HTTP_SERVER_PORT      80

/**
 * Results
 */
enum hoxResult
{
    hoxRESULT_UNKNOWN = -1,

    hoxRESULT_OK = 0,
    hoxRESULT_ERR,   // A generic error.

    hoxRESULT_HANDLED,   // something (request, event,...) has been handled.
    hoxRESULT_NOT_SUPPORTED
};

/**
 * Piece's Type.
 *
 *  King (K), Advisor (A), Elephant (E), chaRiot (R), Horse (H), 
 *  Cannons (C), Pawns (P).
 */
enum hoxPieceType
{
    hoxPIECE_TYPE_INVALID = 0,
    hoxPIECE_TYPE_KING,             // King (or General)
    hoxPIECE_TYPE_ADVISOR,          // Advisor (or Guard, or Mandarin)
    hoxPIECE_TYPE_ELEPHANT,         // Elephant (or Ministers)
    hoxPIECE_TYPE_CHARIOT,          // Chariot ( Rook, or Car)
    hoxPIECE_TYPE_HORSE,            // Horse ( Knight )
    hoxPIECE_TYPE_CANNON,           // Canon
    hoxPIECE_TYPE_PAWN              // Pawn (or Soldier)
};

/**
 * Piece's color.
 */
enum hoxPieceColor
{
    hoxPIECE_COLOR_RED,   // RED color.
    hoxPIECE_COLOR_BLACK, // BLACK color.

    hoxPIECE_COLOR_NONE 
        /* TODO: This type actually does not make sense for 'Piece',
         *       only for "Player". It is used to indicated the role 
         *       of a player who is currently only observe the game,
         *       not playing.
         */
};

/**
 * Game's status.
 */
enum hoxGameStatus
{
    hoxGAME_STATUS_OPEN = 0,    // Open but not enough Player.
    hoxGAME_STATUS_READY,       // Enough (2) players, waiting for 1st Move.
    hoxGAME_STATUS_IN_PROGRESS, // At least 1 Move has been made.
    hoxGAME_STATUS_RED_WIN,     // Game Over. Red won.
    hoxGAME_STATUS_BLACK_WIN,   // Game Over. Black won.
    hoxGAME_STATUS_DRAWN        // Game Over. Drawn.
};

/**
 * Site's type.
 */
enum hoxSiteType
{
    hoxSITE_TYPE_UNKNOWN = -1,

    hoxSITE_TYPE_LOCAL,
    hoxSITE_TYPE_REMOTE,
    hoxSITE_TYPE_HTTP,

	/* Other third-party sites */

	hoxSITE_TYPE_CHESSCAPE    /* http://www.chesscape.com */
};

/**
 * Player's type.
 */
enum hoxPlayerType
{
    hoxPLAYER_TYPE_LOCAL,      
            /* A player who have physical access to the Board.
             * Specifically, he can make Move(s) using the mouse
             * on the local computer.
             * In principle, there can ONLY one local player.
             */

    hoxPLAYER_TYPE_REMOTE,    
            /* Any player who is remotely logging into this computer.
             */

    hoxPLAYER_TYPE_DUMMY       
            /* Dummy player representing a Network (remote) player.
             * This player is created just to make sure that a table
             * being played have 2 players.
             */
};

/**
 * Game's type.
 */
enum hoxGameType
{
    hoxGAME_TYPE_UNKNOWN = -1,

    hoxGAME_TYPE_RATED,
    hoxGAME_TYPE_NONRATED,
    hoxGAME_TYPE_SOLO_BLACK,  // Player = BLACK, computer = RED
	hoxGAME_TYPE_SOLO_RED     // Player = RED, computer = BLACK
};

/**
 * Network constants.
 */
enum hoxNetworkContant
{
   /*
    * !!! Do not change the values nor orders of the following !!!
    */

    hoxNETWORK_MAX_MSG_SIZE         = (5 * 1024), // 5-KByte buffer
    hoxNETWORK_DEFAULT_SERVER_PORT  = 3000
};

/**
 * Time constants.
 */
enum hoxTimeContant
{
   /*
    * !!! Do not change the values the following !!!
    */

    hoxTIME_ONE_SECOND_INTERVAL     = 1000,   // 1 second

    hoxTIME_DEFAULT_GAME_TIME       = 20*60,  // 20 minutes
    hoxTIME_DEFAULT_MOVE_TIME       = 5*60,   // 5 minutes
    hoxTIME_DEFAULT_FREE_TIME       = 30      // 30 seconds
};

/**
 * Socket related constants.
 */
enum hoxSocketContant
{
   /*
    * !!! Do not change the values the following !!!
    */

    hoxSOCKET_CLIENT_SOCKET_TIMEOUT = 10,   // 10 seconds
                /* Timeout applied to client -> server connection */

    hoxSOCKET_SERVER_ACCEPT_TIMEOUT = 5,    // 5 seconds
                /* Timeout applied to server-socket which is waiting  
                 * by wxSocketServer::Accept() for new incoming client 
                 * connections. This timeout is needed so that the server
                 * can process the SHUTDOWN request.
                 */

    hoxSOCKET_HTTP_POLL_INTERVAL = 5,  // 5 seconds
                /* HTTP Polling interval for HTTP Connection */

    hoxSOCKET_HTTP_TIMEOUT = 5         // 5 seconds
                /* Timeout applied to HTTP client -> HTTP server connection */

};

/**
 * Network event-types.
 */
enum hoxNetworkEvenType
{
   /*
    * !!! Do not change the values the following !!!
    *     as they are matched with those on the HTTP server.
    */

    hoxNETWORK_EVENT_TYPE_NEW_PLAYER_RED       = 1,
    hoxNETWORK_EVENT_TYPE_NEW_PLAYER_BLACK     = 2,
    hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_RED     = 3,
    hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_BLACK   = 4,
    hoxNETWORK_EVENT_TYPE_NEW_MOVE             = 5,
    hoxNETWORK_EVENT_TYPE_NEW_WALL_MSG         = 6
};

/**
 * Request types to communicate with Connection Thread.
 */
enum hoxRequestType
{
    hoxREQUEST_TYPE_UNKNOWN = -1,

    hoxREQUEST_TYPE_ACCEPT,

    hoxREQUEST_TYPE_PLAYER_DATA,
        /* Network data incoming from a remote player */

    hoxREQUEST_TYPE_CONNECT,
    hoxREQUEST_TYPE_DISCONNECT,
    hoxREQUEST_TYPE_SHUTDOWN,
    hoxREQUEST_TYPE_POLL,
    hoxREQUEST_TYPE_MOVE,
    hoxREQUEST_TYPE_LIST,
    hoxREQUEST_TYPE_NEW,
    hoxREQUEST_TYPE_JOIN,
    hoxREQUEST_TYPE_LEAVE,

    hoxREQUEST_TYPE_NEW_JOIN,
        /* Event generated from a Table that a new Player just joined */

    hoxREQUEST_TYPE_OUT_DATA,
        /* Outgoing data to be sent out */

    hoxREQUEST_TYPE_WALL_MSG
        /* Message generated (incoming) from a physical Table */
};

/**
 * Request flags to communicate with Connection Thread.
 */
enum hoxRequestFlag
{
    hoxREQUEST_FLAG_NONE        = 0,
    hoxREQUEST_FLAG_KEEP_ALIVE  = 1  // NOTE: This flag is not being used now.
};

/**
 * Response flags to communicate back to the sender.
 */
enum hoxResponseFlag
{
    hoxRESPONSE_FLAG_NONE             = 0,

    hoxRESPONSE_FLAG_CONNECTION_LOST  = 1 
        /* To signal that the connection to the server has been lost. */

};

/**
 * Other constants.
 */
enum
{
    // id for sockets
    CLIENT_SOCKET_ID
};

#endif /* __INCLUDED_HOX_ENUMS_H_ */
