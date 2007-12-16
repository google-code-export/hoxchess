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
// Name:            hoxChesscapeConnection.h
// Created:         12/12/2007
//
// Description:     The Socket-Connection Thread to help Chesscape player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_CHESSCAPE_CONNECTION_H_
#define __INCLUDED_HOX_CHESSCAPE_CONNECTION_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include "hoxThreadConnection.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// hoxChesscapeConnection
// ----------------------------------------------------------------------------

/**
 * A Connection to communicate with Chesscape servers.
 */
class hoxChesscapeConnection : public hoxThreadConnection
{
public:
    hoxChesscapeConnection(); // DUMMY default constructor required for RTTI info.
    hoxChesscapeConnection( const wxString& sHostname,
                         int             nPort );
    virtual ~hoxChesscapeConnection();

protected:
    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void HandleRequest( hoxRequest* request );

private:
    hoxResult   _CheckAndHandleSocketLostEvent( const hoxRequest* request, 
                                                wxString&         response );
    hoxResult   _Connect(const wxString& login, 
		                 const wxString& password,
						 wxString&       responseStr);
    hoxResult   _Disconnect(const wxString& login);
    hoxResult   _Join(const wxString& tableId);
	hoxResult   _Leave();
    void        _DestroySocket();
    hoxResult _ReadLine( wxSocketBase* sock, 
                         wxString&     result );

private:
    wxSocketClient*       m_pSClient;
                /* The socket to handle network connections */

    DECLARE_DYNAMIC_CLASS(hoxChesscapeConnection)
};

#endif /* __INCLUDED_HOX_CHESSCAPE_CONNECTION_H_ */
