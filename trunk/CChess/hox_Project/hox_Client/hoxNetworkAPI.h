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
// Name:            hoxNetworkAPI.h
// Created:         10/26/2007
//
// Description:     Containing network related APIs specific to this project.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_NETWORK_API_H_
#define __INCLUDED_HOX_NETWORK_API_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include "hoxTypes.h"

namespace hoxNetworkAPI
{
    hoxResult ParseCommand( const wxMemoryBuffer& data, 
                            hoxCommand&     command );

    hoxResult ParseOneNetworkTable( const wxString&      tableStr,
                                    hoxNetworkTableInfo &tableInfo );

    hoxResult WriteLine( wxSocketBase*   sock,
                         const wxString& message );

    /**
     * Convert a given socket-error to a (human-readable) string.
     */
    const wxString SocketErrorToString( const wxSocketError socketError );


} /* namespace hoxNetwork */

#endif /* __INCLUDED_HOX_NETWORK_API_H_ */
