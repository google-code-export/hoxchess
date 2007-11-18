/***************************************************************************
 *  Copyright 2007 Huy Phan  <huyphan@playxiangqi.com>                     *
 *                                                                         * 
 *  This file is part of HOXChess.                                         *
 *                                                                         *
 *  HOXChess is free software: you can redistribute it and/or modify       *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  HOXChess is distributed in the hope that it will be useful,            *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with HOXChess.  If not, see <http://www.gnu.org/licenses/>.      *
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// Name:            hoxHttpPlayer.cpp
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

IMPLEMENT_DYNAMIC_CLASS(hoxHttpPlayer, hoxLocalPlayer)

DEFINE_EVENT_TYPE(hoxEVT_HTTP_RESPONSE)

BEGIN_EVENT_TABLE(hoxHttpPlayer, hoxLocalPlayer)
    EVT_TIMER(wxID_ANY, hoxHttpPlayer::OnTimer)
    EVT_COMMAND(hoxREQUEST_TYPE_POLL, hoxEVT_HTTP_RESPONSE, hoxHttpPlayer::OnHTTPResponse_Poll)
    EVT_COMMAND(wxID_ANY, hoxEVT_HTTP_RESPONSE, hoxHttpPlayer::OnHTTPResponse)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxHttpPlayer
//-----------------------------------------------------------------------------

hoxHttpPlayer::hoxHttpPlayer()
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
hoxHttpPlayer::OnTimer( wxTimerEvent& event )
{
    const char* FNAME = "hoxHttpPlayer::OnTimer";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxNetworkEventList  networkEvents;

    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_POLL, this );
    request->content = 
        wxString::Format("op=POLL&pid=%s\r\n", this->GetName().c_str());
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
        wxLogDebug("%s: .... + Network event [%s].", FNAME, infoStr.c_str());

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
        wxLogError("%s: The HTTP response returns ERROR. [%s]", FNAME, returnMsg.c_str());
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
        wxLogError("%s: Failed to find table with ID = [%s].", FNAME, networkEvent.tid.c_str());
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
                wxLogError("%s: Player [%s] not found in the system.", FNAME, otherPlayerId.c_str());
                break;
            }
            wxLogDebug("%s: A player [%s] just left the table.", FNAME, otherPlayerId.c_str());
            table->OnLeave_FromPlayer( foundPlayer );
            break;
        }
        case hoxNETWORK_EVENT_TYPE_NEW_MOVE:    // NEW MOVE
        {
            wxString moveStr = networkEvent.content;
            table->OnMove_FromNetwork( this, moveStr );
            break;
        }
        case hoxNETWORK_EVENT_TYPE_NEW_WALL_MSG: // NEW MESSAGE
        {
            wxString message = networkEvent.content;
            table->OnMessage_FromNetwork( this->GetName(), message );
            break;
        }
        default:
            wxLogError("%s: Unknown event type = [%d].", FNAME, networkEvent.type);
            break;
    }
}

/************************* END OF FILE ***************************************/
