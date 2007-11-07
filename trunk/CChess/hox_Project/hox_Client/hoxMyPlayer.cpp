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
    EVT_COMMAND(hoxREQUEST_TYPE_PLAYER_DATA, hoxEVT_CONNECTION_RESPONSE, hoxMyPlayer::OnConnectionResponse_PlayerData)
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

void
hoxMyPlayer::OnIncomingNetworkData( wxSocketEvent& event )
{
    const char* FNAME = "hoxMyPlayer::OnIncomingNetworkData";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_PLAYER_DATA, this );
    request->socket      = event.GetSocket();
    request->socketEvent = event.GetSocketEvent();
    this->AddRequestToConnection( request );
}

void 
hoxMyPlayer::OnConnectionResponse_PlayerData( wxCommandEvent& event )
{
    const char* FNAME = "hoxMyPlayer::OnConnectionResponse_PlayerData";

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    /* NOTE: Only handle the connection-lost event. */

    if ( (response->flags & hoxRESPONSE_FLAG_CONNECTION_LOST) !=  0 )
    {
        wxLogDebug("%s: Connection has been lost.", FNAME);
        /* Currently, we support one connection per player.
         * Since this ONLY connection is closed, the player must leave
         * all tables.
         */
        this->LeaveAllTables();
        //while ( ! m_roles.empty() )
        //{
        //    const wxString tableId = m_roles.front().tableId;
        //    m_roles.pop_front();

        //    // Find the table hosted on this system using the specified table-Id.
        //    hoxTable* table = hoxTableMgr::GetInstance()->FindTable( tableId );
        //    if ( table == NULL )
        //    {
        //        wxLogError("%s: Failed to find table with ID = [%s].", FNAME, tableId);
        //        continue;
        //    }

        //    // Inform the table that this player is leaving...
        //    table->OnLeave_FromPlayer( this );
        //}
    }

    wxLogDebug("%s: END.", FNAME);
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
