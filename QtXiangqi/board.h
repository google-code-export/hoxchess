#ifndef BOARD_H
#define BOARD_H

#include <QtGui>
#include "enums.h"
#include "types.h"

class Piece;
class Referee;
class ReplayMove;
class BoardOwner;

class Board : public QWidget
{
    Q_OBJECT
public:
    Board(QWidget* parent, BoardOwner* boardOwner);
    virtual ~Board();

    void doReplay_PREVIOUS();
    void doReplay_BEGIN();
    void doReplay_NEXT();
    void doReplay_END();

    void resetBoard();

    void onNewMove(hox::Position from, hox::Position to, bool setupMode = false);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void paintEvent(QPaintEvent *event);

private:
    void _updateUIOnNewMove(ReplayMove* pMove, bool animated);
    void _playSoundAfterMove(const ReplayMove *pMove);

    void _setPiecePosition(Piece* piece, hox::Position newPosition);
    void _animateLatestMove(ReplayMove* pMove);
    void _clearAllHighlight();
    void _clearAllAnimation();

    bool _doReplayPREV(bool animated = true);
    bool _doReplayNEXT(bool animated = true);

    void _onGameOver();
    void _setReplayMode(bool on);
    void _setInfoLabel(const QString& infoText);

    void _setupPieces();
    void _createPiece(PieceEnum type, ColorEnum color, int row, int column);
    hox::Position _pointToPosition(const QPoint& p) const;
    QPoint _positionToPieceOrigin(const hox::Position position) const;

    Piece* _findPieceAt(const hox::Position& position, bool includeInactive = false) const;
    Piece* _getKingOfColor(ColorEnum color) const;
    void _drawPiece(Piece* piece);

    bool _isMoveLegalFrom(hox::Position from, hox::Position to) const;

    bool isInReplay_() const;

    void _initSoundSystem();
    void _playSound(const QString& soundName);

private:
    BoardOwner*         boardOwner_;

    Piece*              _dragPiece;
    Piece*              _latestPiece;
    Piece*              _checkedKing;

    QPoint              _dragHotSpot;
    QLabel*             _dragHighlight;
    QLabel*             _latestHighlight; // ... of the last Piece that moved.
    QLabel*             _infoLabel;       // for Game-Over or Replay status

    QList<Piece*>       _pieces;  // All pieces ("live" and "dead")
    Referee*            _referee;
    bool                _gameOver;

    QList<ReplayMove*>  _moves;    // MOVE history
    int                 _nthMove;  // The pivot for Move Replay

    QHash<QString, QSound*> _sounds;
};

#endif // BOARD_H
