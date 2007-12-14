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

DEFINE_EVENT_TYPE(hoxEVT_APP_SITE_CLOSE_READY)

BEGIN_EVENT_TABLE(MyApp, wxApp)
	EVT_COMMAND(wxID_ANY, hoxEVT_APP_SITE_CLOSE_READY, MyApp::OnCloseReady_FromSite)
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

    //m_log = new hoxLog();
    //m_oldLog = wxLog::SetActiveTarget( m_log );

    // Add PNG image-type handler since our pieces use this format.
    wxImage::AddHandler( new wxPNGHandler );

    // Get display size and position.
    int x, y, width, height;
    ::wxClientDisplayRect( &x, &y, &width, &height );
    wxSize displaySize( width/2 + 200, height - 150 );
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

	m_appClosing = false;

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

    /* NOTE: We rely on the Frame to notify about active players that the
     *      system is being shutdowned.
     */


	delete hoxSiteManager::GetInstance();

    //delete wxLog::SetActiveTarget( m_oldLog );

    return 0;
}

void 
MyApp::OpenServer(int nPort) 
{ 
    const char* FNAME = "MyApp::OpenServer";
    wxLogDebug("%s: ENTER.", FNAME);

	hoxLocalSite* localSite = hoxSiteManager::GetInstance()->GetLocalSite();

	if ( localSite == NULL )
    {
        hoxServerAddress address( "127.0.0.1", nPort ); 
		localSite = static_cast<hoxLocalSite*>( 
			hoxSiteManager::GetInstance()->CreateSite( hoxSITE_TYPE_LOCAL, 
			                                           address ) );
    }

    localSite->OpenServer();

    m_frame->UpdateSiteTreeUI();
}

void 
MyApp::CloseServer( hoxSite* site )
{
    const char* FNAME = "MyApp::CloseServer";

	wxLogDebug("%s: ENTER. (%s)", FNAME, site->GetName().c_str());

    site->Close();
}

void 
MyApp::ConnectRemoteServer( const hoxServerAddress& address )
{
    const char* FNAME = "MyApp::ConnectRemoteServer";
    hoxRemoteSite* remoteSite = NULL;

    /* Search for existing site. */
	remoteSite = hoxSiteManager::GetInstance()->FindRemoteSite( address );

    /* Create a new Remote site if necessary. */
    if ( remoteSite == NULL )
    {
        // FIXME: Cheating here to create HTTP server based on port 80.
        if (   address.name == HOX_HTTP_SERVER_HOSTNAME 
			&& address.port == HOX_HTTP_SERVER_PORT )
        {
			remoteSite = static_cast<hoxHTTPSite*>( 
				hoxSiteManager::GetInstance()->CreateSite( hoxSITE_TYPE_HTTP, 
														   address ) );
        }
        else if (   address.name == "68.163.190.66" /* Chesscape server? */
		  	     && address.port == 3534 )
        {
			remoteSite = static_cast<hoxChesscapeSite*>( 
				hoxSiteManager::GetInstance()->CreateSite( hoxSITE_TYPE_CHESSCAPE, 
														   address ) );
        }
        else
        {
			remoteSite = static_cast<hoxRemoteSite*>( 
				hoxSiteManager::GetInstance()->CreateSite( hoxSITE_TYPE_REMOTE, 
														   address ) );
        }
    }

	if ( remoteSite == NULL )
	{
        wxLogError("%s: Failed to create a site for the remote server [%s:%d].", 
            FNAME, address.name.c_str(), address.port);
		return;
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
MyApp::OnSystemClose()
{
    const char* FNAME = "MyApp::OnSystemClose";
    wxLogDebug("%s: ENTER.", FNAME);

	m_appClosing = true;

	hoxSiteManager::GetInstance()->Close();
}

void 
MyApp::OnCloseReady_FromSite( wxCommandEvent&  event )
{
    const char* FNAME = "MyApp::OnCloseReady_FromSite";

    wxLogDebug("%s: ENTER.", FNAME);

    hoxSite* site = wx_reinterpret_cast(hoxSite*, event.GetEventObject());
    wxCHECK_RET(site, "Site cannot be NULL.");

	hoxSiteManager::GetInstance()->DeleteSite( site );

	m_frame->UpdateSiteTreeUI();

    /* Initiate the App's shutdown if there is no more sites. */
	if (   m_appClosing 
		&& hoxSiteManager::GetInstance()->GetNumberOfSites() == 0 )
	{
        wxLogDebug("%s: Trigger a Frame's Close event.", FNAME);
        m_frame->Close();  // NOTE: Is there a better way?
	}
}

/************************* END OF FILE ***************************************/
