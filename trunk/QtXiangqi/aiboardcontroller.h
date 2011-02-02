#ifndef AIBOARDCONTROLLER_H
#define AIBOARDCONTROLLER_H

#include "tableui.h"

class AIBoardController : public BoardOwner
{
public:
    AIBoardController(QWidget* parent);

    TableUI* getBoard() const { return _board; }

    // ---------- BoardOwner API implementation -------------------------------
    virtual bool isMyTurnNext();
    virtual bool isGameReady();
    virtual void onLocalMoveMadeFrom(hox::Position from, hox::Position to);
    virtual ColorEnum ownerColor();

private:
    TableUI*  _board;
    ColorEnum _myColor;  // The color (role) of the LOCAL player.
};

#endif // AIBOARDCONTROLLER_H
