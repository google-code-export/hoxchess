/////////////////////////////////////////////////////////////////////////////
// Name:            hoxReferee.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         09/30/2007
/////////////////////////////////////////////////////////////////////////////

#include "hoxReferee.h"
#include "hoxPiece.h"
#include "hoxUtility.h"
#include <list>

hoxReferee::hoxReferee()
{
    this->Reset();
}

hoxReferee::~hoxReferee()
{
    hoxUtility::FreePieceInfoList( _pieceInfoList );
}

void
hoxReferee::Reset()
{
    hoxUtility::CreateNewGameInfo( _pieceInfoList );

    _nextColor = hoxPIECE_COLOR_RED;
}

bool 
hoxReferee::ValidateMove(const hoxMove& move)
{
    /* Check for 'turn' */

    if ( move.piece.color != _nextColor )
    {
        return false; // Error! Wrong turn.
    }

    if ( ! _IsValidMove( move ) )
    {
        return false;
    }

    /* At this point, the Move is valid.
     * Record this move (to validate future Moves).
     */

    hoxPieceInfo* pCaptured = _RecordMove(move);

    /* If the Move ends up results in its own check-mate,
     * then it is invalid and must be undone.
     */
    if ( !_CheckForOwnCheckmate( move ) )
    {
        _UndoMove(move, pCaptured);
        return false;
    }

    delete pCaptured;

    /* Set the next-turn. */
    _nextColor = ( _nextColor == hoxPIECE_COLOR_RED 
                   ? hoxPIECE_COLOR_BLACK
                   : hoxPIECE_COLOR_RED);

    return true;
}

void 
hoxReferee::GetGameState( hoxPieceInfoList& pieceInfoList,
                          hoxPieceColor&    nextColor )
{
    pieceInfoList.clear();    // clear old info

    for ( hoxPieceInfoList::const_iterator it = _pieceInfoList.begin(); 
                                           it != _pieceInfoList.end(); 
                                         ++it )
    {
        hoxPieceInfo* pieceInfo = new hoxPieceInfo( *(*it) );
        pieceInfoList.push_back( pieceInfo );
    }

    nextColor = _nextColor;
}

bool 
hoxReferee::GetPieceAtPosition( const hoxPosition& position, 
                                hoxPieceInfo&      pieceInfo ) const
{
    hoxPieceInfo* pPieceInfo = _GetPieceAt( position );
    if ( pPieceInfo == NULL )
        return false;

    pieceInfo = *pPieceInfo;
    return true;
}

bool 
hoxReferee::_IsValidMove(const hoxMove& move)
{
    switch ( move.piece.type )
    {
    case hoxPIECE_TYPE_KING:
        return _IsValidMove_King(move);

    case hoxPIECE_TYPE_ADVISOR:
        return _IsValidMove_Advisor(move);

    case hoxPIECE_TYPE_ELEPHANT:
        return _IsValidMove_Elephant(move);

    case hoxPIECE_TYPE_HORSE:
        return _IsValidMove_Horse(move);

    case hoxPIECE_TYPE_CHARIOT:
        return _IsValidMove_Chariot(move);

    case hoxPIECE_TYPE_CANNON:
        return _IsValidMove_Cannon(move);

    case hoxPIECE_TYPE_PAWN:
        return _IsValidMove_Pawn(move);
  }

  return true;
}

// Validate King's move
bool 
hoxReferee::_IsValidMove_King( const hoxMove& move )
{
    const hoxPieceInfo  piece_info = move.piece;
    const hoxPieceColor color = piece_info.color;
    const hoxPosition   curPos = piece_info.position;
    const hoxPosition   newPos = move.newPosition;

    // Within the palace...?
    if ( !newPos.IsInsidePalace(color) ) 
    {
        return false;
    }

    // Is a 1-cell horizontal or vertical move?
    if (! (  (abs(newPos.x - curPos.x) == 1 && newPos.y == curPos.y)
        || (newPos.x == curPos.x && abs(newPos.y - curPos.y) == 1)) )
    {
        return false;
    }

    // If this is capture-move, make sure the captured piece is an 'enemy'.
    if ( !_IsValidCapture(piece_info, newPos) )
    {
        return false;
    }

    return true;
}

// Validate Advisor's move
bool 
hoxReferee::_IsValidMove_Advisor(const hoxMove& move)
{
    const hoxPieceInfo  piece_info = move.piece;
    const hoxPieceColor color = piece_info.color;
    const hoxPosition   curPos = piece_info.position;
    const hoxPosition   newPos = move.newPosition;

    // Within the palace...?
    if ( !newPos.IsInsidePalace(color) ) 
    {
        return false;
    }

    // Is a 1-cell diagonal move?
    if (! (abs(newPos.x - curPos.x) == 1 && abs(newPos.y - curPos.y) == 1) )
    {
        return false;
    }

    // If this is capture-move, make sure the captured piece is an 'enemy'.
    if ( !_IsValidCapture(piece_info, newPos) )
    {
        return false;
    }

    return true;
}

// Validate Elephant's move
bool 
hoxReferee::_IsValidMove_Elephant(const hoxMove& move)
{
    const hoxPieceInfo  piece_info = move.piece;
    const hoxPieceColor color = piece_info.color;
    const hoxPosition   curPos = piece_info.position;
    const hoxPosition   newPos = move.newPosition;

    // Within the country...?
    if ( !newPos.IsInsideCountry(color) ) 
    {
        return false;
    }

    // Is a 2-cell diagonal move?
    if (! (abs(newPos.x - curPos.x) == 2 && abs(newPos.y - curPos.y) == 2) )
    {
        return false;
    }

    // ... and there is no piece on the diagonal (to hinder the move)
    hoxPosition centerPos((curPos.x+newPos.x)/2, (curPos.y+newPos.y)/2);
    const hoxPieceInfo* centerPiece = _GetPieceAt(centerPos);
    if (centerPiece != NULL)
    {
        return false;
    }

    // If this is capture-move, make sure the captured piece is an 'enemy'.
    if ( !_IsValidCapture(piece_info, newPos) )
    {
        return false;
    }

    return true;
}

// Validate Horse's move
bool 
hoxReferee::_IsValidMove_Horse(const hoxMove& move)
{
    const hoxPieceInfo  piece_info = move.piece;
    const hoxPieceColor color = piece_info.color;
    const hoxPosition   curPos = piece_info.position;
    const hoxPosition   newPos = move.newPosition;

    // Is a 2-1-rectangle move?
    if (! ((abs(newPos.x - curPos.x) == 1 && abs(newPos.y - curPos.y) == 2)
        || (abs(newPos.x - curPos.x) == 2 && abs(newPos.y - curPos.y) == 1)) )
    {
        return false;
    }

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

    hoxPosition neighbors[] = {
    hoxPosition(curPos.x, curPos.y-1),  // top
    hoxPosition(curPos.x+1, curPos.y),  // right
    hoxPosition(curPos.x, curPos.y+1),  // bottom
    hoxPosition(curPos.x-1, curPos.y)   // left
    };

    bool bMoveValid = false;

    for (int i = 0; i < 4; ++i)
    {
        const hoxPosition& nPos = neighbors[i];
        if ( ! _GetPieceAt(nPos) 
           && (abs(newPos.x - nPos.x) == 1 && abs(newPos.y - nPos.y) == 1) )
        {
            bMoveValid = true;
            break;
        }
    }

    // *** If move is still invalid, return 'invalid'.
    if (!bMoveValid)
    {
        return false;
    }

    // If this is capture-move, make sure the captured piece is an 'enemy'.
    if ( ! _IsValidCapture(piece_info, newPos) )
    {
        return false;
    }

    return true;
}

// Validate Chariot's move
bool 
hoxReferee::_IsValidMove_Chariot(const hoxMove& move)
{
    const hoxPieceInfo  piece_info = move.piece;
    const hoxPieceColor color = piece_info.color;
    const hoxPosition   curPos = piece_info.position;
    const hoxPosition   newPos = move.newPosition;
    
    bool bIsValidMove = false;

    // Is a horizontal or vertical move?
    if (! (  (abs(newPos.x - curPos.x) > 0 && newPos.y == curPos.y)
        || (newPos.x == curPos.x && abs(newPos.y - curPos.y) > 0)) )
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

    std::list<hoxPosition *> middlePieces;
    hoxPosition* pPos = 0;
    int i;

    // If the new position is on top.
    if (newPos.y < curPos.y)
    {
        for (i = newPos.y+1; i < curPos.y; ++i)
        {
            pPos = new hoxPosition(curPos.x, i);
            middlePieces.push_back( pPos );
        }
    }
    // If the new position is on the right.
    else if (newPos.x > curPos.x)
    {
        for (i = curPos.x+1; i < newPos.x; ++i)
        {
            pPos = new hoxPosition(i, curPos.y);
            middlePieces.push_back(pPos);
        }
    }
    // If the new position is at the bottom.
    else if (newPos.y > curPos.y)
    {
        for (i = curPos.y+1; i < newPos.y; ++i)
        {
            pPos = new hoxPosition(curPos.x, i);
            middlePieces.push_back(pPos);
        }
    }
    // If the new position is on the left.
    else if (newPos.x < curPos.x)
    {
        for (i = newPos.x+1; i < curPos.x; ++i)
        {
            pPos = new hoxPosition(i, curPos.y);
            middlePieces.push_back(pPos);
        }
    }

    // Check that no middle pieces exist from the new to the current position.
    for ( std::list<hoxPosition *>::iterator it = middlePieces.begin();
                                             it != middlePieces.end();
                                           ++it )
    {
        if ( _GetPieceAt( *(*it) ) )  // piece exists?
        {
            goto cleanup;  // return with 'invalid' move
        }
    }

    // If this is capture-move, make sure the captured piece is an 'enemy'.
    if ( !_IsValidCapture(piece_info, newPos) )
    {
        goto cleanup;  // return with 'invalid' move
    }

    // Finally, return 'valid' move.
    bIsValidMove = true;

cleanup:
    // Release memory.
    for ( std::list<hoxPosition *>::iterator it = middlePieces.begin();
                                             it != middlePieces.end();
                                           ++it )
    {
        delete (*it);
        (*it) = NULL;
    }
    middlePieces.clear();

    return bIsValidMove;
}

// Validate Cannon's move
bool 
hoxReferee::_IsValidMove_Cannon(const hoxMove& move)
{
    const hoxPieceInfo  piece_info = move.piece;
    const hoxPieceColor color = piece_info.color;
    const hoxPosition   curPos = piece_info.position;
    const hoxPosition   newPos = move.newPosition;

    bool bIsValidMove = false;

    // Is a horizontal or vertical move?
    if (! (  (abs(newPos.x - curPos.x) > 0 && newPos.y == curPos.y)
        || (newPos.x == curPos.x && abs(newPos.y - curPos.y) > 0)) )
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

    std::list<hoxPosition *> middlePieces;
    hoxPosition* pPos = 0;
    int i;

    // If the new position is on top.
    if (newPos.y < curPos.y)
    {
        for (i = newPos.y+1; i < curPos.y; ++i)
        {
            pPos = new hoxPosition(curPos.x, i);
            middlePieces.push_back(pPos);
        }
    }
    // If the new position is on the right.
    else if (newPos.x > curPos.x)
    {
        for (i = curPos.x+1; i < newPos.x; ++i)
        {
            pPos = new hoxPosition(i, curPos.y);
            middlePieces.push_back(pPos);
        }
    }
    // If the new position is at the bottom.
    else if (newPos.y > curPos.y)
    {
        for (i = curPos.y+1; i < newPos.y; ++i)
        {
            pPos = new hoxPosition(curPos.x, i);
            middlePieces.push_back(pPos);
        }
    }
    // If the new position is on the left.
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
    for ( std::list<hoxPosition *>::iterator it = middlePieces.begin();
                                             it != middlePieces.end();
                                           ++it )
    {
        if ( _GetPieceAt( *(*it)) )  // piece exists?
        {
            ++numMiddle;
        }
    }

    // If there are more than 1 middle piece, return 'invalid'.
    if (numMiddle > 1)
    {
        goto cleanup;  // return with 'invalid'
    }
    // If there is exactly 1 middle pieces, this must be a capture move.
    else if (numMiddle == 1)
    {
        const hoxPieceInfo* capturedPiece = _GetPieceAt(newPos);
        if ( !capturedPiece || capturedPiece->color == color) 
        {
            goto cleanup;  // return with 'invalid'
        }
    }
    // If there is no middle piece, make sure that no piece is captured.
    else   // numMiddle = 0
    {
        const hoxPieceInfo* capturedPiece = _GetPieceAt(newPos);
        if ( capturedPiece != NULL ) 
        {
            goto cleanup;  // return with 'invalid'
        }
    }

    // Finally, return 'valid' move.
    bIsValidMove = true;

cleanup:
    // Release memory.
    for ( std::list<hoxPosition *>::iterator it = middlePieces.begin();
                                             it != middlePieces.end();
                                           ++it )
    {
        delete (*it);
        (*it) = NULL;
    }
    middlePieces.clear();

    return bIsValidMove;
}

// Validate Pawn's move
bool 
hoxReferee::_IsValidMove_Pawn(const hoxMove& move)
{
    const hoxPieceInfo  piece_info = move.piece;
    const hoxPieceColor color = piece_info.color;
    const hoxPosition   curPos = piece_info.position;
    const hoxPosition   newPos = move.newPosition;

    bool bMoveValid = false;

    // Within the country...?
    if ( newPos.IsInsideCountry(color) ) 
    {
        // Can only move up.
        if ( newPos.x == curPos.x && 
           (   (color == hoxPIECE_COLOR_BLACK && newPos.y == curPos.y+1)
             || color == hoxPIECE_COLOR_RED && newPos.y == curPos.y-1) )
        {
            bMoveValid = true;
        }
    }
    // Outside the country (alread crossed the 'river')
    else
    {
        if ( ( newPos.y == curPos.y 
                  && (newPos.x == curPos.x-1 || newPos.x == curPos.x+1))
            || (newPos.x == curPos.x
                  && ( color == hoxPIECE_COLOR_BLACK && newPos.y == curPos.y+1
                    || color == hoxPIECE_COLOR_RED && newPos.y == curPos.y-1)) )
        {
            bMoveValid = true;
        }
    }

    // *** If move is still invalid, return 'invalid'.
    if (!bMoveValid)
    {
        return false;
    }

    // If this is capture-move, make sure the captured piece is an 'enemy'.
    if ( !_IsValidCapture(piece_info, newPos) )
    {
        return false;
    }

    return true;
}

// Is a valid capture move, if any piece is captured.
bool 
hoxReferee::_IsValidCapture(const hoxPieceInfo& pieceInfo, 
                            const hoxPosition& newPos)
{
    // If this is capture-move, make sure the captured piece is an 'enemy'.
    const hoxPieceInfo* capturedPiece = _GetPieceAt(newPos);
    if (   capturedPiece != NULL 
        && capturedPiece->color == pieceInfo.color )
    {
        return false;
    }

    return true;
}

// This is called after the Move has been validated but before
// checking for onw's OWN check-mate.
// Return the captured piece, if any. 
hoxPieceInfo*
hoxReferee::_RecordMove(const hoxMove& move)
{
    // Remove captured piece, if any.
    hoxPieceInfo* pCaptured = _GetPieceAt(move.newPosition);
    if (pCaptured != NULL)
    {
        _pieceInfoList.remove( pCaptured );
    }

    // Move the piece to the new position.
    hoxPieceInfo* pPieceInfo = _GetPieceAt(move.piece.position);
    wxASSERT_MSG(pPieceInfo != NULL, 
                 "Piece must not be NULL after being validated");
    pPieceInfo->position = move.newPosition;

    return pCaptured;
}

void
hoxReferee::_UndoMove(const hoxMove& move,
                      hoxPieceInfo* pCaptured)
{
    // Return the piece to its original position.
    hoxPieceInfo* pPieceInfo = _GetPieceAt(move.newPosition);
    wxASSERT_MSG(pPieceInfo != NULL, 
                 "Piece must not be NULL after being validated");
    pPieceInfo->position = move.piece.position;

    // "Un-capture" the captured piece, if any.
    if ( pCaptured != NULL )
    {
        pCaptured->position = move.newPosition;
        _pieceInfoList.push_back( pCaptured );
    }
}

// Check if a player's Move results in his OWN checkmate,
// which is not legal.
bool 
hoxReferee::_CheckForOwnCheckmate( const hoxMove& move )
{
  // Check if this move results in one's own-checkmate.
  // This is done as follows:
  //  + For each piece of the 'enemy', check if the king's position
  //    is one of its valid move.
  //

    const hoxPieceInfo* pKing = _GetKing( move.piece.color );
    wxASSERT_MSG( pKing != NULL, "King must not be NULL" );

    for ( hoxPieceInfoList::const_iterator it = _pieceInfoList.begin(); 
                                           it != _pieceInfoList.end(); 
                                         ++it )
    {
        if ( (*it)->color != move.piece.color )  // enemy?
        {
            hoxMove move;
            move.piece = *(*it);
            move.newPosition = pKing->position;

            if ( _IsValidMove( move ) )
            {
                return false;
            }
        }
    }

  return true;  // good
}

//
// Return 'NULL' if no ** active ** piece is at the specified position.
//
hoxPieceInfo*
hoxReferee::_GetPieceAt(const hoxPosition& pos) const
{
    for ( hoxPieceInfoList::const_iterator it = _pieceInfoList.begin(); 
                                           it != _pieceInfoList.end(); 
                                         ++it )
    {
        if ( (*it)->position == pos ) 
        {
            return (*it);
        }
    }

    return NULL;
}

// Get the King of a given color.
const hoxPieceInfo*
hoxReferee::_GetKing(hoxPieceColor color) const
{
    for ( hoxPieceInfoList::const_iterator it = _pieceInfoList.begin(); 
                                           it != _pieceInfoList.end(); 
                                         ++it )
    {
        if (    (*it)->type == hoxPIECE_TYPE_KING 
             && (*it)->color == color )
        {
            return (*it);
        }
    }

    return NULL;
}
