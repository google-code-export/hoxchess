#include "tableui.h"
#include "ui_tableui.h"
#include "board.h"

TableUI::TableUI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TableUI)
{
    ui->setupUi(this);

    QVBoxLayout *verticalLayout = ui->verticalLayout;
    _board = new Board(this);
    verticalLayout->insertWidget(0, _board);
}

TableUI::~TableUI()
{
    delete ui;
}

void TableUI::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void TableUI::on_replayPrevButton_clicked()
{
    _board->doReplay_PREVIOUS();
}

void TableUI::on_replayNextButton_clicked()
{
    _board->doReplay_NEXT();
}

void TableUI::on_replayBeginButton_clicked()
{
    _board->doReplay_BEGIN();
}

void TableUI::on_replayEndButton_clicked()
{
    _board->doReplay_END();
}

void TableUI::on_resetButton_clicked()
{
    _board->resetBoard();
}
