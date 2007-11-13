/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPosition.h
// Program's Name:  Huy's Open Xiangqi
// Created:         09/28/2007
//
// Description:     Representing a Position of Piece on Board.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_POSITION_H_
#define __INCLUDED_HOX_POSITION_H_

#include "hoxEnums.h"

// hoxPosition

class hoxPosition
{
public:
    char x;
    char y;

public:
    hoxPosition(char xx = -1, char yy = -1) : x(xx), y(yy) {}
    hoxPosition(const hoxPosition& pos);
    ~hoxPosition();

    hoxPosition& operator=(const hoxPosition& pos);
    bool operator==(const hoxPosition& pos) const;
    bool operator!=(const hoxPosition& pos) const;

    bool IsValid() const;
    bool IsInsidePalace(hoxPieceColor color) const;
    bool IsInsideCountry(hoxPieceColor color) const;
};

#endif /* __INCLUDED_HOX_POSITION_H_ */
