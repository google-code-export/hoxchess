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
// Name:            MyApp.cpp
// Created:         10/02/2007
//
// Description:     The main Application.
/////////////////////////////////////////////////////////////////////////////

#include "MyApp.h"
#include "MyFrame.h"
#include "hoxEnums.h"
#include "hoxLog.h"
#include "hoxPlayer.h"
#include "hoxTable.h"
#include "hoxPlayerMgr.h"
#include "hoxTableMgr.h"
#include "hoxUtility.h"

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

DEFINE_EVENT_TYPE(hoxEVT_APP_SITE_SHUTDOWN_READY)

BEGIN_EVENT_TABLE(MyApp, wxApp)
    //EVT_COMMAND(wxID_ANY, hoxEVT_APP_PLAYER_SHUTDOWN_DONE, MyApp::OnShutdownDone_FromPlayer)
    EVT_COMMAND(wxID_ANY, hoxEVT_APP_SITE_SHUTDOWN_READY, MyApp::OnShutdownReady_FromSite)
END_EVENT_TABLE()

/**
 * 'Main program' equivalent: the program execution "starts" here
 */
bool 
MyApp::OnInit()
{
    // Call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future.
    if ( !wxApp::OnInit() )
        return false;

    m_frame = NULL;    // To avoid "logging to early to Frame".
    m_httpPlayer  = NULL;

    //m_log = new hoxLog();
    //m_oldLog = wxLog::SetActiveTarget( m_log );

    // Add PNG image-type handler since our pieces use this format.
    wxImage::AddHandler( new wxPNGHandler );

    // Get display size and position.
    int x, y, width, height;
    ::wxClientDisplayRect( &x, &y, &width, &height );
    wxSize displaySize( width/2, height - 150 );
    wxPoint displayPosition( x, y );

    // Create the main application window
    m_frame = new MyFrame( NULL, 
                           wxID_ANY, 
                           _T("HOXChess"),
                           displayPosition,
                           displaySize,
                           wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL );

    SetTopWindow( m_frame );

    // Show the frame (the frames, unlike simple controls, are not shown when
    // created initially)
    m_frame->Show(true);

    m_localSite = NULL;

    // Initialize socket so that secondary threads can use network-related API.
    if ( ! wxSocketBase::Initialize() )
    {
        wxLogError("Failed to initialize socket.");
        return false;
    }

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

int 
MyApp::OnExit()
{
    const char* FNAME = "MyApp::OnExit";

    wxLogDebug("%s: ENTER.", FNAME);

    for ( hoxSiteList::iterator it = m_sites.begin();
                                it != m_sites.end(); ++it )
    {
        (*it)->Close();
        delete (*it);
    }

    /* NOTE: We rely on the Frame to notify about active players that the
     *      system is being shutdowned.
     */


    delete hoxPlayerMgr::GetInstance();
    delete hoxTableMgr::GetInstance();

    //delete wxLog::SetActiveTarget( m_oldLog );

    return 0;
}

void 
MyApp::OnShutdownDone_FromPlayer( wxCommandEvent&  event )
{
    const char* FNAME = "MyApp::OnShutdownDone_FromPlayer";

    hoxPlayer* player = wx_reinterpret_cast(hoxPlayer*, event.GetEventObject());
    wxCHECK_RET(player, "Player cannot be NULL.");

    wxLogDebug("%s: Removing this player [%s] from the system...", 
        FNAME, player->GetName().c_str());

    hoxPlayerMgr::GetInstance()->DeletePlayer( player );

    /* Initiate the App's shutdown if there is no more active players. */
    if ( hoxPlayerMgr::GetInstance()->GetNumberOfPlayers() == 0 )
    {
        m_frame->Close();  // NOTE: Is there a better way?
    }
}

hoxHttpPlayer* 
MyApp::GetHTTPPlayer() const
{ 
    const char* FNAME = "MyApp::GetHTTPPlayer";

    if ( m_httpPlayer == NULL )
    {
        wxLogDebug("%s: Creating the HTTP player...", FNAME);
        wxString playerName = hoxUtility::GenerateRandomString();
        m_httpPlayer = hoxPlayerMgr::GetInstance()->CreateHTTPPlayer( playerName );
    }

    return m_httpPlayer; 
}

void 
MyApp::OpenServer(int nPort) 
{ 
    const char* FNAME = "MyApp::OpenServer";
    wxLogDebug("%s: ENTER.", FNAME);

    if ( m_localSite == NULL )
    {
        hoxServerAddress address( "127.0.0.1", nPort ); 
        m_localSite = new hoxLocalSite( address );
        m_sites.push_back( m_localSite );
    }

    m_localSite->OpenServer();

    m_frame->UpdateSiteTreeUI();
}

void MyApp::CloseServer()
{
    const char* FNAME = "MyApp::CloseServer";

    if ( m_localSite != NULL )
    {
        m_localSite->Close();
        m_sites.remove( m_localSite );
        delete m_localSite;
        m_localSite = NULL;
        m_frame->UpdateSiteTreeUI();
    }
}

void 
MyApp::ConnectRemoteServer( const hoxServerAddress& address )
{
    const char* FNAME = "MyApp::ConnectRemoteServer";
    hoxRemoteSite* remoteSite = NULL;

    /* Search for existing site. */
    for ( hoxSiteList::iterator it = m_sites.begin();
                                it != m_sites.end(); ++it )
    {
        if (   (*it)->GetType() != hoxSITE_TYPE_LOCAL
            && (*it)->GetAddress() == address )
        {
            remoteSite = (hoxRemoteSite*) (*it);
            break;
        }
    }

    /* Create a new Remote site if necessary. */
    if ( remoteSite == NULL )
    {
        // FIXME: Cheating here to create HTTP server based on port 80.
        if ( address.port != 80 )
        {
            remoteSite = new hoxRemoteSite( address );
        }
        else
        {
            remoteSite = new hoxHTTPSite( address );
        }
        m_sites.push_back( remoteSite );
    }

    /* Connect to the Remote site. */
    if ( remoteSite->Connect() != hoxRESULT_OK )
    {
        wxLogError("%s: Failed to connect to the remote server [%s:%d].", 
            FNAME, address.name.c_str(), address.port);
    }

    m_frame->UpdateSiteTreeUI();
}

void 
MyApp::DisconnectRemoteServer(hoxRemoteSite* remoteSite)
{
    remoteSite->Close();
    m_sites.remove( remoteSite );
    delete remoteSite;

    m_frame->UpdateSiteTreeUI();
}

void 
MyApp::CloseLocalSite()
{
    if ( m_localSite != NULL )
    {
        m_localSite->Close();
    }
}

void 
MyApp::OnSystemShutdown()
{
    const char* FNAME = "MyApp::OnSystemShutdown";

    wxLogDebug("%s: ENTER.", FNAME);
    for ( hoxSiteList::iterator it = m_sites.begin();
                                it != m_sites.end(); ++it )
    {
        (*it)->OnSystemShutdown();
    }
}

void 
MyApp::OnShutdownReady_FromSite( wxCommandEvent&  event )
{
    const char* FNAME = "MyApp::OnShutdownReady_FromSite";

    wxLogDebug("%s: ENTER.", FNAME);

    hoxSite* site = wx_reinterpret_cast(hoxSite*, event.GetEventObject());
    wxCHECK_RET(site, "Site cannot be NULL.");

    if ( site == m_localSite )
    {
        m_localSite = NULL;
    }

    m_sites.remove( site );
    site->Close();
    delete site;

    /* Initiate the App's shutdown if there is no more sites. */
    if ( m_sites.empty() )
    {
        wxLogDebug("%s: Trigger a Frame's Close event.", FNAME);
        m_frame->Close();  // NOTE: Is there a better way?
    }

#if 0
    /* Close remote sites first */

    while ( ! m_sites.empty() )
    {
        hoxSite* site = m_sites.front();
        m_sites.pop_front();

        /* Save the local site for last. */
        //if ( site->IsLocal() )
        //{
        //    wxASSERT_MSG(localSite == NULL, "Only support 1 local site now");
        //    localSite = site;
        //}
        //else
        {
            site->Close();
            delete site;
        }
    }

    /* Close the local site */
    //if ( localSite != NULL )
    //{
    //    wxASSERT_MSG(localSite == m_localSite, "Only support 1 local site now");
    //    m_localSite = NULL;

    //    localSite->Close();
    //    delete localSite;
    //}
#endif
}

/************************* END OF FILE ***************************************/
