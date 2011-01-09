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

    void clear() { nGame = nMove = nFree = 0; }

    bool empty() const
        { return (nGame == 0) && (nMove == 0) && (nFree == 0); }
};

/**
 * Table-info.
 */
class TableInfo
{
public:
    std::string   id;           // table-Id.
    std::string   redId;        // RED player's Id.
    std::string   blackId;      // BLACK player's Id.
    std::string   redRating;    // RED player's Rating.
    std::string   blackRating;  // BLACK player's Rating.
    TimeInfo      initialTime;  // The initial allowed Game-Time.
    TimeInfo      blackTime;
    TimeInfo      redTime;
    bool          rated;

    TableInfo( const std::string& tableId = "" )
            : rated( true )
        { id = tableId; }

    bool valid() const { return !id.empty(); }

    void clear()
    {
        id = "";
        redId = "";
        blackId = "";
        redRating = "";
        blackRating = "";
        initialTime.clear();
        blackTime.clear();
        redTime.clear();
        rated = true;
    }
};

} // namespace hox

#endif // TYPES_H
