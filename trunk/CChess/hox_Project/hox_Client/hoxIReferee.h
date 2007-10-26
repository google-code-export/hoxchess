/////////////////////////////////////////////////////////////////////////////
// Name:            hoxIReferre.h
// Program's Name:  Huy's Open Xiangqi
// Created:         09/29/2007
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_IREFEREE_H_
#define __INCLUDED_HOX_IREFEREE_H_

#include "hoxTypes.h"

/**
 * Interface for a referee.
 */
class hoxIReferee
{
  public:
    hoxIReferee() {}
    virtual ~hoxIReferee() {}

    /**
     * Reset the game that this referee is residing over.
     */
    virtual void Reset() = 0;

    /**
     * Validate and record a given move.
     */
    virtual bool ValidateMove(const hoxMove& move) = 0;

    /**
     * Get the current state of the game:
     *   + The info of all 'live' pieces.
     *   + Which side (RED or BLACK) will move next?
     */
    virtual void GetGameState( hoxPieceInfoList& pieceInfoList,
                               hoxPieceColor&    nextColor ) = 0;

    /**
     * Lookup a piece-info at a specified position.
     *
     * @return true if found.
     */
    virtual bool GetPieceAtPosition( const hoxPosition& position, 
                                     hoxPieceInfo&      pieceInfo ) const = 0;
};

#endif /* __INCLUDED_HOX_IREFEREE_H_ */
