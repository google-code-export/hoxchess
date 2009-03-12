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
// Name:            MyFrame.h
// Created:         10/02/2007
//
// Description:     The main Frame of the App.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_MY_FRAME_H__
#define __INCLUDED_MY_FRAME_H__

#include <wx/wx.h>
#include <wx/laywin.h>   // wxSashLayoutWindow
#include <wx/splitter.h>
#include "hoxTypes.h"

/* Forward declarations */
class MyChild;
class hoxSite;
class hoxSitesUI;
class hoxPlayersUI;
class hoxLog;

/* Menu items IDs */
enum
{
    MDI_QUIT = wxID_EXIT,
    MDI_ABOUT = wxID_ABOUT,

    MDI_NEW_TABLE = hoxUI_ID_RANGE_FRAME,
    MDI_PRACTICE, // New Table to practice with the local computer
    MDI_SAVE_TABLE,
    MDI_OPEN_SAVED_TABLE,
    MDI_CLOSE_TABLE,

    MDI_CONNECT_SERVER, // Connect to server
    MDI_DISCONNECT_SERVER, // Disconnect from server
    MDI_LIST_TABLES,     // Get list of tables.

    MDI_SHOW_SERVERS_WINDOW,
    MDI_SHOW_LOG_WINDOW,

    MDI_TOGGLE,   // toggle view
    MDI_CHILD_QUIT,

    MDI_SOUND,   // toggle sound
    MDI_OPTIONS, // App's general options

    // Windows' IDs.
    ID_WINDOW_SITES
};

// ---------------------------------------------------------------------------
// MyFrame class
// ---------------------------------------------------------------------------

/**
 * The main (MDI) frame acting as the App's main GUI.
 * It manages a list of MDI child frames.
 *
 * @see MyChild
 */
class MyFrame : public wxMDIParentFrame
{
public:
    MyFrame() {} // Dummy default constructor required for RTTI info.
    MyFrame( wxWindow*          parent, 
             const wxWindowID   id, 
             const wxString&    title,
             const wxPoint&     pos, 
             const wxSize&      size, 
             const long         style );

    ~MyFrame() {}

    // -----
    void InitToolBar(wxToolBar* toolBar);

    void OnSize(wxSizeEvent& event);
    void OnServersSashDrag(wxSashEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnNewTable(wxCommandEvent& event);
    void OnCloseTable(wxCommandEvent& event);
	void OnUpdateNewTable(wxUpdateUIEvent& event);

    void OnDisconnectServer(wxCommandEvent& event);
    void OnUpdateRemoveSiteRequired(wxUpdateUIEvent& event);
    void OnUpdateCloseTable(wxUpdateUIEvent& event);
    void OnConnectServer(wxCommandEvent& event);
    void OnListTables(wxCommandEvent& event);

    void OnShowServersWindow(wxCommandEvent& event);
    void OnUpdateServersWindow(wxUpdateUIEvent& event);

    void OnShowLogWindow(wxCommandEvent& event);
    void OnUpdateShowLogWindow(wxUpdateUIEvent& event);

    void OnPractice(wxCommandEvent& event);
    void OnUpdatePractice(wxUpdateUIEvent& event);

    void OnSaveTable(wxCommandEvent& event);
    void OnOpenSavedTable(wxCommandEvent& event);
    void OnUpdateOpenSavedTable(wxUpdateUIEvent& event);

    void OnIdle( wxIdleEvent& event );
    void OnQuit( wxCommandEvent& event );
    void OnClose( wxCloseEvent& event );

    /**
     * Invoke by the Child window when it is being closed.
     * NOTE: This *special* API is used by the Child to ask the Parent
     *       for the permission to close.
     *       !!! BE CAREFUL with recursive calls. !!!
     */
    void OnChildClose( wxCloseEvent& event,
                       MyChild*      child,
                       hoxTable_SPtr pTable );

    void OnContextMenu( wxContextMenuEvent& event );
    void OnToggleSound( wxCommandEvent& event );
    void OnOptions( wxCommandEvent& event );

    /**
     * Create a GUI Frame that can be used as a frame for a new Table.
     *
     * @param sTableId The Table's Id.
     */
    MyChild* CreateFrameForTable( const wxString& sTableId );

    void SetActiveSitePlayersUI( hoxPlayersUI* newPlayersUI );
    hoxPlayersUI* CreateNewSitePlayersUI();
    void DeleteSitePlayersUI( hoxPlayersUI* toDeletePlayersUI );

    /**
     * Delete the GUI Frame of a given Table.
     *
     * @param sTableId The table-Id whose frame needs to be deleted.
     */
    void DeleteFrameOfTable( const wxString& sTableId );

private:
    void _CreateSitesUI();
    void _ShowWelcomeDialog();

    /**
     * Close all children (child-frame) of a specified Site.
     */
    void     _CloseChildrenOfSite(hoxSite* site);

    hoxSite* _GetSelectedSite() const;
    hoxSite* _GetSelectedSiteAndTable( hoxTable_SPtr& selectedTable ) const;

	bool _GetDefaultSitesLayout( int& sizeX );
	bool _SaveDefaultSitesLayout( const int sizeX );

	bool _GetDefaultTableLayout( wxSize& size );
	bool _SaveDefaultTableLayout( const wxSize& size );

    void _SaveCurrentTableToDisk( const hoxTable_SPtr& pTable,
                                  const wxString&      fileName ) const;

public: /* Static API */
    static wxMenuBar* Create_Menu_Bar( bool hasTable = false );
    static void Add_Menu_Item( wxMenu*         parentMenu,
                               const int       id,
                               const wxString& name,
                               const wxString& help,
                               const wxBitmap& bitmap );

private:
    // Sites.
    wxSashLayoutWindow* m_sitesWindow;
    wxSplitterWindow*   m_sitesSplitter;
    hoxSitesUI*         m_sitesUI;

    // Logging.
    hoxLog*             m_log;

    typedef std::list<MyChild*> MyChildList;
    MyChildList         m_children;

    bool                m_bShowWelcomeChecked;

    DECLARE_DYNAMIC_CLASS(MyFrame)
    DECLARE_EVENT_TABLE()
};


#endif  /* __INCLUDED_MY_FRAME_H__ */
