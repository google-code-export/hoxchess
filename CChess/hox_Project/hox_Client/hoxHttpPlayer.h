/////////////////////////////////////////////////////////////////////////////
// Name:            hoxHttpPlayer.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/28/2007
//
// Description:     The HTTP Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_HTTP_PLAYER_H_
#define __INCLUDED_HOX_HTTP_PLAYER_H_

#include <wx/wx.h>
#include "hoxLocalPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/* Forward declarations */
class hoxHttpConnection;

/** 
 * WWW (response) event-type for responses.
 */
DECLARE_EVENT_TYPE(hoxEVT_WWW_RESPONSE, wxID_ANY)

/**
 * The HTTP player.
 */

class hoxHttpPlayer :  public hoxLocalPlayer
{
public:
    hoxHttpPlayer(); // DUMMY default constructor required for event handler.
    hoxHttpPlayer( const wxString& name,
                   hoxPlayerType   type,
                   int             score = 1500);

    virtual ~hoxHttpPlayer();

    /*******************************************
     * Override the parent's API
     *******************************************/

protected:
    virtual const wxString BuildRequestContent( const wxString& commandStr );

    virtual hoxConnection* CreateNewConnection( const wxString& sHostname, 
                                                int             nPort );

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

public:
    virtual hoxResult JoinTable( hoxTable* table );
    virtual hoxResult LeaveTable( hoxTable* table );

    /*******************************
     * Income-data event handlers
     *******************************/

    void OnWWWResponse(wxCommandEvent& event);

    /*******************************
     * Other API
     *******************************/

    void OnTimer( wxTimerEvent& WXUNUSED(event) );

private:
    void _StartWWWThread();

private:

    wxTimer        m_timer;    // to poll the WWW server to events.


    DECLARE_CLASS(hoxHttpPlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_HTTP_PLAYER_H_ */
