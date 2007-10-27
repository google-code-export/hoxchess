/////////////////////////////////////////////////////////////////////////////
// Name:            hoxNetworkPlayer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/09/2007
//
// Description:     The NETWORK Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxNetworkPlayer.h"
#include "hoxEnums.h"
#include "hoxServer.h"

// user code intercepting the event
IMPLEMENT_DYNAMIC_CLASS( hoxNetworkPlayer, hoxPlayer )

BEGIN_EVENT_TABLE(hoxNetworkPlayer, hoxPlayer)
    EVT_SOCKET(CLIENT_SOCKET_ID,  hoxNetworkPlayer::OnIncomingNetworkData)
END_EVENT_TABLE()


//-----------------------------------------------------------------------------
// hoxNetworkPlayer
//-----------------------------------------------------------------------------

hoxNetworkPlayer::hoxNetworkPlayer()
            : hoxPlayer( _("Unknown"), 
                         hoxPLAYER_TYPE_NETWORK, 
                         1500 )
            , m_pCBSock( NULL )
            , m_server( NULL )
{ 
    wxFAIL_MSG("This is not meant to be called.");
}

hoxNetworkPlayer::hoxNetworkPlayer( const wxString& name,
                                    hoxPlayerType   type,
                                    int             score /* = 1500 */)
            : hoxPlayer( name, type, score )
            , m_pCBSock( NULL )
            , m_server( NULL )
{ 
}

hoxNetworkPlayer::~hoxNetworkPlayer() 
{
    this->DisconnectFromNetwork();
}

void
hoxNetworkPlayer::OnIncomingNetworkData( wxSocketEvent& event )
{
    const char* FNAME = "hoxNetworkPlayer::OnIncomingNetworkData";
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
hoxNetworkPlayer::OnNewMove_FromTable( hoxPlayerEvent&  event )
{
    const char* FNAME = "hoxNetworkPlayer::OnNewMove_FromTable";
    wxString     tableId     = event.GetTableId();
    hoxPosition  moveFromPos = event.GetOldPosition();
    hoxPosition  moveToPos   = event.GetPosition();

    wxString moveStr = wxString::Format("%d%d%d%d", 
                            moveFromPos.x, moveFromPos.y, moveToPos.x, moveToPos.y);

    wxLogDebug("%s: ENTER. move = [%s].", FNAME, moveStr);

    wxASSERT( m_server != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_TABLE_MOVE, NULL /* this */ );
        request->content = 
                wxString::Format("op=TABLE_MOVE&tid=%s&pid=%s&move=%s\r\n", 
                            tableId, this->GetName(), moveStr);
        request->socket = this->m_pCBSock;
        m_server->AddRequest( request );
    }
}

hoxResult 
hoxNetworkPlayer::SetCBSocket( wxSocketBase* socket )
{
    const char* FNAME = "hoxNetworkPlayer::SetCBSocket";

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
hoxNetworkPlayer::DisconnectFromNetwork()
{
    const char* FNAME = "hoxNetworkPlayer::DisconnectFromNetwork";

    wxLogDebug("%s: ENTER. (*** but do nothing ***).", FNAME);

    //if ( m_pCBSock != NULL )
    //{
    //    m_pCBSock->Destroy();
    //    m_pCBSock = NULL;
    //}

    return hoxRESULT_OK;
}


/************************* END OF FILE ***************************************/
