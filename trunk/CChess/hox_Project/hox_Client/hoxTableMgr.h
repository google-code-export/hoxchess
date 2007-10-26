/////////////////////////////////////////////////////////////////////////////
// Name:            hoxTableMgr.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/06/2007
//
// Description:     The manager that manages ALL the tables in this server.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_TABLE_MGR_H_
#define __INCLUDED_HOX_TABLE_MGR_H_

#include "wx/wx.h"
#include "hoxTable.h"

/**
 * A singleton class that manages all tables in the system.
 */
class hoxTableMgr
{
public:
    static hoxTableMgr* GetInstance();        

    ~hoxTableMgr();

    hoxTable* CreateTable( wxWindow*       parent,
                           const wxString& tableId );
    void RemoveTable( hoxTable* table );
    
    hoxTable* FindTable( const wxString& tableId ) const;
    const hoxTableList& GetTables() const { return m_tables; } 


private:
    hoxTableMgr();

    static hoxTableMgr* m_instance;  // The single instance

    hoxTableList    m_tables; // The list of all tables in the system.
};

#endif /* __INCLUDED_HOX_TABLE_MGR_H_ */
