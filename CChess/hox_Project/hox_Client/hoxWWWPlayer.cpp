/////////////////////////////////////////////////////////////////////////////
// Name:            hoxWWWPlayer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/21/2007
//
// Description:     The WWW Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxWWWPlayer.h"
#include "hoxWWWThread.h"
#include "hoxEnums.h"
#include "hoxTable.h"
#include "hoxTableMgr.h"
#include "hoxBoard.h"
#include "hoxNetworkAPI.h"
#include "hoxUtility.h"


DEFINE_EVENT_TYPE(hoxEVT_WWW_RESPONSE)

IMPLEMENT_DYNAMIC_CLASS( hoxWWWPlayer, hoxPlayer )

BEGIN_EVENT_TABLE(hoxWWWPlayer, hoxPlayer)
    EVT_TIMER(wxID_ANY, hoxWWWPlayer::OnTimer)
    EVT_COMMAND(wxID_ANY, hoxEVT_WWW_RESPONSE, hoxWWWPlayer::OnWWWResponse)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxWWWPlayer
//-----------------------------------------------------------------------------

hoxWWWPlayer::hoxWWWPlayer()
            : hoxPlayer( "Unknown", 
                         hoxPLAYER_TYPE_LOCAL, 
                         1500 )
            , m_wwwThread( NULL )
{ 
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxWWWPlayer::hoxWWWPlayer( const wxString& name,
                            hoxPlayerType   type,
                            int             score /* = 1500 */)
            : hoxPlayer( name, type, score )
            , m_wwwThread( NULL )
{
    const char* FNAME = "hoxWWWPlayer::hoxWWWPlayer";
    wxLogDebug("%s: ENTER.", FNAME);

    m_timer.SetOwner( this );
}

hoxWWWPlayer::~hoxWWWPlayer() 
{
    const char* FNAME = "hoxWWWPlayer::~hoxWWWPlayer";

    if ( m_timer.IsRunning() ) 
    {
        m_timer.Stop();
    }

    if ( m_wwwThread != NULL )
    {
        wxLogDebug("%s: Request the WWW thread to be shutdowned...", FNAME);
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_SHUTDOWN, NULL );
        m_wwwThread->AddRequest( request );
        wxThread::ExitCode exitCode = m_wwwThread->GetThread()->Wait();
        wxLogDebug("%s: WWW thread was shutdowned with exit-code = [%d].", FNAME, exitCode);
        delete m_wwwThread;
    }
}

void 
hoxWWWPlayer::OnNewMove_FromTable( hoxPlayerEvent&  event )
{
    const char* FNAME = "hoxWWWPlayer::OnNewMove_FromTable";
    wxString     tableId     = event.GetTableId();
    hoxPosition  moveFromPos = event.GetOldPosition();
    hoxPosition  moveToPos   = event.GetPosition();

    wxString moveStr = wxString::Format("%d%d%d%d", 
                            moveFromPos.x, moveFromPos.y, moveToPos.x, moveToPos.y);

    wxLogDebug("%s: ENTER. Move = [%s].", FNAME, moveStr);

    wxASSERT( m_wwwThread != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_TABLE_MOVE, this );
        request->content = 
                wxString::Format("/cchess/tables.php?op=MOVE&tid=%s&pid=%s&move=%s", 
                            tableId, this->GetName(), moveStr);
        m_wwwThread->AddRequest( request );
    }
}

hoxResult 
hoxWWWPlayer::JoinTable( hoxTable* table )
{
    const char* FNAME = "hoxWWWPlayer::JoinTable";

    hoxResult result = this->hoxPlayer::JoinTable( table );
    if ( result != hoxRESULT_OK ) // failed?
    {
        return result;
    }

    /* NOTE: Only enable 1-short at a time to be sure that the timer-handler
     *       is only entered ONE at at time.
     */

    wxLogDebug("%s: Start timer to poll for events from WWW server.", FNAME);
    m_timer.Start( 5 * hoxTIME_ONE_SECOND_INTERVAL, // 5-second interval
                   wxTIMER_ONE_SHOT );
    return hoxRESULT_OK;
}

hoxResult 
hoxWWWPlayer::LeaveTable( hoxTable* table )
{
    const char* FNAME = "hoxWWWPlayer::LeaveTable";

    hoxResult result = this->hoxPlayer::LeaveTable( table );
    if ( result != hoxRESULT_OK ) // failed?
    {
        return result;
    }

    if ( m_timer.IsRunning() ) 
    {
        wxLogDebug("%s: Stop timer (to not polling) due to leaving table.", FNAME);
        m_timer.Stop();
    }

    return hoxRESULT_OK;
}

// On timer.
void 
hoxWWWPlayer::OnTimer( wxTimerEvent& WXUNUSED(event) )
{
    const char* FNAME = "hoxWWWPlayer::OnTimer";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxNetworkEventList  networkEvents;

    wxASSERT( m_wwwThread != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_POLL, this );
        request->content = 
            wxString::Format("/cchess/tables.php?op=POLL&pid=%s", this->GetName());
        m_wwwThread->AddRequest( request );
    }
}

hoxResult 
hoxWWWPlayer::ConnectToNetworkServer( const wxString& sHostname, 
                                      int             nPort,
                                      wxEvtHandler*   sender )
{
    m_sHostname = sHostname;
    m_nPort = nPort;

    _StartWWWThread();

    wxASSERT( m_wwwThread != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_CONNECT, sender );
        request->content = 
                wxString::Format("/cchess/tables.php?op=CONNECT");
        m_wwwThread->AddRequest( request );
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxWWWPlayer::QueryForNetworkTables(wxEvtHandler* sender)
{
    wxASSERT( m_wwwThread != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_LIST, sender );
        request->content = 
                wxString::Format("/cchess/tables.php?op=LIST");
        m_wwwThread->AddRequest( request );
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxWWWPlayer::JoinNetworkTable( const wxString&  tableId,
                                wxEvtHandler*    sender )
{
    wxASSERT( m_wwwThread != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_JOIN, sender );
        request->content = 
                wxString::Format("/cchess/tables.php?op=JOIN&tid=%s&pid=%s", tableId, this->GetName());
        m_wwwThread->AddRequest( request );
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxWWWPlayer::OpenNewNetworkTable( wxEvtHandler* sender )
{
    wxASSERT( m_wwwThread != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_NEW, sender );
        request->content = 
                wxString::Format("/cchess/tables.php?op=NEW&pid=%s", this->GetName());
        m_wwwThread->AddRequest( request );
    }

    return hoxRESULT_OK;
}

hoxResult 
hoxWWWPlayer::LeaveNetworkTable( const wxString& tableId,
                                 wxEvtHandler*   sender )
{
    wxASSERT( m_wwwThread != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_LEAVE, sender );
        request->content = 
                wxString::Format("/cchess/tables.php?op=LEAVE&tid=%s&pid=%s", tableId, this->GetName());
        m_wwwThread->AddRequest( request );
    }

    return hoxRESULT_OK;
}

void 
hoxWWWPlayer::OnWWWResponse(wxCommandEvent& event) 
{
    const char* FNAME = "hoxWWWPlayer::OnWWWResponse";
    hoxResult result;

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> safe_response( response ); // take care memory leak!

    /* NOTE: We do not check for the return-code ( event.GetInt() )
     *       because the response's content would be an empty string anyway.
     */

    switch ( response->type )
    {
        case hoxREQUEST_TYPE_POLL:
        {
            hoxNetworkEventList networkEvents;

            result = hoxNetworkAPI::ParseNetworkEvents( response->content,
                                                        networkEvents );

            // Re-start the timer before checking for the result.
            m_timer.Start( -1 /* Use the previous interval */, wxTIMER_ONE_SHOT );

            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Parse table events failed.", FNAME);
                return;
            }

            // Display all events.
            wxLogDebug("%s: We got [%d] event(s).", FNAME, networkEvents.size());
            for ( hoxNetworkEventList::iterator it = networkEvents.begin();
                                                it != networkEvents.end(); ++it )
            {
                wxASSERT_MSG( this->GetName() == (*it)->pid, "Player Id must be the same.");
                wxString infoStr;  // event's info-string.
                infoStr << (*it)->id << " " << (*it)->pid << " " 
                        << (*it)->tid << " " << (*it)->type << " "
                        << (*it)->content;
                wxLogDebug("%s: .... + Network event [%s].", FNAME, infoStr);

                // Find the table hosted on this system using the specified table-Id.
                hoxTable* table = hoxTableMgr::GetInstance()->FindTable( (*it)->tid );
                if ( table == NULL )
                {
                    wxLogError("%s: Failed to find table with ID = [%s].", FNAME, (*it)->tid);
                    return;
                }

                // Inform our table...
                table->OnEvent_FromWWWNetwork( this, *(*it) );
            }

            // Release memory.
            for ( hoxNetworkEventList::iterator it = networkEvents.begin();
                                                it != networkEvents.end(); ++it )
            {
                delete (*it);
            }

        }
        break;

        case hoxREQUEST_TYPE_TABLE_MOVE:
        {
            int        returnCode = -1;
            wxString   returnMsg;

            result = hoxNetworkAPI::ParseSimpleResponse( response->content,
                                                         returnCode,
                                                         returnMsg );
            if ( result != hoxRESULT_OK )
            {
                wxLogError(wxString::Format("%s: Parse for SEND-MOVE's response.", FNAME));
                return;
            }
            else if ( returnCode != 0 )
            {
                wxLogError(wxString::Format("%s: Send MOVE to server failed. [%s]", FNAME, returnMsg));
                return;
            }
        }
        break;

        default:
            wxLogError("%s: Unknown Request type [%s].", 
                FNAME, hoxUtility::RequestTypeToString(response->type));
            break;
    }
}

void 
hoxWWWPlayer::_StartWWWThread()
{
    const char* FNAME = "hoxWWWPlayer::_StartWWWThread";
    wxASSERT_MSG( m_wwwThread == NULL, "WWW Thread should not have been created.");

    wxASSERT_MSG( !m_sHostname.empty(), "Hostname must have been set." );
    m_wwwThread = new hoxWWWThread( m_sHostname, m_nPort );

    if ( m_wwwThread->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError("%s: Failed to create WWW thread.", FNAME);
        return;
    }
    wxASSERT_MSG( !m_wwwThread->GetThread()->IsDetached(), 
                  "The WWW thread must be joinable." );

    m_wwwThread->GetThread()->Run();
}


/************************* END OF FILE ***************************************/
