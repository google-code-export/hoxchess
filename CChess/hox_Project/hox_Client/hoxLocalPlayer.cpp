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
// Name:            hoxLocalPlayer.cpp
// Created:         10/28/2007
//
// Description:     The LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxLocalPlayer.h"

DEFINE_EVENT_TYPE(hoxEVT_CONNECTION_RESPONSE)

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
    wxLogDebug("%s: ENTER.", FNAME);
}

void 
hoxLocalPlayer::OnClose_FromTable( wxCommandEvent&  event )
{
    const char* FNAME = "hoxLocalPlayer::OnClose_FromTable";

    wxLogDebug("%s: ENTER.", FNAME);

    const wxString tableId  = event.GetString();
    this->LeaveNetworkTable( tableId, this );

    this->hoxPlayer::OnClose_FromTable( event );
}

hoxResult 
hoxLocalPlayer::ConnectToNetworkServer( wxEvtHandler* sender )
{
    this->StartConnection();

    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_CONNECT, sender );
    request->content = 
        wxString::Format("op=CONNECT&pid=%s\r\n", this->GetName().c_str());
    this->AddRequestToConnection( request );

    return hoxRESULT_OK;
}

hoxResult 
hoxLocalPlayer::DisconnectFromNetworkServer( wxEvtHandler* sender )
{
	const char* FNAME = "hoxLocalPlayer::DisconnectFromNetworkServer";

	wxLogDebug("%s: ENTER. Do nothing. END.", FNAME);
    
	if ( sender != NULL )
	{
		/* Do nothing. Just return a response. */

		hoxRequestType requestType = hoxREQUEST_TYPE_DISCONNECT;
		std::auto_ptr<hoxResponse> response( new hoxResponse(requestType, 
															 sender) );

		wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, requestType );
		response->code = hoxRESULT_OK;
		event.SetEventObject( response.release() );  // Caller will de-allocate.
		wxPostEvent( sender, event );
	}

    return hoxRESULT_OK;
}

hoxResult 
hoxLocalPlayer::QueryForNetworkTables( wxEvtHandler* sender )
{
    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_LIST, sender );
    request->content = 
        wxString::Format("op=LIST&pid=%s\r\n", this->GetName().c_str());
    this->AddRequestToConnection( request );

    return hoxRESULT_OK;
}

hoxResult 
hoxLocalPlayer::JoinNetworkTable( const wxString& tableId,
                                  wxEvtHandler*   sender )
{
    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_JOIN, sender );
    request->content = 
        wxString::Format("op=JOIN&tid=%s&pid=%s\r\n", tableId.c_str(), this->GetName().c_str());
    this->AddRequestToConnection( request );

    return hoxRESULT_OK;
}

hoxResult 
hoxLocalPlayer::OpenNewNetworkTable( wxEvtHandler*   sender )
{
    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_NEW, sender );
    request->content = 
        wxString::Format("op=NEW&pid=%s\r\n", this->GetName().c_str());
    this->AddRequestToConnection( request );

    return hoxRESULT_OK;
}

hoxResult 
hoxLocalPlayer::LeaveNetworkTable( const wxString& tableId,
                                   wxEvtHandler*   sender )
{
    hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_LEAVE, sender );
    request->content = 
            wxString::Format("op=LEAVE&tid=%s&pid=%s\r\n", 
                tableId.c_str(), this->GetName().c_str());
    this->AddRequestToConnection( request );

    return hoxRESULT_OK;
}

/************************* END OF FILE ***************************************/
