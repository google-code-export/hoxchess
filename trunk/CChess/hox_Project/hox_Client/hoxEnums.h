/***************************************************************************
 *  Copyright 2007, 2008, 2009 Huy Phan  <huyphan@playxiangqi.com>         *
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

#ifndef __INCLUDED_HOX_ENUMS_H__
#define __INCLUDED_HOX_ENUMS_H__

#include <wx/wx.h>

/**
 * @file hoxEnums.h
 *
 * The file contains all basic constants commonly used through out 
 * the project.
 */

/**
 * The Application's Name and Version.
 */
#define HOX_APP_NAME  "HOXChess"
#define HOX_VERSION   "0.6.1.0"

/**
 * The default path of all the pieces' images.
 */
#define DEFAULT_PIECE_PATH "../resource/pieces/1"

/**
 * The default path of other Images.
 */
#define IMAGES_PATH  "../resource/images"

/**
 * The default path of sound files.
 */
#define SOUNDS_PATH  "../resource/sounds"

/* Use the Color-Mixer tool from the following site to generate the colors:
 *       http://colormixers.com/mixers/cmr/
 */
#define DEFAULT_BOARD_BACKGROUND_COLOR   "rgb(64, 64, 64)"
#define DEFAULT_BOARD_FOREGROUND_COLOR   "rgb(255, 255, 255)"

/**
 *  GUEST user-name prefix to identify guest accounts.
 *  Note: This prefix works under chesscape.com.
 */
#define hoxGUEST_PREFIX    "Guest#"

/**
 *  The constant representing an "unknown" score/rating.
 */
#define hoxSCORE_UNKNOWN    -1

/**
 * Results (... Return-Code)
 */
enum hoxResult
{
    hoxRC_UNKNOWN = -1,

    hoxRC_OK = 0,
    hoxRC_ERR,   // A generic error.

    hoxRC_HANDLED,   // something (request, event,...) has been handled.
    hoxRC_CLOSED,    // Something (socket,...) has been closed.
    hoxRC_NOT_FOUND, // Something is not found.
    hoxRC_NOT_SUPPORTED
};

/**
 * Piece's Type.
 *
 *  King (K), Advisor (A), Elephant (E), chaRiot (R), Horse (H), 
 *  Cannons (C), Pawns (P).
 */
enum hoxPieceType
{
    hoxPIECE_INVALID = 0,
    hoxPIECE_KING,             // King (or General)
    hoxPIECE_ADVISOR,          // Advisor (or Guard, or Mandarin)
    hoxPIECE_ELEPHANT,         // Elephant (or Ministers)
    hoxPIECE_CHARIOT,          // Chariot ( Rook, or Car)
    hoxPIECE_HORSE,            // Horse ( Knight )
    hoxPIECE_CANNON,           // Canon
    hoxPIECE_PAWN              // Pawn (or Soldier)
};

/**
 * Color for both Piece and Role.
 */
enum hoxColor
{
    hoxCOLOR_UNKNOWN = -1,
        /* This type indicates the absense of color or role.
		 * For example, it is used to indicate the player is not even
		 * at the table.
         */

    hoxCOLOR_RED,   // RED color.
    hoxCOLOR_BLACK, // BLACK color.

    hoxCOLOR_NONE 
        /* NOTE: This type actually does not make sense for 'Piece',
         *       only for "Player". It is used to indicate the role 
         *       of a player who is currently only observing the game,
         *       not playing.
         */
};

/**
 * Game's status.
 */
enum hoxGameStatus
{
    hoxGAME_STATUS_UNKNOWN = -1,

    hoxGAME_STATUS_OPEN = 0,    // Open but not enough Player.
    hoxGAME_STATUS_READY,       // Enough (2) players, waiting for 1st Move.
    hoxGAME_STATUS_IN_PROGRESS, // At least 1 Move has been made.
    hoxGAME_STATUS_RED_WIN,     // Game Over. Red won.
    hoxGAME_STATUS_BLACK_WIN,   // Game Over. Black won.
    hoxGAME_STATUS_DRAWN        // Game Over. Drawn.
};

/**
 * Player's status.
 */
enum hoxPlayerStatus
{
    hoxPLAYER_STATUS_UNKNOWN = -1,

    hoxPLAYER_STATUS_PLAYING,   // Playing with someone.
    hoxPLAYER_STATUS_OBSERVING, // Observing (or Free/Available).
    hoxPLAYER_STATUS_SOLO       // Playing solo (... with a computer).
};

/**
 * Site's type.
 */
enum hoxSiteType
{
    hoxSITE_TYPE_UNKNOWN = -1,

    hoxSITE_TYPE_LOCAL,
    hoxSITE_TYPE_REMOTE,

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

    hoxPLAYER_TYPE_DUMMY,
            /* Dummy player representing a Network (remote) player.
             * This player is created just to make sure that a table
             * being played have 2 players.
             */

    hoxPLAYER_TYPE_AI      
            /* Artificial Intelligent (AI) player representing the
             * local computer.
             */

};

/**
 * Game's Group.
 */
enum hoxGameGroup
{
    hoxGAME_GROUP_PUBLIC,
    hoxGAME_GROUP_PRIVATE
};

/**
 * Game's Type.
 */
enum hoxGameType
{
    hoxGAME_TYPE_UNKNOWN = -1,

    hoxGAME_TYPE_RATED,
    hoxGAME_TYPE_NONRATED,
    hoxGAME_TYPE_SOLO,    // vs. 'remote' COMPUTER

    hoxGAME_TYPE_PRACTICE // vs. 'local' COMPUTER
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

    hoxSOCKET_CLIENT_SOCKET_TIMEOUT = (15 * 60)  // 15 minutes
                /* Timeout applied to client -> server connection */

};

/**
 * Request types to communicate with Connection Thread.
 */
enum hoxRequestType
{
    hoxREQUEST_UNKNOWN = -1,

    hoxREQUEST_PLAYER_DATA,
        /* Network data incoming from a remote player */

    hoxREQUEST_LOGIN,
    hoxREQUEST_LOGOUT,
    hoxREQUEST_SHUTDOWN,
    hoxREQUEST_MOVE,
    hoxREQUEST_LIST,
    hoxREQUEST_NEW,
    hoxREQUEST_JOIN,
    hoxREQUEST_LEAVE,
    hoxREQUEST_UPDATE,

    hoxREQUEST_RESIGN,
        /* Resign request generated from a physical Table */

    hoxREQUEST_DRAW,
        /* Draw request generated from a physical Table */

    hoxREQUEST_RESET,
        /* Reset request generated from a physical Table */

    hoxREQUEST_E_JOIN,
        /* Event generated from a Table that a new Player just joined */

    hoxREQUEST_E_END,
        /* Event generated from a Table that the game has ended. */

    hoxREQUEST_E_SCORE,
        /* Event generated to inform of a player's new Score. */

    hoxREQUEST_I_TABLE,
        /* Info about a Table */

    hoxREQUEST_I_MOVES,
        /* Info about the "past/history" Moves */

    hoxREQUEST_INVITE,
        /* Invite request for a given Player */

    hoxREQUEST_PLAYER_INFO,
        /* Info request for a given Player */

    hoxREQUEST_PLAYER_STATUS,
        /* Event generated from a Player when his Status is changed. */

    hoxREQUEST_MSG
        /* Message generated (incoming) from a physical Table */
};

/**
 * Ranges for UI_IDs (in order to avoid conflict.)
 */
enum hoxUI_ID_Range
{
    hoxUI_ID_RANGE_FRAME = wxID_HIGHEST + 100,
	hoxUI_ID_RANGE_BOARD = wxID_HIGHEST + 200
};


#endif /* __INCLUDED_HOX_ENUMS_H__ */
