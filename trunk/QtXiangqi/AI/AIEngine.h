#ifndef AIENGINE_H
#define AIENGINE_H

//
// AI error codes (or Return-Codes).
//
#define AI_RC_UNKNOWN       -1
#define AI_RC_OK             0  // A generic success
#define AI_RC_ERR            1  // A generic error
#define AI_RC_NOT_FOUND      2  // Something not found
#define AI_RC_NOT_SUPPORTED  3  // Something not supported

//
// Abstract superclass for all AI Engines.
//
class AIEngine
{
public:
    virtual ~AIEngine() {}

    virtual int setDifficultyLevel(int nAILevel) = 0;
    virtual int initGame() = 0;
    virtual int initGameWithBook(const char* szBookFile) = 0;
    virtual int generateMove(int* pRow1, int* pCol1, int* pRow2, int* pCol2) = 0;
    virtual int onHumanMove(int row1, int col1, int row2, int col2) = 0;
    virtual const char* getInfo() { return "Unknown"; }
};

#endif // AIENGINE_H
