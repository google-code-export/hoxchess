/////////////////////////////////////////////////////////////////////////////
// Name:            hoxEnums.h
// Program's Name:  Huy's Open Xiangqi
// Created:         09/28/2007
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_ENUMS_H_
#define __INCLUDED_HOX_ENUMS_H_


// FIXME: Hard-coded
#define PIECES_PATH "C:/Users/HPhan/Documents/CChess/hox_Project/pieces"

//
// Results
//
enum hoxResult
{
    hoxRESULT_OK = 0,
    hoxRESULT_ERR,   // A generic error.

    hoxRESULT_NOT_SUPPORTED
};

//
// Piece's Type.
//
//  King (K), Advisor (A), Elephant (E), chaRiot (R), Horse (H), 
//  Cannons (C), Pawns (P).
//
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

//
// Piece's color.
//
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

//
// Table's status
//
enum hoxTableStatus
{
  /*
   * !!! Do not change the values nor orders of the following !!!
   */

  hoxTABLE_STATUS_OPEN = 0,  // Open but not enough player
  hoxTABLE_STATUS_READY,   // Enough (2) players, waiting for 1st move
  hoxTABLE_STATUS_IN_PROGRESS,  // At least 1 move has been made
  hoxTABLE_STATUS_CLOSED
};

//
// Player's type.
//
enum hoxPlayerType
{
    hoxPLAYER_TYPE_HOST,      
            /* A player who controls this computer.
             * He has the physical access to the Board.
             * Specifically, he can make Move(s) using the mouse
             * on the local computer.
             * There can ONLY one host player.
             * This player does not have any network connections.
             * If there is a such a need, the Host will dynamically
             * create LOCALl players (see below) to do the network-related
             * tasks.
             */

    hoxPLAYER_TYPE_LOCAL,      
            /* A player who have physical access to the Board.
             * Specifically, he can make Move(s) using the mouse
             * on the local computer.
             * In principle, there can ONLY one local player.
             */

    hoxPLAYER_TYPE_NETWORK,    
            /* Any player who is logging into this computer.
             */

    hoxPLAYER_TYPE_DUMMY       
            /* Dummy player representing a Network (remote) player.
             * This player is created just to make sure that a table
             * being played have 2 players.
             */
};

//
// Network constants
//
enum hoxNetworkContant
{
  /*
   * !!! Do not change the values nor orders of the following !!!
   */

  hoxNETWORK_MAX_MSG_SIZE         = (5 * 1024), // 5-KBytes buffer
  hoxNETWORK_DEFAULT_SERVER_PORT  = 3000
};

//
// Network command
//
enum hoxNetworkCmd
{
  /*
   * !!! Do not change the values nor orders of the following !!!
   */

  hoxNETWORK_CMD_QUERY_TABLE  = 0xBE,
  hoxNETWORK_CMD_JOIN_TABLE   = 0xCE,
  hoxNETWORK_CMD_NEW_MOVE     = 0xDE
}; 

//
// Time constants
//
enum hoxTimeContant
{
  /*
   * !!! Do not change the values the following !!!
   */

  hoxTIME_ONE_SECOND_INTERVAL     = 1000,   // 1 second
  hoxTIME_DEFAULT_GAME_TIME       = 20*60   // 20 minutes
};

//
// Network event-types
//
enum hoxNetworkEvenType
{
   /*
    * !!! Do not change the values the following !!!
    *     as they are matched with those on the WWW server.
    */

    hoxNETWORK_EVENT_TYPE_NEW_PLAYER_RED       = 1,
    hoxNETWORK_EVENT_TYPE_NEW_PLAYER_BLACK     = 2,
    hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_RED     = 3,
    hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_BLACK   = 4,
    hoxNETWORK_EVENT_TYPE_NEW_MOVE             = 5
};

//
// Request types to communicate with WWW Thread
//
enum hoxRequestType
{
    hoxREQUEST_TYPE_UNKNOWN = -1,

    hoxREQUEST_TYPE_ACCEPT,
    hoxREQUEST_TYPE_DATA,

    hoxREQUEST_TYPE_CONNECT,
    hoxREQUEST_TYPE_DISCONNECT,
    hoxREQUEST_TYPE_SHUTDOWN,
    hoxREQUEST_TYPE_POLL,
    hoxREQUEST_TYPE_MOVE,
    hoxREQUEST_TYPE_LIST,
    hoxREQUEST_TYPE_NEW,
    hoxREQUEST_TYPE_JOIN,
    hoxREQUEST_TYPE_LEAVE,

    hoxREQUEST_TYPE_LISTEN,
    hoxREQUEST_TYPE_TABLE_MOVE
};

//
// Request flags to communicate with WWW Thread
//
enum hoxRequestFlag
{
    hoxREQUEST_FLAG_NONE        = 0,
    hoxREQUEST_FLAG_KEEP_ALIVE  = 1
};

// ----------------------------------------------------------------------------
// Other constants
// ----------------------------------------------------------------------------
enum
{
    // id for sockets
    SERVER_ID,
    SERVER_SOCKET_ID,

    // id for sockets
    CLIENT_SOCKET_ID
};

#endif /* __INCLUDED_HOX_ENUMS_H_ */s