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
