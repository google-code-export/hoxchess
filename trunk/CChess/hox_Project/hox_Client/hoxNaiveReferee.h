/////////////////////////////////////////////////////////////////////////////
// Name:            hoxIReferre.h
// Program's Name:  Huy's Open Xiangqi
// Created:         09/29/2007
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_NAIVE_REFEREE_H_
#define __INCLUDED_HOX_NAIVE_REFEREE_H_

#include "hoxIReferee.h"
#include "hoxEnums.h"


class hoxNaiveReferee : public hoxIReferee
{
  public:
    hoxNaiveReferee();
    ~hoxNaiveReferee();

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
    hoxPieceColor    _nextColor;
        /* Whose's turn (RED or BLACK) is it to move? */
};

#endif /* __INCLUDED_HOX_NAIVE_REFEREE_H_ */
