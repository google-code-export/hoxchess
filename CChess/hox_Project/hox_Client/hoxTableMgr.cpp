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
// Name:            hoxTableMgr.cpp
// Created:         10/06/2007
//
// Description:     The manager that manages ALL the tables in this server.
/////////////////////////////////////////////////////////////////////////////

#include "hoxTableMgr.h"
#include "hoxBoard.h"
#include "hoxReferee.h"
#include "MyApp.h"      // wxGetApp
#include "MyFrame.h"
#include "MyChild.h"

hoxTableMgr::hoxTableMgr()
{
}

hoxTableMgr::~hoxTableMgr()
{
    for ( hoxTableList::iterator it = m_tables.begin(); 
                              it != m_tables.end(); ++it )
    {
        delete (*it);
    }
}

hoxTable*
hoxTableMgr::CreateTable( const wxString& tableId )
{
    /* Create a Referee */
    hoxIReferee_SPtr referee( new hoxReferee() );

    /* Create a Table (with the referee).
     *
     * NOTE: Since this App can function as a server, it is important
     *       to have a notion of a Table withOUT a Board (or Table's GUI).
     */
    hoxTable* table = new hoxTable( m_site, tableId, referee );

    /* Save this table to our list */
    m_tables.push_back( table);

    return table;
}

void
hoxTableMgr::RemoveTable( hoxTable* table )
{
    wxASSERT( table != NULL );
    delete table;
    m_tables.remove( table );
}

hoxTable* 
hoxTableMgr::FindTable( const wxString& tableId ) const
{
    for ( hoxTableList::const_iterator it = m_tables.begin(); 
                              it != m_tables.end(); ++it )
    {
        if ( tableId == (*it)->GetId() )
            return (*it);
    }

    return NULL;
}

/************************* END OF FILE ***************************************/
