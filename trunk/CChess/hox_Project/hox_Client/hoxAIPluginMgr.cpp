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
// Name:            hoxAIPluginMgr.cpp
// Created:         02/16/2009
//
// Description:     The Manager that manages a group of AI Engine Plugins.
/////////////////////////////////////////////////////////////////////////////

#include "hoxAIPluginMgr.h"
#include <wx/dir.h>

// --------------------------------------------------------------------------
// hoxAIPlugin
// --------------------------------------------------------------------------

hoxAIPlugin::hoxAIPlugin()
        : m_aiPluginLibrary( NULL )
        , m_pCreateAIEngineLibFunc( NULL )
{
}

hoxAIPlugin::~hoxAIPlugin()
{
    if ( m_aiPluginLibrary )
    {
        m_aiPluginLibrary->Unload();
    }
}

AIEngineLib_APtr
hoxAIPlugin::CreateAIEngineLib()
{
    AIEngineLib_APtr apEngine;

    if ( ! m_pCreateAIEngineLibFunc )
    {
        wxLogWarning("%s: There is no 'Create AI Engine' function.", __FUNCTION__);
        return apEngine;
    }

    apEngine.reset( m_pCreateAIEngineLibFunc() );
    apEngine->initEngine();

    return apEngine;
}

// --------------------------------------------------------------------------
// hoxAIPluginMgr
// --------------------------------------------------------------------------

/* Define (initialize) the single instance */
hoxAIPluginMgr* 
hoxAIPluginMgr::m_instance = NULL;

wxString
hoxAIPluginMgr::m_defaultPluginName = "";

/* static */
hoxAIPluginMgr* 
hoxAIPluginMgr::GetInstance()
{
	if ( m_instance == NULL )
		m_instance = new hoxAIPluginMgr();

	return m_instance;
}

/* static */
void
hoxAIPluginMgr::DeleteInstance()
{
	delete m_instance;
}

/* static */
void
hoxAIPluginMgr::SetDefaultPluginName( const wxString& sDefaultName )
{
    m_defaultPluginName = sDefaultName;
}

const wxString
hoxAIPluginMgr::GetDefaultPluginName() const
{
    return m_defaultPluginName;
}

/* private */
hoxAIPluginMgr::hoxAIPluginMgr()
{
    _loadAvailableAIPlugins();

    // Set the default Plugin to be the 1st if none has been specified.
    if ( m_defaultPluginName.empty() && !m_aiPlugins.empty() )
    {
        m_defaultPluginName = m_aiPlugins.begin()->second->m_name;
        wxLogDebug("%s: *INFO* Select the 1st available Plugin [%s].",
            __FUNCTION__, m_defaultPluginName.c_str());
    }
}

AIEngineLib_APtr
hoxAIPluginMgr::CreateDefaultAIEngineLib()
{
    AIEngineLib_APtr apEngine;

    const wxString sName = m_defaultPluginName;
    if ( sName.empty() )
    {
        ::wxMessageBox( _("There is no AI Engine available."),
                        _("AI Creation"),
                        wxOK | wxICON_EXCLAMATION );
        return apEngine;
    }

    hoxAIPlugin_SPtr pPlugin = _loadPlugin( sName );

    if ( !pPlugin || !pPlugin->IsLoaded() )
    {
        wxLogWarning("%s: The AI Engine [%s] could not be loaded.", __FUNCTION__, sName.c_str());
        return apEngine;
    }

    return pPlugin->CreateAIEngineLib();
}

wxArrayString
hoxAIPluginMgr::GetNamesOfAllAIPlugins() const
{
    wxArrayString aiNames;

    for ( hoxAIPluginMap::const_iterator it = m_aiPlugins.begin();
                                         it != m_aiPlugins.end(); ++it )
    {
        aiNames.Add( it->second->m_name );
    }

    return aiNames;
}

bool
hoxAIPluginMgr::_loadAvailableAIPlugins()
{
    wxString sPluginsDir = AI_PLUGINS_PATH; /* wxGetCwd() + "/../plugins/lib" */
    wxLogDebug("%s: Get Plugins from [%s].", __FUNCTION__, sPluginsDir.c_str());
    wxDir dir(sPluginsDir);
	if ( !dir.IsOpened() )
	{
        wxLogWarning("%s: Fail to open Plugins folder [%s].", __FUNCTION__, sPluginsDir.c_str());
		return false; // failure.
	}

    wxString         filename;
	const wxString   ext = "*"  + wxDynamicLibrary::GetDllExt();
    hoxAIPlugin_SPtr pAIPlugin;

	for ( bool cont = dir.GetFirst(&filename, ext, wxDIR_FILES);
	           cont == true;
               cont = dir.GetNext(&filename) )
    {
        pAIPlugin.reset( new hoxAIPlugin() );
        pAIPlugin->m_name = filename.BeforeFirst('.');
        pAIPlugin->m_path = sPluginsDir + "/" + filename;

        wxLogDebug("%s: AI-name = [%s], AI-path = [%s].", __FUNCTION__,
            pAIPlugin->m_name.c_str(), pAIPlugin->m_path.c_str());

        m_aiPlugins[pAIPlugin->m_name] = pAIPlugin;
	}

    return true;  // success
}

hoxAIPlugin_SPtr
hoxAIPluginMgr::_loadPlugin( const wxString& sName )
{
    hoxAIPlugin_SPtr pPlugin;

    pPlugin = m_aiPlugins[sName];
    if ( ! pPlugin )
    {
        wxLogDebug("%s: *INFO* The AI Plugin [%s] is found.", __FUNCTION__, sName.c_str());
        return pPlugin;
    }

    if ( pPlugin->IsLoaded() ) // already loaded?
    {
        return pPlugin;
    }

    wxPluginLibrary* lib = wxPluginManager::LoadLibrary ( pPlugin->m_path );
	if ( ! lib ) 
    {
        wxLogDebug("%s: Fail to load plugin [%s].", __FUNCTION__, sName.c_str());
        pPlugin.reset();
        return pPlugin;
    }

    wxLogDebug("%s: Loaded [%s].", __FUNCTION__, sName.c_str());

    const char* szFuncName = "CreateAIEngineLib";
    PICreateAIEngineLibFunc pfnCreate = (PICreateAIEngineLibFunc) lib->GetSymbol(szFuncName);
    if ( !pfnCreate )
    {
        wxLogWarning("%s: Function [%s] not found in [%s].",
            __FUNCTION__, szFuncName, sName.c_str());
        lib->Unload();
        pPlugin.reset();
        return pPlugin;
    }

    pPlugin->m_aiPluginLibrary = lib;
    pPlugin->m_pCreateAIEngineLibFunc = pfnCreate;
    wxLogDebug("%s: Return engine [%s].", __FUNCTION__, sName.c_str());

    return pPlugin;
}

/************************* END OF FILE ***************************************/
