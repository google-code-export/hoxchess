/////////////////////////////////////////////////////////////////////////////
// Name:            hoxThreadConnection.h
// Program's Name:  Huy's Open Xiangqi
// Created:         11/05/2007
//
// Description:     The "base" Connection Thread to help a "network" player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_THREAD_CONNECTION_H_
#define __INCLUDED_HOX_THREAD_CONNECTION_H_

#include <wx/wx.h>
#include <wx/thread.h>
#include "hoxConnection.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// hoxThreadConnection
// ----------------------------------------------------------------------------

/** !!!!!!!!!!!!!!!!!!!!!!!
 * NOTE: If deriving from wxThread, acquiring mutex (using lock) would fail.
 *       Thus, I have to derive from wxThreadHelper.
 * !!!!!!!!!!!!!!!!!!!!!!! */
class hoxThreadConnection : public hoxConnection
                          , public wxThreadHelper
{
public:
    hoxThreadConnection(); // DUMMY default constructor required for RTTI info.
    hoxThreadConnection( const wxString& sHostname,
                         int             nPort );
    virtual ~hoxThreadConnection();

    // **** Override the parent's API ****
    virtual void Start();
    virtual void Shutdown();
    virtual void AddRequest( hoxRequest* request );

    // Thread execution starts here
    virtual void* Entry();

public:
    // **** My own public API ****

protected:
    virtual hoxRequest* GetRequest();         
    virtual void        HandleRequest( hoxRequest* request ) = 0;

protected:
    wxString              m_sHostname; 
    int                   m_nPort;

    bool                  m_shutdownRequested;
                /* Has a shutdown-request been received? */

    wxSemaphore           m_semRequests;
    wxMutex               m_mutexRequests;
    hoxRequestList        m_requests;

    DECLARE_ABSTRACT_CLASS(hoxThreadConnection)
};


#endif /* __INCLUDED_HOX_THREAD_CONNECTION_H_ */
