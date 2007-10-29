/////////////////////////////////////////////////////////////////////////////
// Name:            hoxSocketConnection.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/28/2007
//
// Description:     The Socket-Connection Thread to help MY player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_SOCKET_CONNECTION_H_
#define __INCLUDED_HOX_SOCKET_CONNECTION_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include "hoxConnection.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// hoxSocketConnection
// ----------------------------------------------------------------------------

class hoxSocketConnection : public hoxConnection
{
public:
    hoxSocketConnection( const wxString& sHostname,
                         int             nPort );
    ~hoxSocketConnection();

protected:
    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void HandleRequest( hoxRequest* request );

private:
    void        _HandleRequest( hoxRequest* request );
    hoxResult   _SendRequest( const wxString& request, wxString& response );
    hoxResult   _SendRequest_Connect( const wxString& request, wxString& response );
    
    hoxResult   _HandleRequest_Listen( hoxRequest* request );

    void        _Disconnect();

private:
    wxSocketClient*       m_pSClient;
                /* The socket to handle network connections */

};


#endif /* __INCLUDED_HOX_SOCKET_CONNECTION_H_ */
