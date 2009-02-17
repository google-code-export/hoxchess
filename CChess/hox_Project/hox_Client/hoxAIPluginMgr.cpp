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
hoxAIPlugin::createAIEngineLib()
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
hoxAIPluginMgr::SetDefaultPluginName( const wxString& sDefaultName )
{
    m_defaultPluginName = sDefaultName;
}

/* private */
hoxAIPluginMgr::hoxAIPluginMgr()
{
    _loadDefaultPluginFromDisk();
}

hoxAIPluginMgr::~hoxAIPluginMgr()
{
}

AIEngineLib_APtr
hoxAIPluginMgr::createDefaultAIEngineLib()
{
    AIEngineLib_APtr apEngine;

    /* Load the default AI Plugin. */
    if ( ! m_defaultPlugin.IsOk() )
    {
        wxLogWarning("%s: There is no default Plugin.", __FUNCTION__);
        return apEngine;
    }

    return m_defaultPlugin.createAIEngineLib();
}

bool
hoxAIPluginMgr::_loadDefaultPluginFromDisk()
{
    wxPluginLibrary* lib = NULL;

	wxString filename;
	wxString ext = "*"  + wxDynamicLibrary::GetDllExt();
    //wxString sPluginsDir = wxGetCwd() + "/../plugins/lib";
    wxString sPluginsDir = AI_PLUGINS_PATH;
    wxLogDebug("%s: Load Plugins from %s", __FUNCTION__, sPluginsDir.c_str());
	wxDir dir(sPluginsDir);

	if ( !dir.IsOpened() )
	{
        wxLogWarning("%s: Fail to open Plugins folder [%s].", __FUNCTION__, sPluginsDir.c_str());
		return false;
	}

	for ( bool cont = dir.GetFirst(&filename, ext, wxDIR_FILES);
	           cont == true;
               cont = dir.GetNext(&filename) )
    {
        /* NOTE: If the default AI is NOT set, then just load
         *       the FIRST plugin.
         */

        if (   ! m_defaultPluginName.empty()
            && ! filename.StartsWith( m_defaultPluginName ) )
        {
            wxLogDebug("%s: Plugin [%s] not matched [%s]. Skip!",
                __FUNCTION__, filename.c_str(), m_defaultPluginName.c_str());
            continue;
        }

		lib = wxPluginManager::LoadLibrary ( sPluginsDir + "/" + filename );
		if ( ! lib ) 
        {
            wxLogDebug("%s: Fail to load plugin [%s].", __FUNCTION__, filename.c_str());
            continue;
        }

        wxLogDebug("%s: Loaded [%s].", __FUNCTION__, filename.c_str());

        const char* szFuncName = "CreateAIEngineLib";
        PICreateAIEngineLibFunc pfnCreateAIEngineLib =
            (PICreateAIEngineLibFunc) lib->GetSymbol(szFuncName);
        if ( !pfnCreateAIEngineLib )
        {
            wxLogWarning("%s: Function [%s] not found from [%s].",
                __FUNCTION__, szFuncName, filename.c_str());
            lib->Unload();
        }
        else
        {
            m_defaultPlugin.m_aiPluginLibrary = lib;
            m_defaultPlugin.m_pCreateAIEngineLibFunc = pfnCreateAIEngineLib;
            wxLogDebug("%s: Return engine [%s].", __FUNCTION__, filename.c_str());
            return true;  // TODO: Found one engine. Enough!
        }
	}

    return false;  // failure.
}

/************************* END OF FILE ***************************************/
