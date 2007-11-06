/////////////////////////////////////////////////////////////////////////////
// Name:            hoxHostPlayer.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/09/2007
//
// Description:     The HOST Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_HOST_PLAYER_H_
#define __INCLUDED_HOX_HOST_PLAYER_H_

#include <wx/wx.h>
#include "hoxPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"


/**
 * The HOST player.
 */

class hoxHostPlayer :  public hoxPlayer
{
public:
    hoxHostPlayer(); // Default constructor required for event handler.
    hoxHostPlayer( const wxString& name,
                   hoxPlayerType   type,
                   int             score );

    virtual ~hoxHostPlayer();

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void OnNewMove_FromTable( hoxPlayerEvent&  event );

   
private:

    DECLARE_DYNAMIC_CLASS(hoxHostPlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_HOST_PLAYER_H_ */
