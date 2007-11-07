/////////////////////////////////////////////////////////////////////////////
// Name:            hoxRemotePlayer.h
// Program's Name:  Huy's Open Xiangqi
// Created:         11/01/2007
//
// Description:     The REMOTE Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_REMOTE_PLAYER_H_
#define __INCLUDED_HOX_REMOTE_PLAYER_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include "hoxPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/* Forward declarations */
class hoxConnection;

/**
 * The REMOTE player.
 */

class hoxRemotePlayer :  public hoxPlayer
{
  public:
    hoxRemotePlayer(); // DUMMY default constructor required for event handler.
    hoxRemotePlayer( const wxString& name,
                     hoxPlayerType   type,
                     int             score );

    virtual ~hoxRemotePlayer();

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/


    /***************************
     * Network API
     ***************************/

    void OnIncomingNetworkData( wxSocketEvent& event );
    void OnConnectionResponse_PlayerData( wxCommandEvent& event ); 

private:

    DECLARE_DYNAMIC_CLASS(hoxRemotePlayer)
    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_REMOTE_PLAYER_H_ */
