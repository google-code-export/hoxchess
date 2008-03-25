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
// Name:            hoxLocalPlayer.cpp
// Created:         10/28/2007
//
// Description:     The LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxLocalPlayer.h"
#include "hoxNetworkAPI.h"
#include "hoxUtil.h"
#include "hoxSite.h"

DEFINE_EVENT_TYPE(hoxEVT_CONNECTION_RESPONSE)

IMPLEMENT_ABSTRACT_CLASS(hoxLocalPlayer, hoxPlayer)

BEGIN_EVENT_TABLE(hoxLocalPlayer, hoxPlayer)
    EVT_COMMAND(wxID_ANY, hoxEVT_CONNECTION_RESPONSE, hoxLocalPlayer::OnConnectionResponse)
END_EVENT_TABLE()

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
hoxLocalPlayer::OnClose_FromTable( const wxString& tableId )
{
    const char* FNAME = "hoxLocalPlayer::OnClose_FromTable";

    wxLogDebug("%s: ENTER. Table-Id = [%s].", FNAME, tableId.c_str());

    this->LeaveNetworkTable( tableId );

    this->hoxPlayer::OnClose_FromTable( tableId );
}

hoxResult 
hoxLocalPlayer::ConnectToNetworkServer()
{
    this->StartConnection();

    hoxRequest* request = new hoxRequest( hoxREQUEST_LOGIN );
	request->parameters["pid"] = this->GetName();
	request->parameters["password"] = this->GetPassword();
    this->AddRequestToConnection( request );

    return hoxRC_OK;
}

hoxResult 
hoxLocalPlayer::DisconnectFromNetworkServer()
{
    hoxRequest* request = new hoxRequest( hoxREQUEST_LOGOUT );
	request->parameters["pid"] = this->GetName();
	this->AddRequestToConnection( request );

    return hoxRC_OK;
}

hoxResult 
hoxLocalPlayer::QueryForNetworkTables()
{
    hoxRequest* request = new hoxRequest( hoxREQUEST_LIST );
	request->parameters["pid"] = this->GetName();
    this->AddRequestToConnection( request );

    return hoxRC_OK;
}

hoxResult 
hoxLocalPlayer::JoinNetworkTable( const wxString& tableId )
{
	/* Check if this Player is already AT the Table. */
	bool hasRole = this->HasRoleAtTable( tableId );

    hoxRequest* request = new hoxRequest( hoxREQUEST_JOIN );
	request->parameters["pid"] = this->GetName();
	request->parameters["tid"] = tableId;
    request->parameters["color"] = hoxUtil::ColorToString( hoxCOLOR_NONE ); // Observer.
	request->parameters["joined"] = hasRole ? "1" : "0";
    this->AddRequestToConnection( request );

    return hoxRC_OK;
}

hoxResult 
hoxLocalPlayer::OpenNewNetworkTable()
{
    hoxRequest* request = new hoxRequest( hoxREQUEST_NEW );
	request->parameters["pid"] = this->GetName();
	request->parameters["itimes"] = "1500/300/20"; // TODO: Hard-coded initial times.
    this->AddRequestToConnection( request );

    return hoxRC_OK;
}

hoxResult 
hoxLocalPlayer::LeaveNetworkTable( const wxString& tableId )
{
    hoxRequest* request = new hoxRequest( hoxREQUEST_LEAVE );
	request->parameters["pid"] = this->GetName();
	request->parameters["tid"] = tableId;
    this->AddRequestToConnection( request );

    return hoxRC_OK;
}

void 
hoxLocalPlayer::OnConnectionResponse( wxCommandEvent& event )
{
    const char* FNAME = "hoxLocalPlayer::OnConnectionResponse";
    hoxResult     result;
    int           returnCode = -1;
    wxString      returnMsg;

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response_raw = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());
    std::auto_ptr<hoxResponse> response( response_raw ); // take care memory leak!

    /* Make a note to 'self' that one request has been serviced. */
    DecrementOutstandingRequests();

    hoxRemoteSite* remoteSite = static_cast<hoxRemoteSite*>( this->GetSite() );

    const wxString sType = hoxUtil::RequestTypeToString(response->type);

	switch ( response->type )
	{
        case hoxREQUEST_LOGIN:
        {
            result = this->HandleResponseEvent_Connect(event);
			if ( result != hoxRC_OK )
			{
				wxLogDebug("%s: *** WARN *** Failed to handle LOGIN's response [%s].", 
					FNAME, response->content.c_str());
				response->code = result;
			}
            remoteSite->OnResponse_LOGIN( response );
            return;  // *** DONE !!!!!!!!!!!!!!!!!
        }
        case hoxREQUEST_LIST:
		{
			hoxNetworkTableInfoList* pTableList = new hoxNetworkTableInfoList;
			result = hoxNetworkAPI::ParseNetworkTables( response->content,
														*pTableList );
			if ( result != hoxRC_OK )
			{
				wxLogDebug("%s: *** WARN *** Failed to parse LIST's response [%s].", 
					FNAME, response->content.c_str());
				response->code = result;
			}
            std::auto_ptr<hoxNetworkTableInfoList> autoPtr_tablelist( pTableList );  // prevent memory leak!
            remoteSite->DisplayListOfTables( *pTableList );
            return;  // *** DONE !!!!!!!!!!!!!!!!!
		}
        case hoxREQUEST_JOIN:
		{
			std::auto_ptr<hoxNetworkTableInfo> pTableInfo( new hoxNetworkTableInfo() );
			result = hoxNetworkAPI::ParseJoinNetworkTable( response->content,
														   *pTableInfo );
			if ( result != hoxRC_OK )
			{
				wxLogDebug("%s: *** WARN *** Failed to parse JOIN's response [%s].", 
					FNAME, response->content.c_str());
				response->code = result;
			}
			remoteSite->JoinExistingTable( *pTableInfo );
			return;  // *** DONE !!!!!!!!!!!!!!!!!
		}
        case hoxREQUEST_NEW:
		{
			std::auto_ptr<hoxNetworkTableInfo> pTableInfo( new hoxNetworkTableInfo() );
			result = hoxNetworkAPI::ParseNewNetworkTable( response->content,
														  *pTableInfo );
			if ( result != hoxRC_OK )
			{
				wxLogDebug("%s: *** WARN *** Failed to parse NEW's response [%s].", 
					FNAME, response->content.c_str());
				response->code = result;
                return;
			}
			remoteSite->JoinNewTable( *pTableInfo );
			return;  // *** DONE !!!!!!!!!!!!!!!!!
		}
        case hoxREQUEST_OUT_DATA:
        {
		    wxLogDebug("%s: [%s] 's response received. END.", FNAME, sType.c_str());
            return;  // *** DONE !!!!!!!!!!!!!!!!!
        }
		default:
			wxLogDebug("%s: *** WARN *** Unsupported Request [%s].", FNAME, sType.c_str());
			break;
	} // switch


    ///////////////////////////////// OLD CODE ////////////////////////////////


    /* Parse the response */
    result = hoxNetworkAPI::ParseSimpleResponse( response->content,
                                                 returnCode,
                                                 returnMsg );
    if ( result != hoxRC_OK || returnCode != 0 )
    {
        wxLogDebug("%s: *** WARN *** Failed to parse the response. [%d] [%s]", 
            FNAME, returnCode, returnMsg.c_str());
        return;
    }

    wxLogDebug("%s: END.", FNAME);
}

hoxResult 
hoxLocalPlayer::HandleResponseEvent_Connect( wxCommandEvent& event )
{
    const char* FNAME = "hoxLocalPlayer::HandleResponseEvent_Connect";
    hoxResult   result;
    int         returnCode = -1;
    wxString    returnMsg;
	wxString    sessionId;
	int         nScore = 0;

    wxLogDebug("%s: ENTER.", FNAME);

    hoxResponse* response = wx_reinterpret_cast(hoxResponse*, event.GetEventObject());

    result = hoxNetworkAPI::ParseConnectResponse( response->content,
                                                  returnCode,
                                                  returnMsg,
												  sessionId,
												  nScore );
    if ( result != hoxRC_OK || returnCode != 0 )
    {
        wxLogDebug("%s: *** WARN *** Connection ERROR. Error = [%d: %d].", 
            FNAME, result, returnCode);
        return hoxRC_ERR;
    }

	m_sessionId = sessionId; // Extract the session-Id.
	this->SetScore( nScore );
	wxLogDebug("%s: Connection OK. Session-Id = [%s].", FNAME, m_sessionId.c_str());

	/* Return the error-message to the default (parent) handler. */
	response->content = returnMsg;

    return hoxRC_OK;
}

/************************* END OF FILE ***************************************/
