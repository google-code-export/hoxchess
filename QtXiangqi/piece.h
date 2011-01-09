#ifndef PIECE_H
#define PIECE_H

#include <QLabel>
#include "enums.h"
#include "types.h"

class Board;

class Piece : public QLabel
{
public:
    Piece(PieceEnum type, ColorEnum color, int row, int col, Board *board);

    PieceEnum type() const { return _type; }
    ColorEnum color() const { return _color; }

    void setPosition(int row, int col) { _row = row; _col = col; }
    hox::Position position() const { return hox::Position(_row, _col); }
    int row() const { return _row; }
    int col() const { return _col; }
    void resetToInitialPosition();

private:
    const QString _getResource();

private:
    PieceEnum   _type;
    ColorEnum   _color;
    int         _row;
    int         _col;
    int         _initialRow;
    int         _initialCol;
};

#endif // PIECE_H
