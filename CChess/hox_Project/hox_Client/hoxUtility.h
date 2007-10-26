/////////////////////////////////////////////////////////////////////////////
// Name:            hoxUtility.h
// Program's Name:  Huy's Open Xiangqi
// Created:         09/28/2007
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_UTILITY_H_
#define __INCLUDED_HOX_UTILITY_H_

#include "wx/image.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

namespace hoxUtility
{
    void SetPiecesPath(const wxString& piecesPath);

    hoxResult LoadPieceImage(hoxPieceType type, hoxPieceColor color, wxImage& image);

    /**
     * Create a brand new game by specifying the info of ALL pieces initially.
     */
    void CreateNewGameInfo( hoxPieceInfoList& pieceInfoList );

    void FreePieceInfoList( hoxPieceInfoList& pieceInfoList );

    /** 
     * NOTE: I were thinking about deriving a class to handle the list of
     *       tables (to avoid memory leak). However, it seems that we
     *       should NOT derive from STD containers according the following:
     *
     *     + http://www.codeguru.com/forum/archive/index.php/t-267641.html
     *     + http://www.codeguru.com/Cpp/Cpp/cpp_mfc/stl/article.php/c4143
     *
     *
     *  Thus: [ class hoxNetworkTableInfoList : public std::list<hoxNetworkTableInfo*> ]
     *  is a BAD IDEA.
     */
    void FreeNetworkTableInfoList( hoxNetworkTableInfoList& tableList );

    /**
     * A helper to generate a random string.
     */
    wxString GenerateRandomString();

    /**
     * Convert a given socket-event to a (human-readable) string.
     */
    const wxString SocketEventToString( const wxSocketNotify socketEvent );

    /**
     * Convert a given socket-error to a (human-readable) string.
     */
    const wxString SocketErrorToString( const wxSocketError socketError );

    /**
     * Convert a given request-type to a (human-readable) string.
     */
    const wxString RequestTypeToString( const hoxRequestType requestType );

}

#endif /* __INCLUDED_HOX_UTILITY_H_ */