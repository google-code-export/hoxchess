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
// Name:            hoxUtil.cpp
// Created:         09/28/2007
//
// Description:     Containing various helper API in the project.
/////////////////////////////////////////////////////////////////////////////

#include "hoxUtil.h"
#include "hoxReferee.h"
#include <wx/tokenzr.h>

using namespace hoxUtil;

static wxString gPiecePath = "";

/**
 * Get the full-path of the image of a given piece.
 */
static wxString
_get_piece_image_path( hoxPieceType  type, 
                       hoxColor color )
{
    wxString filename;
    wxChar cColor;

    cColor = ( color == hoxCOLOR_RED ? 'r' : 'b' );

    switch ( type )
    {
    case hoxPIECE_KING:     // King
        filename.Printf("%s/%cking.png", gPiecePath.c_str(), cColor);
        break;

    case hoxPIECE_ADVISOR:  // Advisor
        filename.Printf("%s/%cadvisor.png", gPiecePath.c_str(), cColor);
        break;

    case hoxPIECE_ELEPHANT: // Elephant 
        filename.Printf("%s/%celephant.png", gPiecePath.c_str(), cColor);
        break;
  
    case hoxPIECE_HORSE:  // Horse 
        filename.Printf("%s/%chorse.png", gPiecePath.c_str(), cColor);
        break;
  
    case hoxPIECE_CHARIOT: // Chariot
        filename.Printf("%s/%cchariot.png", gPiecePath.c_str(), cColor);
        break;
  
    case hoxPIECE_CANNON: // Cannon
        filename.Printf("%s/%ccannon.png", gPiecePath.c_str(), cColor);
        break;
  
    case hoxPIECE_PAWN: // Soldier
        filename.Printf("%s/%cpawn.png", gPiecePath.c_str(), cColor);
        break;

    default:
        wxASSERT_MSG(false, wxString::Format(_("Unknown piece-type: %d"), (int) type));
    } // switch

    return filename;
}


//
// hoxUtil
//

void 
hoxUtil::SetPiecesPath(const wxString& piecesPath)
{
    gPiecePath = piecesPath;
}

hoxResult 
hoxUtil::LoadPieceImage( hoxPieceType  type, 
                         hoxColor      color, 
                         wxImage&      image)
{
    wxString filename;

    filename = _get_piece_image_path( type, color );
    if ( ! image.LoadFile(filename, wxBITMAP_TYPE_PNG) ) 
    {
        wxLogError("%s: Failed to load piece-image from path [%s].", __FUNCTION__, filename.c_str());
        return hoxRC_ERR;
    }

    return hoxRC_OK;
}

wxBitmap
hoxUtil::LoadImage( const wxString& imageName )
{
    const wxString filename( wxString(IMAGES_PATH) + "/" + imageName );
    wxImage image;
    if ( ! image.LoadFile(filename, wxBITMAP_TYPE_PNG) ) 
    {
        wxLogError("%s: Failed to load Image from path [%s].",
            __FUNCTION__, filename.c_str());
        return wxBitmap(); // *** Empty bitmap.
    }
    
    return wxBitmap(image);
}

const char*
hoxUtil::ResultToStr( const hoxResult result )
{
    switch( result )
    {
        case hoxRC_UNKNOWN:         return "UNKNOWN";

        case hoxRC_OK:              return "OK";
        case hoxRC_ERR:             return "ERR";
        case hoxRC_HANDLED:         return "HANDLED";
        case hoxRC_CLOSED:          return "CLOSED";
        case hoxRC_NOT_FOUND:       return "NOT_FOUND";
        case hoxRC_NOT_SUPPORTED:   return "NOT_SUPPORTED";

        default:                    return "UNKNOWN";
    }
}

wxString 
hoxUtil::GenerateRandomString( const wxString& sPrefix /* = "SomeString" */ )
{
    static bool s_bSeeded = false;
    if ( ! s_bSeeded )
    {
        ::srand( ::time(NULL) );
        s_bSeeded = true;
    }

    int someNumber = ::rand();

    wxString sRandom = sPrefix;
    sRandom << someNumber;

    return sRandom;
}

int
hoxUtil::GenerateRandomNumber( const unsigned int max_value )
{
    /*
     * CREDITS:
     *    http://www.jb.man.ac.uk/~slowe/cpp/srand.html
     */

    static bool s_bSeeded = false;
    if ( ! s_bSeeded )
    {
        ::srand( ::time(NULL) );
        s_bSeeded = true;
    }

    const int randNum =
        1 + (int) ((double)max_value * (rand() / (RAND_MAX + 1.0)));

    return randNum;
}

/**
 * Convert a given request-type to a (human-readable) string.
 */
const wxString 
hoxUtil::RequestTypeToString( const hoxRequestType requestType )
{
    switch( requestType )
    {
        case hoxREQUEST_UNKNOWN:     return "UNKNOWN";

        case hoxREQUEST_PLAYER_DATA: return "PLAYER_DATA";
        case hoxREQUEST_LOGIN:       return "LOGIN";
        case hoxREQUEST_LOGOUT:      return "LOGOUT";
        case hoxREQUEST_SHUTDOWN:    return "SHUTDOWN";
        case hoxREQUEST_MOVE:        return "MOVE";
        case hoxREQUEST_LIST:        return "LIST";
        case hoxREQUEST_NEW:         return "NEW";
        case hoxREQUEST_JOIN:        return "JOIN";
        case hoxREQUEST_LEAVE:       return "LEAVE";
        case hoxREQUEST_UPDATE:      return "UPDATE";
        case hoxREQUEST_RESIGN:      return "RESIGN";
		case hoxREQUEST_DRAW:        return "DRAW";
        case hoxREQUEST_RESET:       return "RESET";
        case hoxREQUEST_E_JOIN:      return "E_JOIN";
        case hoxREQUEST_E_END:       return "E_END";
        case hoxREQUEST_E_SCORE:     return "E_SCORE";
        case hoxREQUEST_I_TABLE:     return "I_TABLE";
        case hoxREQUEST_I_MOVES:     return "I_MOVES";
        case hoxREQUEST_INVITE:      return "INVITE";
        case hoxREQUEST_PLAYER_INFO:   return "PLAYER_INFO";
		case hoxREQUEST_PLAYER_STATUS: return "PLAYER_STATUS";
        case hoxREQUEST_MSG:         return "MSG";

        default:                     return "UNKNOWN";
    }
}

/**
 * Convert a given (human-readable) string to a request-type.
 */
hoxRequestType
hoxUtil::StringToRequestType( const wxString& input )
{
    if ( input == "UNKNOWN" )     return hoxREQUEST_UNKNOWN;

    if ( input == "PLAYER_DATA" ) return hoxREQUEST_PLAYER_DATA;
    if ( input == "LOGIN" )       return hoxREQUEST_LOGIN;
    if ( input == "LOGOUT" )      return hoxREQUEST_LOGOUT;
    if ( input == "SHUTDOWN" )    return hoxREQUEST_SHUTDOWN;
    if ( input == "MOVE" )        return hoxREQUEST_MOVE;
    if ( input == "LIST" )        return hoxREQUEST_LIST;
    if ( input == "NEW" )         return hoxREQUEST_NEW;
    if ( input == "JOIN" )        return hoxREQUEST_JOIN;
    if ( input == "LEAVE" )       return hoxREQUEST_LEAVE;
    if ( input == "UPDATE" )      return hoxREQUEST_UPDATE;
    if ( input == "RESIGN" )      return hoxREQUEST_RESIGN;
	if ( input == "DRAW" )        return hoxREQUEST_DRAW;
    if ( input == "RESET" )       return hoxREQUEST_RESET;
    if ( input == "E_JOIN" )      return hoxREQUEST_E_JOIN;
    if ( input == "E_END" )       return hoxREQUEST_E_END;
    if ( input == "E_SCORE" )     return hoxREQUEST_E_SCORE;
    if ( input == "I_TABLE" )     return hoxREQUEST_I_TABLE;
    if ( input == "I_MOVES" )     return hoxREQUEST_I_MOVES;
    if ( input == "INVITE" )      return hoxREQUEST_INVITE;
    if ( input == "PLAYER_INFO" )   return hoxREQUEST_PLAYER_INFO;
	if ( input == "PLAYER_STATUS" ) return hoxREQUEST_PLAYER_STATUS;
    if ( input == "MSG" )         return hoxREQUEST_MSG;

    return hoxREQUEST_UNKNOWN;
}

/**
 * Convert a given game-type to a (human-readable) string.
 */
const wxString 
hoxUtil::GameTypeToString( const hoxGameType gameType )
{
    switch( gameType )
    {
        case hoxGAME_TYPE_UNKNOWN:     return "UNKNOWN";

        case hoxGAME_TYPE_RATED:       return "Rated";
		case hoxGAME_TYPE_NONRATED:    return "Nonrated";
		case hoxGAME_TYPE_SOLO:        return "Solo";
        case hoxGAME_TYPE_PRACTICE:    return "Practice";

        default:                       return "UNKNOWN";
    }
}

/**
 * Convert a given Color (Piece's Color or Role) to a (human-readable) string.
 */
const wxString 
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

/**
 * Convert a given (human-readable) string to a Color (Piece's Color or Role).
 */
hoxColor 
hoxUtil::StringToColor( const wxString& input )
{
    if ( input == "UNKNOWN" ) return hoxCOLOR_UNKNOWN;

	if ( input == "Red" )     return hoxCOLOR_RED;
	if ( input == "Black" )   return hoxCOLOR_BLACK;
	if ( input == "None" )    return hoxCOLOR_NONE;

	return hoxCOLOR_UNKNOWN;
}

const wxString
hoxUtil::TypeToString( const hoxPieceType type )
{
    switch( type )
    {
		case hoxCOLOR_UNKNOWN:   return "UNKNOWN";

		case hoxPIECE_KING:		return "KING";             // King (or General)
		case hoxPIECE_ADVISOR:	return "ADVISOR";          // Advisor (or Guard, or Mandarin)
		case hoxPIECE_ELEPHANT:	return "ELEPHANT";         // Elephant (or Ministers)
		case hoxPIECE_CHARIOT:	return "CHARIOT";          // Chariot ( Rook, or Car)
		case hoxPIECE_HORSE:	return "HORSE";            // Horse ( Knight )
		case hoxPIECE_CANNON:	return "CANNON";           // Canon
		case hoxPIECE_PAWN:		return "PAWN";             // Pawn (or Soldier)

        default:                 return "INVALID PIECE";
    }
}

/**
 * Convert a given (human-readable) string to a Color (Piece's Color or Role).
 */
hoxPieceType
hoxUtil::StringToType( const wxString type )
{
    switch( type[0].GetValue() )
    {
	case 'K': return hoxPIECE_KING;             // King (or General)
	case 'A': return hoxPIECE_ADVISOR;          // Advisor (or Guard, or Mandarin)
	case 'E': return hoxPIECE_ELEPHANT;         // Elephant (or Ministers)
	case 'C': return (type[1] == 'H'? hoxPIECE_CHARIOT:hoxPIECE_CANNON);          // Chariot ( Rook, or Car) or Cannon
	case 'H': return hoxPIECE_HORSE;            // Horse ( Knight )
	case 'P': return hoxPIECE_PAWN;             // Pawn (or Soldier)

    default:  return hoxPIECE_INVALID;
    }
}
/**
 * Parse a given string of the format "hostname:port" into a host-name
 * and a port.
 *
 * @return true if everything is fine. Otherwise, return false.
 */
bool 
hoxUtil::ParseServerAddress( const wxString&   input,
                                hoxServerAddress& serverAddress )
{
    const char SEPARATOR = ':';

    serverAddress.name = input.BeforeFirst( SEPARATOR );
    if ( serverAddress.name.empty() )
        return false;

    wxString portStr = input.AfterFirst( SEPARATOR );
    if ( !portStr.empty() )
    {
        long longVal;
        if ( !portStr.ToLong( &longVal ) )
            return false;

        serverAddress.port = (int) longVal;
    }
    
    return true;
}

const wxString 
hoxUtil::FormatTime( int nTime )
{
    return wxString::Format( "%d:%.02d", nTime / 60, nTime % 60 );
}

hoxTimeInfo 
hoxUtil::StringToTimeInfo( const wxString& input )
{
	hoxTimeInfo timeInfo;

	wxStringTokenizer tkz( input, "/" );
    int i = 0;

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        switch (i++)
        {
			case 0:   // Game-Time
				timeInfo.nGame = ::atoi(token.c_str());
				break;

			case 1:   // Move-Time
				timeInfo.nMove = ::atoi(token.c_str());
				break;

			case 2:   // Free-Time
				timeInfo.nFree = ::atoi(token.c_str());
				break;

			default:
				// Ignore the rest.
				break;
		}
	}

	return timeInfo;
}

const wxString 
hoxUtil::TimeInfoToString( const hoxTimeInfo timeInfo )
{
	wxString result = 
		wxString::Format("%d/%d/%d", timeInfo.nGame, 
		                             timeInfo.nMove, 
		                             timeInfo.nFree);
	return result;
}

const wxString
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
hoxUtil::StringToGameStatus( const wxString& input )
{
    if ( input == "UNKNOWN" )     return hoxGAME_STATUS_UNKNOWN;

    if ( input == "open" )        return hoxGAME_STATUS_OPEN;
    if ( input == "ready" )       return hoxGAME_STATUS_READY;
    if ( input == "in_progress" ) return hoxGAME_STATUS_IN_PROGRESS;
    if ( input == "red_win" )     return hoxGAME_STATUS_RED_WIN;
    if ( input == "black_win" )   return hoxGAME_STATUS_BLACK_WIN;
    if ( input == "drawn" )       return hoxGAME_STATUS_DRAWN;

    return hoxGAME_STATUS_UNKNOWN;
}

std::string
hoxUtil::wx2std( const wxString& input )
{
    // --- wxString ==> std::string
    return std::string( input.mb_str() );
}

wxString
hoxUtil::std2wx( const std::string& input )
{
    // --- std::string ==> wxString
    return wxString( input.c_str(), wxConvUTF8 );
}

void
hoxUtil::hoxPcsPos2XQWLight( const hoxIReferee_SPtr& pReferee,
                             unsigned char           pcsPos[10][9] )
{
    /* Get the current positions. */

    hoxGameState  gameState;
    pReferee->GetGameState( gameState );

    /* Convert to array-type positions. */

	int x, y, color, type;

    for ( hoxPieceInfoList::const_iterator it = gameState.pieceList.begin();
                                           it != gameState.pieceList.end(); ++it )
    {
		x = (*it).position.x;
		y = (*it).position.y;
		
        switch ( (*it).color )
        {
			case hoxCOLOR_RED:    color = 0x8; break;
			case hoxCOLOR_BLACK:  color = 0x10; break;
			default:              color = -1;
		};

		switch ( (*it).type )
        {
			case hoxPIECE_KING:     type = 0; break;
			case hoxPIECE_ADVISOR:  type = 1; break;
			case hoxPIECE_ELEPHANT: type = 2; break;
			case hoxPIECE_HORSE:    type = 3; break;
			case hoxPIECE_CHARIOT:  type = 4; break;
			case hoxPIECE_CANNON:   type = 5; break;
			case hoxPIECE_PAWN:     type = 6; break;
			default:                type = -1;
		};

		if ( color >= 0 && type >= 0 )
        {
			pcsPos[y][x] = color + type;
        }
	}
}

/************************* END OF FILE ***************************************/
