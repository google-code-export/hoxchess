#ifndef	__ENGINE_H__
#define	__ENGINE_H__

/*
 * Engine.h (c) Noah Roberts 2003-02-26
 * The Engine class is the brain of it all, this is the class that ties the rest of
 * the classes together to actually perform the function of playing chess.  This class
 * creates and navigates the search tree and decides what move to make next.  This class
 * is also the one that will notice and respond when the game has ended.
 */


#include	<string>
#include	<vector>
#include	"Move.h"
#include	"Options.h"
#include	"Timer.h"

#include	<sys/timeb.h>

class Board;
class Evaluator;
class Lawyer;
class OpeningBook;
class TranspositionTable;
// HPHAN: class Interface;
//class Move;
enum { NO_CUTOFF, HASH_CUTOFF, NULL_CUTOFF, MATE_CUTOFF, MISC_CUTOFF };
struct PVEntry
{
  Move move;
  int cutoff;
  PVEntry(Move m, int c) : move(m),cutoff(c) {}
};

//#define	INFIN	0x0FFFFFFF;
#define		INFIN	3000
#define CHECKMATE (-2000)

class Engine : public OptionsObserver
{
 private:
  Board		*board;
  Evaluator	*evaluator;
  Lawyer	*lawyer;
  OpeningBook	*openingBook;
  // HPHAN: Interface	*iface;
  TranspositionTable *transposTable;

  // search result
  std::vector<PVEntry>	principleVariation;
  //bool			continueSearching;
  short			searchState;


  // Search information
  int			maxPly;  // also user configurable.
  std::vector<Move>	killer1;
  std::vector<Move>	killer2;
  std::vector<Move>	moveSortPriorityTable;
  int			nullMoveReductionFactor;
  short			searchAborted;

  Timer			*myTimer;


  // Search statistics
  int		nodeCount;
  int		hashHits;
  int		nullCutoffs;
  struct timeb	startTime;

  // User configurable options
  
  bool		useOpeningBook;
  bool		displayThinking;

  bool 		quiesc; // perform quiescence search or not
  bool		qnull;  // null cutoff in quiescence
  bool		qhash;  // use table in quiescence

  
  int		searchMethod; // Tells system which algorithm to use
  bool		allowNull;    // If false then null move will never be tried.
  bool		useMTDF;      // Use the MTD(f) algorithm.
  bool		useIterDeep;  // if yes then the search is tried at depths 1-maxPly.
  			      // otherwise just maxPly.
  bool		verifyNull;   // Use verified null-move pruning or standard.

  bool		allowTableWindowAdjustments; // If table contains a depth < our target the
                                             // norm is to adjust the window.  User may
                                             // disable that behaviour.
  bool		useTable;

  // Search functions
  long quiescence(long alpha, long beta, int ply, int depth, bool nullOk = true);
  void filterOutNonCaptures(std::list<Move> &l);

  // Search algs...

  //long alphaBeta(long alpha, long beta, int ply, int depth);
  long alphaBeta(std::vector<PVEntry> &pv, std::list<Move> moveList,
                 long alpha, long beta, int ply, int depth,
                 bool legalonly = false, bool nullOk = true, bool verify = true);
  //long pvSearch(std::vector<PVEntry> &pv, long alpha, long beta, int ply, int depth,
  //              bool legalonly = false, bool nullOk = true, bool verify = true);
  long negaScout(std::vector<PVEntry> &pv,  std::list<Move> moveList,
                 long alpha, long beta, int ply, int depth,
                 bool legalonly = false, bool nullOk = true, bool verify = true);

  // Generic search - calls specific search, will add heuristics here...
  long search(std::vector<PVEntry> &pv, long alpha, long beta, int ply, int depth,
              bool legalonly = false, bool nullOk = true, bool verify = true);

  // MTD(f) algorithm...calls search.
  long mtd(std::vector<PVEntry> &pv, long guess, int depth);

  // Support functions...
  // looks for position in table.
  bool tableSearch(int ply, int depth, long &alpha, long &beta, Move &m, long &score, bool &nullok);
  // stores position in table.
  void tableSet(int ply, int depth, long alpha, long beta, Move m, long score);
  // looks for killers and alters priority table
  void setUpKillers(int ply);
  // Adds killer move at ply
  void newKiller(Move& theMove, int ply);
 public:
  Engine(Board *brd, Lawyer *law /* HPHAN: , Interface *ifc */);

  // If board is replaced...
  void setBoard(Board *brd) { board = brd; }

  // Tells engine to think...
  long think();

  // Tells engine that something happened such that search must be stopped.
  void endSearch();
  bool doneThinking();
  bool thinking();

  // Search information retrieval...
  Move getMove();
  std::string variationText(std::vector<PVEntry> pv);

  // OptionObserver requirements
  void optionChanged(std::string whatOption);

  // In case compareMoves needs to access 
  friend bool compareMoves(Move& m1, Move &m2);
};

/*
 * Move ordering sort predicate used by list to sort moves.
 * Major hackery - would like to get rid of this.
 */
extern bool compareMoves(Move& m1, Move& m2);



#endif /* __ENGINE_H__ */
