#ifndef TABLEUI_H
#define TABLEUI_H

#include "ui_tableui.h"
#include "enums.h"
#include "types.h"

class Board;

class BoardOwner
{
public:
    virtual bool isMyTurnNext() = 0;
    virtual bool isGameReady() = 0;
    virtual void onLocalMoveMadeFrom(hox::Position from, hox::Position to) = 0;
    virtual ColorEnum ownerColor() = 0;
};

///////////////////////////////////////////////////////////////////////////////

class TableUI : public QWidget
{
    Q_OBJECT
public:
    TableUI(QWidget* parent, BoardOwner* boardOwner = 0);
    ~TableUI();

    void onNewMove(hox::Position from, hox::Position to, bool setupMode = false);
    void resetBoard();

protected:
    void changeEvent(QEvent* e);

private slots:
    void on_replayEndButton_clicked();
    void on_replayBeginButton_clicked();
    void on_replayNextButton_clicked();
    void on_replayPrevButton_clicked();

    void on_resetButton_clicked();

private:
    Ui::TableUI ui_;
    Board*      board_;
    BoardOwner* boardOwner_;
};

#endif // TABLEUI_H
