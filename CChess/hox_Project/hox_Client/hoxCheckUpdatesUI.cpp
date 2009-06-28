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
// Name:            hoxCheckUpdatesUI.cpp
// Created:         06/27/2009
//
// Description:     The UI to Check for Updates.
/////////////////////////////////////////////////////////////////////////////

#include "hoxCheckUpdatesUI.h"
#include "hoxPlayer.h"   // just for hoxEVT_CONNECTION_RESPONSE

class VersionInfo
{
public:
    std::string version;
    std::string os;
    std::string url;
};
typedef std::list<VersionInfo> VersionInfoList;

// ----------------------------------------------------------------------------
//
//     hoxCheckUpdatesUI
//
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(hoxCheckUpdatesUI, wxProgressDialog)
    EVT_TIMER(wxID_ANY, hoxCheckUpdatesUI::OnTimer)    
    EVT_COMMAND(wxID_ANY, hoxEVT_CONNECTION_RESPONSE, hoxCheckUpdatesUI::OnCheckUpdatesResponse)
END_EVENT_TABLE()

hoxCheckUpdatesUI::hoxCheckUpdatesUI( const wxString& title,
                                      const wxString& message,
                                      int             maximum /* = 100 */,
                                      wxWindow*       parent )
        : wxProgressDialog( title, message, maximum, parent,
                            wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT
                                | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME )
        , m_maximum( maximum )
        , m_timerValue( 0 )
        , m_timer( NULL )
        , m_pHttpSocket( NULL )
        , m_io_service_thread( NULL )
{
    m_timer = new wxTimer( this );
    m_timer->Start( hoxTIME_ONE_SECOND_INTERVAL );
}

hoxCheckUpdatesUI::~hoxCheckUpdatesUI()
{
    if ( m_timer != NULL )
    {
        if ( m_timer->IsRunning() ) m_timer->Stop();
        delete m_timer;
        m_timer = NULL;
    }

    delete m_pHttpSocket;
    delete m_io_service_thread;
}

void
hoxCheckUpdatesUI::Stop()
{
    m_timer->Stop();
    this->Update( m_maximum ); // Make sure to close the dialog.
}

void 
hoxCheckUpdatesUI::OnTimer( wxTimerEvent& event )
{
    const bool bContinued = this->Update( ++m_timerValue );
    if ( ! bContinued || m_timerValue >= m_maximum )
    {
        this->Stop();
    }
}

void
hoxCheckUpdatesUI::runCheck()
{
    const std::string sHost    = "code.google.com";
    const std::string sService = "http";
    wxString sError;

    try
    {
        tcp::resolver resolver( m_io_service );
        tcp::resolver::query query( sHost, sService );
        tcp::resolver::iterator iterator = resolver.resolve(query);

        m_pHttpSocket = new hoxHttpSocket( m_io_service, iterator,
                                           this /* evtHandler */ );

        // Send the HTTP GET request.
        std::string sRequest;
        sRequest = "GET /p/hoxchess/downloads/list HTTP/1.1\r\n";
        sRequest += "Host: " + sHost + "\r\n";
        sRequest += "Connection: close\r\n";
        sRequest += "\r\n";
        m_pHttpSocket->write( sRequest );

        m_io_service_thread = new asio::thread( boost::bind(&asio::io_service::run,
                                                            &m_io_service) );
    }
    catch (std::exception& e)
    {
        sError.Printf("Exception [%s] when connect to [%s]", e.what(), sHost.c_str());
        wxLogWarning("%s: %s.", __FUNCTION__, sError.c_str());
        return;
    }
}

void
hoxCheckUpdatesUI::OnCheckUpdatesResponse( wxCommandEvent& event )
{
    const hoxResponse_APtr apResponse( wxDynamicCast(event.GetEventObject(), hoxResponse) );
    const std::string sResponse = m_pHttpSocket->getResponse();

    m_pHttpSocket->close();

    m_io_service_thread->join();   // ************ WAIT HERE
    delete m_io_service_thread;
    m_io_service_thread = NULL;

    delete m_pHttpSocket;
    m_pHttpSocket = NULL;

    this->Stop();

    const size_t nSize = sResponse.size();
    wxLogDebug("%s: Downloaded %d bytes", __FUNCTION__, nSize);

    std::string::size_type startIndex = 0;
    std::string::size_type versionIndex, osIndex, endIndex;
    const std::string sTOKEN("http://hoxchess.googlecode.com/files/HOXChess-");
    VersionInfoList versions;
    VersionInfo     ver;
    // Examples:
    //   http://hoxchess.googlecode.com/files/HOXChess-0.7.6.0-OSX.zip
    //   http://hoxchess.googlecode.com/files/HOXChess-0.7.5.0-Linux.tar.gz
    //   http://hoxchess.googlecode.com/files/HOXChess-0.7.5.0-Setup.exe
    for (;;)
    {
        startIndex = sResponse.find(sTOKEN, startIndex);
        if ( startIndex == std::string::npos ) break;
        versionIndex = startIndex + sTOKEN.size();
        osIndex = sResponse.find_first_of('-', versionIndex);
        if ( osIndex == std::string::npos ) break;
        endIndex = sResponse.find_first_of('"', osIndex);
        if ( osIndex == std::string::npos ) break;

        ver.version = sResponse.substr(versionIndex, osIndex-versionIndex);
        ver.os = sResponse.substr(osIndex+1, 1);
        ver.url = sResponse.substr(startIndex, endIndex-startIndex);
        versions.push_back(ver);
        startIndex = endIndex;
    }

    wxString sMsg;
    sMsg = wxString::Format("This application is running under %s.\n"
                            "Your version: %s-%s\n\n",
                            wxPlatformInfo::Get().GetOperatingSystemIdName().c_str(),
                            HOX_APP_NAME, HOX_VERSION);
    sMsg += "--- Latest versions available on GoogleCode: ---\n";
    for ( VersionInfoList::const_iterator it = versions.begin();
                                          it != versions.end(); ++it )
    {
        wxString osName = "?";
        if      ( it->os == "O" ) osName = "OSX";
        else if ( it->os == "L" ) osName = "Linux";
        else if ( it->os == "S" ) osName = "Windows";
        sMsg += it->version + "-" + osName + " [" + it->url + "]" + "\n\n";
    }

    wxString sCaption = wxString::Format("Check for Updates - Downloaded %d bytes", nSize );
    ::wxMessageBox( sMsg, sCaption, wxOK|wxICON_INFORMATION );
}

/************************* END OF FILE ***************************************/
