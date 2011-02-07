#include <QtGui>
#include <QtDebug>
#include "mainwindow.h"
#include "tableui.h"

// ********************************************************
MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , aiBoardController_(0)
        , networkBoardController_(0)
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
    QToolBar* mainToolBar = ui_.mainToolBar;
    mainToolBar->addAction(exitAct);

    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(saveAct);

    gameToolBar = addToolBar(tr("Game"));
    gameToolBar->addAction(restartAct);
}

void MainWindow::setupWidgets()
{
    QFrame* frame = new QFrame;
    frameLayout_ = new QHBoxLayout(frame);

    /* -------------------- Left frame. */
    QWidget* sitesWindow = new QWidget;
    QVBoxLayout* siteLayout = new QVBoxLayout(sitesWindow);
    siteLayout->addWidget(new QPushButton(tr("Practice"), sitesWindow));

    QPushButton* button = new QPushButton(tr("Online"), sitesWindow);
    connect(button, SIGNAL(clicked()), this, SLOT(onlineClicked()));
    siteLayout->addWidget(button);

    sitesWindow->setMinimumSize(120, 400);
    sitesWindow->setMaximumSize(120, 400);

    /* -------------------- Central frame. */
    aiBoardController_ = new AIBoardController(this);
    TableUI* tableUI = aiBoardController_->getBoard();

    frameLayout_->addWidget(sitesWindow);
    frameLayout_->addWidget(tableUI);
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
}

bool MainWindow::save()
{
    return false;
}

void MainWindow::restartGame()
{
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Application"),
            tr("This is an Qt based <b>Xiangqi</b> application written as a modern GUI "
               "applications using Qt, with a menu bar, toolbars, and a status bar."));
}

void MainWindow::onlineClicked()
{
    delete aiBoardController_;
    aiBoardController_ = 0;

    networkBoardController_ = new NetworkBoardController(this);
    frameLayout_->addWidget( networkBoardController_->getBoard() );
}
