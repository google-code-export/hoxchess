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

bool 
hoxLocalPlayer::SetConnection( hoxConnection* connection )
{
    const char* FNAME = "hoxLocalPlayer::SetConnection";

    wxLogDebug("%s: ENTER.", FNAME);

    if ( ! this->hoxPlayer::SetConnection( connection ) )
    {
        return false;
    }

    wxLogDebug("%s: Specify this player [%s] as the connection's onwer.", 
        FNAME, this->GetName().c_str());
    m_connection->SetPlayer( this );

    return true;
}

void 
hoxLocalPlayer::OnClose_FromTable( hoxPlayerEvent&  event )
{
    const char* FNAME = "hoxLocalPlayer::OnClose_FromTable";

    wxLogDebug("%s: ENTER.", FNAME);

    this->LeaveNetworkTable( event.GetTableId(), this /* NOT USED */ );

    this->hoxPlayer::OnClose_FromTable( event );
}

hoxResult 
hoxLocalPlayer::ConnectToNetworkServer( wxEvtHandler*   sender )
{
    _StartConnection();

    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_CONNECT, sender );
        request->content = 
            wxString::Format("op=CONNECT&pid=%s\r\n", this->GetName().c_str());
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
        request->content = 
            wxString::Format("op=LIST&pid=%s\r\n", this->GetName().c_str());
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
        request->content = 
            wxString::Format("op=JOIN&tid=%s&pid=%s\r\n", tableId.c_str(), this->GetName().c_str());
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
        request->content = 
            wxString::Format("op=NEW&pid=%s\r\n", this->GetName().c_str());
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
        request->content = 
                wxString::Format("op=LEAVE&tid=%s&pid=%s\r\n", 
                    tableId.c_str(), this->GetName().c_str());
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

    m_connection->Start();
}

/************************* END OF FILE ***************************************/
