#include "aiboardcontroller.h"

AIBoardController::AIBoardController(QWidget* parent)
{
    _board = new TableUI(parent, this);
}

// ---------- BoardOwner API implementation -------------------------------

bool AIBoardController::isMyTurnNext()
{
    return true;
}

bool AIBoardController::isGameReady()
{
    return true;
}

void AIBoardController::onLocalMoveMadeFrom(hox::Position from, hox::Position to)
{
    qDebug("%s: ENTER: Move {(r=%d c=%d) => (r=%d c=%d)}.", __FUNCTION__,
           from.row, from.col, to.row, to.col);
}

ColorEnum AIBoardController::ownerColor()
{
    return HC_COLOR_RED;
}
