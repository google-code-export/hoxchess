#include "networkboardcontroller.h"
#include "logindialog.h"
#include "tablelistui.h"
#include "message/hoxMessage.h"

NetworkBoardController::NetworkBoardController(QWidget *parent)
    : QObject()
    , board_(0)
    , myColor_(HC_COLOR_UNKNOWN)
    , connection_(0)
{
    board_ = new TableUI(parent, this);

    connect(this, SIGNAL(messageReceived(const QString&)),
            this, SLOT(handleMessage(const QString&)));

    readSettings_();

    LoginDialog loginDialog(board_, pid_, password_);
    const int dialogCode = loginDialog.exec();
    if (dialogCode == QDialog::Rejected)
    {
        return;
    }

    pid_ = loginDialog.getPid();
    password_ = loginDialog.getPassword();
    writeSettings_();

    // -----------
    const hox::network::ServerAddress serverAddress("games.playxiangqi.com", "80");
    connection_ = new hox::network::SocketConnection( serverAddress, this );
    connection_->start();

    connection_->send_LOGIN(pid_.toStdString(), password_.toStdString());
}

NetworkBoardController::~NetworkBoardController()
{
    // Send a LOGOUT request and close the online connection.
    if (connection_) {
        connection_->send_LOGOUT();

        connection_->stop();
        delete connection_;
        connection_ = 0;
    }

    delete board_;
}

// ---------- BoardOwner API implementation -------------------------------

bool NetworkBoardController::isMyTurnNext()
{
    return true;
}

bool NetworkBoardController::isGameReady()
{
    return true;
}

void NetworkBoardController::onLocalMoveMadeFrom(hox::Position from, hox::Position to)
{
    qDebug("%s: ENTER: Move {(r=%d c=%d) => (r=%d c=%d)}.", __FUNCTION__,
           from.row, from.col, to.row, to.col);
}

ColorEnum NetworkBoardController::ownerColor()
{
    return myColor_;
}

// ---------- hox::network::DataHandler API implementation -----------------

void
NetworkBoardController::onNewPayload(const hox::network::DataPayload& payload)
{
    if (payload.type() == hox::network::TYPE_ERROR)
    {
        qWarning("%s: Received an ERROR payload: [%s].", __FUNCTION__, payload.data().c_str());
        return;
    }

    const QString data = QString::fromStdString(payload.data());
    emit messageReceived(data);
}

// ----------- My own API -----------------------------------------------------

void NetworkBoardController::readSettings_()
{
    QSettings settings;

    // ... network login information.
    pid_ = settings.value("pid").toString();
    password_ = settings.value("password").toString();
}

void NetworkBoardController::writeSettings_()
{
    QSettings settings;

    // ... network login information.
    settings.setValue("pid", pid_);
    settings.setValue("password", password_);
}

void
NetworkBoardController::handleMessage(const QString& sData)
{
    const std::string data = sData.toStdString();

    hox::Message message;
    hox::Message::string_to_message(data, message);

    const std::string op = message.m_type;
    const std::string content = message.m_parameters["content"];

    if      (op == "LOGIN")       handleMessage_LOGIN_(content);
    else if (op == "LIST")        handleMessage_LIST_(content);
    else
    {
        qDebug("%s: Unhandled payload: { %s }", __FUNCTION__, data.c_str());
    }
}

void
NetworkBoardController::handleMessage_LOGIN_(const std::string& content)
{
    std::string pid;
    int nRating = 0;
    hox::Message::parse_inCommand_LOGIN(content, pid, nRating);

    if ( pid_.toStdString() != pid ) // not my Id?
    {
        return; // Ignore other users' login.
    }
    qDebug("%s: I logged in as [%s %d].", __FUNCTION__, pid.c_str(), nRating);

    connection_->send_LIST(); // Get the latest tables.
}

void
NetworkBoardController::handleMessage_LIST_(const std::string& content)
{
    hox::TableList tables;
    hox::Message::parse_inCommand_LIST(content, tables);

    TableListUI tablesUI(board_);
    tablesUI.setTables(tables);
    const int dialogCode = tablesUI.exec();
    if (dialogCode == QDialog::Rejected)
    {
        return;
    }

    const QString selectedTableId = tablesUI.getSelectedId();
    qDebug("%s: selected table-Id = [%s].", __FUNCTION__, selectedTableId.toStdString().c_str());
}
