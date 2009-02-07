/***************************************************************************
 *  Copyright 2007-2009 Huy Phan  <huyphan@playxiangqi.com>                *
 *                      Bharatendra Boddu (bharathendra at yahoo dot com)  *
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
// Name:            hoxSitesUI.cpp
// Created:         08/30/2008
//
// Description:     The UI containing a list of Sites.
/////////////////////////////////////////////////////////////////////////////

#include "hoxSitesUI.h"
#include "MyApp.h"    // wxGetApp()

IMPLEMENT_DYNAMIC_CLASS(hoxSitesUI, wxTreeCtrl)

/* Event table. */
BEGIN_EVENT_TABLE(hoxSitesUI, wxTreeCtrl)
    EVT_TREE_SEL_CHANGED(wxID_ANY, hoxSitesUI::OnTreeSelChanged)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------
// hoxSitesUI class
// ---------------------------------------------------------------------------

hoxSitesUI::hoxSitesUI( wxWindow* parent )
            : wxTreeCtrl( parent,
                          wxID_ANY,
                          wxDefaultPosition,
                          wxDefaultSize,
                          wxTR_DEFAULT_STYLE | wxNO_BORDER | wxTR_HIDE_ROOT )
{
    /* NOTE:
     * wxTR_HIDE_ROOT is used... Thus, we cannot "Expand(root)" as follows:
     *   this->Expand(rootId); // Make all servers visible.
     */

    this->AddRoot( "Sites" );
}

hoxSite*
hoxSitesUI::GetSelectedSite() const
{
    hoxTable_SPtr selectedTable;  // Not used!
    return this->GetSelectedSite( selectedTable );
}

hoxSite*
hoxSitesUI::GetSelectedSite( hoxTable_SPtr& selectedTable ) const
{
    selectedTable.reset();

    hoxSite* selectedSite = NULL;

    wxTreeItemId selectedItem = this->GetSelection();

    wxTreeItemId itemId;
    ItemData*    itemData = NULL;

    for ( itemId = selectedItem; 
          itemId.IsOk(); 
          itemId = this->GetItemParent( itemId ) )
    {
        itemData = (ItemData*) this->GetItemData(itemId);
        if (    itemData != NULL
             && itemData->type == TREE_ITEM_TYPE_SITE )
        {
            selectedSite = ((ItemData_SITE*) itemData)->site;
            break;
        }
        else if ( itemData != NULL
             &&   itemData->type == TREE_ITEM_TYPE_TABLE )
        {
            selectedTable = ((ItemData_TABLE*) itemData)->pTable;
            selectedSite = selectedTable->GetSite();
            break;
        }
    }

    return selectedSite;
}

bool
hoxSitesUI::AddSite( hoxSite* site )
{
    wxCHECK( site, false );

    const wxString sSiteId = site->GetName();

    /* Remove the old item, if any. */
    bool bRemoved = this->RemoveSite( site );

    wxTreeItemId  rootId = this->GetRootItem();
    wxTreeItemId  siteTreeId;

    siteTreeId = this->AppendItem( rootId, sSiteId );

    /* Set the Site's Item-Data to be used as a key identifying this Site.
     */
    ItemData_SITE* itemData = new ItemData_SITE( site );
    this->SetItemData(siteTreeId, itemData);

    /* Select the first site if there is only one. */
    if ( this->GetChildrenCount(rootId, false /* recursively */ ) )
    {
        this->SelectItem( siteTreeId );
    }

    return ( ! bRemoved );
}

bool
hoxSitesUI::RemoveSite( hoxSite* site )
{
    wxTreeItemId   itemId;

    if ( ! _FindSite( site, itemId ) ) // not found?
    {
        return false;  // Not found?
    }

    this->Delete( itemId );
    return true;  // Found and removed!
}

bool
hoxSitesUI::AddTableToSite( hoxSite*      site,
                            hoxTable_SPtr pTable )
{
    wxTreeItemId   siteItemId;

    if ( ! _FindSite( site, siteItemId ) ) // not found?
    {
        return false;  // Not found
    }

    wxTreeItemId       tableItemId;

    if ( _FindTableInSite( siteItemId,
                           pTable,
                           tableItemId ) ) // found
    {
        return false;  // already exist.
    }

    wxString tableStr;
    tableStr.Printf("Table #%s", pTable->GetId().c_str());
    tableItemId = this->AppendItem(siteItemId, tableStr);

    /* Set the Table's Item-Data to be used as a key identifying this Table.
     */
    ItemData_TABLE* itemData = new ItemData_TABLE( pTable );
    this->SetItemData(tableItemId, itemData);

    this->SelectItem( tableItemId );

    return true;  // Added!
}

bool
hoxSitesUI::RemoveTableFromSite( hoxSite*      site,
                                 hoxTable_SPtr pTable )
{
    wxTreeItemId   siteItemId;

    if ( ! _FindSite( site, siteItemId ) ) // not found?
    {
        return false;  // Not found
    }

    wxTreeItemId       tableItemId;

    if ( ! _FindTableInSite( siteItemId,
                             pTable,
                             tableItemId ) ) // not found?
    {
        return false;  // Not found
    }

    this->Delete( tableItemId );
    return true;  // Found and removed!
}

void
hoxSitesUI::OnTreeSelChanged( wxTreeEvent& event )
{
    hoxSite* selectedSite = this->GetSelectedSite();
    if ( selectedSite )
    {
        hoxPlayersUI* playersUI = selectedSite->GetPlayersUI();
        if ( playersUI )
        {
            wxLogDebug("%s: Set active Players-UI to [%s].",
                __FUNCTION__, selectedSite->GetName().c_str());
            wxGetApp().GetFrame()->SetActiveSitePlayersUI( playersUI );
        }
    }
}

bool
hoxSitesUI::_FindSite( hoxSite*      site,
                       wxTreeItemId& itemId ) const
{
    wxTreeItemId      rootId = this->GetRootItem();
    wxTreeItemIdValue cookie;

    for ( itemId = this->GetFirstChild( rootId, cookie );
          itemId.IsOk();
          itemId = this->GetNextChild( rootId, cookie ) )
    {
        wxTreeItemData* itemData = this->GetItemData( itemId );
        if (   itemData != NULL
            && ((ItemData_SITE*) itemData)->site == site  )
        {
            return true;
        }
    }

    return false;  // Not found?
}

bool
hoxSitesUI::_FindTableInSite( const wxTreeItemId& siteItemId,
                              hoxTable_SPtr       pTable,
                              wxTreeItemId&       itemId ) const
{
    wxTreeItemIdValue cookie;

    for ( itemId = this->GetFirstChild( siteItemId, cookie );
          itemId.IsOk();
          itemId = this->GetNextChild( siteItemId, cookie ) )
    {
        wxTreeItemData* itemData = this->GetItemData( itemId );
        if (   itemData != NULL
            && ((ItemData_TABLE*) itemData)->pTable == pTable  )
        {
            return true;
        }
    }

    return false;  // Not found?
}

/************************* END OF FILE ***************************************/
