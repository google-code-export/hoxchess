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
        wxLogDebug("%s: Processing request Type [%d]...", FNAME, request->type);

        switch( request->type )
        {
            case hoxREQUEST_TYPE_POLL:     /* fall through */
            case hoxREQUEST_TYPE_MOVE:     /* fall through */
            case hoxREQUEST_TYPE_CONNECT:  /* fall through */
            case hoxREQUEST_TYPE_LIST:     /* fall through */
            case hoxREQUEST_TYPE_NEW:      /* fall through */
            case hoxREQUEST_TYPE_JOIN:     /* fall through */
            case hoxREQUEST_TYPE_LEAVE:
                _HandleRequest( request );
                break;

            default:
                wxLogError("%s: Unsupported request Type [%d].", FNAME, request->type);
                break;
        }

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
    hoxResult  result;
    hoxResponse* response = new hoxResponse( request->type );

    result = _SendRequest( request->content,
                           response->content );

    // NOTE: If there was error, just return it to the caller.

    if ( request->sender != NULL )
    {
        wxCommandEvent event( hoxEVT_WWW_RESPONSE );
        event.SetInt( result );
        event.SetEventObject( response );
        wxPostEvent( request->sender, event );
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

    ////////////////////////////////////////////////////////////////////
    // Sample code from: http://www.wxwidgets.org/wiki/index.php/WxHTTP
    //
    
    wxHTTP get;

    get.SetFlags( wxSOCKET_WAITALL | wxSOCKET_BLOCK ); // Block socket + GUI
    get.SetTimeout(5); // 5-second timeout instead of the Default 10 minutes.

    get.SetHeader(_T("Content-type"), _T("text/plain; charset=utf-8"));
    get.SetHeader(_T("User-Agent"), _T("hoxClient"));
 
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
        wxLogError(wxString::Format("%s: GetInputStream failed. Response-code = [%d]. Protocol-error = [%d].",
                        FNAME, get.GetResponse(), (int) get.GetError() ));
        wxMessageBox(wxString::Format(_T("%s: Unable to connect to server [%s:%d]."), 
                        FNAME, m_sHostname, m_nPort));
    }
    else if (get.GetError() == wxPROTO_NOERR)
    {
        wxStringOutputStream out_stream( &response );
        httpStream->Read( out_stream );

        //wxLogDebug( wxString(_(" GetInputStream: ")) << get.GetResponse() << _("- OK") );
        //wxMessageBox(response);
        //wxLogDebug( wxString(_T(" returned document length: ")) << (int)response.size() );

        result = hoxRESULT_OK;
    }
     
    delete httpStream;
    get.Close();

    return result;
}

/* static */
hoxResult 
hoxWWWThread::parse_string_for_simple_response( const wxString& responseStr,
                                                 int&            returnCode,
                                                 wxString&       returnMsg )
{
    const char* FNAME = "hoxWWWThread::parse_string_for_simple_response";

    returnCode = -1;

    wxStringTokenizer tkz( responseStr, wxT("\r\n") );
    int i = 0;

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        switch (i)
        {
            case 0:   // Return-code.
                returnCode = ::atoi( token.c_str() );
                break;
            case 1:    // The additional informative message.
                returnMsg = token;
                wxLogDebug(wxString::Format("%s: Server's message = [%s].", FNAME, returnMsg)) ; 
                break;
            default:
                wxLogError(wxString::Format("%s: Ignore the rest...", FNAME));
                break;
        }
        ++i;
    }

    return hoxRESULT_OK;
}

/* static */
hoxResult 
hoxWWWThread::parse_string_for_network_tables( const wxString&          responseStr,
                                               hoxNetworkTableInfoList& tableList )
{
    hoxResult  result = hoxRESULT_ERR;

    if ( responseStr.size() < 2 )
        return hoxRESULT_ERR;

    // Get the return-code.
    int returnCode = ::atoi( responseStr.c_str() );

    wxStringTokenizer tkz( responseStr.substr(2), wxT(" \r\n") );
    int i = 0;
    hoxNetworkTableInfo* tableInfo = NULL;

    tableList.clear();
   
    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        switch (i)
        {
            case 0: 
                tableInfo = new hoxNetworkTableInfo();
                tableInfo->id = token; 
                tableList.push_back( tableInfo );
                break;
            case 1: 
                tableInfo->status = ::atoi( token.c_str() ); 
                break;
            case 2: 
                tableInfo->redId = token; 
                break;
            default:
                tableInfo->blackId = token;
                break;
        }
        if ( i == 3) i = 0;
        else ++i;
    }

    return hoxRESULT_OK;
}

/* static */
hoxResult 
hoxWWWThread::parse_string_for_new_network_table( const wxString&  responseStr,
                                                  wxString&        newTableId )
{
    const char* FNAME = "hoxWWWThread::parse_string_for_new_network_table";

    int returnCode = hoxRESULT_ERR;

    wxStringTokenizer tkz( responseStr, wxT("\n") );
    int i = 0;

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        switch (i)
        {
            case 0:   // Return-code.
                returnCode = ::atoi( token.c_str() );
                if ( returnCode != 0 ) // failed?
                {
                    return hoxRESULT_ERR;
                }
                break;
            case 1:    // The ID of the new table.
                newTableId = token; 
                break;
            case 2:    // The additional informative message.
                wxLogDebug(wxString::Format("%s: Server's message = [%s].", FNAME, token)) ; 
                break;
            default:
                wxLogError(wxString::Format("%s: Ignore the rest...", FNAME));
                break;
        }
        ++i;
    }

    return hoxRESULT_OK;
}

/* static */
hoxResult 
hoxWWWThread::parse_string_for_join_network_table( const wxString&      responseStr,
                                                   hoxNetworkTableInfo& tableInfo )
{
    const char* FNAME = "hoxWWWThread::parse_string_for_join_network_table";
    hoxResult  result = hoxRESULT_ERR;

    int returnCode = hoxRESULT_ERR;

    wxStringTokenizer tkz( responseStr, wxT("\r\n") );
    int i = 0;

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        switch (i)
        {
            case 0:   // Return-code.
                returnCode = ::atoi( token.c_str() );
                if ( returnCode != 0 ) // failed?
                {
                    return hoxRESULT_ERR;
                }
                break;
            case 1:    // The additional informative message.
                wxLogDebug(wxString::Format("%s: Server's message = [%s].", FNAME, token)) ; 
                break;
            case 2:    // The returned info of the requested table.
            {
                wxString tableInfoStr = token;
                if ( hoxRESULT_OK != _parse_network_table_info_string( tableInfoStr, tableInfo ) )
                {
                    wxLogError(wxString::Format("%s: Failed to parse the Table Info String [%s].", 
                                    FNAME, tableInfoStr)); 
                    return hoxRESULT_ERR;
                }
                
                break;
            }
            default:
                wxLogError(wxString::Format("%s: Ignore the rest...", FNAME));
                break;
        }
        ++i;
    }

    return hoxRESULT_OK;
}

/*static*/
hoxResult 
hoxWWWThread::parse_string_for_network_events( const wxString&      tablesStr,
                                               hoxNetworkEventList& networkEvents )
{
    const char* FNAME = "hoxWWWThread::parse_string_for_network_events";
    hoxResult result;
    int returnCode = hoxRESULT_ERR;

    wxStringTokenizer tkz( tablesStr, wxT("\n") );
    int i = 0;

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        switch (i)
        {
            case 0:   // Return-code.
                returnCode = ::atoi( token.c_str() );
                if ( returnCode != 0 ) // failed?
                {
                    return hoxRESULT_ERR;
                }
                break;
            case 1:    // The additional informative message.
                wxLogDebug(wxString::Format("%s: Server's message = [%s].", FNAME, token)) ; 
                break;
            default:
            {
                wxLogDebug(wxString::Format("%s: Parse and add event to the list...", FNAME));
                const wxString eventStr = token;
                hoxNetworkEvent networkEvent;
                result = hoxWWWThread::_parse_network_event_string( eventStr, networkEvent );
                if ( result != hoxRESULT_OK ) // failed?
                {
                    wxLogError(wxString::Format("%s: Failed to parse network events [%s].", FNAME, eventStr));
                    return result;
                }
                networkEvents.push_back( new hoxNetworkEvent( networkEvent ) );

                break;
            }
        }
        ++i;
    }

    return hoxRESULT_OK;
}

/* static */
hoxResult
hoxWWWThread::_parse_network_table_info_string( const wxString&      tableInfoStr,
                                                hoxNetworkTableInfo& tableInfo )
{
    const char* FNAME = "hoxWWWThread::_parse_network_table_info_string";
    wxStringTokenizer tkz( tableInfoStr, wxT(" ") );
    int i = 0;

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        switch (i)
        {
            case 0:   // Table-Id
                tableInfo.id = token;
                break;
            case 1:    // Table-Status
                tableInfo.status = ::atoi( token.c_str() );
                break;
            case 2:    // RED-player's Id.
                tableInfo.redId = token;
                break;
            case 3:    // BLACK-player's Id.
                tableInfo.blackId = token;
                break;
            default:
                wxLogError(wxString::Format("%s: Ignore the rest...", FNAME));
                break;
        }
        ++i;
    }

    return hoxRESULT_OK;
}

/*static*/
hoxResult 
hoxWWWThread::_parse_network_event_string( const wxString& eventStr,
                                           hoxNetworkEvent& networkEvent )
{
    const char* FNAME = "hoxWWWThread::_parse_network_event_string";
    wxStringTokenizer tkz( eventStr, wxT(" ") );
    int i = 0;

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        switch (i)
        {
            case 0:   // event-Id
                networkEvent.id = token;
                break;
            case 1:    // Player-Id.
                networkEvent.pid = token;
                break;
            case 2:    // Table-Id (if applicable).
                networkEvent.tid = token;
                break;
            case 3:    // Event-type
                networkEvent.type = ::atoi( token.c_str() );
                break;
            case 4:    // event-content.
                networkEvent.content = token;
                break;
            default:
                wxLogError(wxString::Format("%s: Ignore the rest...", FNAME));
                break;
        }
        ++i;
    }

    return hoxRESULT_OK;
}


/************************* END OF FILE ***************************************/
