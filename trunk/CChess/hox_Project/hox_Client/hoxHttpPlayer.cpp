/***************************************************************************
 *  Copyright 2007, 2008 Huy Phan  <huyphan@playxiangqi.com>               *
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

#include "hoxTable.h"
#include "hoxNetworkAPI.h"
#include "MyApp.h"      // wxGetApp()

IMPLEMENT_DYNAMIC_CLASS(hoxHttpPlayer, hoxLocalPlayer)

BEGIN_EVENT_TABLE(hoxHttpPlayer, hoxLocalPlayer)
    EVT_TIMER(wxID_ANY, hoxHttpPlayer::OnTimer)
    EVT_COMMAND(hoxREQUEST_POLL, hoxEVT_CONNECTION_RESPONSE, hoxHttpPlayer::OnConnectionResponse_Poll)
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

void 
hoxHttpPlayer::AddRequestToConnection( hoxRequest_APtr apRequest )
{ 
    const char* FNAME = "hoxHttpPlayer::AddRequestToConnection";

	wxCHECK_RET(apRequest.get() != NULL, "Request cannot be NULL.");

	apRequest->parameters["sid"] = m_sessionId;
	this->hoxPlayer::AddRequestToConnection( apRequest );
}

void 
hoxHttpPlayer::OnTimer( wxTimerEvent& event )
{
    const char* FNAME = "hoxHttpPlayer::OnTimer";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_POLL, this ) );
	apRequest->parameters["pid"] = this->GetName();
    this->AddRequestToConnection( apRequest );
}

void 
hoxHttpPlayer::OnConnectionResponse_Poll(wxCommandEvent& event) 
{
    const char* FNAME = "hoxHttpPlayer::OnConnectionResponse_Poll";
    hoxResult result;

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> safe_response( response ); // take care memory leak!

    /* NOTE: We do not check for the return-code ( event.GetInt() )
     *       because the response's content would be an empty string anyway.
     */

	int                 returnCode = 0;
	wxString            returnMsg;
    hoxNetworkEventList networkEvents;

    result = hoxNetworkAPI::ParseNetworkEvents( response->content,
												returnCode,
												returnMsg,
                                                networkEvents );

    // Re-start the timer before checking for the result.
    m_timer.Start( -1 /* Use the previous interval */, wxTIMER_ONE_SHOT );

	if ( result != hoxRC_OK || returnCode != 0 )
    {
        wxLogDebug("%s: *** WARN *** Failed to parse network events. [%d] [%s]", 
            FNAME, returnCode, returnMsg.c_str());
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

hoxResult 
hoxHttpPlayer::HandleResponseEvent_Connect( wxCommandEvent& event )
{
    const char* FNAME = "hoxHttpPlayer::HandleResponseEvent_Connect";
    hoxResult   result;

    wxLogDebug("%s: ENTER.", FNAME);

    // Invoke the parent-API.

    result = this->hoxLocalPlayer::HandleResponseEvent_Connect(event);

    if ( result == hoxRC_OK )
    {
        /* NOTE: Only enable 1-short at a time to be sure that the timer-handler
         *       is only entered ONE at at time.
         */
        wxLogDebug("%s: Start timer to poll for events from HTTP server.", FNAME);
        m_timer.Start( hoxSOCKET_HTTP_POLL_INTERVAL * hoxTIME_ONE_SECOND_INTERVAL,
                       wxTIMER_ONE_SHOT );
    }

    return result;
}


void 
hoxHttpPlayer::_HandleEventFromNetwork( const hoxNetworkEvent& networkEvent )
{
    const char* FNAME = "hoxHttpPlayer::_HandleEventFromNetwork";
    hoxSite*      site = NULL;
    hoxTable_SPtr pTable;

    wxLogDebug("%s: ENTER.", FNAME);

    const wxString tableId = networkEvent.tid;
    site = this->GetSite();

    /* Lookup table */
    pTable = site->FindTable( tableId );
    if ( pTable.get() == NULL )
    {
        wxLogDebug("%s: *** ERROR *** Failed to find table = [%s].", FNAME, tableId.c_str());
        return;
    }

    switch ( networkEvent.type )
    {
        case hoxNETWORK_EVENT_TYPE_NEW_PLAYER_RED:    // NEW PLAYER (RED)
            /* fall through */
        case hoxNETWORK_EVENT_TYPE_NEW_PLAYER_BLACK:  // NEW PLAYER (BLACK)
            /* fall through */
        case hoxNETWORK_EVENT_TYPE_NEW_PLAYER_NONE:    // NEW PLAYER (OBSERVER)
        {
            hoxColor requestColor;
			switch ( networkEvent.type )
			{
				case hoxNETWORK_EVENT_TYPE_NEW_PLAYER_RED:
					requestColor = hoxCOLOR_RED;
					break;
				case hoxNETWORK_EVENT_TYPE_NEW_PLAYER_BLACK:
					requestColor = hoxCOLOR_BLACK;
					break;
				default:
					requestColor = hoxCOLOR_NONE;  // Observer.
					break;
			}

			const wxChar   separator = ';';
			const wxString playerId = networkEvent.content.BeforeFirst(separator);
			const int      playerScore = ::atoi(networkEvent.content.AfterFirst(separator));

            hoxSite* site = this->GetSite();
            hoxResult result = site->OnPlayerJoined( tableId, 
                                                     playerId, 
                                                     playerScore,
                                                     requestColor );
            if ( result != hoxRC_OK )
            {
                wxLogDebug("%s: *** ERROR *** Failed to ask table to join as color [%d].", FNAME, requestColor);
                break;
            }
            break;
        }
        case hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_RED:    // LEAVE PLAYER (RED)
            /* fall through */
        case hoxNETWORK_EVENT_TYPE_LEAVE_PLAYER_BLACK:    // LEAVE PLAYER (BLACK)
        {
            wxString otherPlayerId = networkEvent.content;
            hoxPlayer* leavePlayer = site->FindPlayer( otherPlayerId );
            if ( leavePlayer == NULL ) 
            {
                wxLogError("%s: Player [%s] not found in the system.", FNAME, otherPlayerId.c_str());
                break;
            }
            wxLogDebug("%s: Player [%s] just left the table.", FNAME, leavePlayer->GetName().c_str());
            pTable->OnLeave_FromNetwork( leavePlayer, this );
            break;
        }
        case hoxNETWORK_EVENT_TYPE_NEW_MOVE:    // NEW MOVE
        {
            wxString moveStr = networkEvent.content;
            pTable->OnMove_FromNetwork( this, moveStr );
            break;
        }
        case hoxNETWORK_EVENT_TYPE_NEW_WALL_MSG: // NEW MESSAGE
        {
            wxString message = networkEvent.content;
            pTable->OnMessage_FromNetwork( this->GetName(), message );
            break;
        }
        default:
            wxLogError("%s: Unknown event type = [%d].", FNAME, networkEvent.type);
            break;
    }
}

/************************* END OF FILE ***************************************/
