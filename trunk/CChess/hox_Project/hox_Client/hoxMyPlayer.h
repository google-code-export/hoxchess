/////////////////////////////////////////////////////////////////////////////
// Name:            hoxMyPlayer.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/23/2007
//
// Description:     The new "advanced" LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_MY_PLAYER_H_
#define __INCLUDED_HOX_MY_PLAYER_H_

#include "wx/wx.h"
#include "wx/socket.h"
#include "hoxPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/* Forward declarations */
class hoxConnection;

/**
 * The MY player.
 */

class hoxMyPlayer :  public hoxPlayer
{
  public:
    hoxMyPlayer(); // Default constructor required for event handler.
    hoxMyPlayer( const wxString& name,
                 hoxPlayerType   type,
                 int             score = 1500);

    virtual ~hoxMyPlayer();

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    //virtual hoxResult DisconnectFromNetwork();
    //virtual void      HandleSocketLostEvent( wxSocketBase* sock );

    /*******************************
     * LOCAL-specific Network API
     *******************************/

    hoxResult ConnectToNetworkServer( const wxString& sHostname, 
                                      int             nPort, 
                                      wxEvtHandler*   sender );

    hoxResult QueryForNetworkTables( wxEvtHandler* sender );
    hoxResult JoinNetworkTable( const wxString& tableId,
                                wxEvtHandler*   sender );
    hoxResult OpenNewNetworkTable( wxEvtHandler*   sender );
    hoxResult LeaveNetworkTable( const wxString& tableId,
                                 wxEvtHandler*   sender );

    hoxResult StartListenForMoves( wxEvtHandler* sender );

    hoxResult ProcessIncomingData( wxSocketEvent& event );

    virtual void OnNewMove_FromTable( hoxPlayerEvent&  event );

    //hoxResult ConnectToNetworkServer( const wxString& sHostname, int nPort );
    //hoxResult QueryForNetworkTables( hoxNetworkTableInfoList& tableList );
    //hoxResult JoinNetworkTable( const wxString& tableId );

private:
    void _StartConnection();

private:
    wxString         m_sHostname; 
    int              m_nPort;

    wxTimer          m_timer;    // to poll the WWW server to events.

    hoxConnection*   m_connection;


    DECLARE_CLASS(hoxMyPlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_MY_PLAYER_H_ */
