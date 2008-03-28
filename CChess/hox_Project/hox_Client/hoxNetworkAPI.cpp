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
// Name:            hoxNetworkAPI.cpp
// Created:         10/26/2007
//
// Description:     Containing network related APIs specific to this project.
/////////////////////////////////////////////////////////////////////////////

#include "hoxNetworkAPI.h"
#include "hoxUtil.h"
#include "hoxPlayer.h"
#include "MyApp.h"

#include <wx/tokenzr.h>

/* Import namespaces */
using namespace hoxNetworkAPI;


//-----------------------------------------------------------------------------
// SocketInputLock
//-----------------------------------------------------------------------------

SocketInputLock::SocketInputLock( wxSocketBase* sock )
            : m_sock( sock )
{
    m_sock->SetNotify(wxSOCKET_LOST_FLAG); // remove the wxSOCKET_INPUT_FLAG!!!
}

SocketInputLock::~SocketInputLock()
{
    // Enable the input flag again.
    m_sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
}

//-----------------------------------------------------------------------------
// hoxNetworkAPI namespace
//-----------------------------------------------------------------------------

hoxResult   
hoxNetworkAPI::SendRequest( wxSocketBase*   sock, 
                            const wxString& request,
                            wxString&       response )
{
    const char* FNAME = "hoxNetworkAPI::SendRequest";
    hoxResult     result;
    //wxUint32      requestSize;
    //wxUint32      nWrite;

    wxLogDebug("%s: ENTER.", FNAME);    

    // We disable input events until we are done processing the current command.
    hoxNetworkAPI::SocketInputLock socketLock( sock );

    // Send request.
    wxLogDebug("%s: Sending the request [%s] over the network...", FNAME, request.c_str());

	result = hoxNetworkAPI::WriteMsg( sock, request );
	if ( result != hoxRC_OK )
	{
        wxLogDebug("%s: *** WARN *** Failed to send request. Error = [%s].", 
            FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
		result = hoxRC_ERR;
        goto exit_label;
	}

#if 0
    requestSize = (wxUint32) request.size();
    sock->Write( request, requestSize );
    nWrite = sock->LastCount();
    if ( nWrite < requestSize )
    {
        wxLogDebug("%s: *** WARN *** Failed to send request [%s] ( %d < %d ). Error = [%s].", 
            FNAME, request.c_str(), nWrite, requestSize, 
            hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
        result = hoxRC_ERR;
        goto exit_label;
    }
#endif

    // Wait until data available (will also return if the connection is lost)
    wxLogDebug("%s: Waiting for response from the network (timeout = 5 sec)...", FNAME);
    if ( ! sock->WaitForRead( 5 /* seconds */ ) )
    {
        wxLogDebug("%s: Timeout while waiting for read. However, we still continue...", FNAME);
    }

    // Read back the response.
    wxLogDebug("%s: Reading back the response from the network...", FNAME);

    result = hoxNetworkAPI::ReadMsg( sock, response );
    if ( result != hoxRC_OK )
    {
        wxLogDebug("%s: *** WARN *** Failed to read response. Error = [%s].", 
            FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
        goto exit_label;
    }
    
    // Finally, successful.
    result = hoxRC_OK;

exit_label:
    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult 
hoxNetworkAPI::SendOutData( wxSocketBase*   sock, 
                            const wxString& contentStr )
{
	return hoxNetworkAPI::WriteMsg( sock, contentStr );
}

hoxResult
hoxNetworkAPI::ParseCommand( const wxString& commandStr, 
                             hoxCommand&     command )
{
    const char* FNAME = "hoxNetworkAPI::ParseCommand";

    wxStringTokenizer tkz( commandStr, "&" );

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();

        size_t foundIndex = token.find( '=' );
        
        if ( foundIndex == wxNOT_FOUND )
            continue;  // ignore this 'error'.

        wxString paramName;
        wxString paramValue;

        paramName = token.substr( 0, foundIndex );
        paramValue = token.substr( foundIndex+1 );

        // Special case for "op" param-name.
        if ( paramName == "op" )
        {
            command.type = hoxUtil::StringToRequestType( paramValue );

            if ( command.type == hoxREQUEST_UNKNOWN )
            {
                wxLogError("%s: Unsupported command-type = [%s].", FNAME, paramValue.c_str());
                return hoxRC_NOT_SUPPORTED;
            }
        }
        else
        {
			paramValue.Trim();
            command.parameters[paramName] = paramValue;
        }
    }

    return hoxRC_OK;
}

hoxResult 
hoxNetworkAPI::ParseSimpleResponse( const wxString& responseStr,
                                    int&            returnCode,
                                    wxString&       returnMsg )
{
    const char* FNAME = "hoxNetworkAPI::ParseSimpleResponse";
    int      i = 0;
    wxString token;

    returnCode = -1;
    returnMsg  = responseStr;

    wxStringTokenizer tkz( responseStr, "\r\n" );

    while ( tkz.HasMoreTokens() )
    {
        token = tkz.GetNextToken();
        switch (i)
        {
            case 0:   // Return-code.
                returnCode = ::atoi( token.c_str() );
                break;
            case 1:    // The additional informative message.
                returnMsg = token;
                wxLogDebug("%s: Server's message = [%s].", FNAME, returnMsg.c_str()); 
                break;
            default:
                wxLogError("%s: Ignore the rest...", FNAME);
                break;
        }
        ++i;
    }

    return hoxRC_OK;
}

hoxResult 
hoxNetworkAPI::ParseConnectResponse( const wxString& responseStr,
 									 int&            returnCode,
									 wxString&       returnMsg,
									 wxString&       sessionId,
									 int&            nScore )
{
    const char* FNAME = "hoxNetworkAPI::ParseSimpleResponse";
    int      i = 0;
    wxString token;

    returnCode = -1;
    returnMsg  = responseStr;

    wxStringTokenizer tkz( responseStr, "\r\n" );

    while ( tkz.HasMoreTokens() )
    {
        token = tkz.GetNextToken();
        switch (i++)
        {
            case 0:   // Return-code.
                returnCode = ::atoi( token.c_str() );
                break;

            case 1:    // The additional informative message.
                returnMsg = token;
                wxLogDebug("%s: Server's message = [%s].", FNAME, returnMsg.c_str()); 
                break;

            case 2:    // Session-Id
                sessionId = token;
                break;

            case 3:   // Playe-Score
                nScore = ::atoi( token.c_str() );
                break;

			default:
                // Ignore the rest.
                break;
        }
    }

    return hoxRC_OK;
}

hoxResult
hoxNetworkAPI::ParseNetworkTables( const wxString&          responseStr,
                                   hoxNetworkTableInfoList& tableList )
{
    const char* FNAME = "hoxNetworkAPI::ParseNetworkTables";
    hoxResult  result = hoxRC_ERR;
	int      returnCode = -1;
	wxString returnMsg;

    wxLogDebug("%s: ENTER.", FNAME);

	wxStringTokenizer tkz( responseStr, "\r\n" );
    int i = 0;
    hoxNetworkTableInfo tableInfo;

    tableList.clear();
   
	while ( tkz.HasMoreTokens() )
	{
        wxString token = tkz.GetNextToken();
        switch (i++)
        {
            case 0:   // Return-code.
                returnCode = ::atoi( token.c_str() );
                break;

            case 1:    // The additional informative message.
				returnMsg = token;
                wxLogDebug("%s: Server's message = [%s].", FNAME, returnMsg.c_str()) ; 
                break;

            default:
				hoxNetworkAPI::ParseOneNetworkTable(token, tableInfo);
				tableList.push_back( tableInfo );
                break;
        }
    }

	return ( returnCode == 0 ? hoxRC_OK : hoxRC_ERR );
}

hoxResult 
hoxNetworkAPI::ParseOneNetworkTable( const wxString&      tableStr,
                                     hoxNetworkTableInfo &tableInfo )
{
    const char* FNAME = "hoxNetworkAPI::ParseOneNetworkTable";
    hoxResult  result = hoxRC_ERR;

    wxLogDebug("%s: ENTER.", FNAME);
	tableInfo.Clear();

	wxStringTokenizer tkz( tableStr, ";" );
    int i = 0;

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        switch (i++)
        {
            case 0:  // Id
                tableInfo.id = token; 
                break;

            case 1:  // Group
				tableInfo.group = (  token == "0" 
					               ? hoxGAME_GROUP_PUBLIC 
								   : hoxGAME_GROUP_PRIVATE ); 
                break;

            case 2:  // Type
				tableInfo.gameType = (  token == "0" 
					               ? hoxGAME_TYPE_RATED 
								   : hoxGAME_TYPE_NONRATED ); 
                break;

            case 3:  // Initial-Time
				tableInfo.initialTime = hoxUtil::StringToTimeInfo( token );
                break;

            case 4:  // RED-Time
				tableInfo.redTime = hoxUtil::StringToTimeInfo( token );
                break;

            case 5:  // BLACK-Time
				tableInfo.blackTime = hoxUtil::StringToTimeInfo( token );
                break;

            case 6:  // RED-Id
                tableInfo.redId = token; 
                break;

            case 7:  // RED-Score
				tableInfo.redScore = token; 
                break;

            case 8:  // BLACK-Id
                tableInfo.blackId = token;
                break;

            case 9:  // BLACK-Score
				tableInfo.blackScore = token;
                break;

			default:
				// Ignore the rest
				break;
        }
    }

    return hoxRC_OK;
}

hoxResult 
hoxNetworkAPI::ParseNewNetworkTable( const wxString&  responseStr,
                                     hoxNetworkTableInfo &tableInfo )
{
    const char* FNAME = "hoxNetworkAPI::ParseNewNetworkTable";
    int returnCode = -1;
	wxString returnMsg;

    wxLogDebug("%s: ENTER.", FNAME);

	tableInfo.Clear();

    wxStringTokenizer tkz( responseStr, wxT("\r\n") );
    int i = 0;

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        switch (i++)
        {
            case 0:   // Return-code.
                returnCode = ::atoi( token.c_str() );
                break;
            case 1:    // The additional informative message.
				returnMsg = token;
                wxLogDebug("%s: Server's message = [%s].", FNAME, returnMsg.c_str()) ; 
                break;
            case 2:    // The ID of the new table.
			{
				hoxNetworkAPI::ParseOneNetworkTable(token, tableInfo);
                break;
			}
            default:
                // Ignore the rest.
                break;
        }
    }

	return (returnCode == 0 ? hoxRC_OK : hoxRC_ERR);
}

hoxResult 
hoxNetworkAPI::ParseJoinNetworkTable( const wxString&      responseStr,
                                      hoxNetworkTableInfo& tableInfo )
{
    const char* FNAME = "hoxNetworkAPI::ParseJoinNetworkTable";
    int        returnCode = -1;
	wxString   returnMsg;

    wxLogDebug("%s: ENTER.", FNAME);

    wxStringTokenizer tkz( responseStr, "\r\n" );
    int i = 0;

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        switch (i++)
        {
            case 0:   // Return-code.
                returnCode = ::atoi( token.c_str() );
                break;
            case 1:    // The additional informative message.
				returnMsg = token;
                wxLogDebug("%s: Server's message = [%s].", FNAME, returnMsg.c_str()) ; 
                break;
            case 2:    // The returned info of the requested table.
            {
				hoxNetworkAPI::ParseOneNetworkTable(token, tableInfo);
                break;
            }
            default:
                // Ignore the rest.
                break;
        }
    }

	return ( returnCode == 0 ? hoxRC_OK : hoxRC_ERR );
}

hoxResult 
hoxNetworkAPI::ParseNetworkEvents( const wxString&      responseStr,
                                   int&                 returnCode,
                                   wxString&            returnMsg,
                                   hoxNetworkEventList& networkEvents )
{
    const char* FNAME = "hoxNetworkAPI::ParseNetworkEvents";
    hoxResult result;

    returnCode = -1;
    returnMsg  = responseStr;

    wxStringTokenizer tkz( responseStr, "\n" );
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
                wxLogDebug("%s: Server's message = [%s].", FNAME, returnMsg.c_str()); 
                break;
            default:
            {
                wxLogDebug("%s: Parsing and then add new event to the list...", FNAME);
                const wxString eventStr = token;
                hoxNetworkEvent networkEvent;
                result = _ParseNetworkEventString( eventStr, networkEvent );
                if ( result != hoxRC_OK ) // failed?
                {
                    wxLogError("%s: Failed to parse network events [%s].", FNAME, eventStr.c_str());
                    return result;
                }
                networkEvents.push_back( new hoxNetworkEvent( networkEvent ) );
                break;
            }
        }
        ++i;
    }

    return hoxRC_OK;
}

hoxResult 
hoxNetworkAPI::ReadCommand( wxSocketBase* sock, 
                            hoxCommand&   command )
{
    const char* FNAME = "hoxNetworkAPI::ReadCommand";
    hoxResult   result = hoxRC_OK;
    wxString    commandStr;

    wxLogDebug("%s: ENTER.", FNAME);
    
    // We disable input events until we are done processing the current command.
    hoxNetworkAPI::SocketInputLock socketLock( sock );

    wxLogDebug("%s: Reading incoming command from the network...", FNAME);
    result = hoxNetworkAPI::ReadLine( sock, commandStr );
    if ( result != hoxRC_OK )
    {
        wxLogError("%s: Failed to read incoming command.", FNAME);
        return hoxRC_ERR;
    }
    wxLogDebug("%s: Received command [%s].", FNAME, commandStr.c_str());

    result = hoxNetworkAPI::ParseCommand( commandStr, command );
    if ( result != hoxRC_OK )
    {
        wxLogError("%s: Failed to parse command-string [%s].", FNAME, commandStr.c_str());
        return hoxRC_ERR;
    }

    return hoxRC_OK;
}


hoxResult
hoxNetworkAPI::ReadLine( wxSocketBase* sock, 
                         wxString&     result )
{
	return hoxNetworkAPI::ReadMsg( sock, result );

#if 0
    const char* FNAME = "hoxNetworkAPI::ReadLine";
    wxString commandStr;

    for (;;)
    {
        wxChar c;

        sock->Read( &c, 1 );
        if ( sock->LastCount() == 1 )
        {
            if ( c == '\n' )
            {
                if ( !commandStr.empty() && commandStr[ commandStr.size()-1 ] == '\r' )
                {
                    result = commandStr.substr(0, commandStr.size()-1);
                    return hoxRC_OK;
                }
            }
            else
            {
                commandStr += c;

                // Impose some limit.
                if ( commandStr.size() >= hoxNETWORK_MAX_MSG_SIZE )
                {
                    wxLogError("%s: Maximum message's size [%d] reached. Likely to be an error.", 
                        FNAME, hoxNETWORK_MAX_MSG_SIZE);
                    wxLogError("%s: Partial read message (64 bytes) = [%s ...].", 
                        FNAME, commandStr.substr(0, 64).c_str());
                    break;
                }
            }
        }
        else if ( sock->Error() )
        {
            wxLogWarning("%s: Fail to read 1 byte from the network. Error = [%s].", 
                FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
            wxLogWarning("%s: Result message accumulated so far = [%s].", FNAME, commandStr.c_str());
            break;
        }
    }

    return hoxRC_ERR;
#endif
}

hoxResult 
hoxNetworkAPI::WriteLine( wxSocketBase*   sock,
                          const wxString& message )
{
    const char* FNAME = "hoxNetworkAPI::WriteLine";
    hoxResult      result = hoxRC_ERR;
    wxUint32       nWrite;

    nWrite = (wxUint32) message.size();
    sock->Write( message, nWrite );
    if ( sock->LastCount() != nWrite )
    {
        wxLogDebug("%s: *** WARN *** Writing to socket failed. Error = [%s]", 
            FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
        goto exit_label;
    }

    result = hoxRC_OK;

exit_label:
    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult 
hoxNetworkAPI::WriteMsg( wxSocketBase*   sock,
                         const wxString& message )
{
    const char* FNAME = "hoxNetworkAPI::WriteMsg";
    hoxResult      result = hoxRC_ERR;
    wxUint32       nWrite;

    //wxLogDebug("%s: ENTER. Message = [%s].", FNAME, message.c_str());

    nWrite = (wxUint32) message.size();
    sock->WriteMsg( message, nWrite );
    if ( sock->LastCount() != nWrite )
    {
        wxLogDebug("%s: *** WARN *** Writing to socket failed. Error = [%s]", 
            FNAME, hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
        goto exit_label;
    }

    result = hoxRC_OK;

exit_label:
    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult
hoxNetworkAPI::ReadMsg( wxSocketBase* sock,
                        wxString&     response )
{
    const char* FNAME = "hoxNetworkAPI::ReadMsg";
    hoxResult result = hoxRC_ERR;  // Assume: failure.
    wxChar*   buf = NULL;
    wxUint32  nRead = 0;

    wxLogDebug("%s: ENTER.", FNAME);

    buf = new wxChar[hoxNETWORK_MAX_MSG_SIZE];
    response = "";

    /* NOTE: Do a ReadMsg operation within a loop because so far this is where
     *       the error sometimes occurs.
     */

    const int MAX_TRIES = 5;   // The number of tries before giving up.
    for ( int tries = 1; tries <= MAX_TRIES; ++tries )
    {
        sock->ReadMsg( buf, hoxNETWORK_MAX_MSG_SIZE );
        nRead = sock->LastCount();
        if ( nRead > 0 )
        {
            wxLogDebug("%s: Received some response data (tries = [%d]). nRead = [%d]. Done reading.", 
                FNAME, tries, nRead);
            break;  // Done reading data.
        }

        if ( sock->Error() ) // Actual IO error occurred?
        {
            wxLogDebug("%s: *** WARN *** Error occurred while reading the response data (tries = [%d]). Error = [%s].", 
                FNAME, tries, hoxNetworkAPI::SocketErrorToString(sock->LastError()).c_str());
            goto exit_label;  // *** Stop trying. Return 'error' immediately.
        }
        wxLogDebug("%s: Receive no response data so far (tries = [%d]). Waiting...", FNAME, tries);
        wxGetApp().Yield( false /* onlyIfNeeded = false */ );
    }

    if ( nRead == 0 )
    {
        wxLogDebug("%s: *** WARN *** Failed to read the response data after [%d] tries.", FNAME, MAX_TRIES);
        goto exit_label;  // *** Stop trying. Return 'error' immediately.
    }

    response.assign( buf, nRead );
    result = hoxRC_OK;  // Finally, success.

exit_label:
    delete[] buf;
    
    wxLogDebug("%s: END.", FNAME);
    return result;
}

/* PRIVATE */
hoxResult 
hoxNetworkAPI::_ParseNetworkEventString( const wxString&  eventStr,
                                         hoxNetworkEvent& networkEvent )
{
    const char* FNAME = "hoxNetworkAPI::_ParseNetworkEventString";
    wxStringTokenizer tkz( eventStr, " " );
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
            case 3:    // Event-type + .. event-content (see NOTE below).
                networkEvent.type = ::atoi( token.c_str() );

                /* NOTE: Due to the fact that white-space characters which may
                 *       be embedded within wall-messages, we will extract the
                 *       rest of the "content" right here.
                 */
                networkEvent.content = tkz.GetString(); // Get the rest of ...
                return hoxRC_OK;  // *** DONE.

            default:
                wxLogError("%s: Ignore the rest...", FNAME);
                break;
        }
        ++i;
    }

    return hoxRC_OK;
}

const wxString 
hoxNetworkAPI::SocketEventToString( const wxSocketNotify socketEvent )
{
    switch( socketEvent )
    {
        case wxSOCKET_INPUT:       return "wxSOCKET_INPUT";
        case wxSOCKET_OUTPUT:      return "wxSOCKET_OUTPUT";
        case wxSOCKET_CONNECTION:  return "wxSOCKET_CONNECTION";
        case wxSOCKET_LOST:        return "wxSOCKET_LOST";

        default:                   return "Unexpected event!";
    }
}

/**
 * Convert a given socket-error to a (human-readable) string.
 */
const wxString 
hoxNetworkAPI::SocketErrorToString( const wxSocketError socketError )
{
    switch( socketError )
    {
        case wxSOCKET_NOERROR:    return "wxSOCKET_NOERROR"; //No error happened
        case wxSOCKET_INVOP:      return "wxSOCKET_INVOP";   // invalid operation
        case wxSOCKET_IOERR:      return "wxSOCKET_IOERR";   // Input/Output error
        case wxSOCKET_INVADDR:    return "wxSOCKET_INVADDR"; // Invalid address passed to wxSocket
        case wxSOCKET_INVSOCK:    return "wxSOCKET_INVSOCK"; // Invalid socket (uninitialized).
        case wxSOCKET_NOHOST:     return "wxSOCKET_NOHOST";  // No corresponding host
        case wxSOCKET_INVPORT:    return "wxSOCKET_INVPORT"; // Invalid port
        case wxSOCKET_WOULDBLOCK: return "wxSOCKET_WOULDBLOCK"; // The socket is non-blocking and the operation would block
        case wxSOCKET_TIMEDOUT:   return "wxSOCKET_TIMEDOUT"; // The timeout for this operation expired
        case wxSOCKET_MEMERR:     return "wxSOCKET_MEMERR";   // Memory exhausted

        default:                  return "Unexpected error!";
    }
}

/************************* END OF FILE ***************************************/
