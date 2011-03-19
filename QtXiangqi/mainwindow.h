#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_mainwindow.h"

#include "aiboardcontroller.h"
#include "networkboardcontroller.h"

// ********************************************************
class MainWindow : public QMainWindow
{
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
    void viewTablesClicked();

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

    QHBoxLayout* frameLayout_;

    AIBoardController*      aiBoardController_;
    NetworkBoardController* networkBoardController_;
};

#endif // MAINWINDOW_H
