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
// Name:            hoxChesscapeConnection.h
// Created:         12/12/2007
//
// Description:     The Socket-Connection Thread to help Chesscape player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_CHESSCAPE_CONNECTION_H__
#define __INCLUDED_HOX_CHESSCAPE_CONNECTION_H__

#include "hoxSocketConnection.h"

// ----------------------------------------------------------------------------
// hoxChesscapeWriter
// ----------------------------------------------------------------------------

class hoxChesscapeWriter : public hoxSocketWriter
{
public:
    hoxChesscapeWriter( wxEvtHandler*           player,
                        const hoxServerAddress& serverAddress )
                : hoxSocketWriter( player, serverAddress ) {}
    virtual ~hoxChesscapeWriter() {}

protected:
    virtual hoxResult HandleRequest( hoxRequest_APtr apRequest,
                                     wxString&       sError );
    virtual hoxAsyncSocket* CreateSocketAgent( asio::io_service&       io_service,
                                               tcp::resolver::iterator endpoint_iter,
                                               wxEvtHandler*           evtHandler);

private:
    // ------
    hoxResult   _Login( hoxRequest_APtr apRequest );
    hoxResult   _Logout( hoxRequest_APtr apRequest );
    hoxResult   _Join( hoxRequest_APtr apRequest );
    hoxResult   _Invite( hoxRequest_APtr apRequest );
    hoxResult   _GetPlayerInfo( hoxRequest_APtr apRequest );
    hoxResult   _UpdateStatus( hoxRequest_APtr apRequest );
	hoxResult   _Leave( hoxRequest_APtr apRequest );
	hoxResult   _Move( hoxRequest_APtr apRequest );
    hoxResult   _New( hoxRequest_APtr apRequest );
	hoxResult   _SendMessage( hoxRequest_APtr apRequest );
    hoxResult   _Update( hoxRequest_APtr apRequest );
    hoxResult   _Resign( hoxRequest_APtr apRequest );
	hoxResult   _Draw( hoxRequest_APtr apRequest );

    // ------
    hoxResult _WriteLine( const wxString& cmdRequest );
};

// ----------------------------------------------------------------------------
// hoxChesscapeSocket
// ----------------------------------------------------------------------------

class hoxChesscapeSocket : public hoxAsyncSocket
{
public:
    hoxChesscapeSocket( asio::io_service&       io_service,
                             tcp::resolver::iterator endpoint_iter,
                             wxEvtHandler*           evtHandler )
                : hoxAsyncSocket( io_service, endpoint_iter, evtHandler ) {}
    virtual ~hoxChesscapeSocket() {}

protected:
    virtual void handleConnect( const asio::error_code& error,
                                tcp::resolver::iterator endpoint_iter );
    virtual void handleIncomingData( const asio::error_code& error );
};

// ----------------------------------------------------------------------------
// hoxChesscapeConnection
// ----------------------------------------------------------------------------

/**
 * A Connection to communicate with Chesscape servers.
 */
class hoxChesscapeConnection : public hoxSocketConnection
{
public:
    hoxChesscapeConnection() {} // DUMMY default constructor required for RTTI.
    hoxChesscapeConnection( const hoxServerAddress& serverAddress,
                            wxEvtHandler*           player )
                : hoxSocketConnection( serverAddress, player ) {}
    virtual ~hoxChesscapeConnection() {}

protected:
    virtual hoxSocketWriter_SPtr CreateWriter( wxEvtHandler*           evtHandler,
                                               const hoxServerAddress& serverAddress );

private:

    DECLARE_DYNAMIC_CLASS(hoxChesscapeConnection)
};

#endif /* __INCLUDED_HOX_CHESSCAPE_CONNECTION_H__ */
