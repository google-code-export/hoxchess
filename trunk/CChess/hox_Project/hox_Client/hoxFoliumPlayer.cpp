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
// Name:            hoxFoliumPlayer.cpp
// Created:         11/24/2007
//
// Description:     The AI Player based on the open-source Xiangqi Engine
//                  called FOLIUM written by Wangmao
//                  (username is 'lwm3751' under Google Code)
//                     http://folium.googlecode.com
/////////////////////////////////////////////////////////////////////////////

#include "hoxFoliumPlayer.h"
#include "folium/engine.h"

unsigned hox2folium(const wxString& sMove)
{
    unsigned sx = sMove[0] - '0';
    unsigned sy = sMove[1] - '0';
    unsigned dx   = sMove[2] - '0';
    unsigned dy   = sMove[3] - '0';
	unsigned src = 89 - (sx + sy * 9);
	unsigned dst = 89 - (dx + dy * 9);
	return src | (dst << 7);
}

wxString folium2hox(unsigned move)
{
	unsigned src = 89 - (move & 0x7f);
	unsigned dst = 89 - ((move >> 7) & 0x7f);
	unsigned sx = src % 9;
	unsigned sy = src / 9;
	unsigned dx = dst % 9;
	unsigned dy = dst / 9;
	wxString sMove;
	sMove.Printf("%d%d%d%d", sx, sy, dx,dy);
	return sMove;
}

//-----------------------------------------------------------------------------
// FoliumEngine
//-----------------------------------------------------------------------------

class FoliumEngine
{
public:
    FoliumEngine()
    {
        m_engine = new Engine(XQ("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1"), 21);
    }
    ~FoliumEngine()
    {
        delete m_engine;
    }
    wxString generateMove()
    {
        set<unsigned> ban;
        unsigned move = m_engine->search(4 /* 7 */, ban);
        wxString sNextMove;
        if (move)
        {
            sNextMove = folium2hox( move );
            m_engine->make_move(move);
        }
        return sNextMove;
    }
    void OnHumanMove( const wxString& sMove )
    {
        unsigned move = hox2folium(sMove);
        m_engine->make_move(move);
    }
private:
    Engine* m_engine;
};

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_CLASS(hoxFoliumPlayer, hoxAIPlayer)

BEGIN_EVENT_TABLE(hoxFoliumPlayer, hoxAIPlayer)
    // Empty
END_EVENT_TABLE()

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
        , m_engine( new FoliumEngine() )
{
}

hoxFoliumEngine::~hoxFoliumEngine()
{
    delete (FoliumEngine*)m_engine;
}

void
hoxFoliumEngine::OnOpponentMove( const wxString& sMove )
{
    ((FoliumEngine*)m_engine)->OnHumanMove( sMove );
}

wxString
hoxFoliumEngine::GenerateNextMove()
{
    return ((FoliumEngine*)m_engine)->generateMove();
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
