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
// Name:            MyChild.h
// Created:         10/20/2007
//
// Description:     The child Window for the Client.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_MY_CHILD_H_
#define __INCLUDED_MY_CHILD_H_

#include <wx/wx.h>


/* Forward declarations */
class hoxTable;

/*
 * My child frame.
 */
class MyChild: public wxMDIChildFrame
{
public:
    MyChild(wxMDIParentFrame *parent, const wxString& title);
    ~MyChild();

    void OnToggle(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnSize(wxSizeEvent& event);

    void SetTable(hoxTable* table);

private:
    void _SetupMenu();
    void _SetupStatusBar();

private:
    hoxTable* m_table;

    DECLARE_EVENT_TABLE()
};

#endif  /* __INCLUDED_MY_CHILD_H_ */
