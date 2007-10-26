/////////////////////////////////////////////////////////////////////////////
// Name:            hoxWWWPlayer.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/21/2007
//
// Description:     The WWW Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_WWW_PLAYER_H_
#define __INCLUDED_HOX_WWW_PLAYER_H_

#include "wx/wx.h"
#include "hoxPlayer.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

/* Forward declarations */
class hoxWWWThread;

// ----------------------------------------------------------------------------
// hoxWWWPlayer
// ----------------------------------------------------------------------------

/** 
 * WWW (response) event-type for polling responses.
 */
DECLARE_EVENT_TYPE(hoxEVT_WWW_RESPONSE, wxID_ANY)

class hoxWWWPlayer :  public hoxPlayer
{
public:
    hoxWWWPlayer(); // Default constructor required for event handler.
    hoxWWWPlayer( const wxString& name,
                  hoxPlayerType   type,
                  int             score = 1500);

    virtual ~hoxWWWPlayer();

    void OnWWWResponse(wxCommandEvent& event);

    /*******************************************
     * Override the parent's event-handler API
     *******************************************/

    virtual void OnNewMove_FromTable( hoxPlayerEvent&  event );

    virtual hoxResult JoinTable( hoxTable* table );
    virtual hoxResult LeaveTable( hoxTable* table );

    /*******************************
     * WWW-specific Network API
     *******************************/

    void OnTimer( wxTimerEvent& WXUNUSED(event) );

    hoxResult ConnectToNetworkServer( const wxString& sHostname, 
                                      int             nPort, 
                                      wxEvtHandler*   sender );

    hoxResult QueryForNetworkTables( wxEvtHandler* sender );

    hoxResult OpenNewNetworkTable( wxEvtHandler* sender );

    hoxResult LeaveNetworkTable( const wxString& tableId,
                                 wxEvtHandler* sender );

    hoxResult JoinNetworkTable( const wxString& tableId,
                                wxEvtHandler* sender );

    wxString GetHostname() const { return m_sHostname; }
    int      GetPort()     const { return m_nPort; }


private:
    void _StartWWWThread();

private:
    wxString       m_sHostname; 
    int            m_nPort;

    wxTimer        m_timer;    // to poll the WWW server to events.

    hoxWWWThread*  m_wwwThread;


    DECLARE_CLASS(hoxWWWPlayer)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_WWW_PLAYER_H_ */
