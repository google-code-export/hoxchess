/////////////////////////////////////////////////////////////////////////////
// Name:            hoxUtility.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         09/28/2007
/////////////////////////////////////////////////////////////////////////////

#include "wx/wx.h"

#include "hoxUtility.h"

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
        filename.sprintf("%s/%cking.png", gPiecePath, cColor);
        break;

    case hoxPIECE_TYPE_ADVISOR:  // Advisor
        filename.sprintf("%s/%cadvisor.png", gPiecePath, cColor);
        break;

    case hoxPIECE_TYPE_ELEPHANT: // Elephant 
        filename.sprintf("%s/%celephant.png", gPiecePath, cColor);
        break;
  
    case hoxPIECE_TYPE_HORSE:  // Horse 
        filename.sprintf("%s/%chorse.png", gPiecePath, cColor);
        break;
  
    case hoxPIECE_TYPE_CHARIOT: // Chariot
        filename.sprintf("%s/%cchariot.png", gPiecePath, cColor);
        break;
  
    case hoxPIECE_TYPE_CANNON: // Cannon
        filename.sprintf("%s/%ccannon.png", gPiecePath, cColor);
        break;
  
    case hoxPIECE_TYPE_PAWN: // Soldier
        filename.sprintf("%s/%cpawn.png", gPiecePath, cColor);
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
    wxString filename;

    filename =  _get_piece_image_path( type, color );
    if ( image.LoadFile(filename, wxBITMAP_TYPE_PNG) ) 
        return hoxRESULT_OK;

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

const wxString 
hoxUtility::SocketEventToString( const wxSocketNotify socketEvent )
{
    switch( socketEvent )
    {
        case wxSOCKET_INPUT:       return "wxSOCKET_INPUT";
        case wxSOCKET_OUTPUT:      return "wxSOCKET_OUTPUT";
        case wxSOCKET_CONNECTION:  return "wxSOCKET_CONNECTION";
        case wxSOCKET_LOST:        return "wxSOCKET_LOST";

        default:                   return "Unexpected event!";
    }
}

/**
 * Convert a given socket-error to a (human-readable) string.
 */
const wxString 
hoxUtility::SocketErrorToString( const wxSocketError socketError )
{
    switch( socketError )
    {
        case wxSOCKET_NOERROR:    return "wxSOCKET_NOERROR"; //No error happened
        case wxSOCKET_INVOP:      return "wxSOCKET_INVOP";   // invalid operation
        case wxSOCKET_IOERR:      return "wxSOCKET_IOERR";   // Input/Output error
        case wxSOCKET_INVADDR:    return "wxSOCKET_INVADDR"; // Invalid address passed to wxSocket
        case wxSOCKET_INVSOCK:    return "wxSOCKET_INVSOCK"; // Invalid socket (uninitialized).
        case wxSOCKET_NOHOST:     return "wxSOCKET_NOHOST";  // No corresponding host
        case wxSOCKET_INVPORT:    return "wxSOCKET_INVPORT"; // Invalid port
        case wxSOCKET_WOULDBLOCK: return "wxSOCKET_WOULDBLOCK"; // The socket is non-blocking and the operation would block
        case wxSOCKET_TIMEDOUT:   return "wxSOCKET_TIMEDOUT"; // The timeout for this operation expired
        case wxSOCKET_MEMERR:     return "wxSOCKET_MEMERR";   // Memory exhausted

        default:                  return "Unexpected error!";
    }
}

/**
 * Convert a given request-type to a (human-readable) string.
 */
const wxString 
hoxUtility::RequestTypeToString( const hoxRequestType requestType )
{
    switch( requestType )
    {
        case hoxREQUEST_TYPE_UNKNOWN:     return "hoxREQUEST_TYPE_UNKNOWN";

        case hoxREQUEST_TYPE_ACCEPT:      return "hoxREQUEST_TYPE_ACCEPT";
        case hoxREQUEST_TYPE_DATA:        return "hoxREQUEST_TYPE_DATA";
        case hoxREQUEST_TYPE_CONNECT:     return "hoxREQUEST_TYPE_CONNECT";
        case hoxREQUEST_TYPE_DISCONNECT:  return "hoxREQUEST_TYPE_DISCONNECT";
        case hoxREQUEST_TYPE_SHUTDOWN:    return "hoxREQUEST_TYPE_SHUTDOWN";
        case hoxREQUEST_TYPE_POLL:        return "hoxREQUEST_TYPE_POLL";
        case hoxREQUEST_TYPE_MOVE:        return "hoxREQUEST_TYPE_MOVE";
        case hoxREQUEST_TYPE_LIST:        return "hoxREQUEST_TYPE_LIST";
        case hoxREQUEST_TYPE_NEW:         return "hoxREQUEST_TYPE_NEW";
        case hoxREQUEST_TYPE_JOIN:        return "hoxREQUEST_TYPE_JOIN";
        case hoxREQUEST_TYPE_LEAVE:       return "hoxREQUEST_TYPE_LEAVE";
        case hoxREQUEST_TYPE_LISTEN:      return "hoxREQUEST_TYPE_LISTEN";
        case hoxREQUEST_TYPE_TABLE_MOVE:  return "hoxREQUEST_TYPE_TABLE_MOVE";

        default:                          return "Unexpected request-type!";
    }
}

/************************* END OF FILE ***************************************/
