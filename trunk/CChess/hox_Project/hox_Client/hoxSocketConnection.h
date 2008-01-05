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
// Name:            hoxSocketConnection.h
// Created:         10/28/2007
//
// Description:     The Socket-Connection Thread to help MY player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_SOCKET_CONNECTION_H_
#define __INCLUDED_HOX_SOCKET_CONNECTION_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include "hoxThreadConnection.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// hoxSocketConnection
// ----------------------------------------------------------------------------

/**
 * A Connection based on a network Socket.
 */
class hoxSocketConnection : public hoxThreadConnection
{
public:
    hoxSocketConnection(); // DUMMY default constructor required for RTTI info.
    hoxSocketConnection( const wxString& sHostname,
                         int             nPort );
    virtual ~hoxSocketConnection();

protected:
    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void HandleRequest( hoxRequest* request );

private:
	const wxString _RequestToString( const hoxRequest& request ) const;

    hoxResult   _CheckAndHandleSocketLostEvent( const hoxRequest* request, 
                                                wxString&         response );
    hoxResult   _Connect();
    void        _Disconnect();

private:
    wxSocketClient*       m_pSClient;
                /* The socket to handle network connections */

    DECLARE_DYNAMIC_CLASS(hoxSocketConnection)
};

#endif /* __INCLUDED_HOX_SOCKET_CONNECTION_H_ */
