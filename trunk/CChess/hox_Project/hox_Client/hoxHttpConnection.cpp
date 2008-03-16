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
// Name:            hoxHttpConnection.cpp
// Created:         10/28/2007
//
// Description:     The HTTP-Connection Thread to help the HTTP player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxHttpConnection.h"
#include "hoxHttpPlayer.h"
#include "hoxLocalPlayer.h"
#include "hoxUtil.h"

#include <wx/sstream.h>
#include <wx/protocol/http.h>

IMPLEMENT_DYNAMIC_CLASS(hoxHttpConnection, hoxThreadConnection)

//-----------------------------------------------------------------------------
// hoxHttpConnection
//-----------------------------------------------------------------------------

hoxHttpConnection::hoxHttpConnection()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxHttpConnection::hoxHttpConnection( const wxString&  sHostname,
                                      int              nPort )
        : hoxThreadConnection( sHostname, nPort )
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
    hoxResult    result = hoxRC_ERR;
    std::auto_ptr<hoxResponse> response( new hoxResponse(request->type, 
                                                         request->sender) );

    switch( request->type )
    {
        case hoxREQUEST_POLL:     /* fall through */
        case hoxREQUEST_MOVE:     /* fall through */
        case hoxREQUEST_LOGIN:    /* fall through */
		case hoxREQUEST_LOGOUT:   /* fall through */
        case hoxREQUEST_LIST:     /* fall through */
        case hoxREQUEST_NEW:      /* fall through */
        case hoxREQUEST_JOIN:     /* fall through */
        case hoxREQUEST_LEAVE:    /* fall through */
        case hoxREQUEST_MSG:

            result = _SendRequest( _RequestToString( *request ),
				                   response->content );
            this->SetConnected( result == hoxRC_OK );
            break;

        default:
            wxLogError("%s: Unsupported request Type [%s].", 
                FNAME, hoxUtil::RequestTypeToString(request->type).c_str());
            result = hoxRC_NOT_SUPPORTED;
            break;
    }

    /* Log error */
    if ( result != hoxRC_OK )
    {
        wxLogError("%s: Error occurred while handling request [%s].", 
            FNAME, hoxUtil::RequestTypeToString(request->type).c_str());
        response->content = "!Error_Result!";
    }

    /* NOTE: If there was error, just return it to the caller. */

    wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, request->type );
    response->code = result;
    event.SetEventObject( response.release() );  // Caller will de-allocate.
    wxPostEvent( m_player, event );
}

const wxString 
hoxHttpConnection::_RequestToString( const hoxRequest& request ) const
{
	wxString result;

	result += "op=" + hoxUtil::RequestTypeToString( request.type );

	hoxCommand::Parameters::const_iterator it;
	for ( it = request.parameters.begin();
		  it != request.parameters.end(); ++it )
	{
		result += "&" + it->first + "=" + it->second;
	}

	return result;
}

hoxResult 
hoxHttpConnection::_SendRequest( const wxString& request,
                                 wxString&       response )
{
    const char* FNAME = "hoxHttpConnection::_SendRequest";
    hoxResult result = hoxRC_ERR;

    wxLogDebug("%s: ENTER.", FNAME);

    /* NOTE: This code is based on the sample code from: 
     *       http://www.wxwidgets.org/wiki/index.php/WxHTTP
     */
    
    wxHTTP get;

    wxLogDebug("%s: Creating a BLOCK-ing HTTP connection with time-out = [%d] seconds.", 
        FNAME, hoxSOCKET_HTTP_TIMEOUT);
    get.SetFlags( wxSOCKET_WAITALL | wxSOCKET_BLOCK ); // Block socket + GUI
    get.SetTimeout( hoxSOCKET_HTTP_TIMEOUT );

    get.SetHeader("Content-type", "text/plain; charset=utf-8");
    get.SetHeader("User-Agent", HOX_APP_NAME);
 
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

    /* Format the request to make sure it is URI-complied. 
     *
     * NOTE: When the wall-messages are retrieved from the remote HTTP server,
     *       they have already been unescaped (by the HTTP server). 
     *       Thus, there is no need to "unescape" here.
     */
    wxString formattedRequest = request;
    formattedRequest = hoxUtil::hoxURI::Escape_String( formattedRequest.Trim() );

    wxString getString = wxString::Format("/cchess/tables.php?%s", formattedRequest.c_str()); 

    wxInputStream* httpStream = get.GetInputStream(getString);

    if ( httpStream == NULL )
    {
        wxLogDebug("%s: *** ERROR *** GetInputStream is NULL. Response-code = [%d]. Protocol-error = [%d].",
            FNAME, get.GetResponse(), (int) get.GetError() );
        wxLogDebug("%s: *** ERROR *** Failed to connect to server [%s:%d].", FNAME, m_sHostname.c_str(), m_nPort);
    }
    else if (get.GetError() == wxPROTO_NOERR)
    {
        wxStringOutputStream out_stream( &response );
        httpStream->Read( out_stream );

        //wxLogDebug("%s: GetInputStream: Response-code = [%d] - OK", FNAME, get.GetResponse());
        //wxLogDebug("%s: Received document length = [%d].", FNAME, response.size());
        //wxMessageBox(response);

        result = hoxRC_OK;
    }
     
    delete httpStream;
    get.Close();

    wxLogDebug("%s: END.", FNAME);
    return result;
}


/************************* END OF FILE ***************************************/
