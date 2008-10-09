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
// Created:         11/24/2008
//
// Description:     The AI Player based on the open-source Xiangqi Engine
//                  called FOLIUM written by Wangmao
//                  (username is 'lwm3751' under Google Code)
//                     http://folium.googlecode.com
/////////////////////////////////////////////////////////////////////////////

#include "hoxFoliumPlayer.h"
#include "../folium/folEngine.h"

//-----------------------------------------------------------------------------
// FOLIUM_Engine
//-----------------------------------------------------------------------------

class hoxFoliumEngine::FOLIUM_Engine
{
public:
    FOLIUM_Engine()
    {
        // FEN starting position of the Board.
        const std::string fenStartPosition = 
            "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1";

        /* TODO:
         *    Wangmao, I think we should hide this '21' magic number.
         *    For a normal developer (who is not familiar with Xiangqi AI),
         *    we have no idea what this 'hash' value is.
         */
        m_engine.reset( new folEngine(XQ(fenStartPosition), 21) );
    }

    unsigned int _hox2folium( const wxString& sMove )
    {
        unsigned int sx = sMove[0] - '0';
        unsigned int sy = sMove[1] - '0';
        unsigned int dx   = sMove[2] - '0';
        unsigned int dy   = sMove[3] - '0';
	    unsigned int src = 89 - (sx + sy * 9);
	    unsigned int dst = 89 - (dx + dy * 9);
	    return src | (dst << 7);
    }

    wxString _folium2hox( unsigned int move )
    {
	    unsigned int src = 89 - (move & 0x7f);
	    unsigned int dst = 89 - ((move >> 7) & 0x7f);
	    unsigned int sx = src % 9;
	    unsigned int sy = src / 9;
	    unsigned int dx = dst % 9;
	    unsigned int dy = dst / 9;
	    wxString sMove;
	    sMove.Printf("%d%d%d%d", sx, sy, dx,dy);
	    return sMove;
    }

    wxString GenerateMove()
    {
        std::set<unsigned int> ban;
        const int searchDepth = 3 /* 7 */;
        unsigned int move = m_engine->search( searchDepth, ban );
        wxString sNextMove;
        if (move)
        {
            sNextMove = _folium2hox( move );
            m_engine->make_move(move);
        }
        return sNextMove;
    }

    void OnHumanMove( const wxString& sMove )
    {
        unsigned int move = _hox2folium(sMove);
        m_engine->make_move(move);
    }

private:
    typedef std::auto_ptr<folEngine> folEngine_APtr;
    folEngine_APtr   m_engine;
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
        , m_engine( new FOLIUM_Engine() )
{
}

hoxFoliumEngine::~hoxFoliumEngine()
{
    delete m_engine;
}

void
hoxFoliumEngine::OnOpponentMove( const wxString& sMove )
{
    m_engine->OnHumanMove( sMove );
}

wxString
hoxFoliumEngine::GenerateNextMove()
{
    return m_engine->GenerateMove();
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
