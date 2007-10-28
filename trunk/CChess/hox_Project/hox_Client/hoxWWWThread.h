/////////////////////////////////////////////////////////////////////////////
// Name:            hoxWWWThread.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/21/2007
//
// Description:     The WWW Thread to help the WWW player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_WWW_THREAD_H_
#define __INCLUDED_HOX_WWW_THREAD_H_

#include "wx/wx.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// WWW worker thread
// ----------------------------------------------------------------------------

/** !!!!!!!!!!!!!!!!!!!!!!!
 * NOTE: If deriving from wxThread, acquiring mutex (using lock) would fail.
 *       Thus, I have to derive from wxThreadHelper.
 * !!!!!!!!!!!!!!!!!!!!!!! */
class hoxWWWThread : public wxThreadHelper
{
public:
    hoxWWWThread( const wxString& sHostname,
                  int             nPort );
    ~hoxWWWThread();

    // Thread execution starts here
    virtual void* Entry();

public:
    // **** My own public API ****

    void AddRequest( hoxRequest* request );

private:
    hoxRequest* _GetRequest();         
    void        _HandleRequest( hoxRequest* request );
    hoxResult   _SendRequest( const wxString& request, wxString& response );

private:
    wxString              m_sHostname; 
    int                   m_nPort;

    bool                  m_shutdownRequested;
                /* Has a shutdown-request been received? */

    wxSemaphore           m_semRequests;
    wxMutex               m_mutexRequests;
    hoxRequestList        m_requests;
};


#endif /* __INCLUDED_HOX_WWW_THREAD_H_ */
