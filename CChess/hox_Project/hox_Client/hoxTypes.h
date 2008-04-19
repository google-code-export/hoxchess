/***************************************************************************
 *  Copyright 2007, 2008 Huy Phan  <huyphan@playxiangqi.com>               *
 *                                                                         * 
 *  This file is part of HOXChess.                                         *
 *                                                                         *
 *  HOXChess is free software: you can redistribute it and/or modify       *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  HOXChess is distributed in the hope that it will be useful,            *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with HOXChess.  If not, see <http://www.gnu.org/licenses/>.      *
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// Name:            hoxTypes.h
// Created:         09/30/2007
//
// Description:     Containing simple types commonly used through out 
//                  the project.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_TYPES_H_
#define __INCLUDED_HOX_TYPES_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include <boost/shared_ptr.hpp>
#include <list>
#include <vector>
#include <map>

#include "hoxEnums.h"
#include "hoxPosition.h"

/* Forward declarations */
class hoxPlayer;
class hoxConnection;
class hoxTable;
class hoxIReferee;

/**
 * Typedef(s)
 */
typedef boost::shared_ptr<hoxTable> hoxTable_SPtr;

typedef std::list<hoxPlayer*>     hoxPlayerList;
typedef std::list<hoxTable_SPtr>  hoxTableList; 

typedef std::auto_ptr<hoxConnection> hoxConnection_APtr;

typedef boost::shared_ptr<hoxIReferee> hoxIReferee_SPtr;

typedef std::list<wxString> hoxStringList;

/**
 * Representing a player's role.
 * NOTE: A play can have multiple roles.
 *       If he has none, then he must be in a place called "Lobby".
 */
class hoxRole
{
  public:
    wxString          tableId;
    hoxColor          color;  // RED, BLACK, or NONE (OBSERVER)

    hoxRole( const wxString& id, hoxColor c) : tableId( id ), color( c )
        { }

    bool operator==(const hoxRole& other) const
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
    hoxColor       color;      // What color? (RED or BLACK)
    hoxPosition    position;   // Position on the Board.

    /**
     * NOTE: We do not store the "active/inactive" state since it is only
     *       needed by the referee who can maintain its own data-type... 
     */

    hoxPieceInfo()
         : type( hoxPIECE_INVALID )
         , color( hoxCOLOR_NONE )
         , position( -1, -1 )
        { }

    hoxPieceInfo(hoxPieceType  t,
                 hoxColor c,
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

    bool IsValid() const { return type != hoxPIECE_INVALID; }
};

typedef std::list<hoxPieceInfo> hoxPieceInfoList;


/**
 * Representing a (game) MOVE.
 */
class hoxMove
{
  public:
    hoxPieceInfo    piece;        // The Piece that moves.
    hoxPosition     newPosition;  // Position on the Board.

    hoxPieceInfo    capturedPiece; 
        /* The Piece being captured as a result of this Moves. 
         * This information is currently filled in by the Referee.
         */

    hoxMove() {}
    bool IsValid() const { return piece.IsValid(); }
    void SetCapturedPiece( const hoxPieceInfo& captured ) 
        { capturedPiece = captured; }
    bool IsAPieceCaptured() const 
        { return capturedPiece.IsValid(); }

    const wxString ToString() const
    {
        wxString moveStr;
        moveStr.Printf("%d%d%d%d", piece.position.x, piece.position.y,
                                   newPosition.x, newPosition.y); 
        return moveStr;
    }
};

typedef std::list<hoxMove>    hoxMoveList;
typedef std::vector<hoxMove>  hoxMoveVector;

/**
 * Game's Time-info.
 */
class hoxTimeInfo
{
  public:
    int  nGame;  // Game-time.
    int  nMove;  // Move-time.
    int  nFree;  // Free-time.

	hoxTimeInfo() : nGame(0), nMove(0), nFree(0) {}
	void Clear()
		{
			nGame = 0;
			nMove = 0;
			nFree = 0;
		}
    bool IsEmpty() const 
        { return (nGame == 0) && (nMove == 0) && (nFree == 0); }
};

/**
 * A network table.
 */
class hoxNetworkTableInfo
{
  public:
    wxString      id;         // table-Id.
	hoxGameGroup  group;      // Public / Private
    wxString      redId;      // RED player's Id.
    wxString      blackId;    // BLACK player's Id.
	wxString      redScore;   // RED player's Score.
	wxString      blackScore;  // BLACK player's Score.
	hoxTimeInfo   initialTime; // The initial allowed Game-Time.
	hoxTimeInfo   blackTime;
	hoxTimeInfo   redTime;
	hoxGameType   gameType;    // Rated / Unrated / Solo

	hoxNetworkTableInfo() 
		: group(hoxGAME_GROUP_PUBLIC)
	    , gameType( hoxGAME_TYPE_UNKNOWN ) {}
	hoxNetworkTableInfo(const wxString& a_id) 
		: group(hoxGAME_GROUP_PUBLIC)
	    , gameType( hoxGAME_TYPE_UNKNOWN ) { id = a_id; }
	void Clear()
		{
			id = "";
			group = hoxGAME_GROUP_PUBLIC;
			redId = "";
			blackId = "";
			redScore = "";
			blackScore = "";
			initialTime.Clear();
			blackTime.Clear();
			redTime.Clear();
			gameType = hoxGAME_TYPE_UNKNOWN;
		}
};
typedef std::list<hoxNetworkTableInfo> hoxNetworkTableInfoList;

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
// Data-types required for Connection Thread.
//////////////////////////////////////////////////////////////////

class hoxCommand : public wxObject
{
public:
    typedef std::map<const wxString, wxString> Parameters;

    hoxRequestType type;
    Parameters     parameters;

    hoxCommand( hoxRequestType t = hoxREQUEST_UNKNOWN ) : type( t ) {}
};

class hoxRequest : public wxObject
{
public:
    hoxRequestType  type;
    wxEvtHandler*   sender;
	hoxCommand::Parameters parameters;

    hoxRequest() : type( hoxREQUEST_UNKNOWN )
                 , sender( NULL ) {}

    hoxRequest(hoxRequestType t, wxEvtHandler* s = NULL) 
                    : type( t )
                    , sender( s ) {}

    const wxString ToString() const;

};
typedef std::auto_ptr<hoxRequest> hoxRequest_APtr;
typedef std::list<hoxRequest*>    hoxRequestList;

class hoxResponse : public wxObject
{
public:
    hoxRequestType   type;
    hoxResult        code;
    wxString         content;
    wxEvtHandler*    sender;

    hoxResponse() : type( hoxREQUEST_UNKNOWN )
                  , code( hoxRC_UNKNOWN )
                  , sender( NULL ) {}
    hoxResponse(hoxRequestType t, wxEvtHandler* s = NULL) 
                  : type( t )
                  , code( hoxRC_UNKNOWN )
                  , sender( s ) {}
};
typedef std::auto_ptr<hoxResponse>     hoxResponse_APtr;
typedef boost::shared_ptr<hoxResponse> hoxResponse_SPtr;

/**
 * Representing a server address.
 */
class hoxServerAddress
{
public:
    wxString   name;
    int        port;

    hoxServerAddress() : port( 0 ) {}
    hoxServerAddress(const wxString& n, int p) : name( n ), port( p ) {}

    bool operator==(const hoxServerAddress& other) const
    {
        return ( (name == other.name) && (port == other.port) );
    }

    const char* c_str() const
    {
        _str.Printf("%s:%d", name.c_str(), port);
        return _str.c_str();
    }

private:
    mutable wxString   _str;  // String representation.
};

// ----------------------------------------------------------------------------
// hoxRequestQueue
// ----------------------------------------------------------------------------

class hoxRequestQueue
{
public:
    hoxRequestQueue();
    ~hoxRequestQueue();

    void            PushBack( hoxRequest_APtr apRequest );
    hoxRequest_APtr PopFront();

private:
    hoxRequestList  m_list;   // The list of requests.
    wxMutex         m_mutex;  // Lock
};

// ----------------------------------------------------------------------------
// hoxPlayerInfo
// ----------------------------------------------------------------------------

class hoxPlayerInfo : public wxObject
{
public:
    wxString       id;   // The player's ID.

    hoxPlayerType  type;       
            /* Is it a Local, Network,... player? */

    int            score;  // The player's Score.

    hoxPlayerInfo( const wxString& i,
                   hoxPlayerType   t,
                   int             s = 1500 )
            : id(i), type(t), score(s) {}
    
    hoxPlayerInfo() 
            : id(""), type(hoxPLAYER_TYPE_DUMMY), score(1500) {}
};
typedef std::auto_ptr<hoxPlayerInfo> hoxPlayerInfo_APtr;
typedef boost::shared_ptr<hoxPlayerInfo> hoxPlayerInfo_SPtr;
typedef std::list<hoxPlayerInfo_SPtr> hoxPlayerInfoList;

// ----------------------------------------------------------------------------
// hoxPlayerStats
// ----------------------------------------------------------------------------

class hoxPlayerStats : public wxObject
{
public:
    wxString       id;      // The player's ID.
    int            score;   // The player's Score.
    int            wins;    // The # of games won.
    int            losses;  // The # of games lost.
    int            draws;   // The # of games drawn.

    hoxPlayerStats( const wxString& i = "" )
            : id(i), score(1500), wins(0), losses(0), draws(0) {}
};
typedef std::auto_ptr<hoxPlayerStats> hoxPlayerStats_APtr;

#endif /* __INCLUDED_HOX_TYPES_H_ */
