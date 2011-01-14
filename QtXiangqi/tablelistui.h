#ifndef TABLELISTUI_H
#define TABLELISTUI_H

#include <QtGui>
#include "ui_tablelistui.h"
#include "types.h"

class TableListUI : public QDialog
{
    Q_OBJECT

public:
    explicit TableListUI(QWidget *parent = 0);
    ~TableListUI();

    void setTables(const hox::TableList& tables);

protected:
    void changeEvent(QEvent *e);

private:
    void addTable(QStandardItemModel *model, const hox::TableInfo& table);

private:
    Ui::TableListUI ui_;
    hox::TableList tables_;
};

#endif // TABLELISTUI_H
