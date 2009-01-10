/***************************************************************************
 *  Copyright 2007, 2008, 2009 Huy Phan  <huyphan@playxiangqi.com>         *
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
// Name:            hoxLog.cpp
// Created:         11/18/2007
//
// Description:     The Log for the Application.
/////////////////////////////////////////////////////////////////////////////

#include <wx/ffile.h>
#include <wx/filename.h>

#include "hoxLog.h"
#include "hoxUtil.h"

/**
 * The default constructor.
 */
hoxLog::hoxLog()
        : wxLog()
{
    const char* FNAME = "hoxLog::hoxLog";

    m_filename = wxFileName::GetTempDir() + "/CChess_"
               + hoxUtil::GenerateRandomString() + ".log";
    wxLogDebug("%s: Opened the log file [%s].", FNAME, m_filename.c_str());
}

/**
 * The destructor.
 */
hoxLog::~hoxLog()
{
}

void 
hoxLog::DoLogString( const wxChar* msg, 
                     time_t        timestamp )
{
    if ( msg == NULL ) return;

    wxFFile logFile( m_filename, "a" );

    if ( logFile.IsOpened() )
    {
        logFile.Write( msg );
        logFile.Write( "\n" );
        logFile.Close();
    }
#if 0
    if ( wxGetApp().GetTopWindow() != NULL )
    {
        wxCommandEvent logEvent( hoxEVT_FRAME_LOG_MSG );
        logEvent.SetString( wxString(msg) );
        ::wxPostEvent( wxGetApp().GetTopWindow(), logEvent );
    }
#endif
}

/************************* END OF FILE ***************************************/
