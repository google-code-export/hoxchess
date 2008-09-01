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
// Name:            hoxSitesUI.h
// Created:         08/30/2008
//
// Description:     The UI containing a list of Sites.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_SITES_UI_H__
#define __INCLUDED_SITES_UI_H__

#include <wx/treectrl.h>
#include "hoxTypes.h"

/* Forward declarations */
class hoxSite;

// ---------------------------------------------------------------------------
// hoxSitesUI class
// ---------------------------------------------------------------------------

class hoxSitesUI : public wxTreeCtrl
{
public:
    class SiteTreeItemData : public wxTreeItemData
    {
    public:
        SiteTreeItemData(hoxSite* site = NULL) : m_site(site) {}
        ~SiteTreeItemData() {}

        hoxSite* GetSite() const { return m_site; }

    private:
        hoxSite*  m_site;
    };

    class TableTreeItemData : public wxTreeItemData
    {
    public:
        TableTreeItemData(hoxTable_SPtr table) : m_table(table) {}
        ~TableTreeItemData() {}

        hoxTable_SPtr GetTable() const { return m_table; }

    private:
        hoxTable_SPtr  m_table;
    };

public:
    hoxSitesUI() {} // DUMMY default constructor required for RTTI info.
    hoxSitesUI( wxWindow* parent );
    virtual ~hoxSitesUI() {}

    /* Public API */

    hoxSite* GetSelectedSite(hoxTable_SPtr& selectedTable) const;

    bool AddSite( hoxSite* site );
    bool RemoveSite( hoxSite* site );

    bool AddTableToSite( hoxSite*      site,
                         hoxTable_SPtr pTable );

    bool RemoveTableFromSite( hoxSite*      site,
                              hoxTable_SPtr pTable );

    /* Event handlers. */

private:
    bool _FindSite( hoxSite*      site,
                    wxTreeItemId& itemId ) const;

    bool _FindTableInSite( const wxTreeItemId& siteItemId,
                           hoxTable_SPtr       pTable,
                           wxTreeItemId&       itemId ) const;

private:

    DECLARE_DYNAMIC_CLASS(hoxSitesUI)
    DECLARE_EVENT_TABLE()

}; // END of hoxSitesUI

#endif /* __INCLUDED_SITES_UI_H__ */
