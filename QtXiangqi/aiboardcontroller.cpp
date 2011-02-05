#include "aiboardcontroller.h"
#include "AI/AI_XQWLight.h"
#include "Referee/Referee.h"

AIBoardController::AIBoardController(QWidget* parent)
    : QObject()
{
    board_ = new TableUI(parent, this);
    aiEngine_ = createAIEngine_();
    connect(&aiWatcher_, SIGNAL(finished()), this, SLOT(handleAIMoveGenerated_()));
}

AIBoardController::~AIBoardController()
{
    delete board_;
}

void AIBoardController::resetAI()
{
    if (AI_RC_OK != aiEngine_->initGame()) {
        qWarning() << "Fail to reset AI.";
    }
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

    //if (_referee->gameStatus() == HC_GAME_STATUS_IN_PROGRESS)
    {
        aiEngine_->onHumanMove(from.row, from.col, to.row, to.col);
        // Delay the AI 's move a little for the end-user to observe his own move.
        QTimer::singleShot(1000, this, SLOT(askAIToGenerateMove_()));
    }
}

ColorEnum AIBoardController::ownerColor()
{
    return HC_COLOR_RED;
}

// ----------- My own API -----------------------------------------------------

AIEngine_APtr AIBoardController::createAIEngine_()
{
    QDir appDir(QApplication::applicationDirPath());
    QString bookPath = appDir.absoluteFilePath("../Resources/books/BOOK.DAT");
    QByteArray utf8Name = bookPath.toUtf8();
    AIEngine_APtr aiEngine( new AI_XQWLight(utf8Name.constData()) );

    if (AI_RC_OK != aiEngine->initGame())
    {
        qWarning() << "Fail to init AI with open AI Book: [" << bookPath << "]";
    }
    aiEngine->setDifficultyLevel(5);

    return aiEngine;
}

void AIBoardController::askAIToGenerateMove_()
{
    QFuture<int> future = QtConcurrent::run(this, &AIBoardController::runAIToGenerateMove_);
    aiWatcher_.setFuture(future);
}

int AIBoardController::runAIToGenerateMove_()
{
    int row1 = 0, col1 = 0, row2 = 0, col2 = 0;
    aiEngine_->generateMove(&row1, &col1, &row2, &col2);
    const int move = MOVE( TOSQUARE(row1, col1), TOSQUARE(row2, col2) );
    return move;
}

void AIBoardController::handleAIMoveGenerated_()
{
    const int move = aiWatcher_.result(); // Retrieve the newly generated move.
    const int sqSrc = SRC(move);
    const int sqDst = DST(move);
    hox::Position from(ROW(sqSrc), COLUMN(sqSrc));
    hox::Position to(ROW(sqDst), COLUMN(sqDst));
    board_->onNewMove(from, to);
}
