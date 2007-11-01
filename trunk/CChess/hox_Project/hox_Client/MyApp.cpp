/////////////////////////////////////////////////////////////////////////////
// Name:            MyApp.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/02/2007
//
// Description:     The main Application.
/////////////////////////////////////////////////////////////////////////////

#include "MyApp.h"
#include "MyFrame.h"
#include "hoxEnums.h"
#include "hoxSocketServer.h"
#include "hoxPlayer.h"
#include "hoxTable.h"
#include "hoxPlayerMgr.h"
#include "hoxTableMgr.h"
#include "hoxUtility.h"
#include "hoxNetworkAPI.h"
#include "hoxServer.h"
#include <wx/filename.h>

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

DEFINE_EVENT_TYPE(hoxEVT_SERVER_RESPONSE)

BEGIN_EVENT_TABLE(MyApp, wxApp)
  EVT_SOCKET(SERVER_SOCKET_ID,  MyApp::OnServerSocketEvent)
END_EVENT_TABLE()


hoxLog::hoxLog()
    : wxLog()
{
    m_filename = wxFileName::GetTempDir() + "/CChess_"
               + hoxUtility::GenerateRandomString() + ".log";
    wxLogDebug(wxString::Format("Opened the log file [%s].", m_filename));
}

hoxLog::~hoxLog()
{
}

void 
hoxLog::DoLogString(const wxChar *msg, time_t timestamp)
{
    if ( msg == NULL ) return;

    wxFFile logFile( m_filename, "a" );

    if ( logFile.IsOpened() )
    {
        logFile.Write( msg );
        logFile.Write( "\n" );
        logFile.Close();
    }
#if 0
    if ( wxGetApp().GetTopWindow() != NULL )
    {
        wxCommandEvent logEvent( hoxEVT_FRAME_LOG_MSG );
        logEvent.SetString( wxString(msg) );
        ::wxPostEvent( wxGetApp().GetTopWindow(), logEvent );
    }
#endif
}


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
    wxSize displaySize( width/2, height );
    wxPoint displayPosition( x, y );

    // Create the main application window
    m_frame = new MyFrame( NULL, 
                           wxID_ANY, 
                           _T("HOX Chess"),
                           displayPosition,
                           displaySize,
                           wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL );

    m_frame->SetupMenu();
    m_frame->SetupStatusBar();
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
    this->CloseServer();

    delete m_socketServer;

    delete hoxPlayerMgr::GetInstance();
    delete hoxTableMgr::GetInstance();

    //delete wxLog::SetActiveTarget( m_oldLog );

    return 0;
}

hoxMyPlayer* 
MyApp::GetMyPlayer() 
{ 
    if ( m_myPlayer == NULL )
    {
        wxString playerName = hoxUtility::GenerateRandomString();
        m_myPlayer = hoxPlayerMgr::GetInstance()->CreateMyPlayer( playerName );
    }

    return m_myPlayer; 
}

hoxHttpPlayer* 
MyApp::GetHTTPPlayer() const
{ 
    if ( m_httpPlayer == NULL )
    {
        wxString playerName = hoxUtility::GenerateRandomString();
        m_httpPlayer = hoxPlayerMgr::GetInstance()->CreateHTTPPlayer( playerName );
    }

    return m_httpPlayer; 
}

void 
MyApp::OpenServer() 
{ 
    const char* FNAME = "MyApp::OpenServer";
    wxLogDebug("%s: ENTER.", FNAME);

    wxASSERT_MSG( m_server == NULL, "The server should not have been created.");

    /* Start the socket-manager */

    m_server = new hoxServer( hoxNETWORK_DEFAULT_SERVER_PORT );

    if ( m_server->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError("%s: Failed to create Server thread.", FNAME);
        return;
    }
    wxASSERT_MSG( !m_server->GetThread()->IsDetached(), "The Server thread must be joinable.");

    m_server->GetThread()->Run();

    /* Start the socket-server */

    wxASSERT_MSG( m_socketServer == NULL, "The socket-server should not have been created.");

    m_socketServer = new hoxSocketServer( hoxNETWORK_DEFAULT_SERVER_PORT,
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

void 
MyApp::OnServerSocketEvent(wxSocketEvent& event)
{
    const char* FNAME = "MyApp::OnServerSocketEvent";  // function's name

    wxLogDebug("%s: ENTER.", FNAME);

    wxLogDebug("%s: Received new socket-event = [%s].", 
        FNAME, hoxNetworkAPI::SocketEventToString(event.GetSocketEvent()) );

    wxASSERT( m_server != NULL );
    {
        hoxRequest* request = new hoxRequest( hoxREQUEST_TYPE_DATA );
        request->socket      = event.GetSocket();
        request->socketEvent = event.GetSocketEvent();
        m_server->AddRequest( request );
    }
}


/************************* END OF FILE ***************************************/
