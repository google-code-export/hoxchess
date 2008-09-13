#ifndef __MOVE_H__
#define __MOVE_H__

/*
 * Move.h (c) Noah Roberts 2003-02-23
 * Class Move: holds all data necisary to represent a single move in XiangQi.
 */

#include <iostream>
#include <string>

class Move
{
private:
    int            _origin;
    int            _destination;
    unsigned char  _capturedPiece;

    static Move    _null;

 public:
  Move() { _origin = _destination = _capturedPiece = 0;}
  // Build from ints.
  Move(int o, int d, unsigned char cP = 0) :
      _origin(o), _destination(d), _capturedPiece(cP){}
  // Build from string
  Move(std::string moveText);
  bool isCapture() { return _capturedPiece != 0; } 
  int origin() { return _origin; }
  int destination() { return _destination; }
  unsigned char capturedPiece() { return _capturedPiece; }

  static Move& nullMove() { return _null; } // The null move.

  void origin(int o) { _origin = o; }
  void destination(int d) { _destination = d; }
  void capturedPiece(unsigned char cP) { _capturedPiece = cP; }
  std::string getText() const;
  friend std::ostream& operator<<(std::ostream& out, Move &move);
  bool operator==(Move &m2) { return (_origin == m2._origin && _destination == m2._destination); }

  friend bool compareMoves(Move& m1, Move &m2); // for move sorting.
};

#endif /* __MOVE_H__ */
