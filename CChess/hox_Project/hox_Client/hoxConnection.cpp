/***************************************************************************
 *  Copyright 2007-2009 Huy Phan  <huyphan@playxiangqi.com>                *
 *                      Bharatendra Boddu (bharathendra at yahoo dot com)  *
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
// Name:            hoxConnection.cpp
// Created:         11/05/2007
//
// Description:     The Connection which is the base for all connections.
/////////////////////////////////////////////////////////////////////////////

#include "hoxConnection.h"

IMPLEMENT_ABSTRACT_CLASS(hoxConnection, wxObject)

//-----------------------------------------------------------------------------
// hoxConnection
//-----------------------------------------------------------------------------

hoxConnection::hoxConnection( wxEvtHandler* player /* = NULL */ )
        : m_player( player )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

hoxConnection::~hoxConnection()
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

//-----------------------------------------------------------------------------
// hoxLocalConnection
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(hoxLocalConnection, hoxConnection)

hoxLocalConnection::hoxLocalConnection( wxEvtHandler* player /* = NULL */ )
        : hoxConnection( player )
{
}

/************************* END OF FILE ***************************************/
