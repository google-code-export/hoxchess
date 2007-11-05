/////////////////////////////////////////////////////////////////////////////
// Name:            hoxHttpPlayer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/28/2007
//
// Description:     The HTTP Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxHttpPlayer.h"
#include "hoxHttpConnection.h"
#include "hoxEnums.h"
#include "hoxTable.h"
#include "hoxTableMgr.h"
#include "hoxNetworkAPI.h"
#include "hoxUtility.h"
#include "hoxPlayerMgr.h"


DEFINE_EVENT_TYPE(hoxEVT_HTTP_RESPONSE)

IMPLEMENT_DYNAMIC_CLASS( hoxHttpPlayer, hoxLocalPlayer )

BEGIN_EVENT_TABLE(hoxHttpPlayer, hoxLocalPlayer)
    EVT_TIMER(wxID_ANY, hoxHttpPlayer::OnTimer)
    EVT_COMMAND(hoxREQUEST_TYPE_POLL, hoxEVT_HTTP_RESPONSE, hoxHttpPlayer::OnHTTPResponse_Poll)
    EVT_COMMAND(wxID_ANY, hoxEVT_HTTP_RESPONSE, hoxHttpPlayer::OnHTTPResponse)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxHttpPlayer
//-----------------------------------------------------------------------------

hoxHttpPlayer::hoxHttpPlayer()
            : hoxLocalPlayer( "Unknown", 
                              hoxPLAYER_TYPE_LOCAL, 
                              1500 )
{ 
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxHttpPlayer::hoxHttpPlayer( const wxString& name,
                              hoxPlayerType   type,
                              int             score )
            : hoxLocalPlayer( name, type, score )
{
    const char* FNAME = "hoxHttpPlayer::hoxHttpPlayer";
    wxLogDebug("%s: ENTER.", FNAME);

    m_timer.SetOwner( this );
}

hoxHttpPlayer::~hoxHttpPlayer() 
{
    const char* FNAME = "hoxHttpPlayer::~hoxHttpPlayer";
    wxLogDebug("%s: ENTER.", FNAME);

    if ( m_timer.IsRunning() ) 
    {
        m_timer.Stop();
    }
}

const wxString  
hoxHttpPlayer::BuildRequestContent( const wxString& commandStr )
{ 
    return wxString::Format("/cchess/tables.php?%s", commandStr); 
}

hoxThreadConnection* 
hoxHttpPlayer::CreateNewConnection( const wxString& sHostname, 
                                    int             nPort )
{
    return new hoxHttpConnection( sHostname, nPort );
}


hoxResult 
hoxHttpPlayer::JoinTable( hoxTable* table )
{
    const char* FNAME = "hoxHttpPlayer::JoinTable";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxResult result = this->hoxLocalPlayer::JoinTable( table );
    if ( result != hoxRESULT_OK )
    {
        return result;
    }

    /* NOTE: Only enable 1-short at a time to be sure that the timer-handler
     *       is only entered ONE at at time.
     */

    wxLogDebug("%s: Start timer to poll for events from HTTP server.", FNAME);
    m_timer.Start( 5 * hoxTIME_ONE_SECOND_INTERVAL, // 5-second interval
                   wxTIMER_ONE_SHOT );
    return hoxRESULT_OK;
}

hoxResult 
hoxHttpPlayer::LeaveTable( hoxTable* table )
{
    const char* FNAME = "hoxHttpPlayer::LeaveTable";
    hoxResult result;

    wxLogDebug("%s: ENTER.", FNAME);

    result = this->LeaveNetworkTable( table->GetId(), this );
    if ( result != hoxRESULT_OK )
        return result;

    return this->hoxLocalPlayer::LeaveTable( table );
}

hoxResult 
hoxHttpPlayer::LeaveNetworkTable( const wxString& tableId,
                                  wxEvtHandler*   sender )
{
    const char* FNAME = "hoxHttpPlayer::LeaveNetworkTable";
    wxLogDebug("%s: ENTER.", FNAME);

    if ( m_timer.IsRunning() ) 
    {
        wxLogDebug("%s: Stop timer (to not polling) due to leaving table.", FNAME);
        m_timer.Stop();
    }

    return this->hoxLocalPlayer::LeaveNetworkTable( tableId, sender );
}


void 
hoxHttpPlayer::OnTimer( wxTimerEvent& WXUNUSED(event) )
{
    const char* FNAME = "hoxHttpPlayer::OnTimer";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxNetworkEventList  networkEvents;

    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_POLL, this );
    wxString commandStr =
        wxString::Format("op=POLL&pid=%s", this->GetName());
    request->content = this->BuildRequestContent( commandStr );
    this->AddRequestToConnection( request );
}

void 
hoxHttpPlayer::OnHTTPResponse_Poll(wxCommandEvent& event) 
{
    const char* FNAME = "hoxHttpPlayer::OnHTTPResponse_Poll";
    hoxResult result;

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> safe_response( response ); // take care memory leak!

    /* NOTE: We do not check for the return-code ( event.GetInt() )
     *       because the response's content would be an empty string anyway.
     */

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

        _HandleEventFromNetwork( *(*it) );
    }

    // Release memory.
    for ( hoxNetworkEventList::iterator it = networkEvents.begin();
                                        it != networkEvents.end(); ++it )
    {
        delete (*it);
    }
}

void 
hoxHttpPlayer::OnHTTPResponse(wxCommandEvent& event) 
{
    const char* FNAME = "hoxHttpPlayer::OnHTTPResponse";
    hoxResult result;

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> safe_response( response ); // take care memory leak!

    int        returnCode = -1;
    wxString   returnMsg;

    result = hoxNetworkAPI::ParseSimpleResponse( response->content,
                                                 returnCode,
                                                 returnMsg );
    if ( result != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to parse the HTTP response.", FNAME);
        return;
    }
    else if ( returnCode != 0 )
    {
        wxLogError("%s: The HTTP response returns ERROR. [%s]", FNAME, returnMsg);
        return;
    }
}

void 
hoxHttpPlayer::_HandleEventFromNetwork( const hoxNetworkEvent& networkEvent )
{
    const char* FNAME = "hoxHttpPlayer::_HandleEventFromNetwork";

    wxLogDebug("%s: ENTER.", FNAME);

    // Find the table hosted on this system using the specified table-Id.
    hoxTable* table = hoxTableMgr::GetInstance()->FindTable( networkEvent.tid );
    if ( table == NULL )
    {
        wxLogError("%s: Failed to find table with ID = [%s].", FNAME, networkEvent.tid);
        return;
    }

    switch ( networkEvent.type )
    {
        case hoxNETWORK_EVENT_TYPE_NEW_PLAYER_RED:    // NEW PLAYER (RED)
            /* fall through */
        case hoxNETWORK_EVENT_TYPE_NEW_PLAYER_BLACK:    // NEW PLAYER (BLACK)
        {
            hoxPieceColor requestColor = 
                    ( networkEvent.type == hoxNETWORK_EVENT_TYPE_NEW_PLAYER_RED
                      ? hoxPIECE_COLOR_RED
                      : hoxPIECE_COLOR_BLACK );

            wxString otherPlayerId = networkEvent.content;
            
            hoxPlayer* newPlayer = 
                hoxPlayerMgr::GetInstance()->CreatePlayer( otherPlayerId,
                                                           hoxPLAYER_TYPE_DUMMY );
            
            hoxResult result = table->RequestJoinFromPlayer( newPlayer,
                                                             requestColor );
            if ( result != hoxRESULT_OK )
            {
                wxLogError("%s: Failed to ask table to join as color [%d].", FNAME, requestColor);
                hoxPlayerMgr::GetInstance()->DeletePlayer( newPlayer );
                break;
            }
            break;
        }
        case hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_RED:    // LEAVE PLAYER (RED)
            /* fall through */
        case hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_BLACK:    // LEAVE PLAYER (BLACK)
        {
            wxString otherPlayerId = networkEvent.content;
            hoxPlayer* foundPlayer = 
                hoxPlayerMgr::GetInstance()->FindPlayer( otherPlayerId );
            if ( foundPlayer == NULL ) 
            {
                wxLogError("%s: Player [%s] not found in the system.", FNAME, otherPlayerId);
                break;
            }
            wxLogDebug("%s: A player [%s] just left the table.", FNAME, otherPlayerId);
            table->OnLeave_FromPlayer( foundPlayer );
            break;
        }
        case hoxNETWORK_EVENT_TYPE_NEW_MOVE:    // NEW MOVE
        {
            wxString moveStr = networkEvent.content;
            table->OnMove_FromNetwork( this, moveStr );
            break;
        }
        default:
            wxLogError("%s: Unknown event type = [%d].", FNAME, networkEvent.type);
            break;
    }
}

/************************* END OF FILE ***************************************/
