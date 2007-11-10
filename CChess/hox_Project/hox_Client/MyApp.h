/////////////////////////////////////////////////////////////////////////////
// Name:            MyApp.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/02/2007
//
// Description:     The main Application.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_MY_APP_H_
#define __INCLUDED_MY_APP_H_

#include <wx/wx.h>
#include <wx/ffile.h>
#include "hoxTypes.h"

/* Forward declarations */
class MyFrame;
class hoxSocketServer;
class hoxPlayer;
class hoxTable;
class hoxHttpPlayer;
class hoxMyPlayer;
class hoxServer;

/**
 * @note We go through all these troubles because the wxLogGui is
 *       not thread-safe (would crash under multi-threads running).
 */
class hoxLog : public wxLog
{
public:
    hoxLog();
    ~hoxLog();

    virtual void DoLogString(const wxChar *msg, time_t timestamp);

private:
    wxString   m_filename;
};

/**
 * The main Application.
 * Everything starts from here (i.e., the entry point).
 */

/** 
 * Server (response) event-type.
 */
DECLARE_EVENT_TYPE(hoxEVT_SERVER_RESPONSE, wxID_ANY)

class MyApp : public wxApp
{
public:

    /*********************************
     * Override base class virtuals
     *********************************/

    bool OnInit();
    int  OnExit();

    /*********************************
     * My own API
     *********************************/

    hoxPlayer*     GetHostPlayer() const { return m_pPlayer; }
    hoxMyPlayer*   GetMyPlayer();
    hoxHttpPlayer* GetHTTPPlayer() const;

    void OpenServer(int nPort);
    void CloseServer();

private:
    hoxLog*             m_log;
    wxLog*              m_oldLog;    // the previous log target (to be restored later)

    MyFrame*            m_frame;  // The main frame.

    /* The server part */
    hoxServer*          m_server;
    hoxSocketServer*    m_socketServer;

    hoxPlayer*          m_pPlayer;
            /* The player representing the host 
             * Even though the host may particiate in more than one game
             * at the same time, it is assumed to be run by only ONE player.
             */

    mutable hoxHttpPlayer*  m_httpPlayer;
            /* The player that this Host uses to connect to the HTTP server. */

    mutable hoxMyPlayer*    m_myPlayer;
            /* The player that this Host uses to connect to the server. */


    DECLARE_EVENT_TABLE()
};

// This macro can be used multiple times and just allows you 
// to use wxGetApp() function
DECLARE_APP(MyApp)

#endif  /* __INCLUDED_MY_APP_H_ */
