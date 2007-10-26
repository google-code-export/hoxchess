/////////////////////////////////////////////////////////////////////////////
// Name:            hoxNetworkPlayer.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/09/2007
//
// Description:     The NETWORK Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_NETWORK_PLAYER_H_
#define __INCLUDED_HOX_NETWORK_PLAYER_H_

#include "wx/wx.h"
#include "wx/socket.h"
#include "hoxPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"


/**
 * The NETWORK player.
 */

class hoxNetworkPlayer :  public hoxPlayer
{
  public:
    hoxNetworkPlayer(); // Default constructor required for event handler.
    hoxNetworkPlayer( const wxString& name,
                      hoxPlayerType   type,
                      int             score = 1500);

    virtual ~hoxNetworkPlayer();

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void OnNewMove_FromTable( hoxPlayerEvent&  event );

    /***************************
     * Network API
     ***************************/

    virtual void OnClientSocketEvent( wxSocketEvent& event );
    virtual void HandleNewMove_FromNetwork( wxSocketBase* sock );

    virtual void      HandleSocketLostEvent( wxSocketBase* sock );
    virtual hoxResult DisconnectFromNetwork();

    wxSocketBase* GetCBSocket() const { return m_pCBSock; }
    hoxResult     SetCBSocket( wxSocketBase* socket );

    ////////////////////
    void HandleCommand_Leave( wxSocketBase*      sock, 
                              hoxCommand&  command );
    void HandleCommand_Move( wxSocketBase*      sock, 
                             hoxCommand&  command );
    ////////////

  private:

    wxSocketBase*     m_pCBSock; 
        /* Callback socket for only NETWORK player.  */


    DECLARE_CLASS(hoxNetworkPlayer)
    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_NETWORK_PLAYER_H_ */
