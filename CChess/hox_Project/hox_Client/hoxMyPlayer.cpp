/////////////////////////////////////////////////////////////////////////////
// Name:            hoxMyPlayer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/28/2007
//
// Description:     The new "advanced" LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxMyPlayer.h"
#include "hoxSocketConnection.h"
#include "hoxEnums.h"
#include "hoxNetworkAPI.h"

IMPLEMENT_DYNAMIC_CLASS(hoxMyPlayer, hoxLocalPlayer)

DEFINE_EVENT_TYPE(hoxEVT_CONNECTION_RESPONSE)

BEGIN_EVENT_TABLE(hoxMyPlayer, hoxLocalPlayer)
    EVT_SOCKET(CLIENT_SOCKET_ID,  hoxMyPlayer::OnIncomingNetworkData)
    EVT_COMMAND(wxID_ANY, hoxEVT_CONNECTION_RESPONSE, hoxMyPlayer::OnConnectionResponse)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxMyPlayer
//-----------------------------------------------------------------------------

hoxMyPlayer::hoxMyPlayer()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxMyPlayer::hoxMyPlayer( const wxString& name,
                          hoxPlayerType   type,
                          int             score )
            : hoxLocalPlayer( name, type, score )
{ 
    const char* FNAME = "hoxMyPlayer::hoxMyPlayer";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxMyPlayer::~hoxMyPlayer() 
{
    const char* FNAME = "hoxMyPlayer::~hoxMyPlayer";
    wxLogDebug("%s: ENTER.", FNAME);
}

const wxString  
hoxMyPlayer::BuildRequestContent( const wxString& commandStr )
{ 
    return wxString::Format("%s\r\n", commandStr); 
}

hoxThreadConnection* 
hoxMyPlayer::CreateNewConnection( const wxString& sHostname, 
                                  int             nPort )
{
    return new hoxSocketConnection( sHostname, nPort );
}

void
hoxMyPlayer::OnIncomingNetworkData( wxSocketEvent& event )
{
    const char* FNAME = "hoxMyPlayer::OnIncomingNetworkData";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_PLAYER_DATA );
    request->socket      = event.GetSocket();
    request->socketEvent = event.GetSocketEvent();
    this->AddRequestToConnection( request );
}

void 
hoxMyPlayer::OnConnectionResponse( wxCommandEvent& event )
{
    const char* FNAME = "hoxMyPlayer::OnConnectionResponse";
    hoxResult     result;
    int           returnCode = -1;
    wxString      returnMsg;

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    /* Parse the response */
    result = hoxNetworkAPI::ParseSimpleResponse( response->content,
                                                 returnCode,
                                                 returnMsg );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse the response.", FNAME);
        return;
    }
    else if ( returnCode != 0 )
    {
        wxLogError("%s: The response returned error: code = [%d], msg = [%s]", 
            FNAME, returnCode, returnMsg);
        return;
    }

    wxLogDebug("%s: The response is OK.", FNAME);
}

hoxResult 
hoxMyPlayer::StartListenForMoves()
{
    /* NOTE: We set 'this' player as the 'sender' so that the player can 
     *       be a socket-event handler.
     *       However, the 'connection' will NOT send back a response for
     *       this particular request. Therefore, there is no need to
     *       'catch' a response.
     */

    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_LISTEN, this );
    this->AddRequestToConnection( request );

    return hoxRESULT_OK;
}


/************************* END OF FILE ***************************************/
