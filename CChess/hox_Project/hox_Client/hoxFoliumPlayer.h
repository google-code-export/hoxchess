/***************************************************************************
 *  Copyright 2007, 2008 Huy Phan  <huyphan@playxiangqi.com>               *
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
// Name:            hoxFoliumPlayer.h
// Created:         11/24/2008
//
// Description:     The AI Player based on the open-source Xiangqi Engine
//                  called FOLIUM written by Wangmao
//                  (username is 'lwm3751' under Google Code)
//                     http://folium.googlecode.com
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_FOLIUM_PLAYER_H__
#define __INCLUDED_HOX_FOLIUM_PLAYER_H__

#include "hoxAIPlayer.h"
#include "../folium/folHOXEngine.h"

/**
 * The folium AI player.
 */
class hoxFoliumPlayer :  public hoxAIPlayer
{
public:
    hoxFoliumPlayer() {} // DUMMY default constructor required for event handler.
    hoxFoliumPlayer( const wxString& name,
                    hoxPlayerType   type,
                    int             score );

    virtual ~hoxFoliumPlayer() {}

    // **** Override the parent's API ****
    virtual void Start();

private:

    DECLARE_DYNAMIC_CLASS(hoxFoliumPlayer)
};

// ----------------------------------------------------------------------------
// hoxFoliumEngine
// ----------------------------------------------------------------------------

class hoxFoliumEngine : public hoxAIEngine
{
public:
    hoxFoliumEngine( wxEvtHandler* player );
    virtual ~hoxFoliumEngine();

protected:
    virtual void     OnOpponentMove( const wxString& sMove );
    virtual wxString GenerateNextMove();

private:
    folHOXEngine*  m_engine;
};

// ----------------------------------------------------------------------------
// hoxFoliumConnection
// ----------------------------------------------------------------------------

/* Typedef(s) */
typedef boost::shared_ptr<hoxFoliumEngine> hoxFoliumEngine_SPtr;

/**
 * The connection to an TSITO AI Engine.
 */
class hoxFoliumConnection : public hoxAIConnection
{
public:
    hoxFoliumConnection() {} // DUMMY default constructor required for RTTI info.
    hoxFoliumConnection( wxEvtHandler* player );
    virtual ~hoxFoliumConnection() {}

protected:
    virtual void CreateAIEngine();

private:

    DECLARE_DYNAMIC_CLASS(hoxFoliumConnection)
};

#endif /* __INCLUDED_HOX_FOLIUM_PLAYER_H__ */
