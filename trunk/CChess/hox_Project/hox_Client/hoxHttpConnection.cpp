/////////////////////////////////////////////////////////////////////////////
// Name:            hoxHttpConnection.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/28/2007
//
// Description:     The HTTP-Connection Thread to help the HTTP player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxHttpConnection.h"
#include "hoxHttpPlayer.h"
#include "hoxEnums.h"
#include "hoxUtility.h"

#include <wx/sstream.h>
#include <wx/protocol/http.h>
#include <wx/tokenzr.h>


//-----------------------------------------------------------------------------
// hoxHttpConnection
//-----------------------------------------------------------------------------


hoxHttpConnection::hoxHttpConnection( const wxString&  sHostname,
                                      int              nPort )
        : hoxConnection( sHostname, nPort )
{
}

hoxHttpConnection::~hoxHttpConnection()
{
    const char* FNAME = "hoxHttpConnection::~hoxHttpConnection";

    wxLogDebug("%s: ENTER.", FNAME);
}

void 
hoxHttpConnection::HandleRequest( hoxRequest* request )
{
    const char* FNAME = "hoxHttpConnection::_HandleRequest";
    hoxResult    result = hoxRESULT_ERR;
    std::auto_ptr<hoxResponse> response( new hoxResponse(request->type) );

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
        wxCommandEvent event( hoxEVT_HTTP_RESPONSE );
        event.SetInt( result );
        event.SetEventObject( response.release() );  // Caller will de-allocate.
        wxPostEvent( request->sender, event );
    }
}

hoxResult 
hoxHttpConnection::_SendRequest( const wxString& request,
                                 wxString&       response )
{
    const char* FNAME = "hoxHttpConnection::_SendRequest";
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
