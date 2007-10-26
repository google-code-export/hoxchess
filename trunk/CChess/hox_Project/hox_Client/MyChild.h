/////////////////////////////////////////////////////////////////////////////
// Name:            MyChild.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/20/2007
//
// Description:     The child Window for the Client.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_MY_CHILD_H_
#define __INCLUDED_MY_CHILD_H_

#include "wx/wx.h"


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

    void SetupMenu();
    void SetupStatusBar();

    void OnToggle(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnChangeSize(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnMove(wxMoveEvent& event);
    void OnClose(wxCloseEvent& event);

    void SetTable(hoxTable* table);

private:

private:
    hoxTable* m_table;

    DECLARE_EVENT_TABLE()
};

#endif  /* __INCLUDED_MY_CHILD_H_ */
