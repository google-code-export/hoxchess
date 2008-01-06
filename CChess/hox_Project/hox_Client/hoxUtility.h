/***************************************************************************
 *  Copyright 2007 Huy Phan  <huyphan@playxiangqi.com>                     *
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
// Name:            hoxUtility.h
// Created:         09/28/2007
//
// Description:     Containing various helper API used in the project.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_UTILITY_H_
#define __INCLUDED_HOX_UTILITY_H_

#include <wx/wx.h>
#include <wx/image.h>
#include <wx/uri.h>
#include "hoxEnums.h"
#include "hoxTypes.h"

namespace hoxUtility
{
    void SetPiecesPath(const wxString& piecesPath);

    hoxResult LoadPieceImage(hoxPieceType type, hoxPieceColor color, wxImage& image);

    /**
     * A helper to generate a random string.
     */
    wxString GenerateRandomString();

    /**
     * Convert a given request-type to a (human-readable) string.
     */
    const wxString RequestTypeToString( const hoxRequestType requestType );

    /**
     * Convert a given (human-readable) string to a request-type.
     */
    hoxRequestType StringToRequestType( const wxString& input );

	/**
	 * Convert a given game-type to a (human-readable) string.
	 */
	const wxString GameTypeToString( const hoxGameType gameType );

    /**
     * Convert a given Color (Piece's Color or Role) to a (human-readable) string.
     */
    const wxString ColorToString( const hoxPieceColor color );

    /**
     * Convert a given (human-readable) string to a Color (Piece's Color or Role).
     */
    hoxPieceColor StringToColor( const wxString& input );

    // ----------------------------------------------------------------------------
    // hoxURI - A simple wrapper for wxURI
    // ----------------------------------------------------------------------------

    class hoxURI : public wxURI
    {
    public:
        /**
         * API to escape/unespace URI-unsafe characters, especially since
         * we transport wall-messages via HTTP's URI-path.
         */
        static wxString Escape_String(const wxString& str);
        static wxString Unescape_String(const wxString& str)
            { return wxURI::Unescape( str ); }
    };

    /**
     * Parse a given string of the format "hostname:port" into a host-name
     * and a port.
     *
     * @return true if everything is fine. Otherwise, return false.
     */
    bool ParseServerAddress( const wxString&   input,
                             hoxServerAddress& serverAddress );

    /**
     * Format a given time (in seconds) into the "mm:ss" format.
     */
	const wxString FormatTime( int nTime );

}

#endif /* __INCLUDED_HOX_UTILITY_H_ */
