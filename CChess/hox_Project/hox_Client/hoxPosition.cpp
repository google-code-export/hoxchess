/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPosition.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         09/28/2007
/////////////////////////////////////////////////////////////////////////////

#include "wx/wx.h"

#include "hoxPosition.h"

//
// hoxPosition
//

hoxPosition::hoxPosition(const hoxPosition& pos)
{
  if ( &pos != this )
  {
    x = pos.x;
    y = pos.y;
  }
}

hoxPosition::~hoxPosition()
{
  // Doing nothing
}

hoxPosition& 
hoxPosition::operator=(const hoxPosition& pos)
{
  x = pos.x;
  y = pos.y;
  return *this;
}

bool
hoxPosition::operator==(const hoxPosition& pos) const
{
  return (x == pos.x && y == pos.y);
}

bool
hoxPosition::operator!=(const hoxPosition& pos) const
{
  return (x != pos.x || y != pos.y);
}

bool 
hoxPosition::IsValid() const 
{ 
  return (x >= 0 && x <= 8 && y >= 0 && y <= 9); 
}

bool 
hoxPosition::IsInsidePalace(hoxPieceColor color) const 
{ 
  if (color == hoxPIECE_COLOR_BLACK)
  {
    return (x >= 3 && x <= 5 && y >= 0 && y <= 2); 
  }
  else  // Red?
  {
    return (x >= 3 && x <= 5 && y >= 7 && y <= 9); 
  }
}

// Is inside one's country (not yet cross the river)?
bool 
hoxPosition::IsInsideCountry(hoxPieceColor color) const 
{ 
  if (color == hoxPIECE_COLOR_BLACK)
  {
    return (y >= 0 && y <= 4);
  }
  else  // Red?
  {
    return (y >= 5 && y <= 9);
  }
}
