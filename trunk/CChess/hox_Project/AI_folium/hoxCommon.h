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
// Name:            hoxCommon.h
// Created:         10/05/2008
//
// Description:     Containing basic Constants and Types commonly used
//                  through out  the project.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_COMMON_H__
#define __INCLUDED_HOX_COMMON_H__

/**
 * @file hoxCommon.h
 *
 * The file contains basic Constants and Types commonly used
 * through out  the project.
 */

#include <string>
#include <list>
#include <boost/tokenizer.hpp>

// ----------------------------------------------------------------- //
//                                                                   //
//                Typedefs                                           //
//                                                                   //
// ----------------------------------------------------------------- //

typedef boost::tokenizer<boost::char_separator<char> > hoxTokenizer;
typedef boost::char_separator<char>                    hoxSeparator;


// ----------------------------------------------------------------- //
//                                                                   //
//                Enums                                              //
//                                                                   //
// ----------------------------------------------------------------- //

/**
 * Results (... Return-Code)
 */
enum hoxResult
{
    hoxRC_UNKNOWN = -1,

    hoxRC_OK = 0,
    hoxRC_ERR,          // A generic error.

    hoxRC_HANDLED,      // Something (request, event,...) was handled.
    hoxRC_CLOSED,       // Something (socket,...) was closed.
    hoxRC_NOT_FOUND,    // Something was not found.
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


// ----------------------------------------------------------------- //
//                                                                   //
//                  Types                                            //
//                                                                   //
// ----------------------------------------------------------------- //

/**
 * Game's Time-info.
 */
class hoxTimeInfo
{
public:
    int  nGame;  // Game-time (in seconds).
    int  nMove;  // Move-time (in seconds).
    int  nFree;  // Free-time (in seconds).

	hoxTimeInfo( int g = 0, int m = 0, int f = 0 ) 
        : nGame( g ), nMove( m ), nFree( f ) {}

    void Clear()
		{
			nGame = 0;
			nMove = 0;
			nFree = 0;
		}

    bool IsEmpty() const 
        { return (nGame == 0) && (nMove == 0) && (nFree == 0); }
};

/**
 * A Table Info.
 */
class hoxTableInfo
{
public:
    std::string   id;         // table-Id.
	hoxGameGroup  group;      // Public / Private
    std::string   redId;      // RED player's Id.
    std::string   blackId;    // BLACK player's Id.
	std::string   redScore;   // RED player's Score.
	std::string   blackScore;  // BLACK player's Score.
	hoxTimeInfo   initialTime; // The initial allowed Game-Time.
	hoxTimeInfo   blackTime;
	hoxTimeInfo   redTime;
	hoxGameType   gameType;    // Rated / Unrated / Solo

	hoxTableInfo() 
		: group(hoxGAME_GROUP_PUBLIC)
	    , gameType( hoxGAME_TYPE_UNKNOWN ) {}

	hoxTableInfo(const std::string& a_id) 
		: group(hoxGAME_GROUP_PUBLIC)
	    , gameType( hoxGAME_TYPE_UNKNOWN ) { id = a_id; }

	void Clear()
		{
			id         = "";
			group      = hoxGAME_GROUP_PUBLIC;
			redId      = "";
			blackId    = "";
			redScore   = "";
			blackScore = "";
			initialTime.Clear();
			blackTime.Clear();
			redTime.Clear();
			gameType   = hoxGAME_TYPE_UNKNOWN;
		}

public:
    static void String_To_Table( const std::string& sInput, 
                                 hoxTableInfo&      tableInfo );

};
typedef std::list<hoxTableInfo> hoxTableInfoList;

namespace hoxUtil
{
    /**
     * Convert a given (human-readable) string to a Time-Info of
	 * of the format "nGame/nMove/nFree".
     */
    hoxTimeInfo StringToTimeInfo( const std::string& sInput );

    /**
     * Convert a given Color (Piece's Color or Role)
     * to a (human-readable) string.
     */
    const std::string ColorToString( const hoxColor color );

    /**
     * Convert a given (human-readable) string
     * to a Color (Piece's Color or Role).
     */
    hoxColor StringToColor( const std::string& sInput );

	/**
	 * Convert a given Game-Status to a (human-readable) string.
	 */
	const std::string GameStatusToString( const hoxGameStatus gameStatus );

    /**
     * Convert a given (human-readable) string to a Game-Status.
     */
    hoxGameStatus StringToGameStatus( const std::string& sInput );

} // namespace hoxUtil

#endif /* __INCLUDED_HOX_COMMON_H__ */
