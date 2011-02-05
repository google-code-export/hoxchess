#ifndef AIBOARDCONTROLLER_H
#define AIBOARDCONTROLLER_H

#include <QtGui>
#include "tableui.h"
#include <memory>

class AIEngine;
typedef std::auto_ptr<AIEngine> AIEngine_APtr;

class AIBoardController : public QObject, public BoardOwner
{
    Q_OBJECT
public:
    AIBoardController(QWidget* parent);
    virtual ~AIBoardController();

    TableUI* getBoard() const { return board_; }

    void resetAI();

    // ---------- BoardOwner API implementation -------------------------------
    virtual bool isMyTurnNext();
    virtual bool isGameReady();
    virtual void onLocalMoveMadeFrom(hox::Position from, hox::Position to);
    virtual ColorEnum ownerColor();

private:
    std::auto_ptr<AIEngine> createAIEngine_();
    int runAIToGenerateMove_();

private slots:
    void askAIToGenerateMove_();
    void handleAIMoveGenerated_();

private:
    TableUI*            board_;
    ColorEnum           myColor_; // The color (role) of the LOCAL player.

    AIEngine_APtr       aiEngine_;
    QFutureWatcher<int> aiWatcher_;
};

#endif // AIBOARDCONTROLLER_H
