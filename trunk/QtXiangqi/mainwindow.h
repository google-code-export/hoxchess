#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>

#include "ui_mainwindow.h"
#include "network/hoxSocketConnection.h"
#include "message/hoxMessage.h"

class NetworkDataHandler;

// ********************************************************
class MainWindow : public QMainWindow,
                   public hox::network::DataHandler
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

   /** Override API from hox::network::DataHandler */
   virtual void onNewPayload(const hox::network::DataPayload& payload);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void restartGame();
    void about();
    bool save();
    void onlineClicked();

    void handleMessage(const QString& sData);

protected:
    void changeEvent(QEvent *e);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void setupWidgets();
    void readSettings();
    void writeSettings();

    // Network message handlers.
    void handleMessage_LOGIN_(const std::string& content);
    void handleMessage_LIST_(const std::string& content);

signals:
     void messageReceived(const QString& sData);

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

    hox::network::SocketConnection*  connection_;
    std::string                      pid_;       // My player-Id (PID).
    std::string                      password_;  // My password.
};

#endif // MAINWINDOW_H
