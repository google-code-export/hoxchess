#include "tableui.h"
#include "board.h"

TableUI::TableUI(QWidget* parent, BoardOwner* boardOwner)
        : QWidget(parent)
        , board_(0)
        , boardOwner_(boardOwner)
{
    ui_.setupUi(this);

    QVBoxLayout *verticalLayout = ui_.verticalLayout;
    board_ = new Board(this, boardOwner_);
    verticalLayout->insertWidget(0, board_);
}

TableUI::~TableUI()
{
}

void TableUI::changeEvent(QEvent* e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui_.retranslateUi(this);
        break;
    default:
        break;
    }
}

void TableUI::on_replayPrevButton_clicked()
{
    board_->doReplay_PREVIOUS();
}

void TableUI::on_replayNextButton_clicked()
{
    board_->doReplay_NEXT();
}

void TableUI::on_replayBeginButton_clicked()
{
    board_->doReplay_BEGIN();
}

void TableUI::on_replayEndButton_clicked()
{
    board_->doReplay_END();
}

void TableUI::on_resetButton_clicked()
{
    board_->resetBoard();
}
