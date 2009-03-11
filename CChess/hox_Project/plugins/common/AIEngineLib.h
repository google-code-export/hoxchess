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

#ifndef __INCLUDED_AI_ENGINE_LIB_H__
#define __INCLUDED_AI_ENGINE_LIB_H__

#ifdef WIN32
  #ifdef EXPORTING
    #define CALL __declspec(dllexport)
  #else
    #define CALL __declspec(dllexport)
  #endif
#else
#define CALL
#endif

#include <string>

/**
 * AIEngineLib interface.
 */
class AIEngineLib
{
public:
    virtual void        destroy() = 0;
    virtual void        initEngine() = 0;

    // ------------
	virtual void        initGame(unsigned char pcsSavedPos[][9]=NULL) = 0;
	virtual std::string generateMove() = 0;
    virtual void        onHumanMove( const std::string& sMove ) = 0;
    // ------------

    void operator delete(void* p)
    {
        if (p)
        {
            AIEngineLib* engine = static_cast<AIEngineLib*>(p);
            engine->destroy();
        }
    }
};


extern "C" CALL AIEngineLib* CreateAIEngineLib();

typedef AIEngineLib* (*PICreateAIEngineLibFunc)();

#endif /* __INCLUDED_AI_ENGINE_LIB_H__ */
