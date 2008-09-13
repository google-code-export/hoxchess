#include	"Move.h"
#include	<iostream>
#include	<string>

/*
 * Move.cpp (c) Noah Roberts 2003-02-27
 * << operator and string parser for Move class.
 */
using namespace std;

#define	ERROR_OUT { _origin = 0; _destination = 0; _capturedPiece = 0; return; }

Move Move::_null; // The null move.

ostream& operator<<(ostream& out, const Move &move)
{
  return out << move.getText();
}
string Move::getText() const // Translate into algebraic notation.
{
  string moveText;

  moveText += (char)(_origin % 9 + 'a');
  moveText += (char)(9 - (_origin / 9) + '0');
  moveText += (char)(_destination % 9 + 'a');
  moveText += (char)(9 - (_destination / 9) + '0');

  return moveText;
}
Move::Move(string moveText) // scan and create a move out of a string.  Depends on ASCII layout...
{
  string::iterator it = moveText.begin();

  // origin

  // file
  if (moveText.length() != 4) ERROR_OUT;
  if (*it >= 'a' && *it <= 'i') // between a and i
    _origin = *it++ - 'a';
  else if (*it >= 'A' && *it <= 'I') // capital version
    _origin = *it++ - 'A';
  else ERROR_OUT;
  // rank
  if (*it < '0' || *it > '9') ERROR_OUT;
  _origin += (9 - (*it++ - '0')) * 9;

  // destination

  // file
  if (*it >= 'a' && *it <= 'i')
    _destination = *it++ - 'a';
  else if (*it >= 'A' && *it <= 'I')
    _destination = *it++ - 'A';
  else ERROR_OUT;
  // rank
  if (*it < '0' || *it > '9') ERROR_OUT;
  _destination += (9 - (*it++ - '0')) * 9;
}
