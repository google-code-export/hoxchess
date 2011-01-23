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
    QString getSelectedId() const { return selectedId_; }

protected:
    void changeEvent(QEvent *e);

private:
    void addTable_(QStandardItemModel *model, const hox::TableInfo& table);

private slots:
    void clicked(const QModelIndex &index);

private:
    Ui::TableListUI ui_;
    hox::TableList tables_;
    QVector<QString> tableIds_;
    QString selectedId_;
};

#endif // TABLELISTUI_H
