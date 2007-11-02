/////////////////////////////////////////////////////////////////////////////
// Name:            MyFrame.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/02/2007
//
// Description:     The main Frame of the App.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_MY_FRAME_H_
#define __INCLUDED_MY_FRAME_H_

#include <wx/wx.h>
#include <wx/laywin.h>   // wxSashLayoutWindow
#include <wx/progdlg.h>
#include <list>

/* Forward declarations */
class hoxNetworkTableInfo;
class hoxTable;
class MyChild;

// menu items ids
enum
{
    MDI_QUIT = wxID_EXIT,
    MDI_NEW_WINDOW = 101,

    MDI_OPEN_SERVER,    // Open server
    MDI_CONNECT_SERVER, // Connect to server
    MDI_DISCONNECT_SERVER, // Disconnect from server

    MDI_CONNECT_HTTP_SERVER,

    MDI_TOGGLE,   // toggle view
    MDI_CHILD_QUIT,
    MDI_ABOUT = wxID_ABOUT
};

/** 
 * Log events
 */
DECLARE_EVENT_TYPE(hoxEVT_FRAME_LOG_MSG, wxID_ANY)

/*
 * Define main (MDI) frame
 */
class MyFrame : public wxMDIParentFrame
{
    typedef std::list<MyChild*> MyChildList;

public:
    MyFrame( wxWindow*          parent, 
             const wxWindowID   id, 
             const wxString&    title,
             const wxPoint&     pos, 
             const wxSize&      size, 
             const long         style );

    ~MyFrame();

    // -----
    void SetupMenu();
    void SetupStatusBar();

    void InitToolBar(wxToolBar* toolBar);

    void OnSize(wxSizeEvent& event);
    void OnSashDrag(wxSashEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnNewWindow(wxCommandEvent& event);

    void OnOpenServer(wxCommandEvent& event);
    void OnConnectServer(wxCommandEvent& event);
    void OnDisconnectServer(wxCommandEvent& event);

    void OnConnectHTTPServer(wxCommandEvent& event);

    void OnQuit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    /**
     * Invoke by the Child window when it is being closed.
     * NOTE: This *special* API is used by the Child to ask the Parent
     *       for the permission to close.
     *       !!! BE CAREFUL with recursive calls. !!!
     */
    bool OnChildClose(MyChild* child, hoxTable* table);

    void DoJoinNewHTTPTable(const wxString& tableId);
    void DoJoinExistingHTTPTable(const hoxNetworkTableInfo& tableInfo);

    void OnHTTPResponse(wxCommandEvent& event);
    void OnMYResponse(wxCommandEvent& event);
    void DoJoinExistingMYTable(const hoxNetworkTableInfo& tableInfo);
    void DoJoinNewMYTable(const wxString& tableId);
    void OnFrameLogMsgEvent( wxCommandEvent &event );

private:
    void _OnHTTPResponse_Connect( const wxString& responseStr );
    void _OnHTTPResponse_List( const wxString& responseStr );
    void _OnHTTPResponse_New( const wxString& responseStr );
    void _OnHTTPResponse_Join( const wxString& responseStr );
    void _OnHTTPResponse_Leave( const wxString& responseStr );

    void _OnMYResponse_Connect( const wxString& responseStr );
    void _OnMYResponse_List( const wxString& responseStr );
    void _OnMYResponse_Join( const wxString& responseStr );
    void _OnMYResponse_New( const wxString& responseStr );

    hoxTable* _CreateNewTable( const wxString& tableId );

private:
    // Logging.  
    wxSashLayoutWindow* m_logWindow; // To contain the log-text below.
    wxTextCtrl*         m_logText;   // Log window for debugging purpose.

    // the progress dialog which we show while worker thread is running
    wxProgressDialog*   m_dlgProgress;

    int                 m_nChildren;   // The number of child-frames.
    MyChildList         m_children;

    DECLARE_EVENT_TABLE()
};


#endif  /* __INCLUDED_MY_FRAME_H_ */
