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
// Name:            hoxTableMgr.h
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

    hoxTable* CreateTable();
    hoxTable* CreateTableWithFrame( wxWindow*       parent,
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
