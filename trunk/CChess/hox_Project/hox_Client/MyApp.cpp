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
#include "hoxSocketServer.h"
#include "hoxPlayer.h"
#include "hoxTable.h"
#include "hoxPlayerMgr.h"
#include "hoxTableMgr.h"
#include "hoxUtility.h"
#include "hoxServer.h"

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

DEFINE_EVENT_TYPE(hoxEVT_APP_PLAYER_SHUTDOWN_DONE)

BEGIN_EVENT_TABLE(MyApp, wxApp)
    EVT_COMMAND(wxID_ANY, hoxEVT_APP_PLAYER_SHUTDOWN_DONE, MyApp::OnShutdownDone_FromPlayer)
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
    m_myPlayer  = NULL;

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

    // Create a server to service remote clients.
    m_server = NULL;
    m_socketServer = NULL;
    
    // Create a "host" player representing this machine.
    m_pPlayer = hoxPlayerMgr::GetInstance()->CreateHostPlayer( "This_HOST" );

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


    delete hoxPlayerMgr::GetInstance();
    delete hoxTableMgr::GetInstance();

    this->CloseServer();
    delete m_socketServer;

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

hoxMyPlayer* 
MyApp::GetMyPlayer() 
{ 
    const char* FNAME = "MyApp::GetMyPlayer";

    if ( m_myPlayer == NULL )
    {
        wxLogDebug("%s: Creating the MY player...", FNAME);
        wxString playerName = hoxUtility::GenerateRandomString();
        m_myPlayer = hoxPlayerMgr::GetInstance()->CreateMyPlayer( playerName );
    }

    return m_myPlayer; 
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

    wxASSERT_MSG( m_server == NULL, "The server should not have been created.");

    /* Start the socket-manager */

    m_server = new hoxServer();

    if ( m_server->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError("%s: Failed to create Server thread.", FNAME);
        return;
    }
    wxASSERT_MSG( !m_server->GetThread()->IsDetached(), "The Server thread must be joinable.");

    m_server->GetThread()->Run();

    /* Start the socket-server */

    wxASSERT_MSG( m_socketServer == NULL, "The socket-server should not have been created.");

    m_socketServer = new hoxSocketServer( nPort,
                                          m_server );

    if ( m_socketServer->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError("%s: Failed to create socker-server thread.", FNAME);
        return;
    }
    wxASSERT_MSG( !m_socketServer->GetThread()->IsDetached(), "The socket-server thread must be joinable.");

    m_socketServer->GetThread()->Run();
}

void MyApp::CloseServer()
{
    const char* FNAME = "MyApp::CloseServer";

    if ( m_socketServer != NULL )
    {
        wxLogDebug("%s: Request the socket-server thread to be shutdowned...", FNAME);
        m_socketServer->RequestShutdown();
        wxThread::ExitCode exitCode = m_socketServer->GetThread()->Wait();
        wxLogDebug("%s: The socket-server thread was shutdowned with exit-code = [%d].", FNAME, exitCode);
        delete m_socketServer;
        m_socketServer = NULL;
    }

    if ( m_server != NULL )
    {
        wxLogDebug("%s: Request the Server thread to be shutdowned...", FNAME);
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_SHUTDOWN, NULL );
        m_server->AddRequest( request );
        wxThread::ExitCode exitCode = m_server->GetThread()->Wait();
        wxLogDebug("%s: The Server thread was shutdowned with exit-code = [%d].", FNAME, exitCode);
        delete m_server;
        m_server = NULL;
    }
}


/************************* END OF FILE ***************************************/
