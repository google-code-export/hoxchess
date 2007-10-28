/////////////////////////////////////////////////////////////////////////////
// Name:            hoxMyPlayer.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/23/2007
//
// Description:     The new "advanced" LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_MY_PLAYER_H_
#define __INCLUDED_HOX_MY_PLAYER_H_

#include <wx/wx.h>
#include "hoxPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/* Forward declarations */
class hoxConnection;

/** 
 * Connection event-type for responses.
 */
DECLARE_EVENT_TYPE(hoxEVT_CONNECTION_RESPONSE, wxID_ANY)

/**
 * The MY player.
 */

class hoxMyPlayer :  public hoxPlayer
{
  public:
    hoxMyPlayer(); // DUMMY default constructor required for event handler.
    hoxMyPlayer( const wxString& name,
                 hoxPlayerType   type,
                 int             score = 1500);

    virtual ~hoxMyPlayer();

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void OnNewMove_FromTable( hoxPlayerEvent&  event );

    /*******************************
     * MY-specific Network API
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

    /*******************************
     * Socket-event handlers
     *******************************/

    void OnIncomingNetworkData( wxSocketEvent& event );

    /*******************************
     * Other API
     *******************************/

    hoxResult StartListenForMoves();

private:
    void _StartConnection();

private:
    wxString         m_sHostname; 
    int              m_nPort;

    hoxConnection*   m_connection;


    DECLARE_CLASS(hoxMyPlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_MY_PLAYER_H_ */
