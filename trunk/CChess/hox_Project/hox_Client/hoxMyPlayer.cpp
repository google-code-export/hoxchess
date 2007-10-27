/////////////////////////////////////////////////////////////////////////////
// Name:            hoxMyPlayer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/23/2007
//
// Description:     The new "advanced" LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxMyPlayer.h"
#include "hoxEnums.h"
#include "hoxTable.h"
#include "hoxBoard.h"
#include "hoxConnection.h"

IMPLEMENT_DYNAMIC_CLASS( hoxMyPlayer, hoxPlayer )

BEGIN_EVENT_TABLE(hoxMyPlayer, hoxPlayer)
    EVT_SOCKET(CLIENT_SOCKET_ID,  hoxMyPlayer::OnIncomingNetworkData)
END_EVENT_TABLE()


//-----------------------------------------------------------------------------
// hoxMyPlayer
//-----------------------------------------------------------------------------

hoxMyPlayer::hoxMyPlayer()
            : hoxPlayer( _("Unknown"), 
                         hoxPLAYER_TYPE_LOCAL, 
                         1500 )
            , m_connection( NULL )
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxMyPlayer::hoxMyPlayer( const wxString& name,
                          hoxPlayerType   type,
                          int             score /* = 1500 */)
            : hoxPlayer( name, type, score )
            , m_connection( NULL )
{ 
    const char* FNAME = "hoxMyPlayer::hoxMyPlayer";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxMyPlayer::~hoxMyPlayer() 
{
    const char* FNAME = "hoxMyPlayer::~hoxMyPlayer";

    if ( m_connection != NULL )
    {
        wxLogDebug("%s: Request the Connection thread to be shutdowned...", FNAME);
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_SHUTDOWN, NULL );
        m_connection->AddRequest( request );
        wxThread::ExitCode exitCode = m_connection->GetThread()->Wait();
        wxLogDebug("%s: The Connection thread was shutdowned with exit-code = [%d].", FNAME, exitCode);
        delete m_connection;
    }
}

hoxResult 
hoxMyPlayer::ConnectToNetworkServer( const wxString& sHostname, 
                                     int             nPort,
                                     wxEvtHandler*   sender )
{
    m_sHostname = sHostname;
    m_nPort = nPort;

    _StartConnection();

    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_CONNECT, sender );
        request->content = 
            wxString::Format("op=HELLO\r\n");
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxMyPlayer::QueryForNetworkTables( wxEvtHandler* sender )
{
    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_LIST, sender );
        request->content = 
            wxString::Format("op=LIST\r\n");
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxMyPlayer::JoinNetworkTable( const wxString& tableId,
                               wxEvtHandler*   sender )
{
    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_JOIN, sender );
        request->content = 
            wxString::Format("op=JOIN&tid=%s&pid=%s\r\n", tableId, this->GetName());
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
}
hoxResult 
hoxMyPlayer::OpenNewNetworkTable( wxEvtHandler*   sender )
{
    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_NEW, sender );
        request->content = 
            wxString::Format("op=NEW&pid=%s\r\n", this->GetName());
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxMyPlayer::LeaveNetworkTable( const wxString& tableId,
                                wxEvtHandler*   sender )
{
    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_LEAVE, sender );
        request->content = 
                wxString::Format("op=LEAVE&tid=%s&pid=%s\r\n", tableId, this->GetName());
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxMyPlayer::StartListenForMoves()
{
    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_LISTEN, this );
        request->content = "";
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
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

void 
hoxMyPlayer::OnNewMove_FromTable( hoxPlayerEvent&  event )
{
    const char* FNAME = "hoxMyPlayer::OnNewMove_FromTable";
    wxString     tableId     = event.GetTableId();
    hoxPosition  moveFromPos = event.GetOldPosition();
    hoxPosition  moveToPos   = event.GetPosition();

    wxString moveStr = wxString::Format("%d%d%d%d", 
                            moveFromPos.x, moveFromPos.y, moveToPos.x, moveToPos.y);

    wxLogDebug("%s: ENTER. move = [%s].", FNAME, moveStr);

    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_TABLE_MOVE, NULL /* this */ );
        request->content = 
                wxString::Format("op=TABLE_MOVE&tid=%s&pid=%s&move=%s\r\n", 
                            tableId, this->GetName(), moveStr);
        m_connection->AddRequest( request );
    }
}

void 
hoxMyPlayer::_StartConnection()
{
    const char* FNAME = "hoxMyPlayer::_StartConnection";
    wxASSERT_MSG( m_connection == NULL, "The connection should not have been created.");

    wxASSERT_MSG( !m_sHostname.empty(), "Hostname must have been set." );
    m_connection = new hoxConnection( m_sHostname, m_nPort, this );

    if ( m_connection->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError(wxString::Format("%s: Failed to create Connection.", FNAME));
        return;
    }
    wxASSERT_MSG( !m_connection->GetThread()->IsDetached(), "The Connection thread must be joinable.");

    m_connection->GetThread()->Run();
}

/************************* END OF FILE ***************************************/
