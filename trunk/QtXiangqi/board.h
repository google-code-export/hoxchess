#ifndef BOARD_H
#define BOARD_H

#include <QtGui>
#include "enums.h"

class Piece;
class Position;
class Referee;
class AIEngine;
class ReplayMove;

class Board : public QWidget
{
    Q_OBJECT
public:
    Board(QWidget *parent = 0);
    virtual ~Board();

    void doReplay_PREVIOUS();
    void doReplay_BEGIN();
    void doReplay_NEXT();
    void doReplay_END();

    void resetBoard();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void paintEvent(QPaintEvent *event);

private slots:
    void _handleAIMoveGenerated();
    void _askAIToGenerateMove();

private:
    void _onNewMove(Position from, Position to, bool setupMode = false);
    void _updateUIOnNewMove(ReplayMove* pMove, bool animated);
    void _playSoundAfterMove(const ReplayMove *pMove);
    void _onLocalMoveMadeFrom(const Position from, const Position to);    

    void _setPiecePosition(Piece* piece, Position newPosition);
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
    Position _pointToPosition(const QPoint& p) const;
    QPoint _positionToPieceOrigin(const Position position) const;

    Piece* _findPieceAt(const Position& position, bool  includeInactive = false) const;
    Piece* _getKingOfColor(ColorEnum color) const;
    void _drawPiece(Piece* piece);

    bool _isMoveLegalFrom(Position from, Position to) const;

    bool _isInReplay() const;

    void _initAIEngine();
    void _initSoundSystem();
    int _runAIToGenerateMove();
    void _playSound(const QString& soundName);

private:
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

    AIEngine*           _aiEngine;
    QFutureWatcher<int> _aiWatcher;

    QHash<QString, QSound*> _sounds;
};

#endif // BOARD_H
