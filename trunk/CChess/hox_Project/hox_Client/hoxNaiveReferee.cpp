/////////////////////////////////////////////////////////////////////////////
// Name:            hoxIReferre.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         09/29/2007
/////////////////////////////////////////////////////////////////////////////

#include "hoxNaiveReferee.h"
#include "hoxPiece.h"

hoxNaiveReferee::hoxNaiveReferee()
{
    this->Reset();
}

hoxNaiveReferee::~hoxNaiveReferee()
{
}

void
hoxNaiveReferee::Reset()
{
    _nextColor = hoxPIECE_COLOR_RED;
}

bool 
hoxNaiveReferee::ValidateMove(const hoxMove& move)
{
    /**
     * NOTE: Only check for 'turns'. Otherwise, allow everything.
     */

    /* Check for 'turn' */

    if ( move.piece.color != _nextColor )
    {
        return false; // Error! Wrong turn.
    }

    /* Set the next-turn. */

    _nextColor = ( _nextColor == hoxPIECE_COLOR_RED 
                   ? hoxPIECE_COLOR_BLACK
                   : hoxPIECE_COLOR_RED);

    return true;
}

void 
hoxNaiveReferee::GetGameState( hoxPieceInfoList& pieceInfoList,
                               hoxPieceColor&    nextColor )
{
    wxLogError(_("Not yet implemented."));
}

bool 
hoxNaiveReferee::GetPieceAtPosition( const hoxPosition& position, 
                                     hoxPieceInfo&      pieceInfo ) const
{
    wxLogError(_("Not yet implemented."));
    return false;
}

/************************* END OF FILE ***************************************/
