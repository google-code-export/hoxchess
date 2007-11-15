/***************************************************************************
 *  Copyright 2007 Huy Phan  <huyphan@playxiangqi.com>                     *
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
// Name:            hoxCoreBoard.h
// Created:         10/05/2007
//
// Description:     The "core" Board.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_CORE_BOARD_H_
#define __INCLUDED_HOX_CORE_BOARD_H_

#include <wx/wx.h>
#include <wx/dragimag.h>
#include <wx/image.h>

#include <list>
#include "hoxPiece.h"
#include "hoxIReferee.h"
#include "hoxNaiveReferee.h"
#include "hoxPlayer.h"

/*
 * typedefs
 */
typedef std::list<hoxPiece*> hoxPieceList;

/*
 * hoxCoreBoard
 *
 *     0 1 2 ..... 8
 *   0  +-----------
 *   1  |
 *   2  |
 *   3  |
 *   .  |
 *   .  |
 *   9  +-----------
 *
 */

class hoxCoreBoard : public wxPanel
{
public:
    /**
     * The Board's Owner. 
     */
    class BoardOwner
    {
    public:
        virtual void OnBoardMove( const hoxMove& move ) = 0;
    };

public:
    hoxCoreBoard(); // Dummy default constructor required for RTTI info.
    hoxCoreBoard( wxWindow*      parent,
                  hoxIReferee*   referee = new hoxNaiveReferee(),
                  const wxPoint& pos = wxDefaultPosition, 
                  const wxSize&  size = wxDefaultSize );
    virtual ~hoxCoreBoard();

    /*********************************
     * My MAIN public API
     *********************************/

    /**
     * Set the path (such as "C:/cchess/pieces") from where
     * all the pieces' images are loaded.
     */
    void SetPiecesPath(const wxString& piecesPath);

    /**
     * Load pieces according the referee's list of pieces' positions.
     */
    void LoadPieces();

    /**
     * Set the referee who can determine whether a move is legal or not.
     *
     * @see hoxIReferee
     */
    void SetReferee( hoxIReferee* referee );

    hoxIReferee* GetReferee() const { return m_referee; }

    /**
     * Set Owner of this Board.
     */
    void SetBoardOwner( BoardOwner* owner ) { m_owner = owner; }

    /**
     * This API is called by Table.
     * Usually, the Move is coming an external source (e.g., over the network).
     *
     * @note This move will trigger referee-validation.
     */
    bool DoMove( hoxMove& move );

    /*********************************
     * Game-reviewing API.
     *********************************/

    bool DoGameReview_PREV();  // Previous move.
    bool DoGameReview_NEXT();

    /*********************************
     * My event-handlers.
     *********************************/

    void OnPaint(wxPaintEvent &WXUNUSED(event));
    void OnEraseBackground( wxEraseEvent &event );
    void OnIdle(wxIdleEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);

    /******************************************
     * My 'other' (less important) public API
     ******************************************/

    void ToggleViewSide();  // toggle view side: Red/Black is at the bottom.
    bool IsViewInverted() const { return m_bViewInverted; }

protected:
    /**
     * The the best size of the WHOLE board.
     */
    wxSize GetBestBoardSize( const int proposedHeight ) const;

    virtual wxSize DoGetBestSize() const;

private:
    /**
     * Move a Piece to a new Position.
     *
     * @note No Referee-validation is involved.
     *
     * @param hightlight - If true, then the Piece will be highlighted.
     */
    bool _MovePieceTo( hoxPiece*          piece, 
                       const hoxPosition& newPosition,
                       bool               hightlight = true );

    /**
     * Find a Piece a given position.
     *
     * @param includeInactive - If true, then inactive pieces are also included
     *                          in the search (useful in REVIEW mode).
     *
     * @return NULL if no Piece was found.
     */
    hoxPiece* _FindPieceAt( const hoxPosition& position,
                            bool               includeInactive = false ) const;

    /**
     * This API is called when a piece is physically moved by the local player
     * using the mouse.
     *
     * @note This move will trigger referee-validation.
     */
    void _MovePieceToPoint(hoxPiece* piece, const wxPoint& point);

    void _OnPieceMoved(hoxPiece* piece, const hoxPosition& newPos);


    void   _DrawBoard(wxDC& dc);
    void   _DrawWorkSpace( wxDC& dc );
    void   _DrawAllPieces(wxDC& dc);
    bool   _DrawAndHighlightPiece( hoxPiece* piece );
    bool   _DrawPiece(const hoxPiece* piece, int op = wxCOPY);
    bool   _DrawPiece(wxDC& dc, const hoxPiece* piece, int op = wxCOPY);
    wxRect _GetPieceRect(const hoxPiece* piece) const;

    void      _DoPaint(wxDC& dc);
    void      _ErasePiece(hoxPiece* piece);
    void      _ErasePieceWithDC(hoxPiece* piece, wxDC& dc);
    void      _ClearPieces();
    hoxPiece* _FindPiece(const wxPoint& pt) const;
    wxPoint   _GetPieceLocation(const hoxPiece* piece) const;
    bool      _PieceHitTest(const hoxPiece* piece, const wxPoint& pt) const;
    hoxPosition _PointToPosition(const hoxPiece* piece, const wxPoint& p) const;

    void      _RecordMove( const hoxMove& move );

private:
    // Board's characteristics.
    wxCoord         m_borderX;  // X-position from the border
    wxCoord         m_borderY;  // Y-position from the border
    wxCoord         m_cellS;    // size of each cell.
    bool            m_bViewInverted; // true if Black is at the bottom

    hoxPieceList    m_pieces;  // list of all pieces

    hoxIReferee*    m_referee; // The Referee of the game.
    BoardOwner*     m_owner;   // This Board's owner.

    // Variables used when a piece is dragged by the mouse.
    int             m_dragMode;
    hoxPiece*       m_draggedPiece;
    wxPoint         m_dragStartPos;
    wxDragImage*    m_dragImage;

    hoxPiece*       m_latestPiece; // piece that last moved.

    /* The History of all Moves 
     * This list is maintained so that the players can review the game.
     */
    hoxMoveVector     m_historyMoves; // All (past) Moves made so far.
    enum HistoryIndex {   // TODO: Temporarily defined here.
        HISTORY_INDEX_UNKNOWN  = -2,
        HISTORY_INDEX_BEGIN    = -1
        //HISTORY_INDEX_END     = -1,
    };
    int               m_historyIndex; // Which Move the user is reviewing.

    DECLARE_DYNAMIC_CLASS(hoxCoreBoard)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_CORE_BOARD_H_ */
