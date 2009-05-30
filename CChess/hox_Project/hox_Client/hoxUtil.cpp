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
#include <wx/textfile.h>

#ifdef __WXMAC__
    #include <wx/stdpaths.h>
#endif

using namespace hoxUtil;

/**
 * Get the full-path of the image of a given piece.
 */
static wxString
_piece_type_to_image_name( hoxPieceType type )
{
    switch ( type )
    {
        case hoxPIECE_KING:     return "king.png";
        case hoxPIECE_ADVISOR:  return "advisor.png";
        case hoxPIECE_ELEPHANT: return "elephant.png";
        case hoxPIECE_HORSE:    return "horse.png";
        case hoxPIECE_CHARIOT:  return "chariot.png";
        case hoxPIECE_CANNON:   return "cannon.png";
        case hoxPIECE_PAWN:     return "pawn.png";
        default:
            wxFAIL_MSG(wxString::Format(_("Unknown piece-type: %d"), (int) type));
    }
    return "";
}

// -----------------------------------------------------------------------
// hoxUtil
// -----------------------------------------------------------------------

wxString
hoxUtil::GetPath( const hoxResourceType rType )
{
    /* Copied from FileZilla source */
    
	/*
	 * Finding the resources in all cases is a difficult task,
	 * due to the huge variety of diffent systems and their filesystem
	 * structure.
	 * Basically we just check a couple of paths for presence of the resources,
	 * and hope we find them. If not, the user can still specify on the cmdline
	 * and using environment variables where the resources are.
	 *
	 * At least on OS X it's simple: All inside application bundle.
	 */
    
    wxString base;
    wxString prefix;
#ifdef __WXMAC__
	base = wxStandardPaths::Get().GetDataDir();
    wxLogDebug("%s: wxStandardPaths::Get().GetDataDir() = [%s].", __FUNCTION__, base.c_str());
    prefix = "";
#else
    base = "..";
    prefix = "/resource";
#endif

    switch ( rType )
    {
        case hoxRT_IMAGE:     return base + prefix + "/images/";
        case hoxRT_SOUND:     return base + prefix + "/sounds/";
        case hoxRT_PIECE:     return base + prefix + "/pieces/";
        case hoxRT_BOARD:     return base + prefix + "/boards/";
        case hoxRT_LOCALE:    return base + prefix + "/locale/";
        case hoxRT_AI_PLUGIN: return base + "/plugins/";

        default /* hoxRT_UNKNOWN */: return "__UNKNOWN__";
    }
}

hoxResult 
hoxUtil::LoadPieceImage( const wxString& sPath,
                         hoxPieceType    type, 
                         hoxColor        color, 
                         wxImage&        image)
{
    const wxString sPrefixPath = hoxUtil::GetPath(hoxRT_PIECE) + "/" + sPath;
    const wxChar   cColor      = (color == hoxCOLOR_RED ? 'r' : 'b');
    const wxString imageName   = _piece_type_to_image_name( type );

    const wxString sFullPath =
        wxString::Format("%s/%c%s", sPrefixPath.c_str(), cColor, imageName.c_str());

    if ( ! image.LoadFile(sFullPath, wxBITMAP_TYPE_PNG) ) 
    {
        wxLogError("%s: Failed to load piece from [%s].", __FUNCTION__, sFullPath.c_str());
        return hoxRC_ERR;
    }
    return hoxRC_OK;
}

wxBitmap
hoxUtil::LoadImage( const wxString& imageName )
{
    const wxString filename( GetPath(hoxRT_IMAGE) + imageName );
    wxImage image;
    if ( ! image.LoadFile(filename, wxBITMAP_TYPE_PNG) ) 
    {
        wxLogError("%s: Failed to load Image from path [%s].",
            __FUNCTION__, filename.c_str());
        return wxBitmap(); // *** Empty bitmap.
    }
    
    return wxBitmap(image);
}

bool
hoxUtil::LoadBoardImage( const wxString&    sImage,
                         wxImage&           image,
                         hoxBoardImageInfo& imageInfo )
{
    const wxString sPrefixPath = hoxUtil::GetPath(hoxRT_BOARD);

    const wxString sIniFile = sPrefixPath + sImage.BeforeFirst('.') + ".ini";
    hoxUtil::LoadBoardInfo( sIniFile, imageInfo );

    const wxString imageFile = sPrefixPath + sImage;
    if ( ! image.LoadFile(imageFile, wxBITMAP_TYPE_PNG) ) 
    {
        wxLogWarning("%s: Failed to load board-image from [%s].", __FUNCTION__, imageFile.c_str());
        return false;
    }

    return true;
}

bool
hoxUtil::LoadBoardInfo( const wxString&    sIniFile,
                        hoxBoardImageInfo& imageInfo )
{
    wxTextFile file( sIniFile );
    if ( !file.Exists() || !file.Open() )
    {
        wxLogDebug("%s: Board INI file [%s] not found.", __FUNCTION__, sIniFile.c_str());
        return false;
    }

    wxLogDebug("%s: Loadding Board INI file [%s].", __FUNCTION__, sIniFile.c_str());
    wxString sKey;
    int      nValue = 0;
    for ( wxString sLine = file.GetFirstLine(); !file.Eof();
                   sLine = file.GetNextLine() )
    {
        if ( sLine.StartsWith("#") ) continue;  // Skip comments.
        sKey = sLine.BeforeFirst('=').Trim(/* fromRight */);
        nValue = ::atoi( sLine.AfterFirst('=').c_str() );
        if      ( sKey == "borderX" ) imageInfo.borderX = nValue;
        else if ( sKey == "borderY" ) imageInfo.borderY = nValue;
        else if ( sKey == "cellS" )   imageInfo.cellS   = nValue;
    }

    return true;
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

const wxString 
hoxUtil::RequestTypeToString( const hoxRequestType requestType )
{
    switch( requestType )
    {
        case hoxREQUEST_UNKNOWN:       return "UNKNOWN";

        case hoxREQUEST_PLAYER_DATA:   return "PLAYER_DATA";
        case hoxREQUEST_LOGIN:         return "LOGIN";
        case hoxREQUEST_LOGOUT:        return "LOGOUT";
        case hoxREQUEST_SHUTDOWN:      return "SHUTDOWN";
        case hoxREQUEST_MOVE:          return "MOVE";
        case hoxREQUEST_LIST:          return "LIST";
        case hoxREQUEST_NEW:           return "NEW";
        case hoxREQUEST_JOIN:          return "JOIN";
        case hoxREQUEST_LEAVE:         return "LEAVE";
        case hoxREQUEST_UPDATE:        return "UPDATE";
        case hoxREQUEST_RESIGN:        return "RESIGN";
		case hoxREQUEST_DRAW:          return "DRAW";
        case hoxREQUEST_RESET:         return "RESET";
        case hoxREQUEST_E_JOIN:        return "E_JOIN";
        case hoxREQUEST_E_END:         return "E_END";
        case hoxREQUEST_E_SCORE:       return "E_SCORE";
        case hoxREQUEST_I_PLAYERS:     return "I_PLAYERS";
        case hoxREQUEST_I_TABLE:       return "I_TABLE";
        case hoxREQUEST_I_MOVES:       return "I_MOVES";
        case hoxREQUEST_INVITE:        return "INVITE";
        case hoxREQUEST_PLAYER_INFO:   return "PLAYER_INFO";
		case hoxREQUEST_PLAYER_STATUS: return "PLAYER_STATUS";
        case hoxREQUEST_MSG:           return "MSG";

        default:                       return "UNKNOWN";
    }
}

hoxRequestType
hoxUtil::StringToRequestType( const wxString& input )
{
    if ( input == "UNKNOWN" )       return hoxREQUEST_UNKNOWN;

    if ( input == "PLAYER_DATA" )   return hoxREQUEST_PLAYER_DATA;
    if ( input == "LOGIN" )         return hoxREQUEST_LOGIN;
    if ( input == "LOGOUT" )        return hoxREQUEST_LOGOUT;
    if ( input == "SHUTDOWN" )      return hoxREQUEST_SHUTDOWN;
    if ( input == "MOVE" )          return hoxREQUEST_MOVE;
    if ( input == "LIST" )          return hoxREQUEST_LIST;
    if ( input == "NEW" )           return hoxREQUEST_NEW;
    if ( input == "JOIN" )          return hoxREQUEST_JOIN;
    if ( input == "LEAVE" )         return hoxREQUEST_LEAVE;
    if ( input == "UPDATE" )        return hoxREQUEST_UPDATE;
    if ( input == "RESIGN" )        return hoxREQUEST_RESIGN;
	if ( input == "DRAW" )          return hoxREQUEST_DRAW;
    if ( input == "RESET" )         return hoxREQUEST_RESET;
    if ( input == "E_JOIN" )        return hoxREQUEST_E_JOIN;
    if ( input == "E_END" )         return hoxREQUEST_E_END;
    if ( input == "E_SCORE" )       return hoxREQUEST_E_SCORE;
    if ( input == "I_PLAYERS" )     return hoxREQUEST_I_PLAYERS;
    if ( input == "I_TABLE" )       return hoxREQUEST_I_TABLE;
    if ( input == "I_MOVES" )       return hoxREQUEST_I_MOVES;
    if ( input == "INVITE" )        return hoxREQUEST_INVITE;
    if ( input == "PLAYER_INFO" )   return hoxREQUEST_PLAYER_INFO;
	if ( input == "PLAYER_STATUS" ) return hoxREQUEST_PLAYER_STATUS;
    if ( input == "MSG" )           return hoxREQUEST_MSG;

    return hoxREQUEST_UNKNOWN;
}

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
hoxUtil::PieceToString( const hoxPieceType type )
{
    switch( type )
    {
		case hoxCOLOR_UNKNOWN:  return "UNKNOWN";

		case hoxPIECE_KING:		return "KING";     // King (or General)
		case hoxPIECE_ADVISOR:	return "ADVISOR";  // Advisor (or Guard, or Mandarin)
		case hoxPIECE_ELEPHANT:	return "ELEPHANT"; // Elephant (or Minister)
		case hoxPIECE_CHARIOT:	return "ROOT";     // Chariot ( Rook, or Car)
		case hoxPIECE_HORSE:	return "HORSE";    // Horse ( Knight )
		case hoxPIECE_CANNON:	return "CANNON";   // Canon
		case hoxPIECE_PAWN:		return "PAWN";     // Pawn (or Soldier)

        default:                return "UNKNOWN";
    }
}

hoxPieceType
hoxUtil::StringToPiece( const wxString& type )
{
    switch( type[0].GetValue() )
    {
	    case 'K': return hoxPIECE_KING;     // King (or General)
	    case 'A': return hoxPIECE_ADVISOR;  // Advisor (or Guard, or Mandarin)
	    case 'E': return hoxPIECE_ELEPHANT; // Elephant (or Minister)
	    case 'R': return hoxPIECE_CHARIOT;  // Chariot ( Rook, or Car)
	    case 'C': return hoxPIECE_CANNON;   // Cannon
	    case 'H': return hoxPIECE_HORSE;    // Horse ( Knight )
	    case 'P': return hoxPIECE_PAWN;     // Pawn (or Soldier)

        default:  return hoxPIECE_INVALID;
    }
}

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
        const wxString token = tkz.GetNextToken();
        switch ( i++ )
        {
			case 0: timeInfo.nGame = ::atoi(token.c_str()); break;
			case 1: timeInfo.nMove = ::atoi(token.c_str()); break;
			case 2: timeInfo.nFree = ::atoi(token.c_str()); break;
			default: /* Ignore the rest. */ break;
		}
	}

	return timeInfo;
}

const wxString 
hoxUtil::TimeInfoToString( const hoxTimeInfo timeInfo )
{
	return wxString::Format("%d/%d/%d", timeInfo.nGame, 
		                                timeInfo.nMove, 
		                                timeInfo.nFree);
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

const std::string
hoxUtil::hoxGameStateToFEN( const hoxGameState& gameState )
{
    // STEP 1: Convert the piece-positions to 90-cell board first.

    std::pair<hoxColor, hoxPieceType> board[90];
    for ( int i = 0; i < 90; ++i )
    {
        board[i].first = hoxCOLOR_UNKNOWN;
        board[i].second = hoxPIECE_INVALID;
    }

    const hoxPieceInfoList& pieces = gameState.pieceList;
    int index = 0;  // cell-index.
    for ( hoxPieceInfoList::const_iterator it = pieces.begin();
                                           it != pieces.end(); ++it )
    {
        index = (it->position.y * 9) + it->position.x;
        board[index].first  = it->color;
        board[index].second = it->type;
    }

    // STEP 2: Convert the 90-cell board to a FEN string.
    //
    // References:
    //    http://en.wikipedia.org/wiki/Xiangqi

    std::string fen; // Forsyth-Edwards Notation (FEN) notation.
    int  files  = 0; // X-position ... or the index of vertical lines.
    int  zeros  = 0; // The # of empty cells.
    char cPiece = 0;
    for ( int i = 0; i < 90; ++i )
    {
        switch ( board[i].second )
        {
            /* Piece notation: 
             *    http://www.wxf.org/xq/computer/fen.pdf
             *    http://www.wxf.org/xq/computer/wxf_notation.html
             */
            case hoxPIECE_KING:     cPiece = 'K'; break;
            case hoxPIECE_ADVISOR:  cPiece = 'A'; break;
            case hoxPIECE_ELEPHANT: cPiece = 'E'; break;
            case hoxPIECE_CHARIOT:  cPiece = 'R'; break;
            case hoxPIECE_HORSE:    cPiece = 'H'; break;
            case hoxPIECE_CANNON:   cPiece = 'C'; break;
            case hoxPIECE_PAWN:     cPiece = 'P'; break;
            default: /* empty */ ++zeros; cPiece = 0;
        }

        // Convert lowercase if the color is BLACK (or BLUE).
        if ( board[i].first == hoxCOLOR_BLACK ) { cPiece += 'a' - 'A'; }

        // Output the piece.
        if ( cPiece != 0 )
        {
            if ( zeros > 0 ) { fen += '0' + zeros; zeros = 0; }
            fen += cPiece;
        }

        if ( ++files == 9 ) // new row (new rank)?
        { 
            files = 0;
            if ( zeros > 0 ) { fen += '0' + zeros; zeros = 0; }
            if ( i != 89 ) fen += '/';
        }
    }

    // STEP 3: Add 'active color' to the FEN string.
    //          'w' - White/Red moves next
    //          'b' - Black/Blue moves next.

    fen += ' ';
    fen += ( gameState.nextColor == hoxCOLOR_RED ? 'w' : 'b' );

    return fen;
}

/************************* END OF FILE ***************************************/
