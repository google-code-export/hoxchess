/***************************************************************************
 *  Copyright 2007, 2008 Huy Phan  <huyphan@playxiangqi.com>               *
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

#include "hoxTypes.h"

/* Forward declarations */
class hoxSite;

/**
 * The child (MDI) frame.
 *
 * @see MyFrame
 */
class MyChild: public wxMDIChildFrame
{
public:
    MyChild( wxMDIParentFrame* parent, 
		     const wxString&   title,
			 const wxPoint&    pos = wxDefaultPosition,
			 const wxSize&     size = wxDefaultSize);
    ~MyChild() {}

    void OnToggle(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    void SetTable(hoxTable_SPtr pTable);
    
    hoxSite* GetSite() const;

    bool IsMyTable( const wxString& sTableId ) const;

private:
    hoxTable_SPtr m_pTable;

    DECLARE_EVENT_TABLE()
};

#endif  /* __INCLUDED_MY_CHILD_H_ */
