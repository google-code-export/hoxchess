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
// Name:            hoxThreadConnection.cpp
// Created:         10/28/2007
//
// Description:     The "base" Connection Thread to help a "network" player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxThreadConnection.h"
#include "hoxEnums.h"
#include "hoxUtility.h"

IMPLEMENT_ABSTRACT_CLASS(hoxThreadConnection, hoxConnection)

//-----------------------------------------------------------------------------
// hoxThreadConnection
//-----------------------------------------------------------------------------

hoxThreadConnection::hoxThreadConnection()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxThreadConnection::hoxThreadConnection( const wxString& sHostname,
                                          int             nPort )
        : hoxConnection()
        , wxThreadHelper()
        , m_sHostname( sHostname )
        , m_nPort( nPort )
        , m_shutdownRequested( false )
        , m_player( NULL )
{
    const char* FNAME = "hoxThreadConnection::hoxThreadConnection";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxThreadConnection::~hoxThreadConnection()
{
    const char* FNAME = "hoxThreadConnection::~hoxThreadConnection";
    wxLogDebug("%s: ENTER.", FNAME);
}

void 
hoxThreadConnection::Start()
{
    const char* FNAME = "hoxThreadConnection::Start";

    wxLogDebug("%s: ENTER.", FNAME);

    if (    this->GetThread() 
         && this->GetThread()->IsRunning() )
    {
        wxLogDebug("%s: The connection has been started. END.", FNAME);
        return;
    }

    if ( this->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError("%s: Failed to create Connection thread.", FNAME);
        return;
    }
    wxASSERT_MSG( !this->GetThread()->IsDetached(), 
                  "The Connection thread must be joinable." );

    this->GetThread()->Run();
}

void 
hoxThreadConnection::Shutdown()
{
    const char* FNAME = "hoxThreadConnection::Shutdown";

    wxLogDebug("%s: Request the Connection thread to be shutdowned...", FNAME);
    wxThread::ExitCode exitCode = this->GetThread()->Wait();
    wxLogDebug("%s: The Connection thread was shutdowned with exit-code = [%d].", FNAME, exitCode);
}

void*
hoxThreadConnection::Entry()
{
    const char* FNAME = "hoxThreadConnection::Entry";
    hoxRequest* request = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

    while (   !m_shutdownRequested 
            && m_semRequests.Wait() == wxSEMA_NO_ERROR )
    {
        request = this->GetRequest();
        if ( request == NULL )
        {
            wxASSERT_MSG( m_shutdownRequested, "This thread must be shutdowning." );
            break;  // Exit the thread.
        }
        wxLogDebug("%s: Processing request Type = [%s]...", 
            FNAME, hoxUtility::RequestTypeToString(request->type).c_str());

        this->HandleRequest( request );
        delete request;
    }

    return NULL;
}

void 
hoxThreadConnection::AddRequest( hoxRequest* request )
{
    const char* FNAME = "hoxThreadConnection::AddRequest";

    wxLogDebug("%s ENTER. Trying to obtain the lock...", FNAME);
    wxMutexLocker lock( m_mutexRequests );

    if ( m_shutdownRequested )
    {
        wxLogWarning("%s: Deny request [%s]. The thread is shutdowning.", 
            FNAME, hoxUtility::RequestTypeToString(request->type).c_str());
        delete request;
        return;
    }

    m_requests.push_back( request );
    m_semRequests.Post();
    wxLogDebug("%s END.", FNAME);
}

hoxRequest*
hoxThreadConnection::GetRequest()
{
    const char* FNAME = "hoxThreadConnection::GetRequest";
    wxMutexLocker lock( m_mutexRequests );

    hoxRequest* request = NULL;

    wxASSERT_MSG( !m_requests.empty(), "We must have at least one request.");
    request = m_requests.front();
    m_requests.pop_front();

    /* Handle SHUTDOWN request here to avoid the possible memory leaks.
     * The reason is that others (timers, for example) may continue to 
     * send requests to this thread while this thread is shutdowning it self. 
     *
     * NOTE: The SHUTDOWN request is (purposely) handled here inside this function 
     *       because the "mutex-lock" is still being held.
     */

    if ( request->type == hoxREQUEST_TYPE_SHUTDOWN )
    {
        wxLogDebug("%s: Shutting down this thread...", FNAME);
        m_shutdownRequested = true;
        delete request; // *** Signal "no more request" ...
        return NULL;    // ... to the caller!
    }

    return request;
}

/************************* END OF FILE ***************************************/
