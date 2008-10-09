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
// Name:            hoxCommon.cpp
// Created:         10/05/2008
//
// Description:     Containing basic Constants and Types commonly used
//                  through out  the project.
/////////////////////////////////////////////////////////////////////////////

#include "hoxCommon.h"


/* static */ void
hoxTableInfo::String_To_Table( const std::string& sInput, 
                               hoxTableInfo&      tableInfo )
{
	tableInfo.Clear();

    hoxSeparator sep(";", 0,
                     boost::keep_empty_tokens);
    hoxTokenizer tok(sInput, sep);

    int i = 0;
    for ( hoxTokenizer::iterator it = tok.begin(); it != tok.end(); ++it )
    {
        const std::string token = (*it);
        switch (i++)
        {
            case 0: /* Id */ tableInfo.id = token; break;

            case 1: /* Group */
				tableInfo.group = ( token == "0" ? hoxGAME_GROUP_PUBLIC 
								                 : hoxGAME_GROUP_PRIVATE );
                break;

            case 2: /* Type */
				tableInfo.gameType = ( token == "0" ? hoxGAME_TYPE_RATED 
								                    : hoxGAME_TYPE_NONRATED );
                break;

            case 3: /* Initial-Time */
				tableInfo.initialTime = hoxUtil::StringToTimeInfo( token );
                break;

            case 4: /* RED-Time */
				tableInfo.redTime = hoxUtil::StringToTimeInfo( token );
                break;

            case 5: /* BLACK-Time */
				tableInfo.blackTime = hoxUtil::StringToTimeInfo( token );
                break;

            case 6: /* RED-Id */      tableInfo.redId      = token; break;
            case 7: /* RED-Score */   tableInfo.redScore   = token; break;
            case 8: /* BLACK-Id */    tableInfo.blackId    = token; break;
            case 9: /* BLACK-Score */ tableInfo.blackScore = token; break;

			default: break; // Ignore the rest
        }
    }
}

hoxTimeInfo 
hoxUtil::StringToTimeInfo( const std::string& sInput )
{
	hoxTimeInfo timeInfo;

    hoxSeparator sep("/");
    hoxTokenizer tok(sInput, sep);

    int i = 0;
    for ( hoxTokenizer::iterator it = tok.begin(); it != tok.end(); ++it )
    {
        const std::string token = (*it);
        switch (i++)
        {
			case 0: /* Game-Time */ timeInfo.nGame = ::atoi(token.c_str()); break;
			case 1: /* Move-Time */ timeInfo.nMove = ::atoi(token.c_str()); break;
			case 2: /* Free-Time */ timeInfo.nFree = ::atoi(token.c_str());	break;

			default: break; // Ignore the rest.
		}
	}

	return timeInfo;
}

const std::string 
hoxUtil::ColorToString( const hoxColor color )
{
    switch( color )
    {
		case hoxCOLOR_UNKNOWN:   return "UNKNOWN";

        case hoxCOLOR_RED:       return "Red";
		case hoxCOLOR_BLACK:     return "Black";
		case hoxCOLOR_NONE:      return "None";

        default:                 return "UNKNOWN";
    }
}

hoxColor 
hoxUtil::StringToColor( const std::string& sInput )
{
    if ( sInput == "UNKNOWN" ) return hoxCOLOR_UNKNOWN;

	if ( sInput == "Red" )     return hoxCOLOR_RED;
	if ( sInput == "Black" )   return hoxCOLOR_BLACK;
	if ( sInput == "None" )    return hoxCOLOR_NONE;

	return hoxCOLOR_UNKNOWN;
}

const std::string
hoxUtil::GameStatusToString( const hoxGameStatus gameStatus )
{
    switch( gameStatus )
    {
        case hoxGAME_STATUS_UNKNOWN:     return "UNKNOWN";

		case hoxGAME_STATUS_OPEN:        return "open";
		case hoxGAME_STATUS_READY:       return "ready";
		case hoxGAME_STATUS_IN_PROGRESS: return "in_progress";
		case hoxGAME_STATUS_RED_WIN:     return "red_win";
		case hoxGAME_STATUS_BLACK_WIN:   return "black_win";
		case hoxGAME_STATUS_DRAWN:       return "drawn";

		default:                         return "UNKNOWN";
    }
}

hoxGameStatus
hoxUtil::StringToGameStatus( const std::string& sInput )
{
    if ( sInput == "UNKNOWN" )     return hoxGAME_STATUS_UNKNOWN;

    if ( sInput == "open" )        return hoxGAME_STATUS_OPEN;
    if ( sInput == "ready" )       return hoxGAME_STATUS_READY;
    if ( sInput == "in_progress" ) return hoxGAME_STATUS_IN_PROGRESS;
    if ( sInput == "red_win" )     return hoxGAME_STATUS_RED_WIN;
    if ( sInput == "black_win" )   return hoxGAME_STATUS_BLACK_WIN;
    if ( sInput == "drawn" )       return hoxGAME_STATUS_DRAWN;

    return hoxGAME_STATUS_UNKNOWN;
}

/************************* END OF FILE ***************************************/
