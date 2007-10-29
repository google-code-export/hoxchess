/////////////////////////////////////////////////////////////////////////////
// Name:            hoxConnection.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/28/2007
//
// Description:     The "base" Connection Thread to help a "network" player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxConnection.h"
#include "hoxEnums.h"
#include "hoxUtility.h"

//-----------------------------------------------------------------------------
// hoxConnection
//-----------------------------------------------------------------------------


hoxConnection::hoxConnection( const wxString&  sHostname,
                              int              nPort )
        : wxThreadHelper()
        , m_sHostname( sHostname )
        , m_nPort( nPort )
        , m_shutdownRequested( false )
{
    const char* FNAME = "hoxConnection::hoxConnection";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxConnection::~hoxConnection()
{
    const char* FNAME = "hoxConnection::~hoxConnection";
    wxLogDebug("%s: ENTER.", FNAME);
}

void*
hoxConnection::Entry()
{
    const char* FNAME = "hoxConnection::Entry";
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
            FNAME, hoxUtility::RequestTypeToString(request->type));

        this->HandleRequest( request );
        delete request;
    }

    return NULL;
}

void 
hoxConnection::AddRequest( hoxRequest* request )
{
    const char* FNAME = "hoxConnection::AddRequest";

    wxLogDebug("%s ENTER. Trying to obtain the lock...", FNAME);
    wxMutexLocker lock( m_mutexRequests );

    if ( m_shutdownRequested )
    {
        wxLogWarning("%s: Deny request [%s]. The thread is shutdowning.", 
            FNAME, hoxUtility::RequestTypeToString(request->type));
        delete request;
        return;
    }

    m_requests.push_back( request );
    m_semRequests.Post();
    wxLogDebug("%s END.", FNAME);
}

hoxRequest*
hoxConnection::GetRequest()
{
    const char* FNAME = "hoxConnection::GetRequest";
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
        wxLogDebug("%s: Shutdowning this thread...", FNAME);
        m_shutdownRequested = true;
        delete request; // *** Signal "no more request" ...
        return NULL;    // ... to the caller!
    }

    return request;
}

/************************* END OF FILE ***************************************/
