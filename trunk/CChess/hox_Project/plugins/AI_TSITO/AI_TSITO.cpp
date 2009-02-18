/**
 * DLL Interface classes for C++ Binary Compatibility article
 * article at http://aegisknight.org/cppinterface.html
 *
 * code author:     Ben Scott   (bscott@iastate.edu)
 * article author:  Chad Austin (aegis@aegisknight.org)
 */


#include <AIEngineLib.h>
#include <DefaultDelete.h>
#include <memory>
#include "Move.h"
#include "Board.h"
#include "Lawyer.h"
#include "tsiEngine.h"


class AIEngineImpl : public DefaultDelete<AIEngineLib>
{
public:
    AIEngineImpl(const char* engineName)
    {
        m_name = engineName ? engineName : "__UNKNOWN__";

        m_board.reset( new Board() );
        m_lawyer.reset( new Lawyer( m_board.get() ) );
        m_engine.reset( new tsiEngine( m_board.get(),
                                    m_lawyer.get() ) );
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
        // TODO: ...
    }

	std::string generateMove()
    {
        std::string sNextMove;

        m_engine->think();

        if ( m_engine->doneThinking() )
        {
          Move move = m_engine->getMove();
          Move x = Move();
          if (!(move == x))
            {
              m_board->makeMove( move );
              sNextMove = _translateMoveToString( move );
            }
        }

        return sNextMove;
    }

    void onHumanMove( const std::string& sMove )
    {
        Move tMove = _translateStringToMove( sMove );
        m_board->makeMove( tMove);
    }

private:
    Move _translateStringToMove( const std::string& sMove )
    {
        char fromX = sMove[0] - '0';
        char fromY = sMove[1] - '0';
        char toX   = sMove[2] - '0';
        char toY   = sMove[3] - '0';

        Move tMove;
        tMove.origin(     9 * fromY + fromX );
        tMove.destination( 9 * toY + toX );
        return tMove;
    }

    std::string _translateMoveToString( Move& tMove )
    {
        std::string sMove;
        sMove += '0' + (tMove.origin() % 9);
        sMove += '0' + (tMove.origin() / 9);
        sMove += '0' + (tMove.destination() % 9);
        sMove += '0' + (tMove.destination() / 9);
        return sMove;
    }

private:
    std::string m_name;

    typedef std::auto_ptr<Board>  TSITO_Board_APtr;
    typedef std::auto_ptr<Lawyer> TSITO_Lawyer_APtr;
    typedef std::auto_ptr<tsiEngine> TSITO_Engine_APtr;

    TSITO_Board_APtr    m_board;
    TSITO_Lawyer_APtr   m_lawyer;
    TSITO_Engine_APtr   m_engine;

}; /* class AIEngineImpl */


//////////////////////////////////////////////////////////////
AIEngineLib* CreateAIEngineLib()
{
  return new AIEngineImpl("MaxQi Engine Lib");
}

/************************* END OF FILE ***************************************/
