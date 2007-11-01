/////////////////////////////////////////////////////////////////////////////
// Name:            hoxRemotePlayer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         11/01/2007
//
// Description:     The REMOTE Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxRemotePlayer.h"
#include "hoxEnums.h"
#include "hoxServer.h"

IMPLEMENT_DYNAMIC_CLASS( hoxRemotePlayer, hoxPlayer )

BEGIN_EVENT_TABLE(hoxRemotePlayer, hoxPlayer)
    EVT_SOCKET(CLIENT_SOCKET_ID,  hoxRemotePlayer::OnIncomingNetworkData)
END_EVENT_TABLE()


//-----------------------------------------------------------------------------
// hoxRemotePlayer
//-----------------------------------------------------------------------------

hoxRemotePlayer::hoxRemotePlayer()
            : hoxPlayer()
            , m_pCBSock( NULL )
            , m_server( NULL )
{ 
    wxFAIL_MSG("This is not meant to be called.");
}

hoxRemotePlayer::hoxRemotePlayer( const wxString& name,
                                    hoxPlayerType   type,
                                    int             score )
            : hoxPlayer( name, type, score )
            , m_pCBSock( NULL )
            , m_server( NULL )
{ 
}

hoxRemotePlayer::~hoxRemotePlayer() 
{
    this->DisconnectFromNetwork();
}

void
hoxRemotePlayer::OnIncomingNetworkData( wxSocketEvent& event )
{
    const char* FNAME = "hoxRemotePlayer::OnIncomingNetworkData";
    wxLogDebug("%s: ENTER.", FNAME);

    wxASSERT( m_server != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_PLAYER_DATA );
        request->socket      = event.GetSocket();
        request->socketEvent = event.GetSocketEvent();
        m_server->AddRequest( request );
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

    wxLogDebug("%s: ENTER. move = [%s].", FNAME, moveStr);

    wxASSERT( m_server != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_TABLE_MOVE );
        request->content =     /* NOTE: Send "MOVE, not "TABLE_MOVE" string */
                wxString::Format("op=MOVE&tid=%s&pid=%s&move=%s\r\n", 
                            tableId, this->GetName(), moveStr);
        request->socket = this->m_pCBSock;
        m_server->AddRequest( request );
    }
}

hoxResult 
hoxRemotePlayer::SetCBSocket( wxSocketBase* socket )
{
    const char* FNAME = "hoxRemotePlayer::SetCBSocket";

    if ( m_pCBSock != NULL )
    {
        wxLogError("%s: Callback socket already exists.", FNAME);
        return hoxRESULT_ERR;
    }

    wxLogDebug("%s: Assign callback socket to this user [%s]", FNAME, GetName());
    m_pCBSock = socket;
    return hoxRESULT_OK;
}

hoxResult 
hoxRemotePlayer::DisconnectFromNetwork()
{
    const char* FNAME = "hoxRemotePlayer::DisconnectFromNetwork";

    wxLogDebug("%s: ENTER. (*** but do nothing ***).", FNAME);

    //if ( m_pCBSock != NULL )
    //{
    //    m_pCBSock->Destroy();
    //    m_pCBSock = NULL;
    //}

    return hoxRESULT_OK;
}


/************************* END OF FILE ***************************************/
