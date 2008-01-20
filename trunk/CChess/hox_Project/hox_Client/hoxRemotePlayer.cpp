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
// Name:            hoxRemotePlayer.cpp
// Created:         11/01/2007
//
// Description:     The REMOTE Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxRemotePlayer.h"
#include "hoxEnums.h"
#include "hoxConnection.h"
#include "hoxServer.h"

IMPLEMENT_DYNAMIC_CLASS(hoxRemotePlayer, hoxPlayer)

BEGIN_EVENT_TABLE(hoxRemotePlayer, hoxPlayer)
    EVT_SOCKET(CLIENT_SOCKET_ID,  hoxRemotePlayer::OnIncomingNetworkData)
    EVT_COMMAND(hoxREQUEST_TYPE_PLAYER_DATA, hoxEVT_SERVER_RESPONSE, hoxRemotePlayer::OnConnectionResponse_PlayerData)
    EVT_COMMAND(wxID_ANY, hoxEVT_SERVER_RESPONSE, hoxRemotePlayer::OnServerResponse)
END_EVENT_TABLE()


//-----------------------------------------------------------------------------
// hoxRemotePlayer
//-----------------------------------------------------------------------------

hoxRemotePlayer::hoxRemotePlayer()
{ 
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxRemotePlayer::hoxRemotePlayer( const wxString& name,
                                  hoxPlayerType   type,
                                  int             score )
            : hoxPlayer( name, type, score )
{ 
}

hoxRemotePlayer::~hoxRemotePlayer() 
{
}

void
hoxRemotePlayer::OnIncomingNetworkData( wxSocketEvent& event )
{
    const char* FNAME = "hoxRemotePlayer::OnIncomingNetworkData";
    wxLogDebug("%s: ENTER.", FNAME);

    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_PLAYER_DATA, this );
    request->socket      = event.GetSocket();
        /* Set the socket here so that the connection can perform 
         * sanity check to make sure the socket matches.
         */

    request->socketEvent = event.GetSocketEvent(); // TODO: Not consistent???
    this->AddRequestToConnection( request );
}

void 
hoxRemotePlayer::OnConnectionResponse_PlayerData( wxCommandEvent& event )
{
    const char* FNAME = "hoxRemotePlayer::OnConnectionResponse_PlayerData";
    hoxResult result = hoxRESULT_OK;

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    /* Make a note to 'self' that one request has been serviced. */
    DecrementOutstandingRequests();

    /* NOTE: Only handle the connection-lost event. */

    if ( (response->flags & hoxRESPONSE_FLAG_CONNECTION_LOST) !=  0 )
    {
        wxLogDebug("%s: Connection has been lost. Leaving all tables...", FNAME);
        /* Currently, we support one connection per player.
         * Since this ONLY connection is closed, the player must leave
         * all tables.
         */
        this->LeaveAllTables();
    }
    else
    {
        /* Handle other type of data. */

        result = this->HandleIncomingData( response->content );
        if ( result != hoxRESULT_OK )
        {
            wxLogDebug("%s: *** WARN *** Error occurred while handling incoming data [%s].", 
                FNAME, response->content.c_str());
            goto exit_label;
        }
    }

exit_label:
    wxLogDebug("%s: END.", FNAME);
}

void 
hoxRemotePlayer::OnServerResponse( wxCommandEvent& event )
{
    const char* FNAME = "hoxRemotePlayer::OnServerResponse";
    hoxResult result = hoxRESULT_OK;

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    const std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    /* Make a note to 'self' that one request has been serviced. */
    DecrementOutstandingRequests();

    wxLogDebug("%s: END.", FNAME);
}

/************************* END OF FILE ***************************************/
