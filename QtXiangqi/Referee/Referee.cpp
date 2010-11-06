#include "Referee.h"

// Declarations of methods defined under "XQWLight_Referee.cpp"
extern void Referee_init_game();
extern int  Referee_generate_move_from( int sqSrc, int *mvs );
extern int  Referee_is_legal_move( int mv );

extern void Referee_make_move( int mv, int* ppcCaptured );
extern int  Referee_rep_status(int nRecur, int *repValue);
extern int  Referee_is_checked();
extern int  Referee_is_mate();
extern int  Referee_get_nMoveNum();
extern int  Referee_get_sdPlayer();

///////////////////////////////////////////////////////////////////////////////
//
//    Implementation of Public methods
//
///////////////////////////////////////////////////////////////////////////////

Referee::Referee()
    : _gameStatus(HC_GAME_STATUS_UNKNOWN)
{
}

Referee::~Referee()
{
}

int Referee::initGame()
{
    Referee_init_game();
    _gameStatus = HC_GAME_STATUS_IN_PROGRESS;
    return HC_RC_REF_OK;
}

int Referee::generateMoveFrom(int sqSrc, int* moves)
{
    return Referee_generate_move_from(sqSrc, moves);
}

bool Referee::isLegalMove(int move)
{
    int bLegal = Referee_is_legal_move( move );
    return ( bLegal == 1 );
}

void Referee::makeMove(int move, int* ppcCaptured /* = 0 */)
{
    Referee_make_move(move, ppcCaptured);

    if ( Referee_is_mate() ) {
        bool redMoved = (this->nextColor() == HC_COLOR_BLACK); // Red just moved?
        _gameStatus = (redMoved ? HC_GAME_STATUS_RED_WIN : HC_GAME_STATUS_BLACK_WIN);
    }
}

GameStatusEnum Referee::gameStatus() const
{
    return _gameStatus;
}

ColorEnum Referee::nextColor() const
{
    return (Referee_get_sdPlayer() ? HC_COLOR_BLACK : HC_COLOR_RED);
}

int Referee::repStatus(int nRecur, int* pRepVal)
{
    return Referee_rep_status(nRecur, pRepVal);
}

bool Referee::isChecked()
{
    return (Referee_is_checked() ? true : false);
}

bool Referee::isMate()
{
    return (Referee_is_mate() ? true : false);
}

int Referee::get_nMoveNum()
{
    return Referee_get_nMoveNum();
}
