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

/*
 * External dependencies (defined in 'haqikidHOX.cpp')
 */

extern void        InitEngine();
extern void        InitGame();
extern const char* GenerateNextMove();
extern void        OnOpponentMove(const char *line);
extern void        DeInitEngine();


/*
 * AI Engine Implementation
 */

class AIEngineImpl : public DefaultDelete<AIEngineLib>
{
public:
    AIEngineImpl(const char* engineName)
    {
        m_name = engineName ? engineName : "__UNKNOWN__";
    }

    ~AIEngineImpl()
    {
        DeInitEngine();
    }

    void destroy()
    {
        delete this;
    }

    void initEngine()
    {
        ::InitEngine();
    }

  	int initGame( const std::string& fen )
    {
        if ( ! fen.empty() ) return hoxAI_RC_NOT_SUPPORTED;

        ::InitGame();
        return hoxAI_RC_OK;
    }

	std::string generateMove()
    {
        const char* szMove = ::GenerateNextMove();
     
        std::string sMove;
        sMove += szMove[0] - 'a' + '0';
        sMove += '9' + '0' - szMove[1];
        sMove += szMove[2] - 'a' + '0';
        sMove += '9' + '0' - szMove[3];
        return sMove;
    }

    void onHumanMove( const std::string& sMove )
    {
        std::string stdMove;
        stdMove += (sMove[0] + 'a' - '0'); 
        stdMove += ('9' + '0' - sMove[1]); 
        stdMove += (sMove[2] + 'a' - '0'); 
        stdMove += ('9' + '0' - sMove[3]); 
        
        ::OnOpponentMove( stdMove.c_str() );
    }


private:
    std::string m_name;

}; /* class AIEngineImpl */


//////////////////////////////////////////////////////////////
AIEngineLib* CreateAIEngineLib()
{
  return new AIEngineImpl("HaQiKi D Engine Lib");
}

/************************* END OF FILE ***************************************/
