/***************************************************************************
 *  Copyright 2007, 2008, 2009 Huy Phan  <huyphan@playxiangqi.com>         *
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
                        const hoxServerAddress& serverAddress );
    virtual ~hoxChesscapeWriter() {}

protected:
    virtual void HandleRequest( hoxRequest_APtr apRequest );
    virtual void StartReader( wxSocketClient* socket );

private:
    // ------
    hoxResult   _Login( hoxRequest_APtr apRequest,
                        wxString&       sResponse );
    hoxResult   _Logout( hoxRequest_APtr apRequest );
    hoxResult   _Join( hoxRequest_APtr apRequest );
    hoxResult   _Invite( hoxRequest_APtr apRequest );
    hoxResult   _GetPlayerInfo( hoxRequest_APtr apRequest );
    hoxResult   _UpdateStatus( hoxRequest_APtr apRequest );
	hoxResult   _Leave( hoxRequest_APtr apRequest );
	hoxResult   _Move( hoxRequest_APtr apRequest );
    hoxResult   _New( hoxRequest_APtr apRequest );
	hoxResult   _WallMessage( hoxRequest_APtr apRequest );
    hoxResult   _Update( hoxRequest_APtr apRequest );
    hoxResult   _Resign( hoxRequest_APtr apRequest );
	hoxResult   _Draw( hoxRequest_APtr apRequest );

    // ------
    hoxResult _WriteLine( const wxString& cmdRequest );
private:

    // no copy ctor/assignment operator
    hoxChesscapeWriter(const hoxChesscapeWriter&);
    hoxChesscapeWriter& operator=(const hoxChesscapeWriter&);
};

// ----------------------------------------------------------------------------
// hoxChesscapeReader
// ----------------------------------------------------------------------------

class hoxChesscapeReader : public hoxSocketReader
{
public:
    hoxChesscapeReader( wxEvtHandler* player );
    virtual ~hoxChesscapeReader() {}

protected:
    virtual hoxResult ReadLine( wxSocketBase*   sock,
                                wxMemoryBuffer& data );

private:

    // no copy ctor/assignment operator
    hoxChesscapeReader(const hoxChesscapeReader&);
    hoxChesscapeReader& operator=(const hoxChesscapeReader&);
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
    hoxChesscapeConnection(); // DUMMY default constructor required for RTTI info.
    hoxChesscapeConnection( const hoxServerAddress& serverAddress,
                            wxEvtHandler*           player );
    virtual ~hoxChesscapeConnection();

protected:
    virtual void StartWriter();

private:

    DECLARE_DYNAMIC_CLASS(hoxChesscapeConnection)
};

#endif /* __INCLUDED_HOX_CHESSCAPE_CONNECTION_H__ */
