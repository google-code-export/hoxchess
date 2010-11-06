#include "piece.h"
#include "board.h"

Piece::Piece(PieceEnum type, ColorEnum color, int row, int col, Board *board)
    : QLabel(board)
    , _type(type)
    , _color(color)
    , _row(row)
    , _col(col)
    , _initialRow(row)
    , _initialCol(col)
{
    const QString imageName = _getResource();
    this->setPixmap(QPixmap(imageName));
    this->setAttribute(Qt::WA_DeleteOnClose);
}

void Piece::resetToInitialPosition()
{
    _row = _initialRow;
    _col = _initialCol;
}

const QString Piece::_getResource()
{
    QString imageName;
    switch (_type)
    {
        case HC_PIECE_KING:     imageName = "king"; break;
        case HC_PIECE_ADVISOR:  imageName = "advisor"; break;
        case HC_PIECE_ELEPHANT: imageName = "elephant"; break;
        case HC_PIECE_CHARIOT:  imageName = "chariot"; break;
        case HC_PIECE_HORSE:    imageName = "horse"; break;
        case HC_PIECE_CANNON:   imageName = "cannon"; break;
        case HC_PIECE_PAWN:     imageName = "pawn"; break;
        default: break;
    };

    return QString(":/pieces/")  + (_color == HC_COLOR_RED ? 'r' : 'b') + imageName + ".png";
}
