#ifndef REFEREE_H
#define REFEREE_H

#include "enums.h"

//
// Referee 's error codes (or Return-Codes).
//
#define HC_RC_REF_UNKNOWN       -1
#define HC_RC_REF_OK             0  // A generic success
#define HC_RC_REF_ERR            1  // A generic error

//
// Constants required as a result of porting XQWLight source code.
//
#define MAX_GEN_MOVES      128

#define INVALID_MOVE       (-1)
#define TOSQUARE(row, col) (16 * ((row) + 3) + ((col) + 3))
#define COLUMN(sq)         ((sq) % 16 - 3)
#define ROW(sq)            ((sq) / 16 - 3)

#define SRC(mv)            ((mv) & 255)
#define DST(mv)            ((mv) >> 8)
#define MOVE(sqSrc, sqDst) ((sqSrc) + (sqDst) * 256)

#define MATE_VALUE  10000
#define WIN_VALUE   (MATE_VALUE - 200)

//
// The Referee to judge a given Game.
//
class Referee
{
public:
    Referee();
    ~Referee();

    int initGame();
    int  generateMoveFrom(int sqSrc, int* moves);
    bool isLegalMove(int move);
    void makeMove(int move, int* ppcCaptured = 0);
    GameStatusEnum gameStatus() const;
    ColorEnum nextColor() const;
    int repStatus(int nRecur, int* pRepVal);
    bool isChecked();
    bool isMate();
    int get_nMoveNum();

private:
    GameStatusEnum _gameStatus;
};

#endif // REFEREE_H
