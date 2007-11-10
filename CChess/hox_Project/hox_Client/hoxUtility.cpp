/////////////////////////////////////////////////////////////////////////////
// Name:            hoxUtility.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         09/28/2007
/////////////////////////////////////////////////////////////////////////////

#include "hoxUtility.h"

using namespace hoxUtility;

static wxString gPiecePath = "";

/**
 * Get the full-path of the image of a given piece.
 */
static wxString
_get_piece_image_path( hoxPieceType  type, 
                       hoxPieceColor color )
{
    wxString filename;
    wxChar cColor;

    cColor = ( color == hoxPIECE_COLOR_RED ? 'r' : 'b' );

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
hoxUtility::LoadPieceImage(hoxPieceType type, hoxPieceColor color, wxImage& image)
{
    const char* FNAME = "hoxUtility::LoadPieceImage";
    wxString filename;

    filename =  _get_piece_image_path( type, color );
    if ( image.LoadFile(filename, wxBITMAP_TYPE_PNG) ) 
        return hoxRESULT_OK;

    wxLogError("%s: Failed to load piece-image from path [%s].", FNAME, filename.c_str());
    return hoxRESULT_ERR;
}

void 
hoxUtility::CreateNewGameInfo( hoxPieceInfoList& pieceInfoList )
{
    int i = 0;
    hoxPieceInfo* piece_info = NULL;

    pieceInfoList.clear();  // clear old info.

    // --------- BLACK

    // 1 red-king
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_KING, hoxPIECE_COLOR_BLACK, 
                                  hoxPosition(4, 0));
    pieceInfoList.push_back(piece_info);

    // 2 red-advisors.
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_ADVISOR, hoxPIECE_COLOR_BLACK,
                                  hoxPosition(3, 0));
    pieceInfoList.push_back(piece_info);
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_ADVISOR, hoxPIECE_COLOR_BLACK, 
                                  hoxPosition(5, 0));
    pieceInfoList.push_back(piece_info);

    // 2 red-elephants.
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_ELEPHANT, hoxPIECE_COLOR_BLACK, 
                                  hoxPosition(2, 0));
    pieceInfoList.push_back(piece_info);
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_ELEPHANT, hoxPIECE_COLOR_BLACK, 
                                  hoxPosition(6, 0));
    pieceInfoList.push_back(piece_info);

    // 2 red-horses.
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_HORSE, hoxPIECE_COLOR_BLACK, 
                                  hoxPosition(1, 0));
    pieceInfoList.push_back(piece_info);
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_HORSE, hoxPIECE_COLOR_BLACK, 
                                  hoxPosition(7, 0));
    pieceInfoList.push_back(piece_info);

    // 2 red-chariots.
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_CHARIOT, hoxPIECE_COLOR_BLACK, 
                                  hoxPosition(0, 0));
    pieceInfoList.push_back(piece_info);
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_CHARIOT, hoxPIECE_COLOR_BLACK, 
                                  hoxPosition(8, 0));
    pieceInfoList.push_back(piece_info);

    // 2 red-cannons.
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_CANNON, hoxPIECE_COLOR_BLACK, 
                                   hoxPosition(1, 2));
    pieceInfoList.push_back(piece_info);
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_CANNON, hoxPIECE_COLOR_BLACK, 
                                  hoxPosition(7, 2));
    pieceInfoList.push_back(piece_info);

    // 5 red-pawns.
    for (i = 0; i < 10; i+=2)
    {
        piece_info = new hoxPieceInfo(hoxPIECE_TYPE_PAWN, hoxPIECE_COLOR_BLACK, 
                                      hoxPosition(i, 3));
        pieceInfoList.push_back(piece_info);
    }

    // --------- RED

    // 1 black-king
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_KING, hoxPIECE_COLOR_RED, 
                                  hoxPosition(4, 9));
    pieceInfoList.push_back(piece_info);

    // 2 black-advisors.
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_ADVISOR, hoxPIECE_COLOR_RED, 
                                  hoxPosition(3, 9));
    pieceInfoList.push_back(piece_info);
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_ADVISOR, hoxPIECE_COLOR_RED, 
                                  hoxPosition(5, 9));
    pieceInfoList.push_back(piece_info);

    // 2 black-elephants.
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_ELEPHANT, hoxPIECE_COLOR_RED, 
                                  hoxPosition(2, 9));
    pieceInfoList.push_back(piece_info);
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_ELEPHANT, hoxPIECE_COLOR_RED, 
                                  hoxPosition(6, 9));
    pieceInfoList.push_back(piece_info);

    // 2 black-horses.
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_HORSE, hoxPIECE_COLOR_RED, 
                                  hoxPosition(1, 9));
    pieceInfoList.push_back(piece_info);
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_HORSE, hoxPIECE_COLOR_RED, 
                                  hoxPosition(7, 9));
    pieceInfoList.push_back(piece_info);

    // 2 black-chariots.
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_CHARIOT, hoxPIECE_COLOR_RED, 
                                  hoxPosition(0, 9));
    pieceInfoList.push_back(piece_info);
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_CHARIOT, hoxPIECE_COLOR_RED, 
                                  hoxPosition(8, 9));
    pieceInfoList.push_back(piece_info);

    // 2 black-cannons.
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_CANNON, hoxPIECE_COLOR_RED, 
                                  hoxPosition(1, 7));
    pieceInfoList.push_back(piece_info);
    piece_info = new hoxPieceInfo(hoxPIECE_TYPE_CANNON, hoxPIECE_COLOR_RED, 
                                  hoxPosition(7, 7));
    pieceInfoList.push_back(piece_info);

    // 5 black-pawns.
    for (i = 0; i < 10; i+=2)
    {
        piece_info = new hoxPieceInfo(hoxPIECE_TYPE_PAWN, hoxPIECE_COLOR_RED, 
                                      hoxPosition(i, 6));
        pieceInfoList.push_back(piece_info);
    }
}

void
hoxUtility::FreePieceInfoList( hoxPieceInfoList& pieceInfoList )
{
    for ( hoxPieceInfoList::iterator it = pieceInfoList.begin();
                                     it != pieceInfoList.end();
                                   ++it )
    {
        delete (*it);
        (*it) = NULL;
    }

    pieceInfoList.clear();
}

void
hoxUtility::FreeNetworkTableInfoList( hoxNetworkTableInfoList& tableList )
{
    for ( hoxNetworkTableInfoList::iterator it = tableList.begin(); 
                                            it != tableList.end(); ++it )
    {
        delete (*it);
    }
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
        case hoxREQUEST_TYPE_UNKNOWN:     return "UNKNOWN";

        case hoxREQUEST_TYPE_ACCEPT:      return "ACCEPT";
        case hoxREQUEST_TYPE_PLAYER_DATA: return "PLAYER_DATA";
        case hoxREQUEST_TYPE_CONNECT:     return "CONNECT";
        case hoxREQUEST_TYPE_DISCONNECT:  return "DISCONNECT";
        case hoxREQUEST_TYPE_SHUTDOWN:    return "SHUTDOWN";
        case hoxREQUEST_TYPE_POLL:        return "POLL";
        case hoxREQUEST_TYPE_MOVE:        return "MOVE";
        case hoxREQUEST_TYPE_LIST:        return "LIST";
        case hoxREQUEST_TYPE_NEW:         return "NEW";
        case hoxREQUEST_TYPE_JOIN:        return "JOIN";
        case hoxREQUEST_TYPE_LEAVE:       return "LEAVE";
        case hoxREQUEST_TYPE_OUT_DATA:    return "OUT_DATA";
        case hoxREQUEST_TYPE_WALL_MSG:    return "WALL_MSG";

        default:                          return "UNKNOWN";
    }
}

/**
 * Convert a given (human-readable) string to a request-type.
 */
hoxRequestType
hoxUtility::StringToRequestType( const wxString& input )
{
    if ( input == "UNKNOWN" )     return hoxREQUEST_TYPE_UNKNOWN;

    if ( input == "ACCEPT" )      return hoxREQUEST_TYPE_ACCEPT;
    if ( input == "PLAYER_DATA" ) return hoxREQUEST_TYPE_PLAYER_DATA;
    if ( input == "CONNECT" )     return hoxREQUEST_TYPE_CONNECT;
    if ( input == "DISCONNECT" )  return hoxREQUEST_TYPE_DISCONNECT;
    if ( input == "SHUTDOWN" )    return hoxREQUEST_TYPE_SHUTDOWN;
    if ( input == "POLL" )        return hoxREQUEST_TYPE_POLL;
    if ( input == "MOVE" )        return hoxREQUEST_TYPE_MOVE;
    if ( input == "LIST" )        return hoxREQUEST_TYPE_LIST;
    if ( input == "NEW" )         return hoxREQUEST_TYPE_NEW;
    if ( input == "JOIN" )        return hoxREQUEST_TYPE_JOIN;
    if ( input == "LEAVE" )       return hoxREQUEST_TYPE_LEAVE;
    if ( input == "OUT_DATA" )    return hoxREQUEST_TYPE_OUT_DATA;
    if ( input == "WALL_MSG" )    return hoxREQUEST_TYPE_WALL_MSG;

    return hoxREQUEST_TYPE_UNKNOWN;
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
hoxUtility::ParserHostnameAndPort( const wxString& input,
                                   wxString&       hostname,
                                   int&            port )
{
    const char SEPARATOR = ':';

    hostname = input.BeforeFirst( SEPARATOR );
    if ( hostname.empty() )
        return false;

    wxString portStr = input.AfterFirst( SEPARATOR );
    if ( !portStr.empty() )
    {
        long longVal;
        if ( !portStr.ToLong( &longVal ) )
            return false;

        port = (int) longVal;
    }
    
    return true;
}

/************************* END OF FILE ***************************************/
