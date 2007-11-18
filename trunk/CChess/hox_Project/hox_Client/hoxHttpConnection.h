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
// Name:            hoxHttpConnection.h
// Created:         10/28/2007
//
// Description:     The HTTP-Connection Thread to help the HTTP player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_HTTP_CONNECTION_H_
#define __INCLUDED_HOX_HTTP_CONNECTION_H_

#include <wx/wx.h>
#include "hoxThreadConnection.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// hoxHttpConnection
// ----------------------------------------------------------------------------

/**
 * A HTTP Connection used by the HTTP Player.
 */
class hoxHttpConnection : public hoxThreadConnection
{
public:
    hoxHttpConnection(); // DUMMY default constructor required for RTTI info.
    hoxHttpConnection( const wxString& sHostname,
                       int             nPort );
    virtual ~hoxHttpConnection();

protected:
    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void HandleRequest( hoxRequest* request );

private:
    hoxResult   _SendRequest( const wxString& request, wxString& response );

    DECLARE_DYNAMIC_CLASS(hoxHttpConnection)
};


#endif /* __INCLUDED_HOX_HTTP_CONNECTION_H_ */
