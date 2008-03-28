/***************************************************************************
 *  Copyright 2007, 2008 Huy Phan  <huyphan@playxiangqi.com>               *
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
// Name:            hoxThreadConnection.h
// Created:         11/05/2007
//
// Description:     The "base" Connection Thread to help a "network" player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_THREAD_CONNECTION_H_
#define __INCLUDED_HOX_THREAD_CONNECTION_H_

#include <wx/wx.h>
#include <wx/thread.h>
#include "hoxConnection.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// hoxThreadConnection
// ----------------------------------------------------------------------------

/**
 * A Connection powered by a Thread.
 * This is an abstract class providing the base for other Connections that
 * need a secondary Thread.
 *
 * @note If deriving from wxThread, acquiring mutex (using lock) would fail.
 *       Thus, I have to derive from wxThreadHelper.
 */
class hoxThreadConnection : public hoxConnection
                          , public wxThreadHelper
{
public:
    hoxThreadConnection(); // DUMMY default constructor required for RTTI info.
    hoxThreadConnection( const hoxServerAddress& serverAddress,
                         hoxPlayer*              player );
    virtual ~hoxThreadConnection();

    // **** Override the parent's API ****
    virtual void Start();
    virtual void Shutdown();
    virtual bool AddRequest( hoxRequest* request );
    virtual bool IsConnected() const { return m_bConnected; }

    /**
     * Thread execution starts here.
     */
    virtual void* Entry();

    /**
     * Is this Thread being shutdowned by the System.
     */
    bool IsBeingShutdowned() const { return m_shutdownRequested; } 

protected:
    virtual hoxRequest* GetRequest();         
    virtual void        HandleRequest( hoxRequest* request ) = 0;

    virtual void SetConnected(bool connected) { m_bConnected = connected; }

protected:
    const hoxServerAddress  m_serverAddress;

    bool                    m_bConnected;
                /* Has the connection been established with the server */

    wxSemaphore             m_semRequests;
    wxMutex                 m_mutexRequests;
    hoxRequestList          m_requests;

private:
    bool                    m_shutdownRequested;
                /* Has a shutdown-request been received? */


    DECLARE_ABSTRACT_CLASS(hoxThreadConnection)
};

#endif /* __INCLUDED_HOX_THREAD_CONNECTION_H_ */
