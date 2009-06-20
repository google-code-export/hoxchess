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

/**
 * DLL Interface classes for C++ Binary Compatibility article
 * article at http://aegisknight.org/cppinterface.html
 *
 * code author:     Ben Scott   (bscott@iastate.edu)
 * article author:  Chad Austin (aegis@aegisknight.org)
 */


#include <AIEngineLib.h>
#include <DefaultDelete.h>
#include "MaxQi.h"

class AIEngineImpl : public DefaultDelete<AIEngineLib>
{
public:
    AIEngineImpl(const char* engineName)
    {
        m_name = engineName ? engineName : "__UNKNOWN__";
    }

    ~AIEngineImpl()
    {
    }

    void destroy()
    {
        delete this;
    }

    void initEngine( int nAILevel = 0 )
    {
        setDifficultyLevel( nAILevel == 0 ? 5 : nAILevel );
    }

  	int initGame( const std::string& fen,
                  const MoveList&    moves )
    {
        if ( ! fen.empty() ) return hoxAI_RC_NOT_SUPPORTED;

        MaxQi::init_game();
        return hoxAI_RC_OK;
    }

	std::string generateMove()
    {
        return MaxQi::generate_move();
    }

    void onHumanMove( const std::string& sMove )
    {
        MaxQi::on_human_move( sMove );
    }

    int setDifficultyLevel( int nAILevel )
    {
        int searchDepth = 1;

        if      ( nAILevel > 10 ) searchDepth = 10;
        else if ( nAILevel < 1 )  searchDepth = 1;
        else                      searchDepth = nAILevel;

        MaxQi::set_max_depth( searchDepth );
        return hoxAI_RC_OK;
    }

private:
    std::string m_name;

}; /* class AIEngineImpl */


//////////////////////////////////////////////////////////////
AIEngineLib* CreateAIEngineLib()
{
  return new AIEngineImpl("MaxQi Engine Lib");
}

/************************* END OF FILE ***************************************/
