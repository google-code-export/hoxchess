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
#include "hoxThreadConnection.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// hoxSocketConnection
// ----------------------------------------------------------------------------

class hoxSocketConnection : public hoxThreadConnection
{
public:
    hoxSocketConnection(); // DUMMY default constructor required for RTTI info.
    hoxSocketConnection( const wxString& sHostname,
                         int             nPort );
    virtual ~hoxSocketConnection();

protected:
    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void HandleRequest( hoxRequest* request );

private:
    void        _HandleRequest( hoxRequest* request );
    hoxResult   _CheckAndHandleSocketLostEvent( const hoxRequest* request, 
                                                wxString&         response );
    hoxResult   _Connect();
    void        _Disconnect();

private:
    wxSocketClient*       m_pSClient;
                /* The socket to handle network connections */

    bool                  m_bConnected;
                /* Has the connection been established with the server */

    DECLARE_DYNAMIC_CLASS(hoxSocketConnection)
};


#endif /* __INCLUDED_HOX_SOCKET_CONNECTION_H_ */
