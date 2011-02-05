#include "board.h"
#include "piece.h"
#include "Referee/Referee.h"
#include "tableui.h"   // For BoardOwner 's definition

static const int borderX = 40;
static const int borderY = 40;
static const int cellS = 60;
static const float pieceS = 45.0;
static const float animateS = 55.0;

////////////////////////////////////////////////////////////////////
//
// Move replay holder unit
//
////////////////////////////////////////////////////////////////////

class ReplayMove
{
public:
    int    move;
    Piece* srcPiece;
    Piece* capturedPiece;
    Piece* checkedKing; // The King that is checked after this Move.

    ReplayMove(int mv) : move(mv) { srcPiece = capturedPiece = checkedKing = 0; }
};

enum HistoryIndex // NOTE: Do not change the constants 'values below.
{
    HISTORY_INDEX_END   = -2,
    HISTORY_INDEX_BEGIN = -1
};

// NOTE: These tags are needed so that the '_infoLabel' label can be shared
//       more effectively.
enum InfoLabelTag
{
    INFO_LABEL_TAG_NONE       = 0,
    INFO_LABEL_TAG_GAME_OVER  = 1,  // Need to be non-zero.
    INFO_LABEL_TAG_REPLAY     = 2
};

// ----------------------------------------------------------------------------
Board::Board(QWidget* parent, BoardOwner* boardOwner)
    : QWidget(parent)
    , boardOwner_(boardOwner)
    , _dragPiece(0)
    , _latestPiece(0)
    , _checkedKing(0)
    , _dragHighlight(0)
    , _latestHighlight(0)
    , _infoLabel(0)
    , _referee(new Referee)
    , _gameOver(false)
    , _nthMove(HISTORY_INDEX_END)
{
    setMinimumSize(560, 620);
    setMaximumSize(minimumSize());

    _setupPieces();

    // The highlight "target" image while dragging a Piece.
    _dragHighlight = new QLabel(this);
    _dragHighlight->setPixmap(QPixmap(":/pieces/highlight_50x50.png"));
    _dragHighlight->hide();

    // The highlight "latest" image of the last Piece that moved.
    _latestHighlight = new QLabel(this);
    _latestHighlight->setPixmap(QPixmap(":/pieces/animate_55x55.png"));
    _latestHighlight->hide();

    _infoLabel = new QLabel(this);
    _infoLabel->setFont(QFont("Helvetica", 30));
    _infoLabel->hide();
    _infoLabel->setProperty("tag", INFO_LABEL_TAG_NONE);

    _referee->initGame();
    _initSoundSystem();
}

Board::~Board()
{
    delete _referee;
    while (!_moves.isEmpty()) { delete _moves.takeFirst(); }
}

void Board::resetBoard()
{
    _clearAllHighlight();
    _clearAllAnimation();

    foreach (Piece* piece, _pieces) {
        piece->resetToInitialPosition();
        piece->move(borderX + piece->col()*cellS - pieceS/2,
                    borderY + piece->row()*cellS - pieceS/2);
        piece->show();
    }

    _infoLabel->hide();
    _infoLabel->setProperty("tag", INFO_LABEL_TAG_NONE);

    _gameOver = false;

    _referee->initGame();
    _moves.clear();
    _nthMove = HISTORY_INDEX_END;
}

void Board::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);

    painter.fillRect(event->rect(), Qt::white);

    const QRect targetRect(rect());  // (0, 0, 560, 620)
    painter.drawPixmap(targetRect, QPixmap(":/images/hoxchess.png"));

    painter.end();
}

void Board::mousePressEvent(QMouseEvent *event)
{
    Piece* piece = static_cast<Piece*>(childAt(event->pos()));
    if (!piece) return;

    if (    isInReplay_()
        || _referee->gameStatus() != HC_GAME_STATUS_IN_PROGRESS
        || !boardOwner_->isMyTurnNext()
        || !boardOwner_->isGameReady() )
    {
        return;
    }

    _playSound("CLICK");
    _dragPiece = piece;
    _dragPiece->raise();
    _dragHotSpot = event->pos() - piece->pos();
}

void Board::mouseMoveEvent(QMouseEvent *event)
{
    if (!_dragPiece) return;
    const QPoint newPoint = event->pos() - _dragHotSpot;
    const hox::Position newPosition = _pointToPosition(newPoint);
    if (newPosition.isValid()) {
        const QPoint newOrigin = _positionToPieceOrigin(newPosition);
        _dragHighlight->move(newOrigin);
        _dragHighlight->show();
    } else {
        _dragHighlight->hide();
    }
    _dragPiece->move(newPoint);
}

void Board::mouseReleaseEvent(QMouseEvent *event)
{
     if (!_dragPiece) return;
     const QPoint newPoint = event->pos() - _dragHotSpot;
     hox::Position newPosition = _pointToPosition(newPoint);
     if (   newPosition.isValid()
         && _isMoveLegalFrom(_dragPiece->position(), newPosition))
     {
         const hox::Position from = _dragPiece->position();
         onNewMove(from, newPosition);

         if (_referee->gameStatus() == HC_GAME_STATUS_IN_PROGRESS)
         {
             boardOwner_->onLocalMoveMadeFrom(from, newPosition);
         }
     } else {
         _drawPiece(_dragPiece); // Move back to the original position.
     }

     _dragPiece = 0;
     _dragHighlight->hide();
}

void Board::onNewMove(hox::Position from, hox::Position to, bool setupMode /* = false */)
{
    int sqSrc = TOSQUARE(from.row, from.col);
    int sqDst = TOSQUARE(to.row, to.col);
    int move = MOVE(sqSrc, sqDst);

    _referee->makeMove(move);

    ReplayMove* pMove = new ReplayMove(move);
    _moves.append(pMove);

    // Delay update the UI if in Replay mode.
    // NOTE: We do not update pMove.srcPiece (leaving it equal to 0)
    //       to signal that it is NOT yet processed.
    if (isInReplay_()) {
        return;
    }

    // Fully update the Move's information.
    pMove->srcPiece = _findPieceAt(from);
    pMove->capturedPiece = _findPieceAt(to);

    if (_referee->isChecked()) {
        ColorEnum checkedColor = (pMove->srcPiece->color() == HC_COLOR_RED
                                  ? HC_COLOR_BLACK : HC_COLOR_RED);
        pMove->checkedKing = _getKingOfColor(checkedColor);
    }

    // Finally, update the Board's UI accordingly.
    _updateUIOnNewMove(pMove, !setupMode /* animated */);
}

void Board::_updateUIOnNewMove(ReplayMove* pMove, bool animated)
{
    int sqDst = DST(pMove->move);
    hox::Position to( ROW(sqDst), COLUMN(sqDst) );

    _setPiecePosition(pMove->srcPiece, to);

    if (pMove->capturedPiece) {
        pMove->capturedPiece->hide();
    }

    if (animated) {
        _playSoundAfterMove(pMove);
        _animateLatestMove(pMove);
    }

    if (   _referee->gameStatus() != HC_GAME_STATUS_IN_PROGRESS
        && (!isInReplay_() || _nthMove == _moves.size()-1))
    {
        _onGameOver();
    }
}

void Board::_playSoundAfterMove(const ReplayMove *pMove)
{
    const GameStatusEnum result     = _referee->gameStatus();
    const ColorEnum      ownerColor = HC_COLOR_RED; // TODO: _boardOwner.ownerColor;
    const ColorEnum      moveColor  = pMove->srcPiece->color();
    QString              sound;

    if (   result != HC_GAME_STATUS_IN_PROGRESS // NOTE: just for optimization!
        && (!isInReplay_() || _nthMove == _moves.size()-1))
    {
        if (   (result == HC_GAME_STATUS_RED_WIN && ownerColor == HC_COLOR_RED)
            || (result == HC_GAME_STATUS_BLACK_WIN && ownerColor == HC_COLOR_BLACK))
        {
            sound = "WIN";
        }
        else if (  (result == HC_GAME_STATUS_RED_WIN && ownerColor == HC_COLOR_BLACK)
                 || (result == HC_GAME_STATUS_BLACK_WIN && ownerColor == HC_COLOR_RED))
        {
            sound = "LOSS";
        }
        else if (result == HC_GAME_STATUS_DRAWN) {
            sound = "DRAW";
        }
        else if (result == HC_GAME_STATUS_TOO_MANY_MOVES) {
            sound = "ILLEGAL";
        }
    }
    else if (pMove->checkedKing) {
        sound = (moveColor == HC_COLOR_RED ? "Check1" : "CHECK2");
    }
    else if (pMove->capturedPiece) {
        sound = (moveColor == HC_COLOR_RED ? "CAPTURE" : "CAPTURE2");
    }
    else {
        sound= (moveColor == HC_COLOR_RED ? "MOVE" : "MOVE2");
    }

    _playSound(sound);
}

bool Board::_doReplayPREV(bool animated)
{
    if (_moves.empty() || _nthMove == HISTORY_INDEX_BEGIN) {
        return false;
    }

    if (_nthMove == HISTORY_INDEX_END ) {
        _nthMove = _moves.size() - 1; // Get the latest move.
    }

    ReplayMove* pMove = _moves.at(_nthMove);
    int move = pMove->move;
    int sqSrc = SRC(move);
    if (animated) _playSound("Replay");

    // For Move-Replay, just reverse the move order (sqDst->sqSrc)
    // Since it's only a replay, no need to make actual move in
    // the underlying game logic.

    _clearAllAnimation();
    hox::Position oldPosition( ROW(sqSrc), COLUMN(sqSrc) );

    _setPiecePosition(pMove->srcPiece, oldPosition);
    if (pMove->capturedPiece) {
        pMove->capturedPiece->show();
    }

    // Highlight the Piece (if any) of the "next-PREV" Move.
    --_nthMove;
    if (_nthMove >= 0) {
        pMove = _moves.at(_nthMove);
        if (animated) _animateLatestMove(pMove);
    }
    return true;
}

void Board::doReplay_PREVIOUS()
{
    if (!isInReplay_()) {
        _clearAllHighlight();
    }

    _doReplayPREV(true /* animation */);
    _setReplayMode( isInReplay_() );
}

void Board::doReplay_BEGIN()
{
    if (!isInReplay_()) {
        _clearAllHighlight();
    }

    if (_moves.empty() || _nthMove == HISTORY_INDEX_BEGIN) {
        return;
    }
    _playSound("Replay");

    while (_doReplayPREV(false /* no animation */)) { /* ... until no more move */ }
    _setReplayMode( isInReplay_() );
}

bool Board::_doReplayNEXT(bool animated)
{
    if (_moves.empty() || _nthMove == HISTORY_INDEX_END) {
        return false;
    }

    ++_nthMove;
    Q_ASSERT_X(_nthMove >= 0 && _nthMove < _moves.size(), "Replay NEXT", "Invalid index");

    ReplayMove* pMove = _moves.at(_nthMove);
    int move = pMove->move;

    if (!pMove->srcPiece) // not yet processed?
    {                    // ... then we process it as a NEW move.
        qDebug("%s: Process pending move [%d]...", __FUNCTION__, move);
        int sqSrc = SRC(move);
        int sqDst = DST(move);
        hox::Position from( ROW(sqSrc), COLUMN(sqSrc));
        hox::Position to( ROW(sqDst), COLUMN(sqDst));
        pMove->srcPiece = _findPieceAt(from);
        pMove->capturedPiece = _findPieceAt(to);
        if (_referee->isChecked()) {
            ColorEnum checkedColor = (pMove->srcPiece->color() == HC_COLOR_RED
                                      ? HC_COLOR_BLACK : HC_COLOR_RED);
            pMove->checkedKing = _getKingOfColor(checkedColor);
        }
        _updateUIOnNewMove(pMove, true /* animated */);
    }
    else
    {
        _clearAllAnimation();
        _updateUIOnNewMove(pMove, animated);
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // NOTE: We delay updating the index to the "END" mark to avoid race
    //       conditions that could occur when there is a NEW move.
    //       The "END" mark is a signal that allows the main UI Thread to
    //       process new incoming Moves immediately.
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if (_nthMove == _moves.size() - 1) {
        _nthMove = HISTORY_INDEX_END;
    }
    return true;
}

void Board::doReplay_NEXT()
{
    _doReplayNEXT(true /* animation */);
    _setReplayMode( isInReplay_() );
}

void Board::doReplay_END()
{
    if (_moves.empty() || _nthMove == HISTORY_INDEX_END) {
        return;
    }
    const int lastMoveIndex = _moves.size() - 2;
    while (_nthMove < lastMoveIndex) {
        _doReplayNEXT(false /* no animation */);
    }
    _doReplayNEXT(true /* animation */);
    _setReplayMode( isInReplay_() );
}

void Board::_setPiecePosition(Piece* piece, hox::Position newPosition)
{
    piece->setPosition(newPosition.row, newPosition.col);

    const QPoint newOrigin = _positionToPieceOrigin(piece->position());
    piece->move(newOrigin);
    //piece->show();
}

void Board::_animateLatestMove(ReplayMove* pMove)
{
    _latestPiece = pMove->srcPiece;
    //_latestPiece.highlightState = HC_HL_ANIMATED;
    QPoint latestPoint(borderX + _latestPiece->col() * cellS - animateS/2,
                       borderY + _latestPiece->row() * cellS - animateS/2);
    _latestHighlight->move(latestPoint);
    _latestHighlight->show();

    if (pMove->checkedKing) {
        _checkedKing = pMove->checkedKing;
        //_checkedKing.highlightState = HC_HL_CHECKED;
    }
}

void Board::_clearAllHighlight()
{
    // TODO: empty for now.
}

void Board::_clearAllAnimation()
{
    if (_latestPiece) {
        //_latestPiece.highlightState = HC_HL_NONE;
        _latestHighlight->hide();
        _latestPiece = 0;
    }
    if (_checkedKing) {
        //_checkedKing.highlightState = HC_HL_NONE;
        _checkedKing = 0;
    }
}

void Board::_onGameOver()
{
    if (_infoLabel->property("tag") != INFO_LABEL_TAG_GAME_OVER) {
        qDebug("%s: *** Set GAME-OVER.", __FUNCTION__);
        _setInfoLabel(tr("Game Over"));
        _infoLabel->setProperty("tag", INFO_LABEL_TAG_GAME_OVER);
    }
    _gameOver = true;
}

void Board::_setReplayMode(bool on)
{
    if (on) {
        QString infoText = QString("%1 %2/%3").arg(tr("Replay")).arg(_nthMove+1).arg(_moves.size());
        if (_infoLabel->property("tag") != INFO_LABEL_TAG_REPLAY) {
            qDebug("%s: *** Set REPLAY MODE.", __FUNCTION__);
            _infoLabel->setProperty("tag", INFO_LABEL_TAG_REPLAY);
        }
        _setInfoLabel(infoText);
    } else if (_referee->gameStatus() != HC_GAME_STATUS_IN_PROGRESS) {
        _onGameOver();
    } else {
        _infoLabel->hide();
    }
}

void Board::_setInfoLabel(const QString& infoText)
{
    QFontMetrics fm(_infoLabel->fontMetrics());
    _infoLabel->setText( QString("<font color='blue'>%1</font>").arg(infoText));
    QPoint infoPoint(borderX + (8*cellS - fm.width(infoText))/2,
                     borderY + 4 * cellS + fm.height()/2);
    _infoLabel->move(infoPoint);
    _infoLabel->setMinimumWidth(fm.width(infoText));
    _infoLabel->show();
}

void Board::_initSoundSystem()
{
    QList<QString> soundList;
    soundList << "CLICK" << "MOVE" << "MOVE2" << "ILLEGAL"
              << "CAPTURE" << "CAPTURE2" << "Check1" << "CHECK2"
              << "WIN" << "DRAW" << "LOSS" << "Replay" << "ChangeRole" << "Undo";
    QDir appDir(QApplication::applicationDirPath());
    QString soundPath = appDir.absoluteFilePath("../Resources/sounds/");
    foreach (QString soundName, soundList) {
        _sounds[soundName] = new QSound(soundPath + soundName + ".WAV");
    }
}

void Board::_setupPieces()
{
    _createPiece(HC_PIECE_CHARIOT, HC_COLOR_BLACK, 0, 0);
    _createPiece(HC_PIECE_CHARIOT, HC_COLOR_BLACK, 0, 8);
    _createPiece(HC_PIECE_CHARIOT, HC_COLOR_RED, 9, 0);
    _createPiece(HC_PIECE_CHARIOT, HC_COLOR_RED, 9, 8);

    _createPiece(HC_PIECE_HORSE, HC_COLOR_BLACK, 0, 1);
    _createPiece(HC_PIECE_HORSE, HC_COLOR_BLACK, 0, 7);
    _createPiece(HC_PIECE_HORSE, HC_COLOR_RED, 9, 1);
    _createPiece(HC_PIECE_HORSE, HC_COLOR_RED, 9, 7);

    _createPiece(HC_PIECE_ELEPHANT, HC_COLOR_BLACK, 0, 2);
    _createPiece(HC_PIECE_ELEPHANT, HC_COLOR_BLACK, 0, 6);
    _createPiece(HC_PIECE_ELEPHANT, HC_COLOR_RED, 9, 2);
    _createPiece(HC_PIECE_ELEPHANT, HC_COLOR_RED, 9, 6);

    _createPiece(HC_PIECE_ADVISOR, HC_COLOR_BLACK, 0, 3);
    _createPiece(HC_PIECE_ADVISOR, HC_COLOR_BLACK, 0, 5);
    _createPiece(HC_PIECE_ADVISOR, HC_COLOR_RED, 9, 3);
    _createPiece(HC_PIECE_ADVISOR, HC_COLOR_RED, 9, 5);

    _createPiece(HC_PIECE_KING, HC_COLOR_BLACK, 0, 4);
    _createPiece(HC_PIECE_KING, HC_COLOR_RED, 9, 4);

    _createPiece(HC_PIECE_CANNON, HC_COLOR_BLACK, 2, 1);
    _createPiece(HC_PIECE_CANNON, HC_COLOR_BLACK, 2, 7);
    _createPiece(HC_PIECE_CANNON, HC_COLOR_RED, 7, 1);
    _createPiece(HC_PIECE_CANNON, HC_COLOR_RED, 7, 7);

    _createPiece(HC_PIECE_PAWN, HC_COLOR_BLACK, 3, 0);
    _createPiece(HC_PIECE_PAWN, HC_COLOR_BLACK, 3, 2);
    _createPiece(HC_PIECE_PAWN, HC_COLOR_BLACK, 3, 4);
    _createPiece(HC_PIECE_PAWN, HC_COLOR_BLACK, 3, 6);
    _createPiece(HC_PIECE_PAWN, HC_COLOR_BLACK, 3, 8);
    _createPiece(HC_PIECE_PAWN, HC_COLOR_RED, 6, 0);
    _createPiece(HC_PIECE_PAWN, HC_COLOR_RED, 6, 2);
    _createPiece(HC_PIECE_PAWN, HC_COLOR_RED, 6, 4);
    _createPiece(HC_PIECE_PAWN, HC_COLOR_RED, 6, 6);
    _createPiece(HC_PIECE_PAWN, HC_COLOR_RED, 6, 8);
}

void Board::_createPiece(PieceEnum type, ColorEnum color, int row, int column)
{
    Piece* piece = new Piece(type, color, row, column, this);
    piece->move(borderX + column*cellS - pieceS/2, borderY + row*cellS - pieceS/2);
    _pieces.push_back(piece);
}

hox::Position Board::_pointToPosition(const QPoint& p) const
{
    hox::Position position;

    // We will work on the center.
    QPoint point(p.x() + pieceS/2, p.y() + pieceS/2);

    /* Get the 4 surrounding positions.
     *
     *    1 ------------ 2
     *    |      ^       |
     *    |      |       |
     *    |  <-- X -->   |
     *    |      |       |
     *    |      V       |
     *    4 ------------ 3
     */

    const QPoint p1( point.x() - ((point.x() - borderX) % cellS),
                     point.y() - ((point.y() - borderY) % cellS) );
    const QPoint p2( p1.x() + cellS, p1.y() );
    const QPoint p3( p2.x(), p2.y() + cellS );
    const QPoint p4( p1.x(), p3.y() );

    const int tolerance = cellS / 2;
    const QRect r1(p1.x() - tolerance, p1.y() - tolerance, tolerance*2, tolerance*2);
    const QRect r2(p2.x() - tolerance, p2.y() - tolerance, tolerance*2, tolerance*2);
    const QRect r3(p3.x() - tolerance, p3.y() - tolerance, tolerance*2, tolerance*2);
    const QRect r4(p4.x() - tolerance, p4.y() - tolerance, tolerance*2, tolerance*2);

    if ( r1.contains(point) ) {
        position.col = (p1.x() - borderX) / cellS;
        position.row = (p1.y() - borderY) / cellS;
    }
    else if ( r2.contains(point) ) {
        position.col = (p2.x() - borderX) / cellS;
        position.row = (p2.y() - borderY) / cellS;
    }
    else if ( r3.contains(point) ) {
        position.col = (p3.x() - borderX) / cellS;
        position.row = (p3.y() - borderY) / cellS;
    }
    else if ( r4.contains(point) ) {
        position.col = (p4.x() - borderX) / cellS;
        position.row = (p4.y() - borderY) / cellS;
    }

    return position;
}

QPoint Board::_positionToPieceOrigin(const hox::Position position) const
{
    // Determine the piece's top-left point.
    return QPoint(borderX + position.col*cellS - pieceS/2,
                  borderY + position.row*cellS - pieceS/2);
}

Piece* Board::_findPieceAt(const hox::Position& position, bool includeInactive) const
{
    foreach (Piece* piece, _pieces) {
        if ( !includeInactive && piece->isHidden() ) continue;
        if ( piece->position() == position ) return piece;
    }
    return 0;
}

Piece* Board::_getKingOfColor(ColorEnum color) const
{
    foreach (Piece* piece, _pieces) {
        if (piece->type() == HC_PIECE_KING && piece->color() == color) {
            return piece;
        }
    }
    return 0;
}

void Board::_drawPiece(Piece* piece)
{
    const QPoint newOrigin = _positionToPieceOrigin(piece->position());
    piece->move(newOrigin);
    piece->show();

    if (piece == _latestPiece) {
        QPoint latestPoint(borderX + piece->col()*cellS - animateS/2,
                           borderY + piece->row()*cellS - animateS/2);
        _latestHighlight->move(latestPoint);
        _latestHighlight->show();
    }
}

bool Board::_isMoveLegalFrom(hox::Position from, hox::Position to) const
{
    int sqSrc = TOSQUARE(from.row, from.col);
    int sqDst = TOSQUARE(to.row, to.col);
    return _referee->isLegalMove( MOVE(sqSrc, sqDst) );
}

bool Board::isInReplay_() const { return _nthMove != HISTORY_INDEX_END; }

void Board::_playSound(const QString& soundName)
{
    QSound* sound = _sounds[soundName];
    sound->play();
}
