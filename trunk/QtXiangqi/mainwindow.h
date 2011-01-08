#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>

#include "ui_mainwindow.h"
#include "network/hoxSocketConnection.h"

class NetworkDataHandler;

// ********************************************************
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void restartGame();
    void about();
    bool save();
    void onlineClicked();

protected:
    void changeEvent(QEvent *e);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void setupWidgets();
    void readSettings();
    void writeSettings();

private:
    Ui::MainWindow ui_;

    QAction *saveAct;
    QAction *exitAct;
    QAction *restartAct;
    QAction *aboutAct;
    QAction *aboutQtAct;

    QMenu *fileMenu;
    QMenu *gameMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *gameToolBar;

    QGraphicsScene *scene;
    QGraphicsView *view;

    hox::network::hoxSocketConnection* onlineConnection_;
    NetworkDataHandler*                handler_;
};

// ********************************************************
class NetworkDataHandler : public hox::network::DataHandler
{
public:
    NetworkDataHandler() {}
    virtual void onNewPayload(const hox::network::DataPayload& payload);
};

#endif // MAINWINDOW_H
