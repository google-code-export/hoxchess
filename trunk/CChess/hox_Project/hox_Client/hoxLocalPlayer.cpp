/////////////////////////////////////////////////////////////////////////////
// Name:            hoxLocalPlayer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/28/2007
//
// Description:     The LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxLocalPlayer.h"
#include "hoxThreadConnection.h"
#include "hoxEnums.h"

IMPLEMENT_ABSTRACT_CLASS(hoxLocalPlayer, hoxPlayer)

//-----------------------------------------------------------------------------
// hoxLocalPlayer
//-----------------------------------------------------------------------------

hoxLocalPlayer::hoxLocalPlayer()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxLocalPlayer::hoxLocalPlayer( const wxString& name,
                                hoxPlayerType   type,
                                int             score )
            : hoxPlayer( name, type, score )
{ 
    const char* FNAME = "hoxLocalPlayer::hoxLocalPlayer";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxLocalPlayer::~hoxLocalPlayer() 
{
    const char* FNAME = "hoxLocalPlayer::~hoxLocalPlayer";

    if ( m_connection != NULL )
    {
        wxLogDebug("%s: Request the Connection thread to be shutdowned...", FNAME);
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_SHUTDOWN, NULL );
        m_connection->AddRequest( request );

        m_connection->Shutdown();
    }
}

void 
hoxLocalPlayer::OnClose_FromTable( hoxPlayerEvent&  event )
{
    const char* FNAME = "hoxLocalPlayer::OnClose_FromTable";

    wxLogDebug("%s: ENTER.", FNAME);

    this->LeaveNetworkTable( event.GetTableId(), this /* NOT USED */ );

    this->hoxPlayer::OnClose_FromTable( event );
}

void 
hoxLocalPlayer::OnNewMove_FromTable( hoxPlayerEvent&  event )
{
    const char* FNAME = "hoxLocalPlayer::OnNewMove_FromTable";
    wxString     tableId     = event.GetTableId();
    hoxPosition  moveFromPos = event.GetOldPosition();
    hoxPosition  moveToPos   = event.GetPosition();

    wxString moveStr = wxString::Format("%d%d%d%d", 
                            moveFromPos.x, moveFromPos.y, moveToPos.x, moveToPos.y);

    wxLogDebug("%s: ENTER. Move = [%s].", FNAME, moveStr);

    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_MOVE, this );
        wxString commandStr =
                wxString::Format("op=MOVE&tid=%s&pid=%s&move=%s", 
                            tableId, this->GetName(), moveStr);
        request->content = this->BuildRequestContent( commandStr );
        m_connection->AddRequest( request );
    }
}

void 
hoxLocalPlayer::OnWallMsg_FromTable( wxCommandEvent&  event )
{
    const char* FNAME = "hoxLocalPlayer::OnWallMsg_FromTable";

    const wxString commandStr = event.GetString();

    wxLogDebug("%s: ENTER. commandStr = [%s].", FNAME, commandStr);

    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_WALL_MSG, this );
        request->content =
                wxString::Format("op=WALL_MSG&%s\r\n", commandStr);
        m_connection->AddRequest( request );
    }
}

hoxResult 
hoxLocalPlayer::ConnectToNetworkServer( const wxString& sHostname, 
                                        int             nPort,
                                        wxEvtHandler*   sender )
{
    m_sHostname = sHostname;
    m_nPort = nPort;

    _StartConnection();

    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_CONNECT, sender );
        wxString commandStr = 
            wxString::Format("op=CONNECT");
        request->content = this->BuildRequestContent( commandStr );
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxLocalPlayer::QueryForNetworkTables( wxEvtHandler* sender )
{
    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_LIST, sender );
        wxString commandStr = 
            wxString::Format("op=LIST");
        request->content = this->BuildRequestContent( commandStr );
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxLocalPlayer::JoinNetworkTable( const wxString& tableId,
                                  wxEvtHandler*   sender )
{
    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_JOIN, sender );
        wxString commandStr = 
            wxString::Format("op=JOIN&tid=%s&pid=%s", tableId, this->GetName());
        request->content = this->BuildRequestContent( commandStr );
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
}
hoxResult 
hoxLocalPlayer::OpenNewNetworkTable( wxEvtHandler*   sender )
{
    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_NEW, sender );
        wxString commandStr = 
            wxString::Format("op=NEW&pid=%s", this->GetName());
        request->content = this->BuildRequestContent( commandStr );
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxLocalPlayer::LeaveNetworkTable( const wxString& tableId,
                                   wxEvtHandler*   sender /* NOT USED */ )
{
    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_LEAVE /*, sender*/ );
        wxString commandStr = 
                wxString::Format("op=LEAVE&tid=%s&pid=%s", tableId, this->GetName());
        request->content = this->BuildRequestContent( commandStr );
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
}

void 
hoxLocalPlayer::AddRequestToConnection( hoxRequest* request )
{ 
    wxCHECK_RET( m_connection, "The connection must have been set." );
    m_connection->AddRequest( request ); 
}

void 
hoxLocalPlayer::_StartConnection()
{
    const char* FNAME = "hoxLocalPlayer::_StartConnection";

    wxCHECK_RET( m_connection, "The connection must have been set." );
    wxCHECK_RET( !m_sHostname.empty(), "Hostname must have been set." );

    m_connection->Start();
}

/************************* END OF FILE ***************************************/
