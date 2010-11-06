#ifndef AIXQWLIGHT_H
#define AIXQWLIGHT_H

#include "AIEngine.h"
#include <string>

class AI_XQWLight : public AIEngine
{
public:
    AI_XQWLight(const char* szBookFile = 0);

    int setDifficultyLevel(int nAILevel);
    int initGame();
    int initGameWithBook(const char* szBookFile);
    int generateMove(int* pRow1, int* pCol1, int* pRow2, int* pCol2);
    int onHumanMove(int row1, int col1, int row2, int col2);
    const char* getInfo();

private:
    std::string _sBookFile;
};

#endif // AIXQWLIGHT_H
