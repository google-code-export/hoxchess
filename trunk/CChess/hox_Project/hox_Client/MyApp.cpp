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
// Name:            MyApp.cpp
// Created:         10/02/2007
//
// Description:     The main Application.
/////////////////////////////////////////////////////////////////////////////

#include "MyApp.h"
#include "hoxAIPluginMgr.h"
#include <wx/socket.h>


// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

wxDEFINE_EVENT(hoxEVT_APP_SITE_CLOSE_READY, wxCommandEvent);

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

    // Add PNG image-type handler since our pieces use this format.
    wxImage::AddHandler( new wxPNGHandler );

	m_config = new wxConfig( HOX_APP_NAME );
    _LoadAppOptions();

    // Get default size and position.
	wxPoint defaultPosition;
	wxSize  defaultSize;

	if ( ! this->GetDefaultFrameLayout( defaultPosition, defaultSize ) ) // not exists?
	{
		int x, y, width, height;
		::wxClientDisplayRect( &x, &y, &width, &height );
		defaultPosition = wxPoint( x, y );
		defaultSize     = wxSize( width/2 + 200, height - 150 );
	}

    // Create the main application window
    m_frame = new MyFrame( NULL, 
                           wxID_ANY, 
                           HOX_APP_NAME,
                           defaultPosition,
                           defaultSize,
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

    // Create the LOCAL site.
    hoxSiteManager::GetInstance()->CreateLocalSite();

    const wxString sDefaultAI = wxGetApp().GetOption("defaultAI");
    hoxAIPluginMgr::SetDefaultPluginName( sDefaultAI );

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

int 
MyApp::OnExit()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    /* NOTE: We rely on the Frame to notify about active players that the
     *      system is being shutdowned.
     */

    delete hoxAIPluginMgr::GetInstance();
	delete hoxSiteManager::GetInstance();
    wxSocketBase::Shutdown(); // Shut down the sockets.
    _SaveAppOptions();
	delete m_config; // The changes will be written back automatically

    return 0;
}

void 
MyApp::ConnectToServer( const hoxSiteType       siteType,
		                const hoxServerAddress& address,
					    const wxString&         userName,
						const wxString&         password )
{
    hoxSite* site = NULL;

    /* Search for existing site. */
	site = hoxSiteManager::GetInstance()->FindSite( address );

    /* Create a new site if necessary. */
    if ( site == NULL )
    {
		site = hoxSiteManager::GetInstance()->CreateSite( siteType, 
			     										  address,
													      userName,
													      password );
    }

    wxCHECK_RET(site != NULL, "Failed to create a Site");

    /* Connect to the site. */
    if ( site->Connect() != hoxRC_OK )
    {
        wxLogError("%s: Failed to connect to server [%s].", __FUNCTION__, address.c_str());
    }
}

void 
MyApp::OnSystemClose()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

	m_appClosing = true;

    // Delete the LOCAL site.
    hoxSiteManager::GetInstance()->DeleteLocalSite();

	hoxSiteManager::GetInstance()->Close();
}

void 
MyApp::OnCloseReady_FromSite( wxCommandEvent&  event )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    hoxSite* site = wx_reinterpret_cast(hoxSite*, event.GetEventObject());
    wxCHECK_RET(site, "Site cannot be NULL.");

    hoxSiteManager::GetInstance()->DeleteSite( site );

    /* Initiate the App's shutdown if there is no more sites. */
	if (   m_appClosing 
		&& hoxSiteManager::GetInstance()->GetNumberOfSites() == 0 )
	{
        wxLogDebug("%s: Trigger a Frame's Close event.", __FUNCTION__);
        m_frame->Close();  // NOTE: Is there a better way?
	}
}

bool 
MyApp::GetDefaultFrameLayout( wxPoint& position, 
	                          wxSize&  size )
{
	position = wxPoint( -1, -1 );
	size = wxSize( 0, 0 );

	// Read the existing layout from Configuration.
	wxConfig* config = this->GetConfig();

	if ( ! config->Read("/Layout/Frame/position/x", &position.x) )
		return false;  // not found.

	if ( ! config->Read("/Layout/Frame/position/y", &position.y) )
		return false;  // not found.

	if ( ! config->Read("/Layout/Frame/size/x", &size.x) )
		return false;  // not found.

	if ( ! config->Read("/Layout/Frame/size/y", &size.y) )
		return false;  // not found.

	return true;   // found old layout?
}

bool 
MyApp::SaveDefaultFrameLayout( const wxPoint& position, 
	                           const wxSize&  size )
{
	// Write the current layout to Configuration.
	wxConfig* config = this->GetConfig();

	config->Write("/Layout/Frame/position/x", position.x);
	config->Write("/Layout/Frame/position/y", position.y);

	config->Write("/Layout/Frame/size/x", size.x);
	config->Write("/Layout/Frame/size/y", size.y);

	return true;
}

void
MyApp::_LoadAppOptions()
{
	m_options["sound"] = m_config->Read("/Options/sound", "1");
    m_options["defaultAI"] = m_config->Read("/Options/defaultAI", "AI_XQWLight");

    m_options["/Board/Piece/path"] =
        m_config->Read("/Board/Piece/path", DEFAULT_PIECE_PATH);

    m_options["/Board/Color/background"] =
        m_config->Read("/Board/Color/background", DEFAULT_BOARD_BACKGROUND_COLOR);

    m_options["/Board/Color/foreground"] =
        m_config->Read("/Board/Color/foreground", DEFAULT_BOARD_FOREGROUND_COLOR);
}

void
MyApp::_SaveAppOptions()
{
	m_config->Write("/Options/sound", m_options["sound"]);
    m_config->Write("/Options/defaultAI", m_options["defaultAI"]);
    m_config->Write("/Board/Piece/path", m_options["/Board/Piece/path"]);
    m_config->Write("/Board/Color/background", m_options["/Board/Color/background"]);
    m_config->Write("/Board/Color/foreground", m_options["/Board/Color/foreground"]);
}

/************************* END OF FILE ***************************************/
