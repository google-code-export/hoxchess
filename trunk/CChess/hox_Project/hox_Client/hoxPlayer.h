/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPlayer.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/06/2007
//
// Description:     The Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_PLAYER_H_
#define __INCLUDED_HOX_PLAYER_H_

#include "wx/wx.h"
#include "wx/socket.h"
#include "hoxEnums.h"
#include "hoxTypes.h"
#include "hoxPlayerEvent.h"

/* Forward declarations */
class hoxTable;

// ----------------------------------------------------------------------------
// Declare player-event types
// ----------------------------------------------------------------------------

BEGIN_DECLARE_EVENT_TYPES()
  DECLARE_EVENT_TYPE(hoxEVT_PLAYER_NOTIFY_BOARD_MOVE, -1)
  DECLARE_EVENT_TYPE(hoxEVT_PLAYER_NEW_MOVE, -1)
  DECLARE_EVENT_TYPE(hoxEVT_PLAYER_MOVE_AVAILABLE, -1)
END_DECLARE_EVENT_TYPES()


// ----------------------------------------------------------------------------
// The Player class
// ----------------------------------------------------------------------------

// NOTE **** According to wxWidgets documentation regarding wxEvtHandler:
//
//         When using multiple inheritance it is imperative that 
//         the wxEvtHandler(-derived) class be the first class inherited 
//         such that the "this" pointer for the overall object will be 
//         identical to the "this" pointer for the wxEvtHandler portion.
//

class hoxPlayer :  public wxEvtHandler
{
  public:
    hoxPlayer(); // Default constructor required for event handler.
    hoxPlayer( const wxString& name,
               hoxPlayerType   type,
               int             score = 1500);

    virtual ~hoxPlayer();

    /***************************
     * Event-handle API
     ***************************/

    virtual void OnNewMove_FromTable( hoxPlayerEvent&  event );
    //void OnNotifyBoardMove( hoxPlayerEvent& event );
    //void OnMoveAvailable( hoxPlayerEvent& event );


    /***************************
     * Accessor API
     ***************************/

    wxString      GetName() const               { return m_name; }
    void          SetName(const wxString& name) { m_name = name; }

    hoxPlayerType GetType() const { return m_type; }

    const hoxRoleList& GetRoles() const { return m_roles; }
    void               AddRole( hoxRole role );
    void               RemoveRole( hoxRole role );
    bool               HasRole( hoxRole role );

    int                GetScore() const    { return m_score; }
    void               SetScore(int score) { m_score = score; }

    
    /***************************
     * Basic action API
     ***************************/

    virtual hoxResult JoinTable( hoxTable* table );
    virtual hoxResult LeaveTable( hoxTable* table );


  private:
    wxString       m_name;   // The player's name.

    hoxPlayerType  m_type;       
            /* Is it a Local, Network,... player? */

    hoxRoleList    m_roles;
            /* Which tables, which side (color) I am playing (RED or BLACK)?
             */

    int            m_score;  // The player's Score.

    DECLARE_CLASS(hoxPlayer)
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// Player event macros
// ----------------------------------------------------------------------------

typedef void (wxEvtHandler::*hoxPlayerEventFunction)(hoxPlayerEvent&);

#define EVT_PLAYER_NOTIFY_BOARD_MOVE(id, fn) DECLARE_EVENT_TABLE_ENTRY(hoxEVT_PLAYER_NOTIFY_BOARD_MOVE, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction)  wxStaticCastEvent( hoxPlayerEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PLAYER_NEW_MOVE(id, fn) DECLARE_EVENT_TABLE_ENTRY(hoxEVT_PLAYER_NEW_MOVE, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction)  wxStaticCastEvent( hoxPlayerEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PLAYER_MOVE_AVAILABLE(id, fn) DECLARE_EVENT_TABLE_ENTRY(hoxEVT_PLAYER_MOVE_AVAILABLE, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction)  wxStaticCastEvent( hoxPlayerEventFunction, & fn ), (wxObject *) NULL ),

#endif /* __INCLUDED_HOX_PLAYER_H_ */
