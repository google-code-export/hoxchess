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
// Name:            hoxPlayersUI.cpp
// Created:         04/20/2008
//
// Description:     The UI containing a list of Players.
/////////////////////////////////////////////////////////////////////////////

#include "hoxPlayersUI.h"

/* Menu Items IDs. */
enum hoxPLAYERS_Menu_Id
{
    hoxPLAYERS_UI_ID_INFO = 2000,
    hoxPLAYERS_UI_ID_INVITE,
    hoxPLAYERS_UI_ID_MSG
};

/* Columns */
static wxString s_columns[] = { _("Id"), _("Rating") };
enum hoxPLAYERS_ColumnIndex
{
    hoxPLAYERS_UI_COLUMN_ID    = 0,
    hoxPLAYERS_UI_COLUMN_SCORE = 1 /* ... or 'Rating' */
};


/* 0-based image-index into the image list.
 * NOTE: The numeric values must be maintained to match
 *       with the indices of image-list.
 */
enum hoxPLAYERS_ImageIndex
{
    hoxPLAYERS_UI_IMG_UP = 0,
    hoxPLAYERS_UI_IMG_DOWN,
    hoxPLAYERS_UI_IMG_GREEN,
    hoxPLAYERS_UI_IMG_RED,
    hoxPLAYERS_UI_IMG_GRAY
};

struct hoxPLAYERS_ImageInfo
{
    int         index;
    wxString    name;
};

static hoxPLAYERS_ImageInfo s_imageList[] =
{
    { hoxPLAYERS_UI_IMG_UP,    "up.png"         },
    { hoxPLAYERS_UI_IMG_DOWN,  "down.png"       },
    { hoxPLAYERS_UI_IMG_GREEN, "leds_GREEN.png" },
    { hoxPLAYERS_UI_IMG_RED,   "leds_RED.png"   },
    { hoxPLAYERS_UI_IMG_GRAY,  "leds_GRAY.png"  }
};

/* Event table. */
BEGIN_EVENT_TABLE(hoxPlayersUI, wxListCtrl)
    EVT_LEFT_DCLICK(                             hoxPlayersUI::OnLMouseDClick)
    EVT_CONTEXT_MENU(                            hoxPlayersUI::OnContextMenu)
    EVT_MENU(           hoxPLAYERS_UI_ID_INFO,   hoxPlayersUI::OnPlayerInfo)
    EVT_MENU(           hoxPLAYERS_UI_ID_INVITE, hoxPlayersUI::OnPlayerInvite)
    EVT_MENU(           hoxPLAYERS_UI_ID_MSG,    hoxPlayersUI::OnPlayerMsg)
    EVT_LIST_COL_CLICK( wxID_ANY,                hoxPlayersUI::OnColumnClick)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------
// CallBack function to sort Players in the list.
// ---------------------------------------------------------------------------

int wxCALLBACK
ComparePlayersCallBack( long item1, 
                        long item2, 
                        long sortOrder )
{
    if (item1 < item2)  return sortOrder == hoxPlayersUI::PLAYERS_SORT_ASCENDING ? -1 : 1;
    if (item1 > item2)  return sortOrder == hoxPlayersUI::PLAYERS_SORT_ASCENDING ? 1 : -1;
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

    for ( long colIndex = 0; colIndex < WXSIZEOF( s_columns ); ++colIndex )
    {
	    this->InsertColumn( colIndex, s_columns[colIndex] );
    }
}

bool
hoxPlayersUI::AddPlayer( const wxString&       sPlayerId,
                         const int             nPlayerScore,
                         const hoxPlayerStatus playerStatus /* = hoxPLAYER_STATUS_UNKNOWN */ )
{
    /* Remove the old item, if any. */
    bool bRemoved = this->RemovePlayer( sPlayerId );

    const long itemIndex = this->InsertItem( 0 /* Front of the list */, 
                                             sPlayerId );

    this->SetItem( itemIndex, hoxPLAYERS_UI_COLUMN_SCORE,
                   wxString::Format("%d", nPlayerScore));

    /* Set the item-date for sorting purpose (sort-by-score). */
	this->SetItemData( itemIndex, nPlayerScore);

    const int imageIndex = _StatusToImageIndex( playerStatus );
    this->SetItemImage( itemIndex, imageIndex );

    /* If the Player was NOT found (to be removed) before being inserted,
     * then he has just joined this Board.
     */
    return ( ! bRemoved );
}

bool
hoxPlayersUI::RemovePlayer( const wxString& sPlayerId )
{
    const long playerIndex = _FindPlayerIndex( sPlayerId );
    if ( playerIndex != -1 ) // found?
    {
        this->DeleteItem( playerIndex );
        return true;
    }
    return false;
}

bool
hoxPlayersUI::UpdateScore( const wxString& sPlayerId,
                           const int       nPlayerScore )
{
    const long playerIndex = _FindPlayerIndex( sPlayerId );
    if ( playerIndex == -1 ) // not found?
    {
        wxLogDebug("%s: Player [%s] not found.", __FUNCTION__, sPlayerId.c_str());
        return false;
    }

    wxListItem     row_info;  

    // Set what row it is (m_itemId is a member of the regular wxListCtrl class)
    row_info.m_itemId = playerIndex;
    // Set what column of that row we want to query for information.
    row_info.m_col = hoxPLAYERS_UI_COLUMN_SCORE;
    row_info.m_mask = wxLIST_MASK_TEXT; // Set text mask

    row_info.m_text = wxString::Format("%d", nPlayerScore);
    SetItem( row_info );

    return true;
}

bool
hoxPlayersUI::UpdateStatus( const wxString&       sPlayerId,
                            const hoxPlayerStatus playerStatus )
{
    const long playerIndex = _FindPlayerIndex( sPlayerId );
    if ( playerIndex == -1 ) // notfound?
    {
        wxLogDebug("%s: Player [%s] not found.", __FUNCTION__, sPlayerId.c_str());
        return false;
    }

    const int imageIndex = _StatusToImageIndex( playerStatus );
    this->SetItemImage( playerIndex, imageIndex );
    return true;
}

bool
hoxPlayersUI::HasPlayer( const wxString& sPlayerId ) const
{
    const long playerIndex = _FindPlayerIndex( sPlayerId );
    return ( playerIndex != -1 );
}

int
hoxPlayersUI::GetPlayerScore( const wxString& sPlayerId ) const
{
    const long playerIndex = _FindPlayerIndex( sPlayerId );
    if ( playerIndex != -1 ) // found?
    {
        const wxString sScore = _GetCellContent( playerIndex,
                                                 hoxPLAYERS_UI_COLUMN_SCORE );
        return ::atoi( sScore.c_str() );
    }
    return hoxSCORE_UNKNOWN;
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
        sPlayerId = _GetCellContent( nSelectedItem, hoxPLAYERS_UI_COLUMN_ID );
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

    menu.Append( hoxPLAYERS_UI_ID_INFO, _("&Info"), _("Info of Player") );
    menu.Append( hoxPLAYERS_UI_ID_INVITE, _("&Invite"), _("Invite Player") );
    menu.Append( hoxPLAYERS_UI_ID_MSG, _("&Message..."), _("Send a private message") );

    PopupMenu(&menu, point.x, point.y);
}
void
hoxPlayersUI::OnPlayerInfo( wxCommandEvent& event )
{
    if ( m_owner == NULL ) return;

    const wxString sPlayerId = this->GetSelectedPlayer();
    if ( sPlayerId.empty() ) return;

    wxLogDebug("%s: Request Info for Player [%s]...", __FUNCTION__, sPlayerId.c_str());
    m_owner->OnPlayersUIEvent( EVENT_TYPE_INFO, sPlayerId );
}

void
hoxPlayersUI::OnPlayerInvite( wxCommandEvent& event )
{
    if ( m_owner == NULL ) return;

    const wxString sPlayerId = this->GetSelectedPlayer();
    if ( sPlayerId.empty() ) return;

    wxLogDebug("%s: Invite Player [%s]...", __FUNCTION__, sPlayerId.c_str());
    m_owner->OnPlayersUIEvent( EVENT_TYPE_INVITE, sPlayerId );
}

void
hoxPlayersUI::OnPlayerMsg( wxCommandEvent& event )
{
    if ( m_owner == NULL ) return;

    const wxString sPlayerId = this->GetSelectedPlayer();
    if ( sPlayerId.empty() ) return;

    wxLogDebug("%s: Send a message to Player [%s]...", __FUNCTION__, sPlayerId.c_str());
    m_owner->OnPlayersUIEvent( EVENT_TYPE_MSG, sPlayerId );
}

void
hoxPlayersUI::OnColumnClick( wxListEvent& event )
{
    int col = event.GetColumn();

    if ( col == hoxPLAYERS_UI_COLUMN_SCORE )  // "Rating" column?
    {
        m_sortOrderByRating = ( m_sortOrderByRating == PLAYERS_SORT_ASCENDING
                               ? PLAYERS_SORT_DESCENDING
                               : PLAYERS_SORT_ASCENDING  );

        this->SortItems( ComparePlayersCallBack, m_sortOrderByRating );

        wxListItem item;
        item.SetMask(wxLIST_MASK_IMAGE);
        item.SetImage( m_sortOrderByRating == PLAYERS_SORT_ASCENDING
                       ? hoxPLAYERS_UI_IMG_UP
                       : hoxPLAYERS_UI_IMG_DOWN );
        SetColumn(col, item);
    }
}

long
hoxPlayersUI::_FindPlayerIndex( const wxString& sPlayerId ) const
{
    const int nCount = this->GetItemCount();
    for ( int iRow = 0; iRow < nCount; ++iRow )
    {
        if ( sPlayerId == _GetCellContent( iRow,
                                           hoxPLAYERS_UI_COLUMN_ID ) ) // matched?
        {
            return iRow;  // Found the index.
        }
    }
    return -1;   // Not found.
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

    wxString filename;
    wxImage image;
    for ( int index = 0; index < WXSIZEOF( s_imageList ); ++index )
    {
        filename.Printf("%s/%s", IMAGES_PATH, s_imageList[index].name.c_str());
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

int 
hoxPlayersUI::_StatusToImageIndex( const hoxPlayerStatus playerStatus ) const
{
    switch ( playerStatus )
    {
        case hoxPLAYER_STATUS_PLAYING:   return hoxPLAYERS_UI_IMG_RED;
        case hoxPLAYER_STATUS_OBSERVING: return hoxPLAYERS_UI_IMG_GREEN;
        case hoxPLAYER_STATUS_SOLO:      return hoxPLAYERS_UI_IMG_GRAY;
        default: /* UNKNOWN */           return hoxPLAYERS_UI_IMG_GREEN;
    }
}

/************************* END OF FILE ***************************************/
