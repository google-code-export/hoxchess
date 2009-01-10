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
// Name:            hoxTableMgr.h
// Created:         10/06/2007
//
// Description:     A Table-Manager that manages a group of tables.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_TABLE_MGR_H_
#define __INCLUDED_HOX_TABLE_MGR_H_

#include "hoxTable.h"

class hoxSite;

/**
 * A Table-Manager that manages a group of tables.
 */
class hoxTableMgr
{
public:
    hoxTableMgr();
    ~hoxTableMgr();

    hoxTable_SPtr CreateTable( const wxString& tableId,
                               hoxSite*        site,
                               hoxGameType     gameType );
    void RemoveTable( hoxTable_SPtr pTable );
    
    hoxTable_SPtr FindTable( const wxString& tableId ) const;
    const hoxTableList& GetTables() const { return m_tables; } 

private:
    hoxTableList    m_tables; // The list of all tables in the system.
};

#endif /* __INCLUDED_HOX_TABLE_MGR_H_ */
