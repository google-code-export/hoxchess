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
 * HOXChess functions as a client to multiple Xiangqi remote servers.
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
 */

#ifndef __INCLUDED_MY_APP_H__
#define __INCLUDED_MY_APP_H__

#include <wx/wx.h>
#include <wx/config.h>
#include "hoxTypes.h"
#include "hoxSite.h"
#include "MyFrame.h"

/* Forward declarations */
class hoxPlayer;
class hoxTable;

wxDECLARE_EVENT(hoxEVT_APP_SITE_CLOSE_READY, wxCommandEvent);

/**
 * The main Application.
 * Everything starts from here (i.e., the entry point).
 */
class MyApp : public wxApp
{
public:

    MyApp();

    /*********************************
     * Override base class virtuals
     *********************************/

    bool OnInit();
    int  OnExit();

    /*********************************
     * My own API
     *********************************/

    void ConnectToServer( const hoxSiteType       siteType,
		                  const hoxServerAddress& address,
						  const wxString&         userName,
						  const wxString&         password );

    void OnSystemClose();
	void OnCloseReady_FromSite( wxCommandEvent&  event ); 

    MyFrame*       GetFrame() const { return m_frame; }
    wxConfig*      GetConfig() const { return m_config; }

    wxString GetOption( const wxString& name ) const
        { return m_options[name]; }
    void SetOption( const wxString& name, const wxString& value )
        { m_options[name] = value; }

	bool GetDefaultFrameLayout( wxPoint& position, wxSize& size );
	bool SaveDefaultFrameLayout( const wxPoint& position, const wxSize& size );

    wxLanguage SelectAndSaveLanguage( const bool bRestartIfChange = false );

private:
    void       _SetupLanguageAndLocale();
    wxLanguage _LoadCurrentLanguage();
    void       _SaveCurrentLanguage( const wxLanguage language );

    void _LoadAppOptions();
    void _SaveAppOptions();

private:
	wxConfig*           m_config;
    MyFrame*            m_frame;  // The main frame.

	bool                m_appClosing;    // The App is being closed?

    typedef std::map<const wxString, wxString> hoxOptions;
    mutable hoxOptions  m_options;

    wxLocale            m_locale;   // The locale we will be using.

    friend class MyFrame;

    DECLARE_EVENT_TABLE()
};

// This macro can be used multiple times and just allows you 
// to use wxGetApp() function
DECLARE_APP(MyApp)

#endif  /* __INCLUDED_MY_APP_H__ */
