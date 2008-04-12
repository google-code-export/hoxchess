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
// Name:            hoxPlayer.cpp
// Created:         10/06/2007
//
// Description:     The Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxPlayer.h"
#include "hoxTable.h"
#include "hoxConnection.h"
#include "hoxTableMgr.h"
#include "hoxPlayerMgr.h"
#include "hoxUtil.h"
#include "hoxNetworkAPI.h"
#include "MyApp.h"

#include <algorithm>  // std::find

IMPLEMENT_DYNAMIC_CLASS(hoxPlayer, wxEvtHandler)

//----------------------------------------------------------------------------
// Event types
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(hoxPlayer, wxEvtHandler)
	// Empty table
END_EVENT_TABLE()


//-----------------------------------------------------------------------------
// hoxPlayer
//-----------------------------------------------------------------------------

hoxPlayer::hoxPlayer()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxPlayer::hoxPlayer( const wxString& name,
                      hoxPlayerType   type,
                      int             score /* = 1500 */)
            : m_info( name, type, score )
            , m_site( NULL )
			, m_siteClosing( false )
{ 
}

hoxPlayer::~hoxPlayer() 
{
    const char* FNAME = "hoxPlayer::~hoxPlayer";
    wxLogDebug("%s: ENTER. (%s)", FNAME, this->GetName().c_str());

    if ( m_connection.get() != NULL )
    {
        this->ShutdownConnection();
    }
}

void               
hoxPlayer::AddRole( hoxRole role )
{
    for ( hoxRoleList::iterator it = m_roles.begin();
                                it != m_roles.end();
                              ++it )
    {
        if ( it->tableId == role.tableId )
        {
			it->color = role.color;
			return;  // *** DONE.
        }
    }

    m_roles.push_back( role );
}

void               
hoxPlayer::RemoveRole( hoxRole role )
{
    wxASSERT( this->HasRole( role ) );
    m_roles.remove( role );

    if ( m_siteClosing && m_roles.empty() )
    {
		_PostSite_ShutdownReady();
    }
}

bool
hoxPlayer::RemoveRoleAtTable( const wxString& tableId )
{
    for ( hoxRoleList::iterator it = m_roles.begin();
                                it != m_roles.end();
                              ++it )
    {
        if ( it->tableId == tableId )
        {
			this->RemoveRole( *it );
            return true; // role found.
        }
    }

    return false;  // role not found.
}

bool               
hoxPlayer::HasRole( hoxRole role )
{
    hoxRoleList::iterator found 
        = std::find( m_roles.begin(), m_roles.end(), role );
    return ( found != m_roles.end() );
}

bool
hoxPlayer::HasRoleAtTable( const wxString& tableId ) const
{
	hoxColor assignedColor;
	return this->FindRoleAtTable( tableId, assignedColor );
}

bool
hoxPlayer::FindRoleAtTable( const wxString& tableId, 
	                        hoxColor&  assignedColor ) const
{
    for ( hoxRoleList::const_iterator it = m_roles.begin();
                                      it != m_roles.end();
                                    ++it )
    {
        if ( it->tableId == tableId )
        {
			assignedColor = it->color;
            return true; // role found.
        }
    }

	return false; // role not found.
}

/**
 * @note We can check the player's color afterwards to 
 *       see which role the player has.
 */
hoxResult 
hoxPlayer::JoinTable( hoxTable_SPtr pTable )
{
    const char* FNAME = "hoxPlayer::JoinTable";

    wxCHECK_MSG( pTable.get() != NULL, hoxRC_ERR, "The table is NULL." );
    // TODO: Check for duplicate!!! (join same table twice)
    hoxColor assignedColor;
    bool     informOthers = true;

    /* NOTE: Except for dummy players, this player will inform other
     *       about his presence.
     */
    if ( this->GetType() == hoxPLAYER_TYPE_DUMMY )
    {
        wxLogDebug("%s: Dummy player [%s] will not inform others about his JOIN.", 
            FNAME, this->GetName().c_str());
        informOthers = false;
    }

    hoxResult result = pTable->AssignPlayer( this, 
                                             assignedColor, 
                                             informOthers );
    if ( result == hoxRC_OK )
    {
        this->AddRole( hoxRole( pTable->GetId(), assignedColor ) );
    }

    return result;
}

hoxResult 
hoxPlayer::JoinTableAs( hoxTable_SPtr pTable,
                        hoxColor      requestColor )
{
    wxCHECK_MSG( pTable.get() != NULL, hoxRC_ERR, "The table is NULL." );

    hoxResult result = pTable->AssignPlayerAs( this, requestColor );
    if ( result == hoxRC_OK )
    {
        this->AddRole( hoxRole( pTable->GetId(), requestColor ) );
    }
    return result;
}

hoxResult 
hoxPlayer::LeaveTable( hoxTable_SPtr pTable )
{
    const char* FNAME = "hoxPlayer::LeaveTable";

    wxCHECK_MSG(pTable.get() != NULL, hoxRC_ERR, "The table is NULL." );

    wxLogDebug("%s: Player [%s] is leaving table [%s]...", 
        FNAME, this->GetName().c_str(), pTable->GetId().c_str());

    pTable->OnLeave_FromPlayer( this );
    this->RemoveRoleAtTable( pTable->GetId() );

    return hoxRC_OK;
}

hoxResult 
hoxPlayer::LeaveAllTables()
{
    const char* FNAME = "hoxPlayer::LeaveAllTables";
    bool bErrorFound = false;

    wxLogDebug("%s: ENTER.", FNAME);

    while ( ! m_roles.empty() )
    {
        const wxString tableId = m_roles.front().tableId;
        m_roles.pop_front();

        // Find the table hosted on this system using the specified table-Id.
        hoxTable_SPtr pTable = m_site->FindTable( tableId );
        if ( pTable.get() == NULL )
        {
            wxLogError("%s: Failed to find table with ID = [%s].", FNAME, tableId.c_str());
            bErrorFound = true;
            continue;
        }

        // Inform the table that this player is leaving...
        this->LeaveTable( pTable );
    }

    return bErrorFound ? hoxRC_ERR : hoxRC_OK;
}

void 
hoxPlayer::ResetConnection()
{ 
    const char* FNAME = "hoxPlayer::ResetConnection";

    wxLogDebug("%s: ENTER.", FNAME);

    if ( m_connection.get() != NULL )
    {
        wxLogDebug("%s: Request the Connection thread to be shutdowned...", FNAME);
        hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_SHUTDOWN ) );
        m_connection->AddRequest( apRequest );

        m_connection->Shutdown();
        m_connection.reset();
    }
}

void 
hoxPlayer::OnClose_FromTable( const wxString& tableId )
{
    this->RemoveRoleAtTable( tableId );
}

void 
hoxPlayer::OnRequest_FromTable( hoxRequest_APtr apRequest )
{
    const char* FNAME = "hoxPlayer::OnRequest_FromTable";

    if ( m_connection.get() == NULL )
    {
        wxLogDebug("%s: No connection. Fine. Ignore this Request.", FNAME);
        return;
    }

    apRequest->sender = this;
    this->AddRequestToConnection( apRequest );
}

void 
hoxPlayer::OnClosing_FromSite()
{
    const char* FNAME = "hoxPlayer::OnClosing_FromSite";

    wxLogDebug("%s: ENTER. (player = [%s])", FNAME, this->GetName().c_str());

    m_siteClosing = true; // *** Turn it ON!

    if ( m_roles.empty() )
    {
		_PostSite_ShutdownReady();
    }
}

bool 
hoxPlayer::SetConnection( hoxConnection_APtr connection )
{
    const char* FNAME = "hoxPlayer::SetConnection";

    if ( m_connection.get() != NULL )
    {
        wxLogDebug("%s: Connection already set to this Player [%s]. Fine. END.", 
            FNAME, GetName().c_str());
        return false;
    }

    wxLogDebug("%s: Assign the connection to this user [%s]", FNAME, GetName().c_str());
    m_connection = connection;

    return true;
}

void 
hoxPlayer::StartConnection()
{
    const char* FNAME = "hoxPlayer::StartConnection";

    wxCHECK_RET( m_connection.get() != NULL, "The connection must have been set." );

    m_connection->Start();
}

void 
hoxPlayer::ShutdownConnection()
{
    const char* FNAME = "hoxPlayer::ShutdownConnection";

    wxLogDebug("%s: Player [%s] requesting the Connection to be shutdowned...", 
        FNAME, this->GetName().c_str());
    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_SHUTDOWN ) );
    this->AddRequestToConnection( apRequest );
}

void 
hoxPlayer::AddRequestToConnection( hoxRequest_APtr apRequest )
{ 
    const char* FNAME = "hoxPlayer::AddRequestToConnection";

    if ( m_connection.get() == NULL )
    {
        wxLogDebug("%s: *** WARN *** No connection set. Deleting the request.", FNAME);
        return;
    }

    bool bIsShutdown = (apRequest->type == hoxREQUEST_SHUTDOWN);

    m_connection->AddRequest( apRequest );

    if ( bIsShutdown )
    {
        wxLogDebug("%s: Request the Connection thread to be shutdowned...", FNAME);
        m_connection->Shutdown();
    }
}

void 
hoxPlayer::_PostSite_ShutdownReady()
{
	const char* FNAME  = "hoxPlayer::_PostSite_ShutdownReady";
	wxLogDebug("%s: ENTER.", FNAME);

    m_site->Handle_ShutdownReadyFromPlayer();
}

/************************* END OF FILE ***************************************/
