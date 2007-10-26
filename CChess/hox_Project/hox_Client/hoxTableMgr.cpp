/////////////////////////////////////////////////////////////////////////////
// Name:            hoxTableMgr.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/06/2007
//
// Description:     The manager that manages ALL the tables in this server.
/////////////////////////////////////////////////////////////////////////////

#include "hoxTableMgr.h"
#include "hoxBoard.h"
#include "hoxReferee.h"

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
hoxTableMgr::CreateTable( wxWindow*       parent,
                          const wxString& tableId )
{
    /* Create a Referee */
    hoxIReferee* referee = new hoxReferee();

    /* Create a Table (with the referee).
     *
     * NOTE: Since this App can function as a server, it is important
     *       to have a notion of a Table withOUT a Board (or Table's GUI).
     */
    hoxTable* table = new hoxTable( tableId, referee );

    /* Create a "hidden" Board */
    hoxSimpleBoard* board = new hoxSimpleBoard( parent, PIECES_PATH, referee );

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
