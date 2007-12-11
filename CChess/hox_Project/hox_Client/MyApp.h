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
// Name:            MyApp.h
// Created:         10/02/2007
//
// Description:     The main Application.
/////////////////////////////////////////////////////////////////////////////

/**
 * @mainpage HOXChess Main Page
 *
 * @section intro_sec Introduction
 *
 * HOXChess can function as a client, a server, or both.
 *
 * @section key_classes_sec Key Classes
 *
 * Here are the list of the main classes:
 *  - MyApp         - The main App.
 *  - MyFrame       - The main (MDI) Frame acting as the App's main GUI.
 *  - hoxTable      - The Table with a board, a referee, and players.
 *  - hoxBoard      - The Board acting as the Table's GUI.
 *  - hoxReferee    - The Referee of a Table. 
 *  - hoxPlayer     - The interface to a Player.
 *  - hoxConnection - The Connection used by a Player for network traffic.
 *  - hoxSocketServer - The server-component listening for new connections.
 *  - hoxServer     - The server-component managed all remote connections.
 */

#ifndef __INCLUDED_MY_APP_H_
#define __INCLUDED_MY_APP_H_

#include <wx/wx.h>
#include "hoxTypes.h"
#include "hoxSite.h"
#include "MyFrame.h"

/* Forward declarations */
class hoxLog;
class hoxPlayer;
class hoxTable;
class hoxHttpPlayer;
class hoxMyPlayer;

DECLARE_EVENT_TYPE(hoxEVT_APP_SITE_CLOSE_READY, wxID_ANY)

/**
 * The main Application.
 * Everything starts from here (i.e., the entry point).
 */
class MyApp : public wxApp
{
public:

    /*********************************
     * Override base class virtuals
     *********************************/

    bool OnInit();
    int  OnExit();

    /*********************************
     * My own API
     *********************************/

    void OpenServer(int nPort);
    void CloseServer( hoxSite* site );

    void ConnectRemoteServer(const hoxServerAddress& address);

    void OnSystemClose();
	void OnCloseReady_FromSite( wxCommandEvent&  event ); 

    MyFrame*       GetFrame() const { return m_frame; }

private:
    hoxLog*             m_log;
    wxLog*              m_oldLog;    // the previous log target (to be restored later)

    MyFrame*            m_frame;  // The main frame.

	bool                m_appClosing;    // The App is being closed?

    friend class MyFrame;

    DECLARE_EVENT_TABLE()
};

// This macro can be used multiple times and just allows you 
// to use wxGetApp() function
DECLARE_APP(MyApp)

#endif  /* __INCLUDED_MY_APP_H_ */
