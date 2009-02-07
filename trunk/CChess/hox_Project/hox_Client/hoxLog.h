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
// Name:            hoxLog.h
// Created:         11/18/2007
//
// Description:     The Log for the Application.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_LOG_H__
#define __INCLUDED_HOX_LOG_H__

#include <wx/log.h>
#include <wx/ffile.h>

/**
 * The Log of the Application.
 * NOTE: Please careful with the fact that it MAY not be safe to log
 *       message coming from secondary threads directly to any GUI controls.
 *
 * @note We go through all these troubles because the wxLogGui is
 *       not thread-safe (would crash under multi-threads running).
 */
class hoxLog : public wxLogWindow
{
public:
    hoxLog( wxWindow*       pParent,
            const wxString& szTitle,
            bool            show = true );
    virtual ~hoxLog() {}

public:
    /**
     * Returns true if the window is shown, false if it has been hidden.
     */
    virtual bool IsShown() const { return m_bIsShown; }

    /**
     * (Override the parent's API)
     *
     * Show or hide a log's frame.
     */
    void Show( bool show = true );

protected:
    /***********************************
     * Override the parent's API.
     ***********************************/

    /**
     * Log a given string with a timestamp.
     */
    virtual void DoLogString( const wxString& szString, 
                              time_t          t );

    /**
     * Called if the user closes the window interactively,
     * will not be called if it is destroyed for another reason
     * (such as when program exits).
     */
    virtual bool OnFrameClose( wxFrame* frame );

private:
    void _LogToDisk( const wxString& szString,
                     time_t          t );

private:
    wxString   m_filename;
    wxFFile    m_logFile;
    bool       m_bIsShown;  // Is the Log shown on the Screen?
};

#endif  /* __INCLUDED_HOX_LOG_H__ */
