#ifndef NETWORKBOARDCONTROLLER_H
#define NETWORKBOARDCONTROLLER_H

#include <QtGui>
#include "tableui.h"
#include "network/hoxSocketConnection.h"

class NetworkBoardController : public QObject
                             , public BoardOwner
                             , public hox::network::DataHandler
{
    Q_OBJECT
public:
    NetworkBoardController(QWidget* parent);
    virtual ~NetworkBoardController();

    TableUI* getBoard() const { return board_; }

    // ---------- BoardOwner API implementation -------------------------------
    virtual bool isMyTurnNext();
    virtual bool isGameReady();
    virtual void onLocalMoveMadeFrom(hox::Position from, hox::Position to);
    virtual ColorEnum ownerColor();

    // ---------- hox::network::DataHandler API implementation -----------------
    virtual void onNewPayload(const hox::network::DataPayload& payload);

signals:
    void messageReceived(const QString& sData);

private slots:
    void handleMessage(const QString& sData);

private:
    void readSettings_();
    void writeSettings_();

    // Network message handlers.
    void handleMessage_LOGIN_(const std::string& content);
    void handleMessage_LIST_(const std::string& content);
    void handleMessage_I_TABLE_(const std::string& content);
    void handleMessage_I_MOVES_(const std::string& content);
    void handleMessage_MOVE_(const std::string& content);

private:
    TableUI*            board_;
    ColorEnum           myColor_; // The color (role) of the LOCAL player.

    hox::network::SocketConnection* connection_;
    QString                         pid_;       // My player-Id (PID).
    QString                         password_;  // My password.
};

#endif // NETWORKBOARDCONTROLLER_H
