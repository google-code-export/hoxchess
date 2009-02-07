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
// Name:            hoxUtil.h
// Created:         09/28/2007
//
// Description:     Containing various helper API used in the project.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_UTIL_H_
#define __INCLUDED_HOX_UTIL_H_

#include <wx/wx.h>
#include <wx/image.h>
#include <wx/uri.h>
#include "hoxEnums.h"
#include "hoxTypes.h"

namespace hoxUtil
{
    void SetPiecesPath(const wxString& piecesPath);

    hoxResult LoadPieceImage(hoxPieceType type, hoxColor color, wxImage& image);

    /**
     * Load and return a Bitmap containing a given Image file.
     */
    wxBitmap LoadImage( const wxString& imageName );

    /**
     * Convert a given Result to a (human-readable) string.
     */
    const char* ResultToStr( const hoxResult result );

    /**
     * A helper to generate a random string.
     * @param sPrefix The OPTIONAL input prefix.
     */
    wxString GenerateRandomString( const wxString& sPrefix = "SomeString" );

    /**
     * Generate a random number of range [1, max_value].
     */
    int GenerateRandomNumber( const unsigned int max_value );

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
    const wxString ColorToString( const hoxColor color );

    /**
     * Convert a given (human-readable) string to a Color (Piece's Color or Role).
     */
    hoxColor StringToColor( const wxString& input );

    /**
     * Convert a given Piece's Type to a (human-readable) string.
     */
    const wxString TypeToString( const hoxPieceType type );

	/**
     * Convert a given (human-readable) string to a Piece's Type.
     */
    hoxPieceType StringToType( const wxString type );
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

    /**
     * Convert a given (human-readable) string to a Time-Info of
	 * of the format "nGame/nMove/nFree".
     */
    hoxTimeInfo StringToTimeInfo( const wxString& input );

	/**
	 * Convert a given Time-Info to a (human-readable) string.
	 */
	const wxString TimeInfoToString( const hoxTimeInfo timeInfo );

	/**
	 * Convert a given Game-Status to a (human-readable) string.
	 */
	const wxString GameStatusToString( const hoxGameStatus gameStatus );

    /**
     * Convert a given (human-readable) string to a Game-Status.
     */
    hoxGameStatus StringToGameStatus( const wxString& input );

    /**
     * API to convert std::string to wxString and vice versa.
     *
     * CREDITS:
     *    http://wiki.wxwidgets.org/Converting_everything_to_and_from_wxString
     */
    std::string wx2std( const wxString&    input );
    wxString    std2wx( const std::string& input );

}

#endif /* __INCLUDED_HOX_UTIL_H_ */
