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
// Name:            hoxHostPlayer.cpp
// Created:         10/09/2007
//
// Description:     The Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxHostPlayer.h"
#include "hoxEnums.h"

IMPLEMENT_DYNAMIC_CLASS(hoxHostPlayer, hoxPlayer)

BEGIN_EVENT_TABLE(hoxHostPlayer, hoxPlayer)
    // Need to have a table even though it is empty.
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxHostPlayer
//-----------------------------------------------------------------------------

hoxHostPlayer::hoxHostPlayer()
{
    wxFAIL_MSG( "This default constructor is never meant to be used." );
}

hoxHostPlayer::hoxHostPlayer( const wxString& name,
                              hoxPlayerType   type,
                              int             score )
            : hoxPlayer( name, type, score )
{ 
}

hoxHostPlayer::~hoxHostPlayer() 
{
}

/************************* END OF FILE ***************************************/
