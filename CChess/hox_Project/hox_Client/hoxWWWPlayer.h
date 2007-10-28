/////////////////////////////////////////////////////////////////////////////
// Name:            hoxWWWPlayer.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/21/2007
//
// Description:     The WWW Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_WWW_PLAYER_H_
#define __INCLUDED_HOX_WWW_PLAYER_H_

#include <wx/wx.h>
#include "hoxPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/* Forward declarations */
class hoxWWWThread;

/** 
 * WWW (response) event-type for responses.
 */
DECLARE_EVENT_TYPE(hoxEVT_WWW_RESPONSE, wxID_ANY)

/**
 * The WWW player.
 */

class hoxWWWPlayer :  public hoxPlayer
{
public:
    hoxWWWPlayer(); // DUMMY default constructor required for event handler.
    hoxWWWPlayer( const wxString& name,
                  hoxPlayerType   type,
                  int             score = 1500);

    virtual ~hoxWWWPlayer();

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void OnNewMove_FromTable( hoxPlayerEvent&  event );

    virtual hoxResult JoinTable( hoxTable* table );
    virtual hoxResult LeaveTable( hoxTable* table );

    /*******************************
     * WWW-specific Network API
     *******************************/

    hoxResult ConnectToNetworkServer( const wxString& sHostname, 
                                      int             nPort, 
                                      wxEvtHandler*   sender );

    hoxResult QueryForNetworkTables( wxEvtHandler* sender );
    hoxResult JoinNetworkTable( const wxString& tableId,
                                wxEvtHandler* sender );
    hoxResult OpenNewNetworkTable( wxEvtHandler* sender );

    hoxResult LeaveNetworkTable( const wxString& tableId,
                                 wxEvtHandler* sender );

    /*******************************
     * Income-data event handlers
     *******************************/

    void OnWWWResponse(wxCommandEvent& event);

    /*******************************
     * Other API
     *******************************/

    void OnTimer( wxTimerEvent& WXUNUSED(event) );

    wxString GetHostname() const { return m_sHostname; }
    int      GetPort()     const { return m_nPort; }

private:
    void _StartWWWThread();

private:
    wxString       m_sHostname; 
    int            m_nPort;

    hoxWWWThread*  m_wwwThread;

    wxTimer        m_timer;    // to poll the WWW server to events.


    DECLARE_CLASS(hoxWWWPlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_WWW_PLAYER_H_ */
