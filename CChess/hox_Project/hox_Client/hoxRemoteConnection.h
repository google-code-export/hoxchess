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
// Name:            hoxRemoteConnection.h
// Created:         11/05/2007
//
// Description:     The Remote Connection for Remote players.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_REMOTE_CONNECTION_H_
#define __INCLUDED_HOX_REMOTE_CONNECTION_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include "hoxConnection.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/* Forward declarations */
class hoxServer;

// ----------------------------------------------------------------------------
// hoxRemoteConnection
// ----------------------------------------------------------------------------

/**
 * A Connection used by a Remote Player.
 */
class hoxRemoteConnection : public hoxConnection
{
public:
    hoxRemoteConnection();
    virtual ~hoxRemoteConnection();

    // **** Override the parent's API ****
    virtual void Start();
    virtual void Shutdown();
    virtual void AddRequest( hoxRequest* request );
    virtual bool IsConnected()   { return false; }

    /**
     * Set the Callback socket. 
     */
    hoxResult SetCBSocket( wxSocketBase* socket );

    /**
     * Set the server component that will manage this connection.
     */
    void SetServer( hoxServer* server);

private:
    wxSocketBase*     m_pCBSock; 
        /* Callback socket to help the server.  */

    hoxServer*        m_server;
        /* Set the server component that will manage this connection. */

    DECLARE_DYNAMIC_CLASS(hoxRemoteConnection)
};


#endif /* __INCLUDED_HOX_REMOTE_CONNECTION_H_ */
