/////////////////////////////////////////////////////////////////////////////
// Name:            hoxRemotePlayer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         11/01/2007
//
// Description:     The REMOTE Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxRemotePlayer.h"
#include "hoxEnums.h"
#include "hoxConnection.h"
#include "MyApp.h"

IMPLEMENT_DYNAMIC_CLASS(hoxRemotePlayer, hoxPlayer)

BEGIN_EVENT_TABLE(hoxRemotePlayer, hoxPlayer)
    EVT_SOCKET(CLIENT_SOCKET_ID,  hoxRemotePlayer::OnIncomingNetworkData)
    EVT_COMMAND(hoxREQUEST_TYPE_PLAYER_DATA, hoxEVT_SERVER_RESPONSE, hoxRemotePlayer::OnConnectionResponse_PlayerData)
END_EVENT_TABLE()


//-----------------------------------------------------------------------------
// hoxRemotePlayer
//-----------------------------------------------------------------------------

hoxRemotePlayer::hoxRemotePlayer()
{ 
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxRemotePlayer::hoxRemotePlayer( const wxString& name,
                                  hoxPlayerType   type,
                                  int             score )
            : hoxPlayer( name, type, score )
{ 
}

hoxRemotePlayer::~hoxRemotePlayer() 
{
}

void
hoxRemotePlayer::OnIncomingNetworkData( wxSocketEvent& event )
{
    const char* FNAME = "hoxRemotePlayer::OnIncomingNetworkData";
    wxLogDebug("%s: ENTER.", FNAME);

    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_PLAYER_DATA, this );
        request->socket      = event.GetSocket();
            /* Set the socket here so that the connection can perform 
             * sanity check to make sure the socket matches.
             */

        request->socketEvent = event.GetSocketEvent(); // TODO: Not consistent???
        m_connection->AddRequest( request );
    }
}

void 
hoxRemotePlayer::OnConnectionResponse_PlayerData( wxCommandEvent& event )
{
    const char* FNAME = "hoxRemotePlayer::OnConnectionResponse_PlayerData";
    hoxResult result = hoxRESULT_OK;

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
    }
    else
    {
        /* Handle other type of data. */

        result = this->HandleIncomingData( response->content );
        if ( result != hoxRESULT_OK )
        {
            wxLogError("%s: Error occurred while handling incoming data.", FNAME);
            return;
        }
    }

    wxLogDebug("%s: END.", FNAME);
}

/************************* END OF FILE ***************************************/
