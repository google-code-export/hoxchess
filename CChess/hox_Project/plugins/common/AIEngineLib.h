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
