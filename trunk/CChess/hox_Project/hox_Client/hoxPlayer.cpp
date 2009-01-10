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
// Name:            hoxPlayer.cpp
// Created:         10/06/2007
//
// Description:     The Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxPlayer.h"
#include "hoxConnection.h"
#include "hoxSite.h"

DEFINE_EVENT_TYPE(hoxEVT_CONNECTION_RESPONSE)

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
    const char* FNAME = __FUNCTION__;
    wxLogDebug("%s: ENTER. (%s)", FNAME, this->GetId().c_str());

    if ( m_connection.get() != NULL )
    {
        this->ShutdownConnection();
    }
}

hoxTable_SPtr
hoxPlayer::GetFrontTable() const
{
    hoxTable_SPtr pTable;  // Default: An 'empty' pointer.

    hoxTableSet::const_iterator front_it = m_tables.begin();
    if ( front_it != m_tables.end() )
    {
        pTable = (*front_it);
    }

    return pTable;
}

hoxColor
hoxPlayer::GetFrontRole( wxString& sTableId ) const
{
    hoxColor      myRole = hoxCOLOR_UNKNOWN;
    hoxTable_SPtr pTable = this->GetFrontTable();

    if ( pTable.get() != NULL )
    {
        myRole = pTable->GetPlayerRole( this->GetId() );
        sTableId = pTable->GetId();
    }

    return myRole;
}

hoxTable_SPtr
hoxPlayer::FindTable( const wxString& sTableId ) const
{
    hoxTable_SPtr pTable;

    for ( hoxTableSet::const_iterator it = m_tables.begin();
                                      it != m_tables.end(); ++it )
    {
        if ( (*it)->GetId() == sTableId )
        {
            pTable = (*it);
            break;
        }
    }

    return pTable;
}

hoxResult 
hoxPlayer::JoinTableAs( hoxTable_SPtr pTable,
                        hoxColor      requestColor )
{
    wxCHECK_MSG( pTable.get() != NULL, hoxRC_ERR, "The table is NULL." );

    hoxResult result = pTable->AssignPlayerAs( this, requestColor );
    if ( result == hoxRC_OK )
    {
        m_tables.insert( pTable );  // Make a copy.
    }
    return result;
}

hoxResult 
hoxPlayer::LeaveTable( hoxTable_SPtr pTable )
{
    const char* FNAME = __FUNCTION__;

    wxCHECK_MSG(pTable.get() != NULL, hoxRC_ERR, "The table is NULL." );

    wxLogDebug("%s: Player [%s] is leaving table [%s]...", 
        FNAME, this->GetId().c_str(), pTable->GetId().c_str());

    pTable->OnLeave_FromPlayer( this );
    m_tables.erase( pTable );

    return hoxRC_OK;
}

hoxResult 
hoxPlayer::LeaveAllTables()
{
    const char* FNAME = __FUNCTION__;

    wxLogDebug("%s: ENTER.", FNAME);

    while ( ! m_tables.empty() )
    {
        hoxTable_SPtr pTable = *(m_tables.begin());

        /* Inform the table that this player is leaving...
         * NOTE: This call also removes the Table from the player's
         *       internal table-list.
         */
        this->LeaveTable( pTable );
    }

    return hoxRC_OK;
}

void 
hoxPlayer::ResetConnection()
{ 
    const char* FNAME = __FUNCTION__;

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
    /* Remove reference to the Table with such Id. */

    hoxTable_SPtr pTable = this->FindTable( tableId );
    if ( pTable.get() != NULL )
    {
        m_tables.erase( pTable );
    }
}

void 
hoxPlayer::OnRequest_FromTable( hoxRequest_APtr apRequest )
{
    const char* FNAME = __FUNCTION__;

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
    const char* FNAME = __FUNCTION__;

    wxLogDebug("%s: ENTER. (player = [%s])", FNAME, this->GetId().c_str());

    m_siteClosing = true; // *** Turn it ON!

    if ( m_tables.empty() )
    {
        m_site->Handle_ShutdownReadyFromPlayer();
    }
}

bool 
hoxPlayer::SetConnection( hoxConnection_APtr connection )
{
    const char* FNAME = __FUNCTION__;

    if ( m_connection.get() != NULL )
    {
        wxLogDebug("%s: Connection already set to this Player [%s]. Fine. END.", 
            FNAME, this->GetId().c_str());
        return false;
    }

    wxLogDebug("%s: Assign the connection to this user [%s]", FNAME, this->GetId().c_str());
    m_connection = connection;

    return true;
}

void 
hoxPlayer::StartConnection()
{
    const char* FNAME = __FUNCTION__;

    wxCHECK_RET( m_connection.get() != NULL, "The connection must have been set." );

    m_connection->Start();
}

void 
hoxPlayer::ShutdownConnection()
{
    const char* FNAME = __FUNCTION__;

    wxLogDebug("%s: Player [%s] requesting the Connection to be shutdowned...", 
        FNAME, this->GetId().c_str());
    hoxRequest_APtr apRequest( new hoxRequest( hoxREQUEST_SHUTDOWN ) );
    this->AddRequestToConnection( apRequest );
}

void 
hoxPlayer::AddRequestToConnection( hoxRequest_APtr apRequest )
{ 
    const char* FNAME = __FUNCTION__;

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

/************************* END OF FILE ***************************************/
