/////////////////////////////////////////////////////////////////////////////
// Name:            hoxWWWThread.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/21/2007
//
// Description:     The WWW Thread to help the WWW player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxWWWThread.h"
#include "hoxWWWPlayer.h"
#include "hoxEnums.h"
#include "hoxUtility.h"

#include <wx/sstream.h>
#include <wx/protocol/http.h>
#include <wx/tokenzr.h>


//-----------------------------------------------------------------------------
// hoxWWWThread
//-----------------------------------------------------------------------------


hoxWWWThread::hoxWWWThread( const wxString&  sHostname,
                            int              nPort )
        : wxThreadHelper()
        , m_sHostname( sHostname )
        , m_nPort( nPort )
        , m_shutdownRequested( false )
{
}

hoxWWWThread::~hoxWWWThread()
{
    const char* FNAME = "hoxWWWThread::~hoxWWWThread";

    wxLogDebug("%s: ENTER.", FNAME);
}

void*
hoxWWWThread::Entry()
{
    const char* FNAME = "hoxWWWThread::Entry";
    hoxRequest* request = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

    while ( !m_shutdownRequested && m_semRequests.Wait() == wxSEMA_NO_ERROR )
    {
        request = _GetRequest();
        if ( request == NULL )
        {
            wxASSERT_MSG( m_shutdownRequested, "This thread must be shutdowning." );
            break;  // Exit the thread.
        }
        wxLogDebug("%s: Processing request Type [%s]...", 
            FNAME, hoxUtility::RequestTypeToString(request->type));

        _HandleRequest( request );
        delete request;
    }

    return NULL;
}

void 
hoxWWWThread::AddRequest( hoxRequest* request )
{
    const char* FNAME = "hoxWWWThread::AddRequest";
    wxMutexLocker lock( m_mutexRequests );

    if ( m_shutdownRequested )
    {
        wxLogWarning(wxString::Format("%s: Deny request [%d]. The thread is shutdowning.", 
                        FNAME, request->type));
        delete request;
        return;
    }

    m_requests.push_back( request );
    m_semRequests.Post();
}

void 
hoxWWWThread::_HandleRequest( hoxRequest* request )
{
    const char* FNAME = "hoxWWWThread::_HandleRequest";
    hoxResult    result = hoxRESULT_ERR;
    hoxResponse* response = new hoxResponse( request->type );

    switch( request->type )
    {
        case hoxREQUEST_TYPE_POLL:     /* fall through */
        case hoxREQUEST_TYPE_TABLE_MOVE: /* fall through */
        case hoxREQUEST_TYPE_CONNECT:  /* fall through */
        case hoxREQUEST_TYPE_LIST:     /* fall through */
        case hoxREQUEST_TYPE_NEW:      /* fall through */
        case hoxREQUEST_TYPE_JOIN:     /* fall through */
        case hoxREQUEST_TYPE_LEAVE:
            result = _SendRequest( request->content, response->content );
            break;

        default:
            wxLogError("%s: Unsupported request Type [%s].", 
                FNAME, hoxUtility::RequestTypeToString(request->type));
            result = hoxRESULT_NOT_SUPPORTED;
            break;
    }

    /* NOTE: If there was error, just return it to the caller. */

    if ( request->sender != NULL )
    {
        wxCommandEvent event( hoxEVT_WWW_RESPONSE );
        event.SetInt( result );
        event.SetEventObject( response );  // Caller will de-allocate.
        wxPostEvent( request->sender, event );
    }
    else
    {
        delete response;
    }
}

hoxRequest*
hoxWWWThread::_GetRequest()
{
    const char* FNAME = "hoxWWWThread::_GetRequest";
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
        wxLogDebug(wxString::Format("%s: Shutdowning this thread...", FNAME));
        m_shutdownRequested = true;
        delete request; // *** Signal "no more request" ...
        return NULL;    // ... to the caller!
    }

    return request;
}

hoxResult 
hoxWWWThread::_SendRequest( const wxString& request,
                            wxString&       response )
{
    const char* FNAME = "hoxWWWThread::_SendRequest";
    hoxResult result = hoxRESULT_ERR;

    wxLogDebug("%s: ENTER.", FNAME);

    /* NOTE: This code is based on the sample code from: 
     *       http://www.wxwidgets.org/wiki/index.php/WxHTTP
     */
    
    wxHTTP get;

    wxLogDebug("%s: Creating a BLOCK-ing HTTP connection with time-out = [%d] seconds.", 
        FNAME, hoxSOCKET_CLIENT_HTTP_TIMEOUT);
    get.SetFlags( wxSOCKET_WAITALL | wxSOCKET_BLOCK ); // Block socket + GUI
    get.SetTimeout( hoxSOCKET_CLIENT_HTTP_TIMEOUT );

    get.SetHeader("Content-type", "text/plain; charset=utf-8");
    get.SetHeader("User-Agent", "hoxClient");
 
    /* This will wait until the user connects to the internet. 
     * It is important in case of dialup (or ADSL) connections.
     */
    while ( !get.Connect( m_sHostname, m_nPort ) ) // only the server, no pages here yet ...
    {
        wxSleep( 1 /* 1-second wait */ );

        //wxThread::Sleep( 1000 /* 1-second wait */ );
          /* This function should be used instead of wxSleep by
           * all worker threads (i.e. all except the main one).
           */
    }
    
    wxASSERT_MSG( wxApp::IsMainLoopRunning(), "Main loop should be running.");
   
    wxInputStream* httpStream = get.GetInputStream(request);

    if ( httpStream == NULL )
    {
        wxLogError("%s: GetInputStream is NULL. Response-code = [%d]. Protocol-error = [%d].",
            FNAME, get.GetResponse(), (int) get.GetError() );
        wxLogError("%s: Failed to connect to server [%s:%d].", FNAME, m_sHostname, m_nPort);
    }
    else if (get.GetError() == wxPROTO_NOERR)
    {
        wxStringOutputStream out_stream( &response );
        httpStream->Read( out_stream );

        wxLogDebug("%s: GetInputStream: Response-code = [%d] - OK", FNAME, get.GetResponse());
        wxLogDebug("%s: Received document length = [%d].", FNAME, response.size());
        //wxMessageBox(response);

        result = hoxRESULT_OK;
    }
     
    delete httpStream;
    get.Close();

    wxLogDebug("%s: END.", FNAME);
    return result;
}


/************************* END OF FILE ***************************************/
