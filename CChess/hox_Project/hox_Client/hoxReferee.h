/////////////////////////////////////////////////////////////////////////////
// Name:            hoxReferee.h
// Program's Name:  Huy's Open Xiangqi
// Created:         09/30/2007
//
// Description:     Implementing the standard Xiangqi referee.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_REFEREE_H_
#define __INCLUDED_HOX_REFEREE_H_

#include "hoxIReferee.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

namespace BoardInfoAPI
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
    BoardInfoAPI::Board*  m_board;  // Board-Info.

    hoxPieceColor         m_nextColor;
        /* Which side (RED or BLACK) will move next? */
};

#endif /* __INCLUDED_HOX_REFEREE_H_ */
