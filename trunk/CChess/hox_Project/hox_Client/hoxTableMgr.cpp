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

// Declare the single instance.
hoxTableMgr* hoxTableMgr::m_instance = NULL;

hoxTableMgr*
hoxTableMgr::GetInstance()
{
    if ( m_instance == NULL )
    {
        m_instance = new hoxTableMgr();
    }
        
    return m_instance;
}

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
hoxTableMgr::CreateTable()
{
    hoxTable* newTable = NULL;
    wxString  tableId;

    /* Create a GUI Frame for the new Table. */
    MyFrame* frame = wxGetApp().GetFrame();
    MyChild* childFrame = frame->CreateFrameForTable( tableId );

    /* Create a new table with newly created Frame. */
    newTable = this->CreateTableWithFrame( childFrame, 
                                           tableId );
    childFrame->SetTable( newTable );
    childFrame->Show( true );

    return newTable;
}

hoxTable*
hoxTableMgr::CreateTableWithFrame( wxWindow*       parent,
                                   const wxString& tableId )
{
    /* Create a Referee */
    hoxIReferee* referee = new hoxReferee();

    /* Create a Table (with the referee).
     *
     * NOTE: Since this App can function as a server, it is important
     *       to have a notion of a Table withOUT a Board (or Table's GUI).
     */
    hoxTable* table = new hoxTable( m_site, tableId, referee );

    /* Create a "hidden" Board */
    hoxBoard* board = new hoxBoard( parent, PIECES_PATH, referee,
                                    wxDefaultPosition,
                                    parent->GetSize() );

    /* Attach the Board to the Table to be served as a Table's GUI 
     * Also, trigger the Board to be displayed.
     */
    table->SetBoard( board );

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
