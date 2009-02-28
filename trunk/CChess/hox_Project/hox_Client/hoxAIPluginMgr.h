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
// Name:            hoxAIPluginMgr.h
// Created:         02/16/2009
//
// Description:     The Manager that manages a group of AI Engine Plugins.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_AI_PLUGIN_MGR_H__
#define __INCLUDED_HOX_AI_PLUGIN_MGR_H__

#include "hoxTypes.h"
#include <list>
#include <wx/dynload.h>
#include "../plugins/common/AIEngineLib.h"

/* Forward declaration. */
class hoxAIPluginMgr;

typedef std::auto_ptr<AIEngineLib> AIEngineLib_APtr;

/**
 * An AI Engine Plugin.
 */
class hoxAIPlugin
{
public:
	hoxAIPlugin();
    ~hoxAIPlugin();

    const wxString GetName() const { return m_name; }
    bool IsLoaded() const { return (m_pCreateAIEngineLibFunc != NULL); }

    AIEngineLib_APtr CreateAIEngineLib();

private:
    wxString                 m_name;   // The unique name.
    wxString                 m_path;   // The full-path on disk.

    wxPluginLibrary*         m_aiPluginLibrary;
    PICreateAIEngineLibFunc  m_pCreateAIEngineLibFunc;

    friend class hoxAIPluginMgr;
};
typedef boost::shared_ptr<hoxAIPlugin> hoxAIPlugin_SPtr;

/**
 * The Manager of AI Plugins.
 * This is implemented as a singleton since we only need one instance.
 */
class hoxAIPluginMgr
{
public:
	static hoxAIPluginMgr* GetInstance();
    static void SetDefaultPluginName( const wxString& sDefaultName );
    static const wxString GetDefaultPluginName();
    ~hoxAIPluginMgr();

    AIEngineLib_APtr CreateDefaultAIEngineLib();
    wxArrayString GetNamesOfAllAIPlugins() const;

private:
    hoxAIPluginMgr();
	static hoxAIPluginMgr* m_instance;
    static wxString        m_defaultPluginName;

    bool _loadAvailableAIPlugins();
    hoxAIPlugin_SPtr _loadPlugin( const wxString& sName );

private:
    typedef std::map<const wxString, hoxAIPlugin_SPtr> hoxAIPluginMap;
    hoxAIPluginMap        m_aiPlugins;
};

#endif /* __INCLUDED_HOX_AI_PLUGIN_MGR_H__ */
