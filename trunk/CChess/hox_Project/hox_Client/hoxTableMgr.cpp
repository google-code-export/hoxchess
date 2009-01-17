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
// Name:            hoxTableMgr.cpp
// Created:         10/06/2007
//
// Description:     The manager that manages ALL the tables in this server.
/////////////////////////////////////////////////////////////////////////////

#include "hoxTableMgr.h"
#include "hoxReferee.h"

hoxTableMgr::hoxTableMgr()
{
}

hoxTableMgr::~hoxTableMgr()
{
}

hoxTable_SPtr
hoxTableMgr::CreateTable( const wxString& tableId,
                          hoxSite*        site,
                          hoxGameType     gameType,
                          const wxString& sSavedFile /* = "" */)
{
    /* Create a Referee */
    hoxIReferee_SPtr referee( new hoxReferee( sSavedFile ) );

    /* Create a Table (with the referee).
     *
     * NOTE: Since this App can function as a server, it is important
     *       to have a notion of a Table withOUT a Board (or Table's GUI).
     */
    hoxTable_SPtr pTable( gameType == hoxGAME_TYPE_PRACTICE
                         ? new hoxPracticeTable( site, tableId, referee )
                         : new hoxTable( site, tableId, referee ) );
    
    pTable->SetGameType( gameType );

    /* Save this table to our list */
    m_tables.push_back( pTable);

    return pTable;
}

void
hoxTableMgr::RemoveTable( hoxTable_SPtr pTable )
{
    wxASSERT( pTable.get() != NULL );
    m_tables.remove( pTable );
}

hoxTable_SPtr
hoxTableMgr::FindTable( const wxString& tableId ) const
{
    hoxTable_SPtr foundTable;

    for ( hoxTableList::const_iterator it = m_tables.begin(); 
                                       it != m_tables.end(); ++it )
    {
        if ( tableId == (*it)->GetId() )
        {
            foundTable = (*it);
            break;
        }
    }

    return foundTable;
}

/************************* END OF FILE ***************************************/
