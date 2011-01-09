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

#ifndef __INCLUDED_HOX_UTIL_H__
#define __INCLUDED_HOX_UTIL_H__

/**
 * @file hoxUtil.h
 *
 * The file contains utility functions commonly used through out  the project.
 */

#include "enums.h"
#include "types.h"

namespace hox {
namespace util {

    /**
     * Convert a given (human-readable) string to a Time-Info of
     * of the format "nGame/nMove/nFree".
     */
    TimeInfo stringToTimeInfo( const std::string& sInput );

    /**
     * Convert a given Color (Piece's Color or Role)
     * to a (human-readable) string.
     */
    const std::string colorToString( const ColorEnum color );

    /**
     * Convert a given (human-readable) string
     * to a Color (Piece's Color or Role).
     */
    ColorEnum stringToColor( const std::string& sInput );

    /**
     * Convert a given Game-Status to a (human-readable) string.
     */
    const std::string gameStatusToString( const GameStatusEnum gameStatus );

    /**
     * Convert a given (human-readable) string to a Game-Status.
     */
    GameStatusEnum stringToGameStatus( const std::string& sInput );

    /**
     * Generate a random number of range [1, max_value].
     */
    unsigned generateRandomNumber( const unsigned max_value );

} // namespace util
} // namespace hox

#endif /* __INCLUDED_HOX_UTIL_H__ */
