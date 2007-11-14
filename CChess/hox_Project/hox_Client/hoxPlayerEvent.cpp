/***************************************************************************
 *  Copyright 2007 Huy Phan  <huyphan@playxiangqi.com>                     *
 *                                                                         * 
 *  This file is part of HOXChess.                                         *
 *                                                                         *
 *  HOXChess is free software: you can redistribute it and/or modify       *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  HOXChess is distributed in the hope that it will be useful,            *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with HOXChess.  If not, see <http://www.gnu.org/licenses/>.      *
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPlayerEvent.cpp
// Created:         10/10/2007
//
// Description:     The Player-Event.
/////////////////////////////////////////////////////////////////////////////

#include "hoxPlayerEvent.h"
#include "hoxEnums.h"

//-----------------------------------------------------------------------------
// hoxPlayerEvent
//-----------------------------------------------------------------------------

hoxPlayerEvent::hoxPlayerEvent( wxEventType commandType, 
                                int         id )
        : wxNotifyEvent( commandType, id )
{
    m_pPiece = NULL;
}

wxEvent*
hoxPlayerEvent::Clone() const 
{ 
    hoxPlayerEvent* newEvent = new hoxPlayerEvent(*this);
    newEvent->m_pPiece = m_pPiece;
    newEvent->m_position = m_position;
    return newEvent; 
}


/************************* END OF FILE ***************************************/
