/////////////////////////////////////////////////////////////////////////////
// Name:            hoxMyPlayer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/23/2007
//
// Description:     The new "advanced" LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxMyPlayer.h"
#include "hoxSocketConnection.h"
#include "hoxEnums.h"


DEFINE_EVENT_TYPE(hoxEVT_CONNECTION_RESPONSE)

IMPLEMENT_DYNAMIC_CLASS( hoxMyPlayer, hoxLocalPlayer )

BEGIN_EVENT_TABLE(hoxMyPlayer, hoxLocalPlayer)
    EVT_SOCKET(CLIENT_SOCKET_ID,  hoxMyPlayer::OnIncomingNetworkData)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxMyPlayer
//-----------------------------------------------------------------------------

hoxMyPlayer::hoxMyPlayer()
            : hoxLocalPlayer( "Unknown", 
                               hoxPLAYER_TYPE_LOCAL, 
                               1500 )
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

hoxConnection* 
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

    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_PLAYER_DATA );
        request->socket      = event.GetSocket();
        request->socketEvent = event.GetSocketEvent();
        m_connection->AddRequest( request );
    }
}

hoxResult 
hoxMyPlayer::StartListenForMoves()
{
    wxASSERT( m_connection != NULL );
    {
        /* NOTE: We set 'this' player as the 'sender' so that the player can 
         *       be a socket-event handler.
         *       However, the 'connection' will NOT send back a response for
         *       this particular request. Therefore, there is no need to
         *       'catch' a response.
         */

        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_LISTEN, this );
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
}


/************************* END OF FILE ***************************************/
