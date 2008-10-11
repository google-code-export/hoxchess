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
// Name:            hoxXQWLightPlayer.cpp
// Created:         10/11/2008
//
// Description:     The AI Player based on the open-source (?) Xiangqi Engine
//                  called XQWLight written by Huang Chen at
//                  www.elephantbase.net
//
//  (Original Chinese URL)
//        http://www.elephantbase.net/computer/stepbystep1.htm
//
//  (Translated English URL using Goold Translate)
//       http://74.125.93.104/translate_c?hl=en&langpair= \
//       zh-CN|en&u=http://www.elephantbase.net/computer/stepbystep1.htm& \
//       usg=ALkJrhj7W0v3J1P-xmbufsWzYq7uKciL1w
//
/////////////////////////////////////////////////////////////////////////////

#include "hoxXQWLightPlayer.h"
#include "hoxUtil.h"
#include "../xqwlight/XQWLight.h"

IMPLEMENT_DYNAMIC_CLASS(hoxXQWLightPlayer, hoxAIPlayer)

//-----------------------------------------------------------------------------
// hoxXQWLightPlayer
//-----------------------------------------------------------------------------

hoxXQWLightPlayer::hoxXQWLightPlayer( const wxString& name,
                                  hoxPlayerType   type,
                                  int             score )
            : hoxAIPlayer( name, type, score )
{
}

void
hoxXQWLightPlayer::Start()
{
    hoxConnection_APtr connection( new hoxXQWLightConnection( this ) );
    this->SetConnection( connection );
    this->GetConnection()->Start();
}


// ----------------------------------------------------------------------------
// hoxXQWLightEngine
// ----------------------------------------------------------------------------

hoxXQWLightEngine::hoxXQWLightEngine( wxEvtHandler* player )
        : hoxAIEngine( player )
{
    XQWLight::initialize();
}

hoxXQWLightEngine::~hoxXQWLightEngine()
{
}

void
hoxXQWLightEngine::OnOpponentMove( const wxString& sMove )
{
    const std::string stdMove = hoxUtil::wx2std( sMove );
    XQWLight::on_human_move( stdMove );
}

wxString
hoxXQWLightEngine::GenerateNextMove()
{
    std::string stdMove = XQWLight::generate_move();
    return hoxUtil::std2wx( stdMove );
}


// ----------------------------------------------------------------------------
// hoxXQWLightConnection
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(hoxXQWLightConnection, hoxAIConnection)

hoxXQWLightConnection::hoxXQWLightConnection( wxEvtHandler* player )
        : hoxAIConnection( player )
{
}

void
hoxXQWLightConnection::CreateAIEngine()
{
    m_aiEngine.reset( new hoxXQWLightEngine( this->GetPlayer() ) );
}

/************************* END OF FILE ***************************************/
