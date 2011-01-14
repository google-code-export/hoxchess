#include "tablelistui.h"

TableListUI::TableListUI(QWidget *parent) :
    QDialog(parent)
{
    ui_.setupUi(this);
    setWindowTitle(tr("Table List"));

    QStandardItemModel *model = new QStandardItemModel(0, 5, parent);

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Id"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Timer"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Red Player"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Black Player"));

    QTreeView* treeView = ui_.tablesTreeView;
    treeView->setAlternatingRowColors(true);
    treeView->setModel(model);
    treeView->setSortingEnabled(true);
}

TableListUI::~TableListUI()
{
}

void TableListUI::changeEvent(QEvent *e)
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

void TableListUI::setTables(const hox::TableList& tables)
{
    tables_ = tables;

    QTreeView* treeView = ui_.tablesTreeView;
    QStandardItemModel *model = (QStandardItemModel*) treeView->model();

    for (hox::TableList::const_iterator it = tables_.begin(); it != tables_.end(); ++it)
    {
        addTable(model, *(*it));
    }
}

void TableListUI::addTable(QStandardItemModel *model, const hox::TableInfo& table)
{
    QString redInfo("*");
    if (!table.redId.empty()) {
        redInfo.sprintf("%s (%s)", table.redId.c_str(), table.redRating.c_str());
    }
    QString blackInfo("*");
    if (!table.blackId.empty()) {
        blackInfo.sprintf("%s (%s)", table.blackId.c_str(), table.blackRating.c_str());
    }

    QString sTimer;
    sTimer.sprintf("%d/%d/%d", table.initialTime.nGame, table.initialTime.nMove,
                   table.initialTime.nFree);

    QList<QStandardItem *> items;
    items.append( new QStandardItem(QString("#").append(QString::fromStdString(table.id))) );
    items.append( new QStandardItem(table.rated ? tr("Rated") : tr("NotRated")) );
    items.append( new QStandardItem(sTimer) );
    items.append( new QStandardItem(redInfo) );
    items.append( new QStandardItem(blackInfo) );
    model->appendRow(items);

    /*
    model->appendRow(0);
    model->setData(model->index(0, 0), QString("#").append(QString::fromStdString(table.id)));
    model->setData(model->index(0, 1), table.rated ? tr("Rated") : tr("NotRated"));
    model->setData(model->index(0, 2), sTimer);
    model->setData(model->index(0, 3), redInfo);
    model->setData(model->index(0, 4), blackInfo);
    */
}
