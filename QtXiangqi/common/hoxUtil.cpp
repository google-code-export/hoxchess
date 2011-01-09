/***************************************************************************
 *  Copyright 2010-2011 Huy Phan  <huyphan@playxiangqi.com>                *
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

#include "hoxUtil.h"

namespace hox {
namespace util {

TimeInfo
stringToTimeInfo( const std::string& sInput )
{
    TimeInfo timeInfo;

    Separator sep("/");
    Tokenizer tok(sInput, sep);

    int i = 0;
    for ( Tokenizer::iterator it = tok.begin(); it != tok.end(); ++it )
    {
        const std::string token = (*it);
        switch (i++)
        {
            case 0: timeInfo.nGame = ::atoi(token.c_str()); break;
            case 1: timeInfo.nMove = ::atoi(token.c_str()); break;
            case 2: timeInfo.nFree = ::atoi(token.c_str()); break;

            default: break; // Ignore the rest.
        }
    }

    return timeInfo;
}

const std::string 
colorToString( const ColorEnum color )
{
    switch( color )
    {
        case HC_COLOR_UNKNOWN:   return "UNKNOWN";

        case HC_COLOR_RED:       return "Red";
        case HC_COLOR_BLACK:     return "Black";
        case HC_COLOR_NONE:      return "None";

        default:                 return "UNKNOWN";
    }
}

ColorEnum
stringToColor( const std::string& sInput )
{
    if ( sInput == "UNKNOWN" ) return HC_COLOR_UNKNOWN;

    if ( sInput == "Red" )     return HC_COLOR_RED;
    if ( sInput == "Black" )   return HC_COLOR_BLACK;
    if ( sInput == "None" )    return HC_COLOR_NONE;

    return HC_COLOR_UNKNOWN;
}

const std::string
gameStatusToString( const GameStatusEnum gameStatus )
{
    switch( gameStatus )
    {
        case HC_GAME_STATUS_UNKNOWN:     return "UNKNOWN";

        case HC_GAME_STATUS_OPEN:        return "open";
        case HC_GAME_STATUS_READY:       return "ready";
        case HC_GAME_STATUS_IN_PROGRESS: return "in_progress";
        case HC_GAME_STATUS_RED_WIN:     return "red_win";
        case HC_GAME_STATUS_BLACK_WIN:   return "black_win";
        case HC_GAME_STATUS_DRAWN:       return "drawn";

        default:                         return "UNKNOWN";
    }
}

GameStatusEnum
stringToGameStatus( const std::string& sInput )
{
    if ( sInput == "UNKNOWN" )     return HC_GAME_STATUS_UNKNOWN;

    if ( sInput == "open" )        return HC_GAME_STATUS_OPEN;
    if ( sInput == "ready" )       return HC_GAME_STATUS_READY;
    if ( sInput == "in_progress" ) return HC_GAME_STATUS_IN_PROGRESS;
    if ( sInput == "red_win" )     return HC_GAME_STATUS_RED_WIN;
    if ( sInput == "black_win" )   return HC_GAME_STATUS_BLACK_WIN;
    if ( sInput == "drawn" )       return HC_GAME_STATUS_DRAWN;

    return HC_GAME_STATUS_UNKNOWN;
}

unsigned
generateRandomNumber( const unsigned max_value )
{
    /*
     * CREDITS:
     *    http://www.jb.man.ac.uk/~slowe/cpp/srand.html
     */

    static bool s_bSeeded = false;  // *** Only generate seed once per process.
    if ( ! s_bSeeded )
    {
        srand( time(NULL) + getpid() );
        s_bSeeded = true;
    }

    const unsigned randNum =
        1 + (unsigned) ((double)max_value * (rand() / (RAND_MAX + 1.0)));

    return randNum;
}

} // namespace util
} // namespace hox

/************************* END OF FILE ***************************************/
