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

IMPLEMENT_DYNAMIC_CLASS(hoxRemotePlayer, hoxPlayer)

BEGIN_EVENT_TABLE(hoxRemotePlayer, hoxPlayer)
    EVT_SOCKET(CLIENT_SOCKET_ID,  hoxRemotePlayer::OnIncomingNetworkData)
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
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_PLAYER_DATA );
        request->socket      = event.GetSocket();
            /* Set the socket here so that the connection can perform 
             * sanity check to make sure the socket matches.
             */

        request->socketEvent = event.GetSocketEvent(); // TODO: Not consistent???
        m_connection->AddRequest( request );
    }
}


/************************* END OF FILE ***************************************/
