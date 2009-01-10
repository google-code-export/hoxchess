/***************************************************************************
 *  Copyright 2007, 2008, 2009 Huy Phan  <huyphan@playxiangqi.com>         *
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
#include "hoxConnection.h"
#include "hoxUtil.h"
#include "hoxTable.h"

IMPLEMENT_DYNAMIC_CLASS(hoxLocalPlayer, hoxPlayer)

BEGIN_EVENT_TABLE(hoxLocalPlayer, hoxPlayer)
    // empty
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
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxLocalPlayer::~hoxLocalPlayer() 
{
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER.", FNAME);
}

void
hoxLocalPlayer::Start()
{
    hoxConnection_APtr connection( new hoxLocalConnection( this ) );
    this->SetConnection( connection );
}

void 
hoxLocalPlayer::OnClose_FromTable( const wxString& tableId )
{
    const char* FNAME = __FUNCTION__;

    wxLogDebug("%s: ENTER. Table-Id = [%s].", FNAME, tableId.c_str());

    hoxTable_SPtr pTable = this->FindTable( tableId );
    if ( pTable.get() != NULL )
    {
        if ( pTable->GetGameType() != hoxGAME_TYPE_PRACTICE )
        {
            this->LeaveNetworkTable( tableId );
        }
        this->hoxPlayer::OnClose_FromTable( tableId );
    }
}

hoxResult 
hoxLocalPlayer::ConnectToNetworkServer()
{
    this->StartConnection();

    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_LOGIN ) );
	apRequest->parameters["pid"] = this->GetId();
	apRequest->parameters["password"] = this->GetPassword();
    this->AddRequestToConnection( apRequest );

    return hoxRC_OK;
}

hoxResult 
hoxLocalPlayer::DisconnectFromNetworkServer()
{
    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_LOGOUT ) );
	apRequest->parameters["pid"] = this->GetId();
	this->AddRequestToConnection( apRequest );

    return hoxRC_OK;
}

hoxResult 
hoxLocalPlayer::QueryForNetworkTables()
{
    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_LIST ) );
	apRequest->parameters["pid"] = this->GetId();
    this->AddRequestToConnection( apRequest );

    return hoxRC_OK;
}

hoxResult 
hoxLocalPlayer::JoinNetworkTable( const wxString& tableId )
{
	/* Check if this Player is already AT the Table. */
    hoxTable_SPtr pTable = this->FindTable( tableId );
    bool hasRole = ( pTable.get() != NULL );

    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_JOIN ) );
	apRequest->parameters["pid"] = this->GetId();
	apRequest->parameters["tid"] = tableId;
    apRequest->parameters["color"] = hoxUtil::ColorToString( hoxCOLOR_NONE ); // Observer.
	apRequest->parameters["joined"] = hasRole ? "1" : "0";
    this->AddRequestToConnection( apRequest );

    return hoxRC_OK;
}

hoxResult 
hoxLocalPlayer::OpenNewNetworkTable()
{
    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_NEW ) );
	apRequest->parameters["pid"] = this->GetId();
    const hoxTimeInfo timeInfo( 1500, 300, 20 ); // TODO: Hard-coded time-info.
    apRequest->parameters["itimes"] = hoxUtil::TimeInfoToString( timeInfo );
    this->AddRequestToConnection( apRequest );

    return hoxRC_OK;
}

hoxResult 
hoxLocalPlayer::LeaveNetworkTable( const wxString& tableId )
{
    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_LEAVE ) );
	apRequest->parameters["pid"] = this->GetId();
	apRequest->parameters["tid"] = tableId;
    this->AddRequestToConnection( apRequest );

    return hoxRC_OK;
}

hoxResult
hoxLocalPlayer::InvitePlayer( const wxString& sInviteeId )
{
    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_INVITE ) );
	apRequest->parameters["pid"] = this->GetId();  // Invitor
    apRequest->parameters["invitee"] = sInviteeId;   // Invitor

    this->AddRequestToConnection( apRequest );

    return hoxRC_OK;
}

/************************* END OF FILE ***************************************/
