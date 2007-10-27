/////////////////////////////////////////////////////////////////////////////
// Name:            hoxTypes.h
// Program's Name:  Huy's Open Xiangqi
// Created:         09/30/2007
//
// Description:     Containing simple types commonly used through out 
//                  the project.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_TYPES_H_
#define __INCLUDED_HOX_TYPES_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include <list>
#include <map>

#include "hoxEnums.h"
#include "hoxPosition.h"

/* Forward declarations */
class hoxPlayer;
class hoxTable;
class wxSocketBase;

/**
 * Typedef(s)
 */
typedef std::list<hoxPlayer*>  hoxPlayerList;
typedef std::list<hoxTable*>   hoxTableList; 

/**
 * Representing a player's role.
 * NOTE: A play can have multiple roles.
 *       If he has none, then he must be in a place called "Lobby".
 */
class hoxRole
{
  public:
    wxString          tableId;
    hoxPieceColor     color;  // RED, BLACK, or NONE (OBSERVER)

    hoxRole( const wxString& id, hoxPieceColor c) : tableId( id ), color( c )
        { }

    bool operator==(const hoxRole& other)
    {
        return ( (tableId == other.tableId) && (color == other.color) );
    }
};
typedef std::list<hoxRole>     hoxRoleList; 

/**
 * Representing a piece's info.
 */
class hoxPieceInfo
{
  public:
    hoxPieceType   type;       // What type? (Canon, Soldier, ...)
    hoxPieceColor  color;      // What color? (RED or BLACK)
    hoxPosition    position;   // Position on the Board.

    /**
     * NOTE: We do not store the "active/inactive" state since it is only
     *       needed by the referee who can maintain its own data-type... 
     */

    hoxPieceInfo() {}

    hoxPieceInfo(hoxPieceType  t,
                 hoxPieceColor c,
                 hoxPosition   p)
         : type( t )
         , color( c )
         , position( p )
        { }

    hoxPieceInfo( const hoxPieceInfo& other )
        : type( other.type )
        , color( other.color )
        , position( other.position )
        { }
};

typedef std::list<hoxPieceInfo*> hoxPieceInfoList;


/**
 * Representing a (game) MOVE.
 */
class hoxMove
{
  public:
    hoxPieceInfo   piece;
        /* NOTE: Let's just use this structure to present the involved piece
         *       even though the field [piece.active] MUST always be "active".
         */

    hoxPosition    newPosition;   // Position on the Board.

    hoxMove() {}
};

/**
 * A network table.
 */
class hoxNetworkTableInfo
{
  public:
    wxString  id;         // table-Id.
    int       status;     // table's Status.
    wxString  redId;      // RED player's Id.
    wxString  blackId;    // BLACK player's Id.

    hoxNetworkTableInfo() { }
    hoxNetworkTableInfo(const wxString& a_id) { id = a_id; }
};
typedef std::list<hoxNetworkTableInfo*> hoxNetworkTableInfoList;

/**
 * A network event.
 */
class hoxNetworkEvent
{
  public:
    wxString  id;         // event-Id.
    wxString  pid;        // player-Id (whom this event belongs to).
    wxString  tid;        // table-Id (if applicable).
    int       type;       // event-type.
    wxString  content;    // event-content.

    hoxNetworkEvent() { }
};
typedef std::list<hoxNetworkEvent*> hoxNetworkEventList;


//////////////////////////////////////////////////////////////////
// Data-types required for WWW connection.
//////////////////////////////////////////////////////////////////

class hoxRequest : public wxObject
{
public:
    hoxRequestType  type;
    wxString        content;
    int             flags;
    wxEvtHandler*   sender;
    wxSocketBase*   socket;  // TODO Put it here temporarily
    wxSocketNotify  socketEvent;

    hoxRequest() : type( hoxREQUEST_TYPE_UNKNOWN )
                 , flags( hoxREQUEST_FLAG_NONE )
                 , sender( NULL )
                 , socket( NULL )
                 , socketEvent( wxSOCKET_INPUT ) {}

    hoxRequest(hoxRequestType t, wxEvtHandler* s = NULL) 
                    : type( t )
                    , flags( hoxREQUEST_FLAG_NONE )
                    , sender( s )
                    , socket( NULL )
                    , socketEvent( wxSOCKET_INPUT )  {}
};
typedef std::list<hoxRequest*> hoxRequestList;

class hoxResponse : public wxObject
{
public:
    int       type;
    wxString  content;

    hoxResponse() : type( hoxREQUEST_TYPE_UNKNOWN ) {}
    hoxResponse(int t) : type( t ) {}
};

class hoxCommand : public wxObject
{
public:
    typedef std::map<const wxString, wxString> Parameters;

    hoxRequestType type;
    Parameters     parameters;

    hoxCommand() : type( hoxREQUEST_TYPE_UNKNOWN ) {}
};

#endif /* __INCLUDED_HOX_TYPES_H_ */
