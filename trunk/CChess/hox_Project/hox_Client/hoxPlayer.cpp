/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPlayer.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/06/2007
//
// Description:     The Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxPlayer.h"
#include "hoxEnums.h"
#include "hoxTable.h"
#include "hoxTableMgr.h"

#include <algorithm>

//----------------------------------------------------------------------------
// event types
//----------------------------------------------------------------------------
DEFINE_EVENT_TYPE(hoxEVT_PLAYER_NOTIFY_BOARD_MOVE)
DEFINE_EVENT_TYPE(hoxEVT_PLAYER_NEW_MOVE)
DEFINE_EVENT_TYPE(hoxEVT_PLAYER_MOVE_AVAILABLE)  //not used now!


// user code intercepting the event
IMPLEMENT_DYNAMIC_CLASS( hoxPlayer, wxEvtHandler )

BEGIN_EVENT_TABLE(hoxPlayer, wxEvtHandler)
    EVT_PLAYER_NEW_MOVE (wxID_ANY,  hoxPlayer::OnNewMove_FromTable)
    //EVT_PLAYER_NOTIFY_BOARD_MOVE (wxID_ANY,  hoxPlayer::OnNotifyBoardMove)
    //EVT_PLAYER_MOVE_AVAILABLE (wxID_ANY,  hoxPlayer::OnMoveAvailable)
END_EVENT_TABLE()


//-----------------------------------------------------------------------------
// hoxPlayer
//-----------------------------------------------------------------------------

hoxPlayer::hoxPlayer()
            : m_name( _("Unknown") )
            , m_type( hoxPLAYER_TYPE_DUMMY )
            , m_score( 1500 )
{ 
}

hoxPlayer::hoxPlayer( const wxString& name,
                      hoxPlayerType   type,
                      int             score /* = 1500 */)
            : m_name( name )
            , m_type( type )
            , m_score( score )
{ 
}

hoxPlayer::~hoxPlayer() 
{
    const char* FNAME = "hoxPlayer::~hoxPlayer";
    wxLogDebug(wxString::Format("%s: ENTER.", FNAME));
}

void               
hoxPlayer::AddRole( hoxRole role )
{
    // TODO: Check for duplicate!!! (join same table twice)
    m_roles.push_back( role );
}

void               
hoxPlayer::RemoveRole( hoxRole role )
{
    wxASSERT( this->HasRole( role ) );
    m_roles.remove( role );
}

bool               
hoxPlayer::HasRole( hoxRole role )
{
    hoxRoleList::iterator found 
        = std::find( m_roles.begin(), m_roles.end(), role );
    return ( found != m_roles.end() );
}

// We can check the player's color afterwards to 
// see which role the player has.
hoxResult 
hoxPlayer::JoinTable( hoxTable* table )
{
    wxASSERT( table != NULL );
    // TODO: Check for duplicate!!! (join same table twice)
    hoxResult result = table->AssignPlayer( this );
    return result;
}

hoxResult 
hoxPlayer::LeaveTable( hoxTable* table )
{
    wxASSERT( table != NULL );
    // TODO: Check for duplicate!!! (join same table twice)
    hoxResult result = table->UnassignPlayer( this );
    return result;
}

void 
hoxPlayer::OnNewMove_FromTable( hoxPlayerEvent&  event )
{
    const char* FNAME = "hoxPlayer::OnNewMove_FromTable";
    wxLogDebug(wxString::Format(_("%s: Just a DUMMY player. Do nothing."), FNAME));
}


/************************* END OF FILE ***************************************/
