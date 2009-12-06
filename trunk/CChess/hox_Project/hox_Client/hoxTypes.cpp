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
// Name:            hoxTypes.cpp
// Created:         03/27/2008
//
// Description:     Containing simple types commonly used through out 
//                  the project.
/////////////////////////////////////////////////////////////////////////////

#include "hoxTypes.h"
#include "hoxUtil.h"

// ----------------------------------------------------------------------------
// hoxPosition
// ----------------------------------------------------------------------------

bool
hoxPosition::operator==(const hoxPosition& pos) const
{
    return (x == pos.x && y == pos.y);
}

bool 
hoxPosition::IsValid() const 
{ 
    return (x >= 0 && x <= 8 && y >= 0 && y <= 9); 
}

bool 
hoxPosition::IsInsidePalace(hoxColor color) const 
{ 
    if (color == hoxCOLOR_BLACK)
    {
        return (x >= 3 && x <= 5 && y >= 0 && y <= 2); 
    }
    else  // Red?
    {
        return (x >= 3 && x <= 5 && y >= 7 && y <= 9); 
    }
}

bool 
hoxPosition::IsInsideCountry(hoxColor color) const 
{ 
    if (color == hoxCOLOR_BLACK) return (y >= 0 && y <= 4);
    else  /* Red? */             return (y >= 5 && y <= 9);
}

// ----------------------------------------------------------------------------
// hoxRequest
// ----------------------------------------------------------------------------

const wxString
hoxRequest::ToString() const
{
	wxString result;

	result += "op=" + hoxUtil::RequestTypeToString( this->type );

	for ( hoxParameters::const_iterator it = this->parameters.begin();
		                                it != this->parameters.end();
                                      ++it )
	{
		result += "&" + it->first + "=" + it->second;
	}
	
	return result;
}


// ----------------------------------------------------------------------------
// hoxRequestQueue
// ----------------------------------------------------------------------------

hoxRequestQueue::hoxRequestQueue()
{
}

hoxRequestQueue::~hoxRequestQueue()
{
    while ( ! m_list.empty() )
    {
        hoxRequest_APtr apRequest( m_list.front() );
        m_list.pop_front();
        wxLogDebug("%s: Deleting request [%s]...", __FUNCTION__, 
            hoxUtil::RequestTypeToString(apRequest->type).c_str());
    }
}

void
hoxRequestQueue::PushBack( hoxRequest_APtr apRequest )
{
    wxMutexLocker lock( m_mutex ); // Gain exclusive access.

    m_list.push_back( apRequest.release() );
}

hoxRequest_APtr
hoxRequestQueue::PopFront()
{
    hoxRequest_APtr apRequest;   // Empty pointer.

    wxMutexLocker lock( m_mutex ); // Gain exclusive access.

    if ( ! m_list.empty() )
    {
        apRequest.reset( m_list.front() );
        m_list.pop_front();
    }

    return apRequest;
}

/************************* END OF FILE ***************************************/
