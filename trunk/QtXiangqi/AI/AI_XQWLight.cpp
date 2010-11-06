#include "AI_XQWLight.h"

#define XQWLight_DEFAULT_LEVEL 3  // AI Default difficulty level

// Declarations of methods defined under "XQWLight.cpp"
extern void XQWLight_init_engine( int searchDepth );
extern void XQWLight_init_game();
extern void XQWLight_generate_move( int* pRow1, int* pCol1, int* pRow2, int* pCol2 );
extern void XQWLight_on_human_move( int row1, int col1, int row2, int col2 );
extern int XQWLight_load_book( const char *bookfile );

///////////////////////////////////////////////////////////////////////////////
//
//    Implementation of Public methods
//
///////////////////////////////////////////////////////////////////////////////

AI_XQWLight::AI_XQWLight(const char* szBookFile)
    : AIEngine()
{
    if (szBookFile) {
        _sBookFile.assign(szBookFile);
    }
    this->setDifficultyLevel(XQWLight_DEFAULT_LEVEL);
}

int AI_XQWLight::setDifficultyLevel(int nAILevel)
{
    int searchDepth = 1;

    if      ( nAILevel > 10 ) searchDepth = 10;
    else if ( nAILevel < 1 )  searchDepth = 1;
    else                      searchDepth = nAILevel;

    XQWLight_init_engine( searchDepth );
    return AI_RC_OK;
}

int AI_XQWLight::initGame()
{
    XQWLight_init_game();
    if (!_sBookFile.empty()) {
        const int nOpenings = XQWLight_load_book(_sBookFile.c_str());
        if (nOpenings <= 0) {
            printf("%s: Fail to load the Opening Book [%s].", __FUNCTION__, _sBookFile.c_str());
            return AI_RC_ERR;
        }
    }
    return AI_RC_OK;
}

int AI_XQWLight::initGameWithBook(const char* szBookFile)
{
    XQWLight_init_game();
    const int nOpenings = XQWLight_load_book(szBookFile);
    return (nOpenings > 0 ? AI_RC_OK : AI_RC_ERR);
}

int AI_XQWLight::generateMove(int* pRow1, int* pCol1, int* pRow2, int* pCol2)
{
    XQWLight_generate_move( pCol1, pRow1, pCol2, pRow2 );
    return AI_RC_OK;
}

int AI_XQWLight::onHumanMove(int row1, int col1, int row2, int col2)
{
    XQWLight_on_human_move( col1, row1, col2, row2 );
    return AI_RC_OK;
}

const char* AI_XQWLight::getInfo()
{
    return "Morning Yellow\n"
           "www.elephantbase.net";
}
