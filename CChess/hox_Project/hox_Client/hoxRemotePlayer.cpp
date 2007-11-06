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

void 
hoxRemotePlayer::OnNewMove_FromTable( hoxPlayerEvent&  event )
{
    const char* FNAME = "hoxRemotePlayer::OnNewMove_FromTable";
    wxString     tableId     = event.GetTableId();
    hoxPosition  moveFromPos = event.GetOldPosition();
    hoxPosition  moveToPos   = event.GetPosition();

    wxString moveStr = wxString::Format("%d%d%d%d", 
                            moveFromPos.x, moveFromPos.y, moveToPos.x, moveToPos.y);

    wxLogDebug("%s: ENTER. Move = [%s].", FNAME, moveStr);

    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_MOVE );
        request->content =
                wxString::Format("op=MOVE&tid=%s&pid=%s&move=%s\r\n", 
                            tableId, this->GetName(), moveStr);
        m_connection->AddRequest( request );
    }
}

void 
hoxRemotePlayer::OnWallMsg_FromTable( wxCommandEvent&  event )
{
    const char* FNAME = "hoxRemotePlayer::OnWallMsg_FromTable";

    const wxString commandStr = event.GetString();

    wxLogDebug("%s: ENTER. commandStr = [%s].", FNAME, commandStr);

    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_WALL_MSG );
        request->content =
                wxString::Format("op=WALL_MSG&%s\r\n", commandStr);
        m_connection->AddRequest( request );
    }
}


/************************* END OF FILE ***************************************/
