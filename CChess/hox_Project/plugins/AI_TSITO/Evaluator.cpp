#include	"Evaluator.h"
#include	"Lawyer.h"
#include	"Board.h"
#include	"Move.h"
#include	<list>

#define CHECKMATE -2000

using namespace std;
/*
 * Evaluator.cpp (c) Noah Roberts 2003-02-26
 */

static int pieceValues[] = 
  {
    0,		// NULL Piece
    10, 	// Pawn
    45,		// Canon
    90,		// Cart
    40,		// Horse
    20,		// Elephant
    20,		// Guard
    1500	// King - 1000 Means that if the score > 1000 or score < -1000 the king has
                //        been taken.  This leaves room for all other pieces as well as other
                //        considerations.
  };


// Positional scores - taken straight out of VSCCP by Pham Hong Nguyen.  I don't know enough
// about chinese chess to build my own tables yet.  Trusting these table because this guy
// has been around for a while and knows more than I do.
static const int pieceValuesByLoc[8][2][BOARD_AREA] =
  {
    {
      { // blue non-piece
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
      },
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
      }
    },
    {
      { // Blue pawns...
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
        10,  0, 12,  0, 15,  0, 12,  0, 10,
        10,  0, 13,  0, 10,  0, 13,  0, 10,
        20, 20, 20, 20, 20, 20, 20, 20, 20,
        20, 21, 21, 22, 22, 22, 21, 21, 20,
        20, 21, 21, 23, 23, 23, 21, 21, 20,
        20, 21, 21, 23, 22, 23, 21, 21, 20,
        11, 12, 13, 14, 14, 14, 13, 12, 11
      },
      { // Red pawns...
        11, 12, 13, 14, 14, 14, 13, 12, 11,
        20, 21, 21, 23, 22, 23, 21, 21, 20,
        20, 21, 21, 23, 23, 23, 21, 21, 20,
        20, 21, 21, 22, 22, 22, 21, 21, 20,
        20, 20, 20, 20, 20, 20, 20, 20, 20,
        10,  0, 13,  0, 10,  0, 13,  0, 10,
        10,  0, 12,  0, 15,  0, 12,  0, 10,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
      }
    },
    {
      { // Blue canon...
        50, 50, 50, 50, 50, 50, 50, 50, 50, 
        50, 50, 50, 50, 50, 50, 50, 50, 50,
        50, 51, 53, 53, 55, 53, 53, 51, 50,
        50, 51, 50, 50, 50, 50, 50, 51, 50,
        50, 51, 51, 51, 51, 51, 51, 51, 50,
        50, 51, 50, 50, 50, 50, 50, 51, 50,
        50, 51, 50, 50, 50, 50, 50, 51, 50,
        50, 51, 50, 50, 50, 50, 50, 51, 50,
        50, 51, 50, 50, 50, 50, 50, 51, 50,
        50, 50, 50, 50, 50, 50, 50, 50, 50
      },
      { // Red canon...
        50, 50, 50, 50, 50, 50, 50, 50, 50,                 
        50, 51, 50, 50, 50, 50, 50, 51, 50,
        50, 51, 50, 50, 50, 50, 50, 51, 50,
        50, 51, 50, 50, 50, 50, 50, 51, 50,
        50, 51, 50, 50, 50, 50, 50, 51, 50,
        50, 51, 51, 51, 51, 51, 51, 51, 50,
        50, 51, 50, 50, 50, 50, 50, 51, 50,
        50, 51, 53, 53, 55, 53, 53, 51, 50,
        50, 50, 50, 50, 50, 50, 50, 50, 50,
        50, 50, 50, 50, 50, 50, 50, 50, 50
      }
    },
    {
      { // Blue cart...
        89, 92, 90, 90, 90, 90, 90, 92, 89,
        91, 92, 90, 93, 90, 93, 90, 92, 91,
        90, 92, 90, 91, 90, 91, 90, 92, 90,
        90, 91, 90, 91, 90, 91, 90, 91, 90,
        90, 94, 90, 94, 90, 94, 90, 94, 90,
        90, 93, 90, 91, 90, 91, 90, 93, 90,
        90, 91, 90, 91, 90, 91, 90, 91, 90,
        90, 91, 90, 90, 90, 90, 90, 91, 90,
        90, 92, 91, 91, 90, 91, 91, 92, 90,
        90, 90, 90, 90, 90, 90, 90, 90, 90
      },
      { // Red cart...
        90, 90, 90, 90, 90, 90, 90, 90, 90,
        90, 92, 91, 91, 90, 91, 91, 92, 90,
        90, 91, 90, 90, 90, 90, 90, 91, 90,
        90, 91, 90, 91, 90, 91, 90, 91, 90,
        90, 93, 90, 91, 90, 91, 90, 93, 90,
        90, 94, 90, 94, 90, 94, 90, 94, 90,
        90, 91, 90, 91, 90, 91, 90, 91, 90,
        90, 92, 90, 91, 90, 91, 90, 92, 90,
        91, 92, 90, 93, 90, 93, 90, 92, 91,
        89, 92, 90, 90, 90, 90, 90, 92, 89
      }
    },
    {
      { // Blue Knight...
        40, 35, 40, 40, 40, 40, 40, 35, 40,
        40, 41, 42, 40, 20, 40, 42, 41, 40,
        40, 42, 43, 40, 40, 40, 43, 42, 40,
        40, 42, 43, 43, 43, 43, 43, 42, 40,
        40, 43, 44, 44, 44, 44, 44, 43, 40,
        40, 43, 44, 44, 44, 44, 44, 43, 40,
        40, 43, 44, 44, 44, 44, 44, 43, 40,
        40, 43, 44, 44, 44, 44, 44, 43, 40,
        40, 41, 42, 42, 42, 42, 42, 41, 40,
        40, 40, 40, 40, 40, 40, 40, 40, 40
      },
      { // Red Knight
        40, 40, 40, 40, 40, 40, 40, 40, 40,
        40, 41, 42, 42, 42, 42, 42, 41, 40,
        40, 43, 44, 44, 44, 44, 44, 43, 40,
        40, 43, 44, 44, 44, 44, 44, 43, 40,
        40, 43, 44, 44, 44, 44, 44, 43, 40,
        40, 43, 44, 44, 44, 44, 44, 43, 40,
        40, 42, 43, 43, 43, 43, 43, 42, 40,
        40, 42, 43, 40, 40, 40, 43, 42, 40,
        40, 41, 42, 40, 20, 40, 42, 41, 40,
        40, 35, 40, 40, 40, 40, 40, 35, 40
      }
    },
    {
      { // Blue elephant...
         0,  0, 25,  0,  0,  0, 25,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
        23,  0,  0,  0, 28,  0,  0,  0, 23,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0, 22,  0,  0,  0, 22,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0
      },
      { // Red elephant...
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0, 22,  0,  0,  0, 22,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
        23,  0,  0,  0, 28,  0,  0,  0, 23,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0, 25,  0,  0,  0, 25,  0,  0
      }
    },
    {
      { // Blue guard...
        0,  0,  0, 20,  0, 20,  0,  0,  0,
        0,  0,  0,  0, 22,  0,  0,  0,  0,
        0,  0,  0, 19,  0, 19,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0
      },
      { // Red guard...
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0, 19,  0, 19,  0,  0,  0,
        0,  0,  0,  0, 22,  0,  0,  0,  0,
        0,  0,  0, 20,  0, 20,  0,  0,  0
      }
    },
    {
      { // blue king...
        0,  0,  0, 30, 35, 30,  0,  0,  0,
        0,  0,  0, 15, 15, 15,  0,  0,  0,
        0,  0,  0,  1,  1,  1,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0
      },
      { // red king...
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  1,  1,  1,  0,  0,  0,
        0,  0,  0, 15, 15, 15,  0,  0,  0,
        0,  0,  0, 30, 35, 30,  0,  0,  0
      }
    }
  };
        
        


Evaluator Evaluator::theEvaluator;
long Evaluator::evaluatePosition(Board &theBoard, Lawyer &lawyer)
{
  color frend = theBoard.sideToMove();
  color enemy  = frend == RED ? BLUE:RED;
  long total = 0;
  for (int i = 1; i < 8; ++i) // Iterate through piece types...
    {
      // Gather each side's pieces.
      vector<int> friendly   = theBoard.pieces(frend, (piece)i);
      vector<int> unfriendly = theBoard.pieces(enemy, (piece)i);

      // Add piece values to score based on position on board.
      for (vector<int>::iterator it = friendly.begin(); it != friendly.end(); it++)
        total += pieceValuesByLoc[i][0][*it]; // add values of friendly pieces
      for (vector<int>::iterator it = unfriendly.begin(); it != unfriendly.end(); it++)
        total -= pieceValuesByLoc[i][1][*it]; // subtract values of enemy pieces.
    }

  // If one side is in check it is a more valuable position.
  if      (lawyer.inCheck(enemy))  total += 100;
  else if (lawyer.inCheck(frend))  total -= 100;

  return total;
}

long Evaluator::evaluateMaterial(Board &theBoard)
{
  color frend = theBoard.sideToMove();
  color enemy  = frend == RED ? BLUE:RED;
  long total = 0;
  for (int i = 1; i < 8; i++) // Iterate through piece types...
    {
      // Gather each side's pieces.
      vector<int> friendly   = theBoard.pieces(frend, (piece)i);
      vector<int> unfriendly = theBoard.pieces(enemy, (piece)i);

      total += friendly.size() * pieceValues[i];
      total -= unfriendly.size() * pieceValues[i];
    }
  return total;
}
int Evaluator::pieceValue(int piece)
{
  if (piece < 0 || piece > 7)
    cerr << (int)piece << endl;
  return pieceValues[piece];
}
