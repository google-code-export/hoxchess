/**
 * DLL Interface classes for C++ Binary Compatibility article
 * article at http://aegisknight.org/cppinterface.html
 *
 * code author:     Ben Scott   (bscott@iastate.edu)
 * article author:  Chad Austin (aegis@aegisknight.org)
 */


#include <AIEngineLib.h>
#include <DefaultDelete.h>
#include "XQWLight.h"

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

    void initEngine()
    {
    }

  	void initGame(unsigned char pcsSavedPos[][9]=NULL)
    {
        XQWLight::initialize( pcsSavedPos );
    }

	std::string generateMove()
    {
        return XQWLight::generate_move();
    }

    void onHumanMove( const std::string& sMove )
    {
        XQWLight::on_human_move( sMove );
    }


private:
    std::string m_name;

}; /* class AIEngineImpl */


AIEngineLib* CreateAIEngineLib()
{
  return new AIEngineImpl("XQWLight Engine Lib");
}

/************************* END OF FILE ***************************************/
