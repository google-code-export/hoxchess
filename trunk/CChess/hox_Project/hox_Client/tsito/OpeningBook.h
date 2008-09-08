#ifndef	__OPENINGBOOK_H__
#define	__OPENINGBOOK_H__

/*
 * OpeningBook.h (c) Noah Roberts 2003-03-24
 * In charge of giving the engine some hints on what to play in the beginning of the game.
 * Provides adiquate opening play since the engine is unable to strategize.  Also gives play
 * that is not redundant - ie it won't always respond with the same move every damn time.
 */

class Board;
class Move;

#include	<map>
#include	<vector>
#include	<string>
#include	<cstdlib>
#include	<ctime>

//typedef unsigned int u_int32;
typedef unsigned short u_int16;

typedef std::vector< u_int16 > BookEntry;
typedef std::map< std::string, BookEntry > Book;

class OpeningBook
{
  Book	*bookContents;
  bool  validBook;
  void read(std::string filename);
  u_int16 selectMove(BookEntry entry);
 public:
  OpeningBook(std::string filename) { bookContents = new Book; read(filename); srand( (unsigned int)time(NULL) ); }
  ~OpeningBook() { delete bookContents; }
  u_int16 getMove(Board *board);

  bool operator!() { return validBook; }
  bool valid() { return validBook; }
};

#endif	/* __OPENINGBOOK_H__ */
