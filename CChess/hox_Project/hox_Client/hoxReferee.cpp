/////////////////////////////////////////////////////////////////////////////
// Name:            hoxReferee.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         09/30/2007
//
// Description:     Implementing the standard Xiangqi referee.
/////////////////////////////////////////////////////////////////////////////

#include "hoxReferee.h"
#include <list>
#include <algorithm>  // std::find

//************************************************************
//
//     0     1    2    3    4    5    6    7    8
//
//      +--------------+==============+--------------+
//  0   |  0 |  1 |  2 #  3 |  4 |  5 #  6 |  7 |  8 |
//      |--------------#--------------#--------------|
//  1   |  9 | 10 | 11 # 12 | 13 | 14 # 15 | 16 | 17 | 
//      |--------------#--------------#--------------|
//  2   | 18 | 19 | 20 # 21 | 22 | 23 # 24 | 25 | 26 | 
//      |--------------+==============+--------------|
//  3   | 27 | 28 | 29 | 30 | 31 | 32 | 33 | 34 | 35 | 
//      |--------------------------------------------|
//  4   | 36 | 37 | 38 | 39 | 40 | 41 | 42 | 43 | 44 |
//      |============================================| <-- The River 
//  5   | 45 | 46 | 47 | 48 | 49 | 50 | 51 | 52 | 53 |
//      |--------------------------------------------|
//  6   | 54 | 55 | 56 | 57 | 58 | 59 | 60 | 61 | 62 | 
//      |--------------+==============+--------------|
//  7   | 63 | 64 | 65 # 66 | 67 | 68 # 69 | 70 | 71 | 
//      |--------------#--------------#--------------|
//  8   | 72 | 73 | 74 # 75 | 76 | 77 # 78 | 79 | 80 | 
//      |--------------#--------------#--------------|
//  9   | 81 | 82 | 83 # 84 | 85 | 86 # 87 | 88 | 89 | 
//      +--------------+==============+--------------+
//
//************************************************************

namespace BoardInfoAPI
{
    /* Forward declarations */

    class Board;
    class Piece;

    /* Typedefs */

    typedef std::list<hoxPosition *> PositionList;
    typedef std::list<Piece* >       PieceList;

    /* ----- */

    class Piece
    {
    public:
        Piece() : m_info( hoxPIECE_TYPE_INVALID, hoxPIECE_COLOR_NONE,
                          hoxPosition(-1, -1) ) { }
        Piece(hoxPieceType t) : m_info( t, hoxPIECE_COLOR_NONE, 
                                        hoxPosition(-1, -1) ) { }
        Piece(hoxPieceType t, hoxPieceColor c) 
                              : m_info( t, c, hoxPosition(-1, -1) ) { }
        Piece(hoxPieceType t, hoxPieceColor c, const hoxPosition& p)
                              : m_info( t, c, p ) { }
        virtual ~Piece() {}

        hoxPieceType  GetType()     const { return m_info.type; }
        hoxPieceColor GetColor()    const { return m_info.color; }
        hoxPosition   GetPosition() const { return m_info.position; }

        void SetPosition( const hoxPosition& pos ) { m_info.position = pos; }
        void SetBoard(Board* board) { m_board = board; }

        virtual bool IsValidMove(const hoxPosition& newPos) const = 0;
        virtual bool DoesNextMoveExist() const;

    protected:
        virtual void GetPotentialNextPositions(PositionList& positions) const {} // FIXME

    public: /* Static Public API */ 
        static bool Is_Inside_Palace( hoxPieceColor color, const hoxPosition& position );

    protected:
        hoxPieceInfo  m_info;
        Board*        m_board;
    };

    class KingPiece : public Piece
    {
    public:
        KingPiece(hoxPieceColor c, const hoxPosition& p) 
            : Piece(hoxPIECE_TYPE_KING, c, p) {}
        virtual bool IsValidMove(const hoxPosition& newPos) const;
        virtual void GetPotentialNextPositions(PositionList& positions) const;
    };

    class AdvisorPiece : public Piece
    {
    public:
        AdvisorPiece(hoxPieceColor c, const hoxPosition& p) 
            : Piece(hoxPIECE_TYPE_ADVISOR, c, p) {}
        virtual bool IsValidMove(const hoxPosition& newPos) const;
        virtual void GetPotentialNextPositions(PositionList& positions) const;
    };

    class ElephantPiece : public Piece
    {
    public:
        ElephantPiece(hoxPieceColor c, const hoxPosition& p) 
            : Piece(hoxPIECE_TYPE_ELEPHANT, c, p) {}
        virtual bool IsValidMove(const hoxPosition& newPos) const;
        virtual void GetPotentialNextPositions(PositionList& positions) const;
    };

    class ChariotPiece : public Piece
    {
    public:
        ChariotPiece(hoxPieceColor c, const hoxPosition& p) 
            : Piece(hoxPIECE_TYPE_CHARIOT, c, p) {}
        virtual bool IsValidMove(const hoxPosition& newPos) const;
        virtual void GetPotentialNextPositions(PositionList& positions) const;
    };

    class HorsePiece : public Piece
    {
    public:
        HorsePiece(hoxPieceColor c, const hoxPosition& p) 
            : Piece(hoxPIECE_TYPE_HORSE, c, p) {}
        virtual bool IsValidMove(const hoxPosition& newPos) const;
        virtual void GetPotentialNextPositions(PositionList& positions) const;
    };

    class CannonPiece : public Piece
    {
    public:
        CannonPiece(hoxPieceColor c, const hoxPosition& p) 
            : Piece(hoxPIECE_TYPE_CANNON, c, p) {}
        virtual bool IsValidMove(const hoxPosition& newPos) const;
        virtual void GetPotentialNextPositions(PositionList& positions) const;
    };

    class PawnPiece : public Piece
    {
    public:
        PawnPiece(hoxPieceColor c, const hoxPosition& p) 
            : Piece(hoxPIECE_TYPE_PAWN, c, p) {}
        virtual bool IsValidMove(const hoxPosition& newPos) const;
        virtual void GetPotentialNextPositions(PositionList& positions) const;
    };

    class Cell
    {
    public:
        hoxPosition  position;
        
        Piece*       pPiece;
            /* The piece that currrently occupies the cell.
             * NULL indicates that this cell is empty.
             */

        Cell() : position(-1, -1), pPiece( NULL ) { }
        bool IsEmpty() { return ( pPiece == NULL ); }    
    };

    class Board
    {
    public:
        Board( hoxPieceColor nextColor = hoxPIECE_COLOR_NONE );
        virtual ~Board();

        // ------------ Main Public API -------
        void SetNextColor( hoxPieceColor c ) { m_nextColor = c; }
        bool ValidateMove(const hoxMove& move);

        void GetGameState( hoxPieceInfoList& pieceInfoList,
                           hoxPieceColor&    nextColor );

        bool GetPieceAtPosition( const hoxPosition& position, 
                                 hoxPieceInfo&      pieceInfo ) const;

        // ------------ Other Public API -------
        bool IsValidMove(const hoxMove& move);
        bool Simulation_IsValidMove(const hoxMove& move);

        Piece* GetPieceAt( const hoxPosition& position ) const;
        bool   HasPieceAt( const hoxPosition& position ) const;
        bool   IsValidCapture( hoxPieceColor      myColor, 
                               const hoxPosition& newPos );

    private:
        void         _CreateNewGame();
        void         _AddNewPiece(Piece* piece);

        void         _SetPiece( Piece* piece );
        void         _UnsetPiece( Piece* piece );
        void         _CapturePiece( Piece* piece );

        void         _MovePieceTo( Piece* piece, const hoxPosition& newPos );
        void         _AddPiece(Piece* piece);
        void         _PutbackPiece(Piece* piece);

        Piece*       _RecordMove(const hoxMove& move);
        void         _UndoMove( const hoxMove& move, Piece* pCaptured );

        const Piece* _GetKing(hoxPieceColor color) const;
        bool         _IsKingBeingChecked(hoxPieceColor color);
        bool         _IsKingFaceKing() const;

        bool         _DoesNextMoveExist();

    private:
        PieceList      m_pieces;       // ACTIVE pieces
        PieceList      m_deadPieces;   // INACTIVE (dead) pieces
        Cell           m_cells[9][10];

        hoxPieceColor  m_nextColor;
            /* Which side (RED or BLACK) will move next? */
    };


    /* Other utility API */

    void PositionList_Clear( PositionList& positions );

} // namespace BoardInfoAPI



/* Import namespaces */

using namespace BoardInfoAPI;


//-----------------------------------------------------------------------------
// API Definitions
//-----------------------------------------------------------------------------


Board::Board( hoxPieceColor nextColor /* = hoxPIECE_COLOR_NONE */ )
        : m_nextColor( nextColor )
{
    /* Initialize Piece-Cells. */
    for ( int x = 0; x <= 8; ++x )
    {
        for ( int y = 0; y <= 9; ++y )
        {
            m_cells[x][y].pPiece = NULL;
            m_cells[x][y].position = hoxPosition(-1,-1);
        }
    }

    /* Initialize Board. */
    _CreateNewGame();
}

Board::~Board()
{
    PieceList::const_iterator it;

    for ( it = m_pieces.begin(); it != m_pieces.end(); ++it )
    {
        delete (*it);
    }

    for ( it = m_deadPieces.begin(); it != m_deadPieces.end(); ++it )
    {
        delete (*it);
    }
}

/**
 * Create a brand new game by specifying the info of ALL pieces initially.
 */
void
Board::_CreateNewGame()
{
    hoxPieceColor color;        // The current color.
    int           i;

    // --------- BLACK

    color = hoxPIECE_COLOR_BLACK;

    _AddNewPiece( new KingPiece(     color, hoxPosition(4, 0) ) );
    _AddNewPiece( new AdvisorPiece(  color, hoxPosition(3, 0) ) );
    _AddNewPiece( new AdvisorPiece(  color, hoxPosition(5, 0) ) );
    _AddNewPiece( new ElephantPiece( color, hoxPosition(2, 0) ) );
    _AddNewPiece( new ElephantPiece( color, hoxPosition(6, 0) ) );
    _AddNewPiece( new HorsePiece(    color, hoxPosition(1, 0) ) );
    _AddNewPiece( new HorsePiece(    color, hoxPosition(7, 0) ) );
    _AddNewPiece( new ChariotPiece(  color, hoxPosition(0, 0) ) );
    _AddNewPiece( new ChariotPiece(  color, hoxPosition(8, 0) ) );
    _AddNewPiece( new CannonPiece(   color, hoxPosition(1, 2) ) );
    _AddNewPiece( new CannonPiece(   color, hoxPosition(7, 2) ) );
    for ( i = 0; i < 10; i += 2 ) // 5 Pawns.
    {
        _AddNewPiece( new PawnPiece(   color, hoxPosition(i, 3) ) );
    }

    // --------- RED

    color = hoxPIECE_COLOR_RED;

    _AddNewPiece( new KingPiece(     color, hoxPosition(4, 9) ) );
    _AddNewPiece( new AdvisorPiece(  color, hoxPosition(3, 9) ) );
    _AddNewPiece( new AdvisorPiece(  color, hoxPosition(5, 9) ) );
    _AddNewPiece( new ElephantPiece( color, hoxPosition(2, 9) ) );
    _AddNewPiece( new ElephantPiece( color, hoxPosition(6, 9) ) );
    _AddNewPiece( new HorsePiece(    color, hoxPosition(1, 9) ) );
    _AddNewPiece( new HorsePiece(    color, hoxPosition(7, 9) ) );
    _AddNewPiece( new ChariotPiece(  color, hoxPosition(0, 9) ) );
    _AddNewPiece( new ChariotPiece(  color, hoxPosition(8, 9) ) );
    _AddNewPiece( new CannonPiece(   color, hoxPosition(1, 7) ) );
    _AddNewPiece( new CannonPiece(   color, hoxPosition(7, 7) ) );
    for ( i = 0; i < 10; i += 2 ) // 5 Pawns.
    {
        _AddNewPiece( new PawnPiece(   color, hoxPosition(i, 6) ) );
    }
}

void 
Board::GetGameState( hoxPieceInfoList& pieceInfoList,
                     hoxPieceColor&    nextColor )
{
    pieceInfoList.clear();    // Clear the old info, if exists.

    /* Return all the ACTIVE Pieces. */
    for ( PieceList::const_iterator it = m_pieces.begin();
                                    it != m_pieces.end(); ++it )
    {
        pieceInfoList.push_back( new hoxPieceInfo( (*it)->GetType(), 
                                                   (*it)->GetColor(), 
                                                   (*it)->GetPosition() ) );
    }

    /* Return the Next Color */
    nextColor = m_nextColor;
}

bool 
Board::GetPieceAtPosition( const hoxPosition& position, 
                           hoxPieceInfo&      pieceInfo ) const
{
    Piece* piece = this->GetPieceAt( position );
    if ( piece == NULL )
        return false;

    pieceInfo.type     = piece->GetType();
    pieceInfo.color    = piece->GetColor();
    pieceInfo.position = piece->GetPosition();

    return true;
}

/**
 * Put a given Piece on Board according to its position.
 */
void
Board::_SetPiece( Piece* piece )
{
    wxCHECK_RET(piece, "Piece is NULL.");

    hoxPosition pos = piece->GetPosition();  // The piece's position.

    wxCHECK_RET(pos.IsValid(), "The piece's position is not valid.");

    wxCHECK_RET( m_cells[pos.x][pos.y].pPiece == NULL, 
                 "The destination cell is not empty." );
    m_cells[pos.x][pos.y].pPiece = piece;
    m_cells[pos.x][pos.y].position = pos;
}

/**
 * Unset an existing piece from Board.
 */
void
Board::_UnsetPiece( Piece* piece )
{
    wxCHECK_RET(piece, "Piece is NULL.");

    hoxPosition curPos = piece->GetPosition();  // The current position.

    wxCHECK_RET(curPos.IsValid(), "The current position is not valid.");
    wxCHECK_RET(m_cells[curPos.x][curPos.y].pPiece == piece, 
                "There is no such as piece on Board.");

    /* 'Undo' old position, if any */
    m_cells[curPos.x][curPos.y].pPiece = NULL;
    m_cells[curPos.x][curPos.y].position = hoxPosition(-1,-1);
}

/**
 * Carry out the 'capture' action toward a given piece:
 *   + Unset the piece from the Board.
 *   + Move the piece from the ACTIVE list to the INACTIVE list.
 */
void 
Board::_CapturePiece( Piece* piece )
{
    _UnsetPiece( piece );

    PieceList::iterator found;

    found = std::find( m_pieces.begin(), m_pieces.end(), piece );
    wxCHECK_RET( found != m_pieces.end(), "The piece is not ACTIVE." );

    found = std::find( m_deadPieces.begin(), m_deadPieces.end(), piece );
    wxCHECK_RET( found == m_deadPieces.end(), "The piece is ready INACTIVE." );

    m_pieces.remove( piece );
    m_deadPieces.push_back( piece );
}

/**
 * Move an existing piece from its current position to a new position.
 *
 * @side-affects: The piece's position is modified as well.
 */
void
Board::_MovePieceTo( Piece*             piece, 
                     const hoxPosition& newPos )
{
    wxCHECK_RET(piece, "Piece is NULL.");

    hoxPosition curPos = piece->GetPosition();  // The current position.

    wxCHECK_RET(curPos.IsValid(), "The current position is not valid.");
    wxCHECK_RET(newPos.IsValid(), "The new position is not valid.");

    _UnsetPiece( piece );
    piece->SetPosition( newPos );  // Adjust piece's position.
    _SetPiece( piece );
}

Piece* 
Board::GetPieceAt( const hoxPosition& position ) const
{
    if ( ! position.IsValid() )
        return NULL;

    return m_cells[position.x][position.y].pPiece;
}

bool
Board::HasPieceAt( const hoxPosition& position ) const
{
    return ( NULL != this->GetPieceAt( position ) );
}

/**
 * Put a brand new piece on Board.
 */
void
Board::_AddNewPiece(Piece* piece)
{
    wxCHECK_RET(piece, "Piece is NULL.");

    piece->SetBoard( this );
    this->_AddPiece( piece );
}

/**
 * Put a piece on Board.
 */
void
Board::_AddPiece(Piece* piece)
{
    wxCHECK_RET(piece, "Piece is NULL.");

    m_pieces.push_back( piece );
    _SetPiece( piece );
}

void 
Board::_PutbackPiece(Piece* piece)
{
    PieceList::iterator found =
        std::find( m_deadPieces.begin(), m_deadPieces.end(), piece );
    wxCHECK_RET( found != m_deadPieces.end(), "Dead piece should be found." );
    
    m_deadPieces.remove( piece );
    this->_AddPiece( piece );
}

/**
 * Record a valid Move.
 */
Piece*
Board::_RecordMove( const hoxMove& move )
{
    // Remove captured piece, if any.
    Piece* pCaptured = this->GetPieceAt(move.newPosition);
    if (pCaptured != NULL)
    {
        _CapturePiece( pCaptured );
    }

    // Move the piece to the new position.
    Piece* piece = this->GetPieceAt(move.piece.position);
    wxASSERT_MSG(piece != NULL, 
                 "Piece must not be NULL after being validated");
    _MovePieceTo( piece, move.newPosition );

    return pCaptured;
}

void
Board::_UndoMove( const hoxMove& move,
                  Piece*         pCaptured )
{
    // Return the piece from its NEW position to its ORIGINAL position.
    Piece* piece = this->GetPieceAt(move.newPosition);
    wxCHECK_RET(piece != NULL, "Piece must not be NULL after being validated");
    wxCHECK_RET(piece->GetPosition() == move.newPosition,  // TODO: Move inside GetPieceAt()
                "The positions do not match.");
    _MovePieceTo( piece, move.piece.position );

    // "Un-capture" the captured piece, if any.
    if ( pCaptured != NULL )
    {
        pCaptured->SetPosition( move.newPosition );
        this->_PutbackPiece( pCaptured );
    }
}

// Get the King of a given color.
const Piece*
Board::_GetKing(hoxPieceColor color) const
{
    for ( PieceList::const_iterator it = m_pieces.begin(); 
                                    it != m_pieces.end(); 
                                  ++it )
    {
        if (    (*it)->GetType() == hoxPIECE_TYPE_KING 
             && (*it)->GetColor() == color )
        {
            return (*it);
        }
    }

    return NULL;
}

// Check if a King (of a given color) is in CHECK position (being "checked").
// @return true if the King is being checked.
//         false, otherwise.
bool 
Board::_IsKingBeingChecked( hoxPieceColor color )
{
  // Check if this move results in one's own-checkmate.
  // This is done as follows:
  //  + For each piece of the 'enemy', check if the king's position
  //    is one of its valid move.
  //

    const Piece* pKing = _GetKing( color );
    wxASSERT_MSG( pKing != NULL, "King must not be NULL" );

    for ( PieceList::const_iterator it = m_pieces.begin(); 
                                    it != m_pieces.end(); ++it )
    {
        if ( (*it)->GetColor() != color )  // enemy?
        {
            if ( (*it)->IsValidMove( pKing->GetPosition() ) )
            {
                return true;
            }
        }
    }

  return false;  // Not in "checked" position.
}

// Check if one king is facing another.
bool 
Board::_IsKingFaceKing() const
{
    const Piece* blackKing = _GetKing( hoxPIECE_COLOR_BLACK );
    wxASSERT_MSG( blackKing != NULL, "Black King must not be NULL" );

    const Piece* redKing = _GetKing( hoxPIECE_COLOR_RED );
    wxASSERT_MSG( redKing != NULL, "Red King must not be NULL" );

    if ( blackKing->GetPosition().x != redKing->GetPosition().x ) // not the same column.
    {
        return false;  // Not facing
    }

    // If they are in the same column, check if there is any piece in between.
    for ( PieceList::const_iterator it = m_pieces.begin(); 
                                    it != m_pieces.end(); 
                                  ++it )
    {
        if ( (*it) == blackKing || (*it) == redKing )
            continue;

        if ( (*it)->GetPosition().x == blackKing->GetPosition().x )  // in between?
        {
            return false;  // Not facing
        }
    }

    return true;  // Not facing
}

bool
Board::ValidateMove(const hoxMove& move)
{
    /* Check for 'turn' */

    if ( move.piece.color != m_nextColor )
        return false; // Error! Wrong turn.

    /* Perform a basic validation */

    if ( ! this->IsValidMove( move ) )
        return false;

    /* At this point, the Move is valid.
     * Record this move (to validate future Moves).
     */

    Piece* pCaptured = _RecordMove( move );

    /* If the Move ends up results in its own check-mate OR
     * there is a KING-face-KING problem...
     * then it is invalid and must be undone.
     */
    if (   _IsKingBeingChecked( move.piece.color )
        || _IsKingFaceKing() )
    {
        _UndoMove(move, pCaptured);
        return false;
    }

    //delete pCaptured; // TODO: Should use some kind of smart pointer here?

    /* Set the next-turn. */
    m_nextColor = ( m_nextColor == hoxPIECE_COLOR_RED 
                   ? hoxPIECE_COLOR_BLACK
                   : hoxPIECE_COLOR_RED);

    /* Check for end game:
     * ------------------
     *   Checking if this Move makes the Move's Player
     *   the winner of the game. The step is done by checking to see if the
     *   opponent can make ANY valid Move at all.
     *   If no, then the opponent has just lost the game.
     */

    if ( ! _DoesNextMoveExist() )
    {
        wxLogWarning("The game is over.");
    }

    return true;
}

bool 
Board::IsValidMove( const hoxMove& move )
{
    hoxPosition curPos = move.piece.position; // current position
    Piece* piece = this->GetPieceAt( curPos );
    if (piece == NULL)
        return false;

    return piece->IsValidMove( move.newPosition );
}

bool
Board::Simulation_IsValidMove( const hoxMove& move )
{
    hoxPosition curPos = move.piece.position; // current position
    Piece* piece = this->GetPieceAt( curPos );
    if (piece == NULL)
        return false;

    if ( ! piece->IsValidMove( move.newPosition ) )
        return false;

    /* Simulate the Move. */

    Piece* pCaptured = _RecordMove( move );

    /* If the Move ends up results in its own check-mate,
     * then it is invalid and must be undone.
     */
    bool beingChecked = _IsKingBeingChecked( piece->GetColor() );
    bool areKingsFacing = _IsKingFaceKing();

    _UndoMove( move, pCaptured );

    if ( beingChecked || areKingsFacing )
    {
        return false;  // Not a good move. Process the next move.
    }

    return true;  // It is a valid Move.
}

bool 
Board::_DoesNextMoveExist()
{
    for ( PieceList::const_iterator it = m_pieces.begin(); 
                                    it != m_pieces.end(); ++it )
    {
        if ( (*it)->GetColor() == m_nextColor )
        {
            if ( (*it)->DoesNextMoveExist() )
                return true;
        }
    }

    return false;
}

/**
 * If the given Move (piece -> newPosition) results in a piece 
 * being captured, then make sure that the captured piece is of
 * an 'enemy'.
 *
 * @return false - If a piece would be captured and NOT an 'enemy'.
 *         true  - If no piece would be captured OR the would-be-capured
 *                 piece is an 'enemy'.
 */
bool 
Board::IsValidCapture( hoxPieceColor      myColor, 
                       const hoxPosition& newPos )
{
    const Piece* capturedPiece = this->GetPieceAt(newPos);
    if (   capturedPiece != NULL 
        && capturedPiece->GetColor() == myColor )
    {
        return false; // Capture your OWN piece! Not legal.
    }

    return true;
}

bool 
Piece::DoesNextMoveExist() const
{
    bool  nextMoveExits = false;

    /* Generate all potential 'next' positions. */

    PositionList positions;  // all potential 'next' positions.

    this->GetPotentialNextPositions( positions );

    /* For each potential 'next' position, check if this piece can 
     * actually move there. 
     */

    const hoxPieceInfo pieceInfo( this->GetType(), 
                                  this->GetColor(), 
                                  this->GetPosition() );
    hoxMove move;
    move.piece = pieceInfo;

    for ( PositionList::const_iterator it = positions.begin();
                                       it != positions.end(); ++it )
    {
        if ( ! (*it)->IsValid() ) continue;

        move.newPosition = *(*it);
        
        /* Ask the Board to validate this Move in Simulation mode. */
        if ( m_board->Simulation_IsValidMove( move ) )
        {
            nextMoveExits = true;
            break;
        }
    }

    PositionList_Clear( positions ); // Release memory.

    return nextMoveExits;
}

bool 
KingPiece::IsValidMove(const hoxPosition& newPos) const
{
    const hoxPieceColor myColor = m_info.color;
    const hoxPosition   curPos = m_info.position;

    // Within the palace...?
    if ( !newPos.IsInsidePalace(myColor) ) 
        return false;

    // Is a 1-cell horizontal or vertical move?
    if (! (  (abs(newPos.x - curPos.x) == 1 && newPos.y == curPos.y)
        ||   (newPos.x == curPos.x && abs(newPos.y - curPos.y) == 1)) )
    {
        return false;
    }

    // If this is capture-move, make sure the captured piece is an 'enemy'.
    if ( ! m_board->IsValidCapture(myColor, newPos) )
        return false;

    return true;
}

void 
KingPiece::GetPotentialNextPositions(PositionList& positions) const
{
    const hoxPosition p = this->GetPosition();
    
    positions.clear();

    // ... Simply use the 4 possible positions.
    positions.push_back( new hoxPosition(p.x, p.y-1) );
    positions.push_back( new hoxPosition(p.x, p.y+1) );
    positions.push_back( new hoxPosition(p.x-1, p.y) );
    positions.push_back( new hoxPosition(p.x+1, p.y) );
}

bool 
AdvisorPiece::IsValidMove(const hoxPosition& newPos) const
{
    const hoxPieceColor myColor = m_info.color;
    const hoxPosition   curPos = m_info.position;

    // Within the palace...?
    if ( !newPos.IsInsidePalace(myColor) ) 
        return false;

    // Is a 1-cell diagonal move?
    if (! (abs(newPos.x - curPos.x) == 1 && abs(newPos.y - curPos.y) == 1) )
        return false;

    // If this is capture-move, make sure the captured piece is an 'enemy'.
    if ( ! m_board->IsValidCapture(myColor, newPos) )
        return false;

    return true;

}

void 
AdvisorPiece::GetPotentialNextPositions(PositionList& positions) const
{
    const hoxPosition p = this->GetPosition();
    
    positions.clear();

    // ... Simply use the 4 possible positions.
    positions.push_back(new hoxPosition(p.x-1, p.y-1));
    positions.push_back(new hoxPosition(p.x-1, p.y+1));
    positions.push_back(new hoxPosition(p.x+1, p.y-1));
    positions.push_back(new hoxPosition(p.x+1, p.y+1));
}

bool 
ElephantPiece::IsValidMove(const hoxPosition& newPos) const
{
    const hoxPieceColor myColor = m_info.color;
    const hoxPosition   curPos = m_info.position;

    // Within the country...?
    if ( !newPos.IsInsideCountry(myColor) ) // crossed the river/border?
        return false;

    // Is a 2-cell diagonal move?
    if (! (abs(newPos.x - curPos.x) == 2 && abs(newPos.y - curPos.y) == 2) )
        return false;

    // ... and there is no piece on the diagonal (to hinder the move)
    hoxPosition centerPos((curPos.x+newPos.x)/2, (curPos.y+newPos.y)/2);
    if ( m_board->HasPieceAt(centerPos) )
        return false;

    // If this is capture-move, make sure the captured piece is an 'enemy'.
    if ( ! m_board->IsValidCapture(myColor, newPos) )
        return false;

    return true;
}

void 
ElephantPiece::GetPotentialNextPositions(PositionList& positions) const
{
    const hoxPosition p = this->GetPosition();
    
    positions.clear();

    // ... Simply use the 4 possible positions.
    positions.push_back(new hoxPosition(p.x-2, p.y-2));
    positions.push_back(new hoxPosition(p.x-2, p.y+2));
    positions.push_back(new hoxPosition(p.x+2, p.y-2));
    positions.push_back(new hoxPosition(p.x+2, p.y+2));
}

bool 
ChariotPiece::IsValidMove(const hoxPosition& newPos) const
{
    const hoxPieceColor myColor = m_info.color;
    const hoxPosition   curPos = m_info.position;

    bool bIsValidMove = false;

    // Is a horizontal or vertical move?
    if (! (  (newPos.x != curPos.x && newPos.y == curPos.y)
          || (newPos.x == curPos.x && newPos.y != curPos.y) ) )
    {
        return false;
    }

    // Make sure there is no piece that hinders the move from the current
    // position to the new.
    //
    //          top
    //           ^
    //           |
    //  left <-- +  ---> right
    //           |
    //           v
    //         bottom
    // 

    PositionList middlePieces;
    hoxPosition* pPos = 0;
    int i;

    // If the new position is on TOP.
    if (newPos.y < curPos.y)
    {
        for (i = newPos.y+1; i < curPos.y; ++i)
        {
            pPos = new hoxPosition(curPos.x, i);
            middlePieces.push_back( pPos );
        }
    }
    // If the new position is on the RIGHT.
    else if (newPos.x > curPos.x)
    {
        for (i = curPos.x+1; i < newPos.x; ++i)
        {
            pPos = new hoxPosition(i, curPos.y);
            middlePieces.push_back(pPos);
        }
    }
    // If the new position is at the BOTTOM.
    else if (newPos.y > curPos.y)
    {
        for (i = curPos.y+1; i < newPos.y; ++i)
        {
            pPos = new hoxPosition(curPos.x, i);
            middlePieces.push_back(pPos);
        }
    }
    // If the new position is on the LEFT.
    else if (newPos.x < curPos.x)
    {
        for (i = newPos.x+1; i < curPos.x; ++i)
        {
            pPos = new hoxPosition(i, curPos.y);
            middlePieces.push_back(pPos);
        }
    }

    // Check that no middle pieces exist from the new to the current position.
    for ( PositionList::iterator it = middlePieces.begin();
                                 it != middlePieces.end();
                               ++it )
    {
        if ( m_board->HasPieceAt( *(*it) ) )
        {
            goto cleanup;  // return with 'invalid' move
        }
    }

    // If this is capture-move, make sure the captured piece is an 'enemy'.
    if ( ! m_board->IsValidCapture(myColor, newPos) )
        goto cleanup;  // return with 'invalid' move

    // Finally, return 'valid' move.
    bIsValidMove = true;

cleanup:
    PositionList_Clear( middlePieces ); // Release memory.

    return bIsValidMove;
}

void 
ChariotPiece::GetPotentialNextPositions(PositionList& positions) const
{
    const hoxPosition p = this->GetPosition();
    
    positions.clear();

    // ... Horizontally.
    for ( int x = 0; x <= 8; ++x )
    {
        if ( x == p.x ) continue;
        positions.push_back(new hoxPosition(x, p.y));

    }

    // ... Vertically
    for ( int y = 0; y <= 9; ++y )
    {
        if ( y == p.y ) continue;
        positions.push_back(new hoxPosition(p.x, y));

    }
}

bool 
HorsePiece::IsValidMove(const hoxPosition& newPos) const
{
    const hoxPieceColor myColor = m_info.color;
    const hoxPosition   curPos = m_info.position;

    bool bMoveValid = false;

    /* Is a 2-1-rectangle move? */

    // Make sure there is no piece that hinders the move if the move
    // start from one of the four 'neighbors' below.
    //
    //          top
    //           ^
    //           |
    //  left <-- +  ---> right
    //           |
    //           v
    //         bottom
    // 

    hoxPosition neighbor;

    // If the new position is on TOP.
    if ( (curPos.y - 2) == newPos.y && abs(newPos.x - curPos.x) == 1 )
    {
        neighbor = hoxPosition(curPos.x, curPos.y-1); 
    }
    // If the new position is at the BOTTOM.
    else if ( (curPos.y + 2) == newPos.y && abs(newPos.x - curPos.x) == 1 )
    {
        neighbor = hoxPosition(curPos.x, curPos.y+1); 
    }
    // If the new position is on the RIGHT.
    else if ( (curPos.x + 2) == newPos.x && abs(newPos.y - curPos.y) == 1 )
    {
        neighbor = hoxPosition(curPos.x+1, curPos.y); 
    }
    // If the new position is on the LEFT.
    else if ( (curPos.x - 2) == newPos.x && abs(newPos.y - curPos.y) == 1 )
    {
        neighbor = hoxPosition(curPos.x-1, curPos.y); 
    }
    else
    {
        return false;
    }

    // If the neighbor exists, then the move is invalid.
    if ( m_board->HasPieceAt( neighbor ) ) 
        return false;

    // If this is capture-move, make sure the captured piece is an 'enemy'.
    if ( ! m_board->IsValidCapture(myColor, newPos) )
        return false;

    return true;
}

void 
HorsePiece::GetPotentialNextPositions(PositionList& positions) const
{
    const hoxPosition p = this->GetPosition();
    
    positions.clear();

    // ... Check for the 8 possible positions.
    positions.push_back(new hoxPosition(p.x-1, p.y-2));
    positions.push_back(new hoxPosition(p.x-1, p.y+2));
    positions.push_back(new hoxPosition(p.x-2, p.y-1));
    positions.push_back(new hoxPosition(p.x-2, p.y+1));
    positions.push_back(new hoxPosition(p.x+1, p.y-2));
    positions.push_back(new hoxPosition(p.x+1, p.y+2));
    positions.push_back(new hoxPosition(p.x+2, p.y-1));
    positions.push_back(new hoxPosition(p.x+2, p.y+1));
}

bool 
CannonPiece::IsValidMove(const hoxPosition& newPos) const
{
    const hoxPieceColor myColor = m_info.color;
    const hoxPosition   curPos = m_info.position;

    bool bIsValidMove = false;

    // Is a horizontal or vertical move?
    if (! (  (newPos.x != curPos.x && newPos.y == curPos.y)
          || (newPos.x == curPos.x && newPos.y != curPos.y) ) )
    {
        return false;
    }

    // Make sure there is no piece that hinders the move from the current
    // position to the new.
    //
    //          top
    //           ^
    //           |
    //  left <-- +  ---> right
    //           |
    //           v
    //         bottom
    // 

    PositionList middlePieces;
    hoxPosition* pPos = 0;
    int i;

    // If the new position is on TOP.
    if (newPos.y < curPos.y)
    {
        for (i = newPos.y+1; i < curPos.y; ++i)
        {
            pPos = new hoxPosition(curPos.x, i);
            middlePieces.push_back(pPos);
        }
    }
    // If the new position is on the RIGHT.
    else if (newPos.x > curPos.x)
    {
        for (i = curPos.x+1; i < newPos.x; ++i)
        {
            pPos = new hoxPosition(i, curPos.y);
            middlePieces.push_back(pPos);
        }
    }
    // If the new position is at the BOTTOM.
    else if (newPos.y > curPos.y)
    {
        for (i = curPos.y+1; i < newPos.y; ++i)
        {
            pPos = new hoxPosition(curPos.x, i);
            middlePieces.push_back(pPos);
        }
    }
    // If the new position is on the LEFT.
    else if (newPos.x < curPos.x)
    {
        for (i = newPos.x+1; i < curPos.x; ++i)
        {
            pPos = new hoxPosition(i, curPos.y);
            middlePieces.push_back(pPos);
        }
    }

    // Check to see how many middle pieces exist from the 
    // new to the current position.
    int numMiddle = 0;
    for ( PositionList::const_iterator it = middlePieces.begin();
                                       it != middlePieces.end(); ++it )
    {
        if ( m_board->HasPieceAt( *(*it)) )
            ++numMiddle;
    }

    // If there are more than 1 middle piece, return 'invalid'.
    if (numMiddle > 1)
    {
        goto cleanup;  // return with 'invalid'
    }
    // If there is exactly 1 middle pieces, this must be a Capture Move.
    else if (numMiddle == 1)
    {
        const Piece* capturedPiece = m_board->GetPieceAt(newPos);
        if ( !capturedPiece || capturedPiece->GetColor() == myColor) 
        {
            goto cleanup;  // return with 'invalid'
        }
    }
    // If there is no middle piece, make sure that no piece is captured.
    else   // numMiddle = 0
    {
        if ( m_board->HasPieceAt(newPos) ) 
        {
            goto cleanup;  // return with 'invalid'
        }
    }

    // Finally, return 'valid' move.
    bIsValidMove = true;

cleanup:
    PositionList_Clear( middlePieces ); // Release memory.

    return bIsValidMove;
}

void 
CannonPiece::GetPotentialNextPositions(PositionList& positions) const
{
    const hoxPosition p = this->GetPosition();
    
    positions.clear();

    // ... Horizontally.
    for ( int x = 0; x <= 8; ++x )
    {
        if ( x == p.x ) continue;
        positions.push_back(new hoxPosition(x, p.y));

    }

    // ... Vertically
    for ( int y = 0; y <= 9; ++y )
    {
        if ( y == p.y ) continue;
        positions.push_back(new hoxPosition(p.x, y));

    }
}

bool 
PawnPiece::IsValidMove(const hoxPosition& newPos) const
{
    const hoxPieceColor myColor = m_info.color;
    const hoxPosition   curPos = m_info.position;

    bool bMoveValid = false;

    // Within the country...?
    if ( newPos.IsInsideCountry(myColor) ) 
    {
        // Can only move up.
        if ( newPos.x == curPos.x && 
           (   (myColor == hoxPIECE_COLOR_BLACK && newPos.y == curPos.y+1)
             || myColor == hoxPIECE_COLOR_RED && newPos.y == curPos.y-1) )
        {
            bMoveValid = true;
        }
    }
    // Outside the country (alread crossed the 'river')
    else
    {
        // Only horizontally (LEFT or RIGHT)
        // ... or 1-way-UP vertically.
        if ( ( newPos.y == curPos.y 
                    && abs(newPos.x - curPos.x) == 1 )
            || (newPos.x == curPos.x
                  && ( myColor == hoxPIECE_COLOR_BLACK && newPos.y == curPos.y+1
                    || myColor == hoxPIECE_COLOR_RED && newPos.y == curPos.y-1)) )
        {
            bMoveValid = true;
        }
    }

    // *** If move is still invalid, return 'invalid'.
    if ( ! bMoveValid )
        return false;

    // If this is capture-move, make sure the captured piece is an 'enemy'.
    if ( ! m_board->IsValidCapture(myColor, newPos) )
        return false;

    return true;
}

void 
PawnPiece::GetPotentialNextPositions(PositionList& positions) const
{
    const hoxPosition p = this->GetPosition();
    
    positions.clear();

    // ... Simply use the 4 possible positions.
    positions.push_back(new hoxPosition(p.x, p.y-1));
    positions.push_back(new hoxPosition(p.x, p.y+1));
    positions.push_back(new hoxPosition(p.x-1, p.y));
    positions.push_back(new hoxPosition(p.x+1, p.y));
}

//-----------------------------------------------------------------------------
// Other utility API
//-----------------------------------------------------------------------------

void
BoardInfoAPI::PositionList_Clear( PositionList& positions )
{
    for ( PositionList::const_iterator it = positions.begin();
                                       it != positions.end(); ++it )
    {
        delete (*it);
    }
}



//-----------------------------------------------------------------------------
// hoxReferee
//-----------------------------------------------------------------------------

hoxReferee::hoxReferee()
            : m_nextColor( hoxPIECE_COLOR_NONE )
            , m_board( NULL )
{
    this->Reset();
}

hoxReferee::~hoxReferee()
{
    delete m_board;
}

void
hoxReferee::Reset()
{
    delete m_board;   // Delete the old Board, if exists.

    m_nextColor = hoxPIECE_COLOR_RED;
    m_board = new Board( m_nextColor );
}

bool 
hoxReferee::ValidateMove(const hoxMove& move)
{
    wxCHECK_MSG(m_board, false, "The Board is NULL.");
    return m_board->ValidateMove( move );
}

void 
hoxReferee::GetGameState( hoxPieceInfoList& pieceInfoList,
                          hoxPieceColor&    nextColor )
{
    wxCHECK_RET(m_board, "The Board is NULL.");
    return m_board->GetGameState( pieceInfoList, nextColor );
}

bool 
hoxReferee::GetPieceAtPosition( const hoxPosition& position, 
                                hoxPieceInfo&      pieceInfo ) const
{
    wxCHECK_MSG(m_board, false, "The Board is NULL.");
    return m_board->GetPieceAtPosition( position, pieceInfo );
}

/************************* END OF FILE ***************************************/
