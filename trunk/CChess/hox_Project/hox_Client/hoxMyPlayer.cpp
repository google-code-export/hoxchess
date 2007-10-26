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
    // Need to have a table even though it is empty.
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
hoxMyPlayer::StartListenForMoves( wxEvtHandler* sender )
{
    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_LISTEN, sender );
        request->content = "";
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxMyPlayer::ProcessIncomingData( wxSocketEvent& event )
{
    wxASSERT( m_connection != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_DATA, NULL /* sender */ );
        request->content = "";
        request->socket = event.GetSocket();
        request->socketEvent = event.GetSocketEvent();
        m_connection->AddRequest( request );
    }

    return hoxRESULT_OK;
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


#if 0
hoxResult 
hoxMyPlayer::DisconnectFromNetwork()
{
    m_pSClient = NULL;
    return hoxNetworkPlayer::DisconnectFromNetwork();
}

void 
hoxMyPlayer::HandleSocketLostEvent(wxSocketBase *sock)
{
    m_pSClient = NULL;
    return hoxNetworkPlayer::HandleSocketLostEvent( sock );
}
#endif

#if 0
hoxResult 
hoxMyPlayer::ConnectToNetworkServer( const wxString& sHostname, 
                                        int             nPort )
{
    /* Delete the old connection, if any. */
    this->DisconnectFromNetwork();
    wxASSERT( m_pSClient == NULL );

    /* Make a new connection */

    m_pSClient = new wxSocketClient( wxSOCKET_NONE );
    this->SetCBSocket( m_pSClient );  // NOTE: Let the parent handle network events

    // Setup the event handler and let the player handles all socket events.
    m_pSClient->SetEventHandler(*this, CLIENT_SOCKET_ID);

    // We disable input events util we can successfully join a table
    // by disabling wxSOCKET_INPUT_FLAG.

    wxLogDebug(_("*** INFO: Disable socket INPUT event until joining table."));
    m_pSClient->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
    m_pSClient->Notify(true);

    // Get the server address.
    wxIPV4address addr;
    addr.Hostname(sHostname);
    addr.Service(nPort);

    wxLogDebug(wxString::Format(_("Trying to connect to %s:%d (timeout = 10 sec)..."), 
                        sHostname, nPort));
    m_pSClient->Connect(addr, false);
    m_pSClient->WaitOnConnect(10 /* wait for 10 seconds */);

    if (m_pSClient->IsConnected()) 
    {
        wxLogDebug(_("Succeeded ! Connection established."));
    }
    else 
    {
        wxLogWarning(wxString::Format(_("Failed ! Unable to connect to the specified host [%s:%d]")),
                                       sHostname, nPort);
        this->DisconnectFromNetwork();
        return hoxRESULT_ERR;
    }

    return hoxRESULT_OK;
}

//
// Send Query-Tables command.
//
hoxResult 
hoxMyPlayer::QueryForNetworkTables( hoxNetworkTableInfoList& tableList )
{
    static const char* CMD_STR = "Query-Tables";

    wxLogDebug(wxString::Format(_("Send a [%s] command to the server..."), CMD_STR));
  
    /***************************
     * Send the request
     ***************************/

    if ( m_pSClient == NULL || !m_pSClient->IsConnected()) 
    {
        wxLogError(_("Connection is NOT yet opened. Send command failed!"));
        return hoxRESULT_ERR;
    }

    // Tell the server which 'command' we are sending
    unsigned char command = hoxNETWORK_CMD_QUERY_TABLE;
    m_pSClient->Write(&command, 1);

    // Wait until data available (will also return if the connection is lost)
    wxLogDebug(_("Waiting for an event (timeout = 2 sec)..."));
    m_pSClient->WaitForRead(2 /* seconds */);

    /***************************
     * Read the response
     ***************************/

    wxUint32 len = hoxNETWORK_MAX_MSG_SIZE;
    wxChar *msg2 = new wxChar[len];

    //unsigned char bTableOpen;
    //wxString sPlayerScore;
    int returnCode = -1;

    if ( ! m_pSClient->IsData() )
    {
        wxLogWarning(_("Timeout! Sending comand failed."));
    }
    else
    {
        wxLogDebug(_("Reading the string back with ReadMsg ..."));
    
        // Get the status-code.
        m_pSClient->Read(&returnCode, 1);
        if ( returnCode == 0 )
        {
            wxLogDebug(_("The return code is BAD."));
        }
        else
        {
            wxLogDebug(_("The return code is GOOD."));

            // Get the number of tables.
            size_t tableCount = 0;
            m_pSClient->Read( &tableCount, sizeof(size_t) );
            wxLogDebug(wxString::Format(_("The return # of tables = [%d]"), tableCount));

            // All all tables' info.
            tableList.clear();
            for ( size_t i = 0; i < tableCount; ++i )
            {
                hoxNetworkTableInfo* tableInfo = new hoxNetworkTableInfo();

                m_pSClient->ReadMsg(msg2, len);
                if (m_pSClient->Error()) 
                {
                    wxLogWarning(_("failed !"));
                    break;
                }
                tableInfo->id = msg2;
                wxLogDebug(wxString::Format(_("... table [%s]"), tableInfo->id));

                tableList.push_back( tableInfo );
            }
#if 0
            // Get the player's name.
            m_pSClient->ReadMsg(msg2, len);
            if (m_pSClient->Error()) wxLogWarning(_T("failed !"));
            sPlayerName = msg2;

            // Get the player's score.
            m_pSClient->ReadMsg(msg2, len);
            if (m_pSClient->Error()) wxLogWarning(_T("failed !"));
            sPlayerScore = msg2;
            long lVal;
            sPlayerScore.ToLong(&lVal);
            nPlayerScore = lVal;

            wxLogDebug(wxString::Format(_T("Remote-player: name=[%s], score=[%d]."), 
                sPlayerName, nPlayerScore));
            bTableExists = true;
#endif
        }
    }

    delete[] msg2;

    wxLogDebug(wxString::Format(_("[%s]: END"), CMD_STR));
    return hoxRESULT_OK;
}

//
// Send Join-Table command.
//
hoxResult 
hoxMyPlayer::JoinNetworkTable( const wxString& tableId )
{
    static const char* CMD_STR = "Join-Table";
    hoxResult result = hoxRESULT_ERR;

    wxLogDebug(wxString::Format(_("Send a [%s] command to the server..."), CMD_STR));

    /***************************
     * Send the request
     ***************************/

    if ( !m_pSClient || !m_pSClient->IsConnected()) 
    {
        wxLogError(_("Connection is NOT yet opened!"));
        return hoxRESULT_ERR;
    }

    // Tell the server which 'command' we are sending
    unsigned char command = hoxNETWORK_CMD_JOIN_TABLE;
    m_pSClient->Write(&command, 1);

    // Tell the server which table we want to join
    m_pSClient->WriteMsg(tableId.c_str(), (wxUint32)((tableId.Len()+1) * sizeof(wxChar)));
    if (m_pSClient->Error()) wxLogWarning(_("failed !"));

    // Send my name and score.
    wxString playerName = this->GetName();
    wxLogDebug(wxString::Format(_("Sending player-name = [%s]..."), playerName));
    m_pSClient->WriteMsg(playerName.c_str(), (wxUint32)((playerName.Len()+1) * sizeof(wxChar)));
    if (m_pSClient->Error()) wxLogWarning(_("failed !"));

    wxString sVal;
    int playerScore = 1500;
    sVal.Printf("%d", playerScore);
    wxLogDebug(wxString::Format(_("Sending player-score = [%s]..."), sVal));
    m_pSClient->WriteMsg(sVal.c_str(), (wxUint32)((sVal.Len()+1) * sizeof(wxChar)));
    if (m_pSClient->Error()) wxLogWarning(_("failed !"));

    /***************************
     * Read the response
     ***************************/

    // Wait until data available (will also return if the connection is lost)
    wxLogDebug(_("Waiting for data from the server (timeout = 3 sec)..."));
    m_pSClient->WaitForRead(3);

    unsigned char bRetCode = 0;

    if (m_pSClient->IsData())
    {
        wxLogDebug(_("Reading the return-code ..."));

        // Is table open?
        m_pSClient->Read(&bRetCode, 1);
        if ( bRetCode == 0 )
        {
            wxLogWarning(_("The request is rejected!"));
        }
        else
        {
            wxLogDebug(_("The request is OK (accepted)."));
            if (m_pSClient->Error()) wxLogWarning(_("Read failed !"));
            result = hoxRESULT_OK;

            // We re-enable INPUT events since we already joined a table.

            wxLogDebug(_T("*** Re-enable socket INPUT event after joining table."));
            m_pSClient->SetNotify(  wxSOCKET_CONNECTION_FLAG 
                                  | wxSOCKET_INPUT_FLAG
                                  | wxSOCKET_LOST_FLAG );
        }
    }
    else 
    {
        wxLogWarning(wxString::Format(_("[%s]: Timeout ! Request failed."), CMD_STR));
    }

    wxLogDebug(wxString::Format(_("[%s]: END"), CMD_STR));
    return result;
}
#endif

/************************* END OF FILE ***************************************/
