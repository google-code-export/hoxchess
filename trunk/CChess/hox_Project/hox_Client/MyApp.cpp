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
#include "hoxUtil.h"

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

static hoxLanguageInfo s_languages[] =
{
    { wxLANGUAGE_DEFAULT,              _("(Use Default Language)") },
    // Adding new languages below ....

    { wxLANGUAGE_ENGLISH,              _("English")                },
    { wxLANGUAGE_CHINESE_SIMPLIFIED,   _("Chinese (Simplified)")   },
    { wxLANGUAGE_FRENCH,               _("French")                 },
    { wxLANGUAGE_VIETNAMESE,           _("Vietnamese")             },
};

///////////////////////////////////////////////////////////////////////////////

MyApp::MyApp()
        : wxApp()
        , m_frame( NULL )
        , m_language( wxLANGUAGE_UNKNOWN )
{
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

	m_config = new wxConfig( HOX_APP_NAME );
    _LoadAppOptions();

    _SetupLanguageAndLocale();

    // Add PNG image-type handler since our pieces use this format.
    wxImage::AddHandler( new wxPNGHandler );

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

    hoxSiteManager::GetInstance()->CreateLocalSite();

    const wxString sDefaultAI = wxGetApp().GetOption("defaultAI");
    hoxAIPluginMgr::SetDefaultPluginName( sDefaultAI );

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

void
MyApp::_SetupLanguageAndLocale()
{
    m_language = _LoadCurrentLanguage();
    if ( m_language == wxLANGUAGE_UNKNOWN )
    {
        m_language = _SelectAndSaveLanguage();
    }

    // For English, no need to do anything since it is the "built-in" language.
    if ( m_language == wxLANGUAGE_ENGLISH )
    {
        return;
    }

    // Don't use wxLOCALE_LOAD_DEFAULT flag so that Init() doesn't return
    // false just because it failed to load wxstd catalog
    if ( ! m_locale.Init(m_language, wxLOCALE_CONV_ENCODING) )
    {
        wxLogWarning(_("This language is not supported by the system."));
        // NOTE: still allowed to continue!
    }

    // Normally this wouldn't be necessary as the catalog files would be found
    // in the default locations, but when the program is not installed the
    // catalogs are in the build directory where we wouldn't find them by default.
    wxLocale::AddCatalogLookupPathPrefix( hoxUtil::GetPath(hoxRT_LOCALE) );

    // Initialize the catalogs we'll be using
    if ( ! m_locale.AddCatalog( HOX_CATALOG_NAME ) )
    {
        wxLogWarning(_("Fail to find/load the [%s] catalog."), HOX_CATALOG_NAME);
    }
}

void
MyApp::GetLanguageList( hoxLanguageInfoVector& langs ) const
{
    langs.clear();
    for ( int i = 0; i < WXSIZEOF(s_languages); ++i )
    {
        hoxLanguageInfo langInfo;
        langInfo.code = s_languages[i].code;
        langInfo.desc = s_languages[i].desc;

        langs.push_back( langInfo );
    }
}

wxLanguage
MyApp::SelectLanguage()
{
    /* References:
     *  See the open source project 'Poedit' at http://www.poedit.net
     */

    wxArrayString langDescriptions;
    for ( int i = 0; i < WXSIZEOF(s_languages); ++i )
    {
        langDescriptions.Add( s_languages[i].desc );
    }

    const int nChoice = ::wxGetSingleChoiceIndex(
                            _("Select your language"),
                            _("Language selection"),
                            langDescriptions );
    return ( nChoice == -1 ? wxLANGUAGE_UNKNOWN : s_languages[nChoice].code );
}

void
MyApp::SaveCurrentLanguage( const wxLanguage language )
{
    const wxString sLanguage = this->GetLanguageName( language );
    wxLogDebug("%s: Save current language [%s].", __FUNCTION__, sLanguage.c_str());
    wxGetApp().SetOption("language", sLanguage);
}

const wxString
MyApp::GetLanguageName( const wxLanguage language )
{
    wxString sLanguage;
    if ( language == wxLANGUAGE_DEFAULT )
    {
        sLanguage = "default";
    }
    else
    {
        const wxLanguageInfo* langInfo = wxLocale::GetLanguageInfo( language );
        wxCHECK_MSG(langInfo != NULL, _("Unknown"), "Language is unknown");
        sLanguage = langInfo->CanonicalName;  
    }

    return sLanguage;
}

const wxString
MyApp::GetLanguageDesc( const wxLanguage language )
{
    for ( int i = 0; i < WXSIZEOF(s_languages); ++i )
    {
        if ( s_languages[i].code == language )
        {
            return s_languages[i].desc;
        }
    }

    return _("Unknown");
}

wxLanguage
MyApp::_SelectAndSaveLanguage()
{
    wxLanguage language = this->SelectLanguage();
    if ( language == wxLANGUAGE_UNKNOWN )
    {
        language = wxLANGUAGE_DEFAULT;
    }

    this->SaveCurrentLanguage( language );

    return language;
}

wxLanguage
MyApp::_LoadCurrentLanguage()
{
    const wxString sLanguage = wxGetApp().GetOption("language");
    wxLogDebug("%s: Load current language [%s].", __FUNCTION__, sLanguage.c_str());

    if      ( sLanguage.empty() )      return wxLANGUAGE_UNKNOWN;
    else if ( sLanguage == "default" ) return wxLANGUAGE_DEFAULT;

    const wxLanguageInfo* langInfo = wxLocale::FindLanguageInfo( sLanguage );
    if ( langInfo == NULL )
    {
        wxLogWarning(_("Unknown language setting [%s]."), sLanguage.c_str());
        return wxLANGUAGE_UNKNOWN;
    }

    return (wxLanguage) langInfo->Language;
}

int 
MyApp::OnExit()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

    /* NOTE: We rely on the Frame to notify about active players that the
     *      system is being shutdowned.
     */

    hoxAIPluginMgr::DeleteInstance();
	hoxSiteManager::DeleteInstance();
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
    site->Connect();
}

void 
MyApp::OnSystemClose()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);

	m_appClosing = true;

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
    m_options["language"] = m_config->Read("/Options/language", "");
    m_options["sound"] = m_config->Read("/Options/sound", "1");
    m_options["welcome"] = m_config->Read("/Options/welcome", "1");
    m_options["showTables"] = m_config->Read("/Options/showTables", "1");
    m_options["moveMode"] = m_config->Read("/Options/moveMode", "0");
    m_options["defaultAI"] = m_config->Read("/Options/defaultAI", "");
    m_options["optionsPage"] = m_config->Read("/Options/optionsPage", "0");

    m_options["/Board/Image/path"] =
        m_config->Read("/Board/Image/path", "");

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
    m_config->Write("/Options/language", m_options["language"]);
    m_config->Write("/Options/sound", m_options["sound"]);
    m_config->Write("/Options/welcome", m_options["welcome"]);
    m_config->Write("/Options/showTables", m_options["showTables"]);
    m_config->Write("/Options/moveMode", m_options["moveMode"]);
    m_config->Write("/Options/defaultAI", m_options["defaultAI"]);
    m_config->Write("/Options/optionsPage", m_options["optionsPage"]);
    m_config->Write("/Board/Image/path", m_options["/Board/Image/path"]);
    m_config->Write("/Board/Piece/path", m_options["/Board/Piece/path"]);
    m_config->Write("/Board/Color/background", m_options["/Board/Color/background"]);
    m_config->Write("/Board/Color/foreground", m_options["/Board/Color/foreground"]);
}

/************************* END OF FILE ***************************************/
