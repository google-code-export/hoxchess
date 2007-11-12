/////////////////////////////////////////////////////////////////////////////
// Name:            hoxReferee.h
// Program's Name:  Huy's Open Xiangqi
// Created:         09/30/2007
//
// Implementing the standard Xiangqi referee.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_REFEREE_H_
#define __INCLUDED_HOX_REFEREE_H_

#include "hoxIReferee.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

namespace BoardAPI
{
   class Board;
}

class hoxReferee : public hoxIReferee
{
public:
    hoxReferee();
    virtual ~hoxReferee();

    /*********************************
     * Override base class virtuals
     *********************************/

    virtual void Reset();
    virtual bool ValidateMove(const hoxMove& move);
    virtual void GetGameState( hoxPieceInfoList& pieceInfoList,
                               hoxPieceColor&    nextColor );
    virtual bool GetPieceAtPosition( const hoxPosition& position, 
                                     hoxPieceInfo&      pieceInfo ) const;

private:
    bool _IsValidMove( const hoxMove& move );
    bool _IsValidCapture( const hoxPieceInfo& pieceInfo, 
                          const hoxPosition& newPos);

    bool _IsValidMove_King( const hoxMove& move );
    bool _IsValidMove_Advisor( const hoxMove& move );
    bool _IsValidMove_Elephant( const hoxMove& move );
    bool _IsValidMove_Horse( const hoxMove& move );
    bool _IsValidMove_Chariot( const hoxMove& move );
    bool _IsValidMove_Cannon( const hoxMove& move );
    bool _IsValidMove_Pawn( const hoxMove& move );

    hoxPieceInfo* _RecordMove( const hoxMove& move );
    void _UndoMove( const hoxMove& move, hoxPieceInfo* pCaptured );
    bool _IsKingBeingChecked( hoxPieceColor color );
    bool _IsKingFaceKing() const;

    hoxPieceInfo* _GetPieceAt( const hoxPosition& position ) const;
    const hoxPieceInfo* _GetKing( hoxPieceColor color ) const;

    // ---------- For End game.
    bool _DoesNextMoveExist();
    bool _DoesNextMoveExist_Piece( const hoxPieceInfo& pieceInfo );
    bool _DoesNextMoveExist_King( const hoxPieceInfo& pieceInfo );
    bool _DoesNextMoveExist_Advisor( const hoxPieceInfo& pieceInfo );
    bool _DoesNextMoveExist_Elephant( const hoxPieceInfo& pieceInfo );
    bool _DoesNextMoveExist_Horse( const hoxPieceInfo& pieceInfo );
    bool _DoesNextMoveExist_Chariot( const hoxPieceInfo& pieceInfo );
    bool _DoesNextMoveExist_Cannon( const hoxPieceInfo& pieceInfo );
    bool _DoesNextMoveExist_Pawn( const hoxPieceInfo& pieceInfo );

private:
    BoardAPI::Board*  m_board;  /// Board-Info.

    hoxPieceInfoList _pieceInfoList;

    hoxPieceColor    _nextColor;
        /* Which side (RED or BLACK) will move next? */
};

#endif /* __INCLUDED_HOX_REFEREE_H_ */
