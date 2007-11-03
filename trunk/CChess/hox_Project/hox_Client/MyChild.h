/////////////////////////////////////////////////////////////////////////////
// Name:            MyChild.h
// Program's Name:  Huy's Open Xiangqi
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
