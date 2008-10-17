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
// Name:            hoxPlayersUI.h
// Created:         04/20/2008
//
// Description:     The UI containing a list of Players.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_PLAYERS_UI_H__
#define __INCLUDED_HOX_PLAYERS_UI_H__

#include <wx/listctrl.h>
#include <wx/imaglist.h>

enum PlayersSortOrder
{
    PLAYERS_SORT_NONE,
    PLAYERS_SORT_ASCENDING,
    PLAYERS_SORT_DESCENDING
};

// ---------------------------------------------------------------------------
// hoxPlayersUI class
// ---------------------------------------------------------------------------

class hoxPlayersUI : public wxListCtrl
{
public:
    /**
     * Request types to communicate with the Owner.
     */
    enum EventType
    {
        EVENT_TYPE_INFO,
        EVENT_TYPE_INVITE
    };

    /**
     * The UI's Owner. 
     */
    class UIOwner
    {
    public:
        virtual void OnPlayersUIEvent( EventType       eventType,
                                       const wxString& sPlayerId ) = 0;
    };

public:
    hoxPlayersUI( wxWindow* parent );
    virtual ~hoxPlayersUI() {}

    void SetOwner( UIOwner* owner ) { m_owner = owner; }

    bool AddPlayer( const wxString& sPlayerId,
                    const int       nPlayerScore );

    bool RemovePlayer( const wxString& sPlayerId );

    void RemoveAllPlayers();

    wxString GetSelectedPlayer() const;

    void OnLMouseDClick( wxMouseEvent& event );
    void OnContextMenu( wxContextMenuEvent& event );
    void OnPlayerInfo( wxCommandEvent& event );
    void OnPlayerInvite( wxCommandEvent& event );
    void OnColumnClick( wxListEvent& event );

private:
    wxString _GetCellContent( const long row_number,
                              const int  column ) const;

    void _InitializeImageList();

private:
    UIOwner*          m_owner;   // This UI's owner.

    PlayersSortOrder  m_sortOrderByRating;
            /* NOTE: Only support sorting by Rating for now */

    wxImageList*      m_imageList;

    DECLARE_EVENT_TABLE()

}; // END of hoxPlayersUI

#endif /* __INCLUDED_HOX_PLAYERS_UI_H__ */
