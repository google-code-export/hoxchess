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
// Name:            hoxUtility.cpp
// Created:         09/28/2007
//
// Description:     Containing various helper API in the project.
/////////////////////////////////////////////////////////////////////////////

#include "hoxUtility.h"
#include <wx/tokenzr.h>

using namespace hoxUtility;

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
    case hoxPIECE_TYPE_KING:     // King
        filename.sprintf("%s/%cking.png", gPiecePath.c_str(), cColor);
        break;

    case hoxPIECE_TYPE_ADVISOR:  // Advisor
        filename.sprintf("%s/%cadvisor.png", gPiecePath.c_str(), cColor);
        break;

    case hoxPIECE_TYPE_ELEPHANT: // Elephant 
        filename.sprintf("%s/%celephant.png", gPiecePath.c_str(), cColor);
        break;
  
    case hoxPIECE_TYPE_HORSE:  // Horse 
        filename.sprintf("%s/%chorse.png", gPiecePath.c_str(), cColor);
        break;
  
    case hoxPIECE_TYPE_CHARIOT: // Chariot
        filename.sprintf("%s/%cchariot.png", gPiecePath.c_str(), cColor);
        break;
  
    case hoxPIECE_TYPE_CANNON: // Cannon
        filename.sprintf("%s/%ccannon.png", gPiecePath.c_str(), cColor);
        break;
  
    case hoxPIECE_TYPE_PAWN: // Soldier
        filename.sprintf("%s/%cpawn.png", gPiecePath.c_str(), cColor);
        break;

    default:
        wxASSERT_MSG(false, wxString::Format(_("Unknown piece-type: %d"), (int) type));
    } // switch

    return filename;
}


//
// hoxUtility
//

void 
hoxUtility::SetPiecesPath(const wxString& piecesPath)
{
    gPiecePath = piecesPath;
}

hoxResult 
hoxUtility::LoadPieceImage( hoxPieceType  type, 
                            hoxColor color, 
                            wxImage&      image)
{
    const char* FNAME = "hoxUtility::LoadPieceImage";
    wxString filename;

    filename = _get_piece_image_path( type, color );
    if ( ! image.LoadFile(filename, wxBITMAP_TYPE_PNG) ) 
    {
        wxLogError("%s: Failed to load piece-image from path [%s].", FNAME, filename.c_str());
        return hoxRESULT_ERR;
    }

    return hoxRESULT_OK;
}

wxString 
hoxUtility::GenerateRandomString()
{
    ::srand( ::time(NULL) );
    int someNumber = ::rand();

    return wxString("SomeString") << someNumber;
}

/**
 * Convert a given request-type to a (human-readable) string.
 */
const wxString 
hoxUtility::RequestTypeToString( const hoxRequestType requestType )
{
    switch( requestType )
    {
        case hoxREQUEST_UNKNOWN:     return "UNKNOWN";

        case hoxREQUEST_ACCEPT:      return "ACCEPT";
        case hoxREQUEST_PLAYER_DATA: return "PLAYER_DATA";
        case hoxREQUEST_LOGIN:       return "LOGIN";
        case hoxREQUEST_LOGOUT:      return "LOGOUT";
        case hoxREQUEST_SHUTDOWN:    return "SHUTDOWN";
        case hoxREQUEST_POLL:        return "POLL";
        case hoxREQUEST_MOVE:        return "MOVE";
        case hoxREQUEST_LIST:        return "LIST";
        case hoxREQUEST_NEW:         return "NEW";
        case hoxREQUEST_JOIN:        return "JOIN";
        case hoxREQUEST_LEAVE:       return "LEAVE";
		case hoxREQUEST_DRAW:        return "DRAW";
        case hoxREQUEST_E_JOIN:      return "E_JOIN";
		case hoxREQUEST_PLAYER_STATUS: return "PLAYER_STATUS";
        case hoxREQUEST_OUT_DATA:    return "OUT_DATA";
        case hoxREQUEST_MSG:         return "MSG";

        default:                          return "UNKNOWN";
    }
}

/**
 * Convert a given (human-readable) string to a request-type.
 */
hoxRequestType
hoxUtility::StringToRequestType( const wxString& input )
{
    if ( input == "UNKNOWN" )     return hoxREQUEST_UNKNOWN;

    if ( input == "ACCEPT" )      return hoxREQUEST_ACCEPT;
    if ( input == "PLAYER_DATA" ) return hoxREQUEST_PLAYER_DATA;
    if ( input == "LOGIN" )       return hoxREQUEST_LOGIN;
    if ( input == "LOGOUT" )      return hoxREQUEST_LOGOUT;
    if ( input == "SHUTDOWN" )    return hoxREQUEST_SHUTDOWN;
    if ( input == "POLL" )        return hoxREQUEST_POLL;
    if ( input == "MOVE" )        return hoxREQUEST_MOVE;
    if ( input == "LIST" )        return hoxREQUEST_LIST;
    if ( input == "NEW" )         return hoxREQUEST_NEW;
    if ( input == "JOIN" )        return hoxREQUEST_JOIN;
    if ( input == "LEAVE" )       return hoxREQUEST_LEAVE;
	if ( input == "DRAW" )        return hoxREQUEST_DRAW;
    if ( input == "E_JOIN" )      return hoxREQUEST_E_JOIN;
	if ( input == "PLAYER_STATUS" ) return hoxREQUEST_PLAYER_STATUS;
    if ( input == "OUT_DATA" )    return hoxREQUEST_OUT_DATA;
    if ( input == "MSG" )         return hoxREQUEST_MSG;

    return hoxREQUEST_UNKNOWN;
}

/**
 * Convert a given game-type to a (human-readable) string.
 */
const wxString 
hoxUtility::GameTypeToString( const hoxGameType gameType )
{
    switch( gameType )
    {
        case hoxGAME_TYPE_UNKNOWN:     return "UNKNOWN";

        case hoxGAME_TYPE_RATED:       return "Rated";
		case hoxGAME_TYPE_NONRATED:    return "Nonrated";
		case hoxGAME_TYPE_SOLO:        return "Solo";

        default:                       return "UNKNOWN";
    }
}

/**
 * Convert a given Color (Piece's Color or Role) to a (human-readable) string.
 */
const wxString 
hoxUtility::ColorToString( const hoxColor color )
{
    switch( color )
    {
		case hoxCOLOR_UNKNOWN:   return "UNKNOWN";

        case hoxCOLOR_RED:       return "Red";
		case hoxCOLOR_BLACK:     return "Black";
		case hoxCOLOR_NONE:      return "None";

        default:                       return "UNKNOWN";
    }
}

/**
 * Convert a given (human-readable) string to a Color (Piece's Color or Role).
 */
hoxColor 
hoxUtility::StringToColor( const wxString& input )
{
    if ( input == "UNKNOWN" ) return hoxCOLOR_UNKNOWN;

	if ( input == "Red" )     return hoxCOLOR_RED;
	if ( input == "Black" )   return hoxCOLOR_BLACK;
	if ( input == "None" )    return hoxCOLOR_NONE;

	return hoxCOLOR_UNKNOWN;
}

// ----------------------------------------------------------------------------
// hoxURI
// ----------------------------------------------------------------------------

/* static */
wxString 
hoxURI::Escape_String(const wxString& str)
{
    /*
     * @see Uniform Resource Identifier (URI): Generic Syntax
     *          http://www.ietf.org/rfc/rfc3986.txt
     */

    wxString new_uri;
    char     c;

    for (size_t i = 0; i < str.length(); ++i)
    {
        c = str[i];
        if ( ::isspace( c ) )
            wxURI::Escape( new_uri, c );
        else
            new_uri += c;
    }

    return new_uri;
}

/**
 * Parse a given string of the format "hostname:port" into a host-name
 * and a port.
 *
 * @return true if everything is fine. Otherwise, return false.
 */
bool 
hoxUtility::ParseServerAddress( const wxString&   input,
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
hoxUtility::FormatTime( int nTime )
{
    return wxString::Format( "%d:%.02d", nTime / 60, nTime % 60 );
}

hoxTimeInfo 
hoxUtility::StringToTimeInfo( const wxString& input )
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
hoxUtility::TimeInfoToString( const hoxTimeInfo timeInfo )
{
	wxString result = 
		wxString::Format("%d/%d/%d", timeInfo.nGame, 
		                             timeInfo.nMove, 
		                             timeInfo.nFree);
	return result;
}

/************************* END OF FILE ***************************************/
