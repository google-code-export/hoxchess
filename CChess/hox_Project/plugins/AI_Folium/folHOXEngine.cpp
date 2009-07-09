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
// Name:            folHOXEngine.cpp
// Created:         10/09/2008
//
// Description:     This is 'folium' Engine to interface with HOXChess.
/////////////////////////////////////////////////////////////////////////////

#include "utility/time.h"
#include "engine.h"
#include "folHOXEngine.h"
#include <sstream>     // ostringstream


// ----------------------------------------------------------------------------
//
// folHOXEngine
//
// ----------------------------------------------------------------------------

folHOXEngine::folHOXEngine( const int searchDepth /* = 3 */ )
        : _engine( NULL )
        , _searchDepth( searchDepth )
{
}

folHOXEngine::~folHOXEngine()
{
    delete _engine;
}

void
folHOXEngine::InitGame( const std::string& fen )
{
	/* FEN starting position of the Board. */
	std::string fenStartPosition = fen;
    if ( fenStartPosition.empty() )
    {
		fenStartPosition = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1";
    }
    else
    {
        // FIXME: Hack to make it look the 'starting' positions above.
        //        Still, this engine crashes.
        fenStartPosition += " - - 0 1";
    }

    delete _engine;
	_engine = new folium::Engine();
	_engine->load(fenStartPosition);
}

std::string
folHOXEngine::GenerateMove()
{
	std::set<folium::uint> ban;
	_engine->m_stop = false;
	_engine->m_depth = std::max(_searchDepth, 5);
	_engine->m_mintime = folium::now_time() + 1000;
	_engine->m_maxtime = folium::now_time() + 3000;
	unsigned int move = _engine->search( ban );
	std::string sNextMove;
	if (move)
	{
		sNextMove = _folium2hox( move );
		_engine->make_move(move);
	}
	return sNextMove;
}

void
folHOXEngine::OnHumanMove( const std::string& sMove )
{
	unsigned int move = _hox2folium(sMove);
	_engine->make_move(move);
}

unsigned int
folHOXEngine::_hox2folium( const std::string& sMove ) const
{
	unsigned int sx = sMove[0] - '0';
	unsigned int sy = sMove[1] - '0';
	unsigned int dx = sMove[2] - '0';
	unsigned int dy = sMove[3] - '0';
	unsigned int src = 89 - (sx + sy * 9);
	unsigned int dst = 89 - (dx + dy * 9);
	return src | (dst << 7);
}

std::string
folHOXEngine::_folium2hox( unsigned int move ) const
{
	unsigned int src = 89 - (move & 0x7f);
	unsigned int dst = 89 - ((move >> 7) & 0x7f);
	unsigned int sx = src % 9;
	unsigned int sy = src / 9;
	unsigned int dx = dst % 9;
	unsigned int dy = dst / 9;

	std::ostringstream ostr;
	ostr << sx << sy << dx << dy;
	return ostr.str();
}

/************************* END OF FILE ***************************************/
