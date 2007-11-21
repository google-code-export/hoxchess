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
// Name:            hoxHostPlayer.h
// Created:         10/09/2007
//
// Description:     The HOST Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_HOST_PLAYER_H_
#define __INCLUDED_HOX_HOST_PLAYER_H_

#include <wx/wx.h>
#include "hoxPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"


/**
 * The HOST player.
 */

class hoxHostPlayer :  public hoxPlayer
{
public:
    hoxHostPlayer(); // Default constructor required for event handler.
    hoxHostPlayer( const wxString& name,
                   hoxPlayerType   type,
                   int             score );

    virtual ~hoxHostPlayer();

private:

    DECLARE_DYNAMIC_CLASS(hoxHostPlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_HOST_PLAYER_H_ */
