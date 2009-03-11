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
 * 'Folium' AI Engine Plugin.
 */


#include <AIEngineLib.h>
#include <DefaultDelete.h>
#include "folHOXEngine.h"

class AIEngineImpl : public DefaultDelete<AIEngineLib>
{
public:
    AIEngineImpl(const char* engineName)
        : m_name( engineName ? engineName : "__UNKNOWN__" )
        , m_engine( new folHOXEngine() )
    {
    }

    ~AIEngineImpl()
    {
        delete m_engine;
    }

    void destroy()
    {
        delete this;
    }

    void initEngine()
    {
    }

  	void initGame(unsigned char pcsSavedPos[][9]=NULL)
    {
        // TODO: ...
    }

	std::string generateMove()
    {
        return m_engine->GenerateMove();
    }

    void onHumanMove( const std::string& sMove )
    {
        m_engine->OnHumanMove( sMove );
    }


private:
    std::string    m_name;
    folHOXEngine*  m_engine;

}; /* class AIEngineImpl */


//////////////////////////////////////////////////////////////
AIEngineLib* CreateAIEngineLib()
{
  return new AIEngineImpl("Folium Engine Lib");
}

/************************* END OF FILE ***************************************/
