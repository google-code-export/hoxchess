#ifndef TYPES_H
#define TYPES_H

/**
 * Representing a piece's position.
 */
class Position
{
public:
    int row;
    int col;

    Position(int r = -1, int c = -1) : row(r), col(c) {}

    bool operator==(const Position& pos) const { return (row == pos.row && col == pos.col); }
    bool operator!=(const Position& pos) const { return !( *this == pos ); }

    bool isValid() const { return (row >= 0 && row <= 9 && col >= 0 && col <= 8); }
};

#endif // TYPES_H
