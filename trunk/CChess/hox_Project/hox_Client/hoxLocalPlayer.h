/////////////////////////////////////////////////////////////////////////////
// Name:            hoxLocalPlayer.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/09/2007
//
// Description:     The LOCAL Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_LOCAL_PLAYER_H_
#define __INCLUDED_HOX_LOCAL_PLAYER_H_

#include "wx/wx.h"
#include "wx/socket.h"
#include "hoxNetworkPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/**
 * The LOCAL player.
 */

class hoxLocalPlayer :  public hoxNetworkPlayer
{
  public:
    hoxLocalPlayer(); // Default constructor required for event handler.
    hoxLocalPlayer( const wxString& name,
                    hoxPlayerType   type,
                    int             score = 1500);

    virtual ~hoxLocalPlayer();

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual hoxResult DisconnectFromNetwork();
    virtual void      HandleSocketLostEvent( wxSocketBase* sock );

    /*******************************
     * LOCAL-specific Network API
     *******************************/

    hoxResult ConnectToNetworkServer( const wxString& sHostname, int nPort );
    hoxResult QueryForNetworkTables( hoxNetworkTableInfoList& tableList );
    hoxResult JoinNetworkTable( const wxString& tableId );

  private:

    wxSocketClient*   m_pSClient;
        /* NULL if this player is not a part of any network server.
         * Otherwise, this member variable establishes a connection with a network.
         * The class is responsible for managing this connection.
         */


    DECLARE_CLASS(hoxLocalPlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_LOCAL_PLAYER_H_ */
