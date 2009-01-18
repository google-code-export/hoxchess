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

#include <wx/filename.h>
#include "hoxLog.h"

hoxLog::hoxLog( wxWindow*       pParent,
                const wxString& szTitle,
                bool            show /* = true */ )
        : wxLogWindow(pParent, szTitle, show)
{
    m_filename.Printf("%s/HOXChess.log", wxFileName::GetTempDir());
    wxLogDebug("%s: Log file [%s].", __FUNCTION__, m_filename.c_str());

    if ( ! m_logFile.Open( m_filename, "w" ) )
    {
        wxLogWarning("%s: Fail to open Log file [%s].", __FUNCTION__, m_filename.c_str());
    }
}

void hoxLog::DoLogString( const wxString& szString, 
                          time_t          t )
{
    _LogToDisk( szString, t );

    this->wxLogWindow::DoLogString( szString, t );
}

void
hoxLog::_LogToDisk( const wxString& szString,
                    time_t          t )
{
    if ( m_logFile.IsOpened() )
    {
        wxString msg;
        TimeStamp(&msg);
        msg << szString << '\n';

        m_logFile.Write( msg );
        m_logFile.Flush();
    }
}

/************************* END OF FILE ***************************************/
