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
// Name:            hoxCheckUpdatesUI.h
// Created:         06/27/2009
//
// Description:     The UI to Check for Updates.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_CHECK_UPDATES_UI_H__
#define __INCLUDED_HOX_CHECK_UPDATES_UI_H__

#include "hoxAsyncSocket.h"
#include <wx/progdlg.h>
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// hoxCheckUpdatesUI
// ----------------------------------------------------------------------------

/**
 * A progress Dialog with Timer built-in.
 */
class hoxCheckUpdatesUI : public wxProgressDialog
{
public:
    hoxCheckUpdatesUI( const wxString& title,
                       const wxString& message,
                       int             maximum = 100,
                       wxWindow*       parent = NULL );
    virtual ~hoxCheckUpdatesUI();

    virtual void Stop();

    void runCheck();

protected:
    void OnTimer( wxTimerEvent& event );
    void OnCheckUpdatesResponse( wxCommandEvent& event );

private:
    const int          m_maximum;     // Progress-MAXIMUM value.
    int                m_timerValue;  // Timer's value.
    wxTimer*           m_timer;       // To keep track of time.

    asio::io_service   m_io_service;
    hoxHttpSocket*     m_pHttpSocket;
    asio::thread*      m_io_service_thread;

    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_CHECK_UPDATES_UI_H__ */
