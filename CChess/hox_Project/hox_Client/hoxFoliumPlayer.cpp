/***************************************************************************
 *  Copyright 2007, 2008, 2009 Huy Phan  <huyphan@playxiangqi.com>         *
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
// Name:            hoxFoliumPlayer.cpp
// Created:         11/24/2008
//
// Description:     The AI Player based on the open-source Xiangqi Engine
//                  called FOLIUM written by Wangmao
//                  (username is 'lwm3751' under Google Code)
//                     http://folium.googlecode.com
/////////////////////////////////////////////////////////////////////////////

#include "hoxFoliumPlayer.h"
#include "hoxUtil.h"

IMPLEMENT_DYNAMIC_CLASS(hoxFoliumPlayer, hoxAIPlayer)

//-----------------------------------------------------------------------------
// hoxFoliumPlayer
//-----------------------------------------------------------------------------

hoxFoliumPlayer::hoxFoliumPlayer( const wxString& name,
                                  hoxPlayerType   type,
                                  int             score )
            : hoxAIPlayer( name, type, score )
{
}

void
hoxFoliumPlayer::Start()
{
    hoxConnection_APtr connection( new hoxFoliumConnection( this ) );
    this->SetConnection( connection );
    this->GetConnection()->Start();
}


// ----------------------------------------------------------------------------
// hoxFoliumEngine
// ----------------------------------------------------------------------------

hoxFoliumEngine::hoxFoliumEngine( wxEvtHandler* player )
        : hoxAIEngine( player )
        , m_engine( new folHOXEngine() )
{
}

hoxFoliumEngine::~hoxFoliumEngine()
{
    delete m_engine;
}

void
hoxFoliumEngine::OnOpponentMove( const wxString& sMove )
{
    m_engine->OnHumanMove( hoxUtil::wx2std( sMove ) );
}

wxString
hoxFoliumEngine::GenerateNextMove()
{
    return hoxUtil::std2wx( m_engine->GenerateMove() );
}


// ----------------------------------------------------------------------------
// hoxFoliumConnection
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(hoxFoliumConnection, hoxAIConnection)

hoxFoliumConnection::hoxFoliumConnection( wxEvtHandler* player )
        : hoxAIConnection( player )
{
}

void
hoxFoliumConnection::CreateAIEngine()
{
    m_aiEngine.reset( new hoxFoliumEngine( this->GetPlayer() ) );
}

/************************* END OF FILE ***************************************/
