/////////////////////////////////////////////////////////////////////////////
// Name:            hoxNetworkAPI.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/26/2007
//
// Description:     Containing network related APIs specific to this project.
/////////////////////////////////////////////////////////////////////////////

#include "hoxNetworkAPI.h"
#include "hoxUtility.h"
#include "hoxPlayer.h"
#include "hoxTableMgr.h"

#include <wx/tokenzr.h>

//-----------------------------------------------------------------------------
// hoxNetworkAPI namespace
//-----------------------------------------------------------------------------

hoxResult   
hoxNetworkAPI::SendMove( wxSocketBase*   sock, 
                         const wxString& commandInput )
{
    const char* FNAME = "hoxNetworkAPI::SendMove";

    hoxResult     result;
    wxString      commandStr;
    wxUint32      requestSize;
    wxString      request;
    wxUint32      nWrite;
    wxUint32      nRead;
    wxString      responseStr;
    int           returnCode = -1;
    wxString      returnMsg;
    hoxCommand    command;
    wxString      tableId;
    wxString      playerId;
    wxString      moveStr;
    wxChar*       buf = NULL;

    wxLogDebug("%s: ENTER.", FNAME);    

    // We disable input events until we are done processing the current command.
    sock->SetNotify(wxSOCKET_LOST_FLAG); // remove the wxSOCKET_INPUT_FLAG!!!

    // Remove new-line characters.
    commandStr = commandInput;
    commandStr = commandStr.Trim();

    result = hoxNetworkAPI::ParseCommand( commandStr, command );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse command-string [%s].", FNAME, commandStr);
        result = hoxRESULT_ERR;
        goto exit_label;
    }

    tableId  = command.parameters["tid"];
    playerId = command.parameters["pid"];
    moveStr  = command.parameters["move"];

    request = wxString::Format("op=MOVE&tid=%s&pid=%s&move=%s\r\n", 
                        tableId, playerId, moveStr);

    // Send request.
    wxLogDebug("%s: Sending the request [%s] over the network...", FNAME, request);
    requestSize = (wxUint32) request.size();
    sock->Write( request, requestSize );
    nWrite = sock->LastCount();
    if ( nWrite < requestSize )
    {
        wxLogError("%s: Failed to send request [%s] ( %d < %d ). Error = [%s].", 
            FNAME, request, nWrite, requestSize, 
            hoxUtility::SocketErrorToString(sock->LastError()));
        result = hoxRESULT_ERR;
        goto exit_label;
    }

#if 0
    // Wait until data available (will also return if the connection is lost)
    wxLogDebug(wxString::Format("%s: Waiting for response from the network (timeout = 2 sec)...", FNAME));
    sock->WaitForRead( 2 /* seconds */ );
#endif

    // Read back the response.
    wxLogDebug("%s: Reading back the response from the network...", FNAME);
    buf = new wxChar[hoxNETWORK_MAX_MSG_SIZE];

    sock->ReadMsg( buf, hoxNETWORK_MAX_MSG_SIZE );
    nRead = sock->LastCount();
    if ( nRead == 0 )
    {
        wxLogError("%s: Failed to read response. Error = [%s].", 
            FNAME, hoxUtility::SocketErrorToString(sock->LastError()));
        result = hoxRESULT_ERR;
        goto exit_label;
    }

    responseStr.assign( buf, nRead );

    /* Parse the response */
    result = hoxNetworkAPI::ParseSimpleResponse( responseStr,
                                                 returnCode,
                                                 returnMsg );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse the SEND-MOVE's response.", FNAME);
        result = hoxRESULT_ERR;
        goto exit_label;
    }
    else if ( returnCode != 0 )
    {
        wxLogError("%s: Send MOVE failed. [%s]", FNAME, returnMsg);
        result = hoxRESULT_ERR;
        goto exit_label;
    }

    result = hoxRESULT_OK;

exit_label:
    delete[] buf;
    // Enable the input flag again.
    sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);

    return result;
}

hoxResult   
hoxNetworkAPI::HandleMove( wxSocketBase* sock,
                           hoxCommand&   command )
{
    const char* FNAME = "hoxNetworkAPI::HandleMove";
    hoxResult       result = hoxRESULT_ERR;   // Assume: failure.
    wxUint32        nWrite;
    wxString        response;
    hoxNetworkEvent networkEvent;

    wxString moveStr = command.parameters["move"];
    wxString tableId = command.parameters["tid"];
    wxString playerId = command.parameters["pid"];
    hoxPlayer* player = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

    // Find the table hosted on this system using the specified table-Id.
    hoxTable* table = hoxTableMgr::GetInstance()->FindTable( tableId );

    if ( table == NULL )
    {
        wxLogError("%s: Table [%s] not found.", FNAME, tableId);
        response << "1\r\n"  // code
                 << "Table " << tableId << " not found.\r\n";
        goto exit_label;
    }

    /* Look up player */

    if ( table->GetRedPlayer() != NULL && table->GetRedPlayer()->GetName() == playerId )
    {
        player = table->GetRedPlayer();
    }
    else if ( table->GetBlackPlayer() != NULL && table->GetBlackPlayer()->GetName() == playerId )
    {
        player = table->GetBlackPlayer();
    }
    else
    {
        wxLogError("%s: Player [%s] not found at the table [%s].", 
            FNAME, playerId, tableId);
        response << "2\r\n"  // code
                 << "Player " << playerId << " not found.\r\n";
        goto exit_label;
    }

    networkEvent.content = moveStr;
    networkEvent.type = hoxNETWORK_EVENT_TYPE_NEW_MOVE;

    // Inform our table...
    table->OnEvent_FromWWWNetwork( player, networkEvent );

    // Finally, return 'success'.
    response << "0\r\n"       // error-code = SUCCESS
             << "INFO: (MOVE) Move at Table [" << tableId << "] OK\r\n";

    result = hoxRESULT_OK;

exit_label:
    // Send back response.
    nWrite = (wxUint32) response.size();
    sock->WriteMsg( response, nWrite );
    if ( sock->LastCount() != nWrite )
    {
        wxLogError("%s: Writing to socket failed. Error = [%s]", 
            FNAME, hoxUtility::SocketErrorToString(sock->LastError()));
        result = hoxRESULT_ERR;
    }

    wxLogDebug("%s: END.", FNAME);
    return result;
}

hoxResult 
hoxNetworkAPI::HandleLeave( wxSocketBase*  sock,
                            hoxCommand&    command )
{
    const char* FNAME = "hoxNetworkAPI::HandleLeave";
    hoxResult       result = hoxRESULT_ERR;   // Assume: failure.
    wxUint32        nWrite;
    wxString        response;
    wxString        tableId;
    wxString        requesterId;
    hoxNetworkEvent networkEvent;
    hoxPlayer*      player = NULL;

    wxLogDebug("%s: ENTER.", FNAME);

    tableId = command.parameters["tid"];
    requesterId = command.parameters["pid"];

    // Find the table hosted on this system using the specified table-Id.
    hoxTable* table = hoxTableMgr::GetInstance()->FindTable( tableId );

    if ( table == NULL )
    {
        wxLogError(wxString::Format("%s: Table [%s] not found.", FNAME, tableId));
        response << "1\r\n"  // code
                 << "Table " << tableId << " not found.\r\n";
        goto exit_label;
    }

    /* Create a LEAVE-event and send it to the table. */

    if ( table->GetRedPlayer() != NULL && table->GetRedPlayer()->GetName() == requesterId )
    {
        networkEvent.type = hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_RED;
        player = table->GetRedPlayer();
    }
    else if ( table->GetBlackPlayer() != NULL && table->GetBlackPlayer()->GetName() == requesterId )
    {
        networkEvent.type = hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_BLACK;
        player = table->GetBlackPlayer();
    }
    else
    {
        wxLogError(wxString::Format("%s: Player [%s] not found at the table [%s].", FNAME, requesterId, tableId));
        response << "2\r\n"  // code
                 << "Player " << requesterId << " not found at the table " << tableId << ".\r\n";
        goto exit_label;
    }

    networkEvent.content = requesterId;

    // Inform our table...
    table->OnEvent_FromWWWNetwork( player, networkEvent );

	// Finally, return 'success'.
	response << "0\r\n"       // error-code = SUCCESS
	         << "INFO: (LEAVE) Leave Table [" << tableId << "] OK\r\n";

    result = hoxRESULT_OK;

exit_label:
    // Send back response.
    nWrite = (wxUint32) response.size();
    sock->WriteMsg( response, nWrite );
    if ( sock->LastCount() != nWrite )
    {
        wxLogError("%s: Writing to socket failed. Error = [%s]", 
            FNAME, hoxUtility::SocketErrorToString(sock->LastError()));
        result = hoxRESULT_ERR;
    }

    wxLogDebug("%s: END.", FNAME);
    return result;
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
            if ( paramValue == "HELLO" )
            {
                command.type = hoxREQUEST_TYPE_CONNECT;
            }
            else if ( paramValue == "LIST" )
            {
                command.type = hoxREQUEST_TYPE_LIST;
            }
            else if ( paramValue == "JOIN" )
            {
                command.type = hoxREQUEST_TYPE_JOIN;
            }
            else if ( paramValue == "NEW" )
            {
                command.type = hoxREQUEST_TYPE_NEW;
            }
            else if ( paramValue == "LEAVE" )
            {
                command.type = hoxREQUEST_TYPE_LEAVE;
            }
            else if ( paramValue == "MOVE" )
            {
                command.type = hoxREQUEST_TYPE_MOVE;
            }
            else if ( paramValue == "TABLE_MOVE" )
            {
                command.type = hoxREQUEST_TYPE_TABLE_MOVE;
            }
            else
            {
                wxLogError("%s: Unsupported command-type = [%s].", FNAME, paramValue);
                return hoxRESULT_NOT_SUPPORTED;
            }
        }
        else
        {
            command.parameters[paramName] = paramValue;
        }
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxNetworkAPI::ParseSimpleResponse( const wxString& responseStr,
                                    int&            returnCode,
                                    wxString&       returnMsg )
{
    const char* FNAME = "hoxNetworkAPI::ParseSimpleResponse";

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

/************************* END OF FILE ***************************************/
