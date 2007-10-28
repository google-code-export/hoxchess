/////////////////////////////////////////////////////////////////////////////
// Name:            hoxHttpConnection.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/28/2007
//
// Description:     The HTTP Connection to help the HTTP player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_HTTP_CONNECTION_H_
#define __INCLUDED_HOX_HTTP_CONNECTION_H_

#include <wx/wx.h>
#include "hoxConnection.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// hoxHttpConnection
// ----------------------------------------------------------------------------

class hoxHttpConnection : public hoxConnection
{
public:
    hoxHttpConnection( const wxString& sHostname,
                  int             nPort );
    ~hoxHttpConnection();

    // Thread execution starts here
    virtual void* Entry();

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void AddRequest( hoxRequest* request );

private:
    hoxRequest* _GetRequest();         
    void        _HandleRequest( hoxRequest* request );
    hoxResult   _SendRequest( const wxString& request, wxString& response );

private:
    bool                  m_shutdownRequested;
                /* Has a shutdown-request been received? */

    wxSemaphore           m_semRequests;
    wxMutex               m_mutexRequests;
    hoxRequestList        m_requests;
};


#endif /* __INCLUDED_HOX_HTTP_CONNECTION_H_ */
