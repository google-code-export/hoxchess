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
// Name:            hoxPlayersUI.cpp
// Created:         04/20/2008
//
// Description:     The UI containing a list of Players.
/////////////////////////////////////////////////////////////////////////////

#include "hoxPlayersUI.h"
#include "hoxTypes.h"

/* Menu Items IDs. */
enum
{
    hoxPLAYERS_UI_ID_INFO = 2000,
    hoxPLAYERS_UI_ID_INVITE
};

/* Event table. */
BEGIN_EVENT_TABLE(hoxPlayersUI, wxListCtrl)
    EVT_LEFT_DCLICK(hoxPlayersUI::OnLMouseDClick)
    EVT_CONTEXT_MENU(hoxPlayersUI::OnContextMenu)
    EVT_MENU(hoxPLAYERS_UI_ID_INFO, hoxPlayersUI::OnPlayerInfo)
    EVT_MENU(hoxPLAYERS_UI_ID_INVITE, hoxPlayersUI::OnPlayerInvite)
    EVT_LIST_COL_CLICK(wxID_ANY, hoxPlayersUI::OnColumnClick)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------
// CallBack function to sort Players in the list.
// ---------------------------------------------------------------------------

int wxCALLBACK
ComparePlayersCallBack( long item1, 
                        long item2, 
                        long sortOrder /* not used */ )
{
    if (item1 < item2)  return sortOrder == PLAYERS_SORT_ASCENDING ? -1 : 1;
    if (item1 > item2)  return sortOrder == PLAYERS_SORT_ASCENDING ? 1 : -1;
    return 0;
}

// ---------------------------------------------------------------------------
// hoxPlayersUI class
// ---------------------------------------------------------------------------

hoxPlayersUI::hoxPlayersUI( wxWindow* parent )
            : wxListCtrl( parent,
                          wxID_ANY,
                          wxDefaultPosition,
                          wxDefaultSize,
                          wxLC_REPORT | wxLC_SINGLE_SEL )
            , m_owner( NULL )
            , m_sortOrderByRating( PLAYERS_SORT_NONE )
            , m_imageList( NULL )
{
    _InitializeImageList();

    wxString columns[] = { "Id", "Rating" };

    for ( long colIndex = 0; colIndex < WXSIZEOF( columns ); ++colIndex )
    {
	    this->InsertColumn( colIndex, columns[colIndex] );
    }
}

bool
hoxPlayersUI::AddPlayer( const wxString& sPlayerId,
                         const int       nPlayerScore )
{
    /* Remove the old item, if any. */
    bool bRemoved = this->RemovePlayer( sPlayerId );

    long   itemIndex = 0;
    long   colIndex = 0;

    itemIndex = this->InsertItem( itemIndex, sPlayerId );

    this->SetItem( itemIndex, ++colIndex,
                   wxString::Format("%d", nPlayerScore));

	this->SetItemData( itemIndex, nPlayerScore);

    this->SetItemImage( itemIndex, -1 ); // No image on items.

    /* If the Player was NOT found (to be removed) before being inserted,
     * then he has just joined this Board.
     */
    return ( ! bRemoved );
}

bool
hoxPlayersUI::RemovePlayer( const wxString& sPlayerId )
{
    const int nCount = this->GetItemCount();
    for ( int i = 0; i < nCount; ++i )
    {
        if ( _GetCellContent( i, 0 ) == sPlayerId ) // matched?
        {
            this->DeleteItem( i );
            return true;
        }
    }
    return false;
}

void
hoxPlayersUI::RemoveAllPlayers()
{
    this->DeleteAllItems();
}

wxString
hoxPlayersUI::GetSelectedPlayer() const
{
    wxString sPlayerId;  // The selected Player-Id.

    long nSelectedItem = this->GetNextItem( -1,
                                            wxLIST_NEXT_ALL,
                                            wxLIST_STATE_SELECTED );
    if ( nSelectedItem != -1 ) // Got a selected item?
    {
        sPlayerId = _GetCellContent( nSelectedItem, 0 );
    }
    return sPlayerId;
}

void
hoxPlayersUI::OnLMouseDClick( wxMouseEvent& event )
{
    wxCommandEvent DUMMY_event;
    
    this->OnPlayerInfo( DUMMY_event );
}

void
hoxPlayersUI::OnContextMenu( wxContextMenuEvent& event )
{ 
    wxPoint point = event.GetPosition(); 
    point = ScreenToClient(point);
    
    const wxString sPlayerId = this->GetSelectedPlayer();
    if ( sPlayerId.empty() ) return;

    wxMenu menu;

    menu.Append( hoxPLAYERS_UI_ID_INFO, _("&Info\tCtrl-P"), _("Info of Player") );
    menu.Append( hoxPLAYERS_UI_ID_INVITE, _("&Invite\tCtrl-I"), _("Invite Player") );

    PopupMenu(&menu, point.x, point.y);
}
void
hoxPlayersUI::OnPlayerInfo( wxCommandEvent& event )
{
    const char* FNAME = __FUNCTION__;

    if ( m_owner == NULL ) return;

    const wxString sPlayerId = this->GetSelectedPlayer();
    if ( sPlayerId.empty() ) return;

    wxLogDebug("%s: Request Info for Player [%s]...", FNAME, sPlayerId.c_str());
    m_owner->OnPlayersUIEvent( EVENT_TYPE_INFO, sPlayerId );
}

void
hoxPlayersUI::OnPlayerInvite( wxCommandEvent& event )
{
    const char* FNAME = __FUNCTION__;

    if ( m_owner == NULL ) return;

    const wxString sPlayerId = this->GetSelectedPlayer();
    if ( sPlayerId.empty() ) return;

    wxLogDebug("%s: Invite Player [%s]...", FNAME, sPlayerId.c_str());
    m_owner->OnPlayersUIEvent( EVENT_TYPE_INVITE, sPlayerId );
}

void
hoxPlayersUI::OnColumnClick( wxListEvent& event )
{
    int col = event.GetColumn();

    if ( col == 1 )  // "Rating" column?
    {
        m_sortOrderByRating = ( m_sortOrderByRating == PLAYERS_SORT_ASCENDING
                               ? PLAYERS_SORT_DESCENDING
                               : PLAYERS_SORT_ASCENDING  );

        this->SortItems( ComparePlayersCallBack, m_sortOrderByRating );

        wxListItem item;
        item.SetMask(wxLIST_MASK_IMAGE);
        item.SetImage( m_sortOrderByRating == PLAYERS_SORT_ASCENDING ? 0 : 1 );
        SetColumn(col, item);
    }
}

/**
 * CREDITS: The following code was extracted from this site:
 * http://wiki.wxwidgets.org/WxListCtrl#Get_the_string_contents_of_a_.22cell.22_in_a_LC_REPORT_wxListCtrl
 */
wxString
hoxPlayersUI::_GetCellContent( const long row_number,
                               const int  column ) const
{
    wxListItem     row_info;  
    wxString       cell_contents_string;

    // Set what row it is (m_itemId is a member of the regular wxListCtrl class)
    row_info.m_itemId = row_number;
    // Set what column of that row we want to query for information.
    row_info.m_col = column;
    // Set text mask
    row_info.m_mask = wxLIST_MASK_TEXT;

    // Get the info and store it in row_info variable.   
    GetItem( row_info );

    // Extract the text out that cell
    cell_contents_string = row_info.m_text; 

    return cell_contents_string;
}

void
hoxPlayersUI::_InitializeImageList()
{
    m_imageList = new wxImageList(8, 8);

    hoxStringList imageFilenames;
    imageFilenames.push_back( _("up.png") );
    imageFilenames.push_back( _("down.png") );
    
    wxString filename;
    wxImage image;
    for ( hoxStringList::const_iterator it = imageFilenames.begin();
                                     it != imageFilenames.end(); ++it )
    {
        filename.Printf("%s/%s", IMAGES_PATH, it->c_str());
        if ( ! image.LoadFile(filename, wxBITMAP_TYPE_PNG) ) 
        {
            wxLogWarning("%s: Failed to load Image for Player-List from path [%s].",
                __FUNCTION__, filename.c_str());
            // *** Ignore this error.
        }
        else
        {
            m_imageList->Add( wxBitmap(image) );
        }
    }
    this->AssignImageList( m_imageList, wxIMAGE_LIST_SMALL );
}

/************************* END OF FILE ***************************************/
