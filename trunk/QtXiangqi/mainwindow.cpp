#include <QtGui>
#include <QtDebug>
#include "mainwindow.h"
#include "tableui.h"
#include "logindialog.h"
#include "tablelistui.h"
#include "piece.h"

// ********************************************************
MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , aiBoardController_(0)
        , connection_(0)
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

    // Send a LOGOUT request and close the online connection.
    if (connection_) {
        connection_->send_LOGOUT();

        connection_->stop();
        delete connection_;
        connection_ = 0;
    }

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
    aiBoardController_ = new AIBoardController(this);
    TableUI* tableUI = aiBoardController_->getBoard();

    frameLayout->addWidget(sitesWindow);
    frameLayout->addWidget(tableUI);
    setCentralWidget(frame);
}

void MainWindow::readSettings()
{
    QSettings settings;

    // Restore the main window 's position and size.
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    resize(size);
    move(pos);
}

void MainWindow::writeSettings()
{
    QSettings settings;

    // Save the main window 's position and size.
    settings.setValue("pos", pos());
    settings.setValue("size", size());

    // Save the network login information.
    settings.setValue("pid", QString::fromStdString(pid_));
    settings.setValue("password", QString::fromStdString(password_));
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
    connect(this, SIGNAL(messageReceived(const QString&)),
            this, SLOT(handleMessage(const QString&)));

    QSettings settings;
    const QString defaultPid = settings.value("pid").toString();
    const QString defaultPassword = settings.value("password").toString();

    LoginDialog loginDialog(this, defaultPid, defaultPassword);
    const int dialogCode = loginDialog.exec();
    if (dialogCode == QDialog::Rejected) {
        return;
    }

    pid_ = loginDialog.getPid().toStdString();
    password_ = loginDialog.getPassword().toStdString();

    // -----------
    const hox::network::ServerAddress serverAddress("games.playxiangqi.com", "80");
    connection_ = new hox::network::SocketConnection( serverAddress, this );
    connection_->start();

    connection_->send_LOGIN(pid_, password_);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Application"),
            tr("This is an Qt based <b>Xiangqi</b> application written as a modern GUI "
               "applications using Qt, with a menu bar, toolbars, and a status bar."));
}

// --------------------------------------------------------
//
//     Network messages handlers
//
// --------------------------------------------------------

void
MainWindow::onNewPayload(const hox::network::DataPayload& payload)
{
    if (payload.type() == hox::network::TYPE_ERROR) {
        qWarning("%s: Received an ERROR payload: [%s].", __FUNCTION__, payload.data().c_str());
        return;
    }

    const QString data = QString::fromStdString(payload.data());
    emit messageReceived(data);
}

void
MainWindow::handleMessage(const QString& sData)
{
    const std::string data = sData.toStdString();

    hox::Message message;
    hox::Message::string_to_message(data, message);

    const std::string op = message.m_type;
    const std::string content = message.m_parameters["content"];

    if      (op == "LOGIN")       handleMessage_LOGIN_(content);
    else if (op == "LIST")        handleMessage_LIST_(content);
    else {
        qDebug("%s: Unhandled payload: { %s }", __FUNCTION__, data.c_str());
    }
}

void
MainWindow::handleMessage_LOGIN_(const std::string& content)
{
    std::string pid;
    int nRating = 0;
    hox::Message::parse_inCommand_LOGIN(content, pid, nRating);

    if ( pid_ != pid ) { // not my Id?
        return; // Other users' login. Ignore for now.
    }
    qDebug("%s: I logged in as [%s %d].", __FUNCTION__, pid.c_str(), nRating);
    const QString playerInfo = QString::fromStdString(pid)
                               % "(" % QString::number(nRating) % ")";
    setWindowTitle( tr("QtXiangqi: ") % playerInfo);

    connection_->send_LIST(); // Get the latest tables.
}

void
MainWindow::handleMessage_LIST_(const std::string& content)
{
    hox::TableList tables;
    hox::Message::parse_inCommand_LIST(content, tables);

    TableListUI tablesUI(this);
    tablesUI.setTables(tables);
    const int dialogCode = tablesUI.exec();
    if (dialogCode == QDialog::Rejected) {
        return;
    }

    const QString selectedTableId = tablesUI.getSelectedId();
    qDebug("%s: selected table-Id = [%s].", __FUNCTION__, selectedTableId.toStdString().c_str());
}
