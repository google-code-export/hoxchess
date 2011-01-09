#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <list>
#include <vector>
#include <boost/tokenizer.hpp>

namespace hox {

// ----------------------------------------------------------------- //
//                                                                   //
//                Typedefs                                           //
//                                                                   //
// ----------------------------------------------------------------- //

typedef std::list<std::string>                         StringList;
typedef std::vector<std::string>                       StringVector;
typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
typedef boost::char_separator<char>                    Separator;

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

/**
 * Game's Time-info.
 */
class TimeInfo
{
public:
    int  nGame;  // Game-time (in seconds).
    int  nMove;  // Move-time (in seconds).
    int  nFree;  // Free-time (in seconds).

    TimeInfo( int g = 0, int m = 0, int f = 0 )
        : nGame( g ), nMove( m ), nFree( f ) {}

    void Clear() { nGame = nMove = nFree = 0; }

    bool IsEmpty() const
        { return (nGame == 0) && (nMove == 0) && (nFree == 0); }
};

} // namespace hox

#endif // TYPES_H
