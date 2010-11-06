#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>

namespace Ui {
    class MainWindow;
}

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
    Ui::MainWindow *ui;

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
};

#endif // MAINWINDOW_H
