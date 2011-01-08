#ifndef TABLEUI_H
#define TABLEUI_H

#include "ui_tableui.h"

class Board;

class TableUI : public QWidget {
    Q_OBJECT
public:
    TableUI(QWidget *parent = 0);
    ~TableUI();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TableUI ui_;
    Board       *board_;

private slots:
    void on_replayEndButton_clicked();
    void on_replayBeginButton_clicked();
    void on_replayNextButton_clicked();
    void on_replayPrevButton_clicked();

    void on_resetButton_clicked();
};

#endif // TABLEUI_H
