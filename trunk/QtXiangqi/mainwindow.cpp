#include "mainwindow.h"
#include "tableui.h"
#include "logindialog.h"
#include "piece.h"
#include <QtGui>
#include <QtDebug>

// ********************************************************
MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , onlineConnection_(NULL)
        , handler_(NULL)
{
    ui_.setupUi(this);

    createActions();
    createMenus();
    createToolBars();
    setupWidgets();

    readSettings();

    //setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    setWindowTitle(tr("QtXiangqi (main)"));

    setUnifiedTitleAndToolBarOnMac(true);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();

    // -----
    // Send a LOGOUT request and close the online connection.
    if (onlineConnection_) {
        onlineConnection_->send_LOGOUT();

        onlineConnection_->stop();
        delete onlineConnection_;
        onlineConnection_ = NULL;
    }

    if (handler_) {
        delete handler_;
        handler_ = NULL;
    }
    // ------

    event->accept();
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui_.retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::createActions()
{
    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    exitAct = new QAction(QIcon(":/images/exit.png"), tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    restartAct = new QAction(QIcon(":/images/paste.png"), tr("&Restart"), this);
    restartAct->setShortcuts(QKeySequence::Paste);
    restartAct->setStatusTip(tr("Restart the current Game"));
    connect(restartAct, SIGNAL(triggered()), this, SLOT(restartGame()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    saveAct->setEnabled(false);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));

    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    gameMenu = menuBar()->addMenu(tr("&Game"));
    gameMenu->addAction(restartAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
    QToolBar *mainToolBar = ui_.mainToolBar;
    mainToolBar->addAction(exitAct);

    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(saveAct);

    gameToolBar = addToolBar(tr("Game"));
    gameToolBar->addAction(restartAct);
}

void MainWindow::setupWidgets()
{
    QFrame *frame = new QFrame;
    QHBoxLayout *frameLayout = new QHBoxLayout(frame);

    /* -------------------- Left frame. */
    QWidget *sitesWindow = new QWidget;
    QVBoxLayout *siteLayout = new QVBoxLayout(sitesWindow);
    siteLayout->addWidget(new QPushButton(tr("Practice"), sitesWindow));

    QPushButton* button = new QPushButton(tr("Online"), sitesWindow);
    connect(button, SIGNAL(clicked()), this, SLOT(onlineClicked()));
    siteLayout->addWidget(button);

    sitesWindow->setMinimumSize(120, 400);
    sitesWindow->setMaximumSize(120, 400);

    /* -------------------- Central frame. */
    TableUI *tableUI = new TableUI(this);

    frameLayout->addWidget(sitesWindow);
    frameLayout->addWidget(tableUI);
    setCentralWidget(frame);
}

void MainWindow::readSettings()
{
    QSettings settings("PlayXiangqi", "QtXiangqi");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    resize(size);
    move(pos);
}

void MainWindow::writeSettings()
{
    QSettings settings("PlayXiangqi", "QtXiangqi");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

bool MainWindow::save()
{
    return false;
}

void MainWindow::restartGame()
{
}

void MainWindow::onlineClicked()
{
    LoginDialog loginDialog(this);
    loginDialog.setWindowTitle(tr("Login"));
    const int dialogCode = loginDialog.exec();
    if (dialogCode == QDialog::Rejected) {
        return;
    }

    const std::string sUsername = loginDialog.getUsername().toStdString();
    const std::string sPassword = loginDialog.getPassword().toStdString();

    // -----------
    using namespace hox::network;
    const ServerAddress serverAddress("games.playxiangqi.com", "80");
    handler_ = new NetworkDataHandler;
    onlineConnection_ = new hoxSocketConnection( serverAddress, handler_ );
    onlineConnection_->start();

    onlineConnection_->send_LOGIN(sUsername, sPassword);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Application"),
            tr("This is an Qt based <b>Xiangqi</b> application written as a modern GUI "
               "applications using Qt, with a menu bar, toolbars, and a status bar."));
}


// ********************************************************

void
NetworkDataHandler::onNewPayload(const hox::network::DataPayload& payload)
{
    qDebug() << "Payload: " << payload.type()
            << ", data=[" << payload.data().c_str() << "]";
}
