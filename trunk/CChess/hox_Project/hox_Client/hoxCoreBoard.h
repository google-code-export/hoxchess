/***************************************************************************
 *  Copyright 2007-2009 Huy Phan  <huyphan@playxiangqi.com>                *
 *                      Bharatendra Boddu (bharathendra at yahoo dot com)  *
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

#ifndef __INCLUDED_HOX_CORE_BOARD_H__
#define __INCLUDED_HOX_CORE_BOARD_H__

#include <wx/wx.h>
#include <wx/dragimag.h>
#include <list>
#include "hoxPiece.h"
#include "hoxIReferee.h"

///////////////////////////////////////////////////////////////////////////////
class hoxCoreBackground
{
public:
    hoxCoreBackground();
    virtual ~hoxCoreBackground() {}

    virtual void OnPaint( wxDC& dc ) = 0;
    virtual void OnResize( const wxSize totalSize ) = 0;

    virtual void OnBgColor( wxColor color ) {}
    virtual void OnFgColor( wxColor color ) {}

    virtual void OnReverseView() { m_bViewInverted = !m_bViewInverted; }
    void SetGameOver( bool isGameOver ) { m_isGameOver = isGameOver; }

    wxCoord BorderX() const { return m_borderX; }
    wxCoord BorderY() const { return m_borderY; }
    wxCoord CellS()   const { return m_cellS; }

    virtual wxSize GetBestSize( const wxSize proposedSize );

protected:
    void DrawGameOverText( wxDC& dc );

protected:
    wxSize     m_totalSize;

    wxCoord    m_borderX;
    wxCoord    m_borderY;
    wxCoord    m_cellS;  // Squares ' size.

    bool       m_bViewInverted;
    bool       m_isGameOver;
};

// ----------------------------------------------------------------------------
class hoxCustomBackground : public hoxCoreBackground
{
public:
    hoxCustomBackground();
    virtual ~hoxCustomBackground() {}

    virtual void OnPaint( wxDC& dc );
    virtual void OnResize( const wxSize totalSize );
    virtual void OnBgColor( wxColor color );
    virtual void OnFgColor( wxColor color );

    virtual wxSize GetBestSize( const wxSize proposedSize );

private:
    void _DrawWorkSpace( wxDC& dc, const wxSize totalSize );
    void _DrawBoard( wxDC& dc, const wxSize totalSize );

private:
    wxColor  m_backgroundColor;
    wxColor  m_foregroundColor;
};

// ----------------------------------------------------------------------------
class hoxImageBackground : public hoxCoreBackground
{
public:
    hoxImageBackground( const wxString& sImage );
    virtual ~hoxImageBackground() {}

    virtual void OnReverseView();
    virtual void OnPaint( wxDC& dc );
    virtual void OnResize( const wxSize totalSize );

    virtual wxSize GetBestSize( const wxSize proposedSize );

private:
    void _DrawBoardImage( wxDC& dc );

private:
    wxString   m_imageFile;
    wxBitmap   m_bitmap;
};
///////////////////////////////////////////////////////////////////////////////

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

/**
 * The 'core' Board providing UI for the Board and Pieces.
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
        virtual void OnBoardMove( const hoxMove& move,
			                      hoxGameStatus  status ) = 0;
        virtual void OnBoardMsg( const wxString& message ) = 0;
        
        virtual bool OnBoardAskMovePermission( const hoxPieceInfo& pieceInfo ) = 0;
    };

public:
    hoxCoreBoard() {} // Dummy default constructor required for RTTI info.
    hoxCoreBoard( wxWindow*        parent,
                  hoxIReferee_SPtr referee,
                  const wxString&  sBgImage,
                  const wxString&  piecesPath,
                  wxColor          bgColor = DEFAULT_BOARD_BACKGROUND_COLOR,
                  wxColor          fgColor = DEFAULT_BOARD_FOREGROUND_COLOR,
                  const wxPoint&   pos = wxDefaultPosition, 
                  const wxSize&    size = wxDefaultSize );
    virtual ~hoxCoreBoard();

    /*********************************
     * My MAIN public API
     *********************************/

    void SetBackgroundImage( const wxString& sImage );
    void SetBgColor( wxColor color ); // Set Board's Background Color
    void SetFgColor( wxColor color ); // Set Board's Foreground Color
    void SetPiecesPath( const wxString& piecesPath );
    void Repaint(); // Paint again using the current settings.

    /**
     * Load pieces according the Referee and set the
     * game-status accordingly (e.g. showing 'game-over' message).
     */
    void LoadPiecesAndStatus();

    /**
     * Reset the Board.
     */
    void ResetBoard();

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
    bool DoMove( hoxMove&   move,
                 const bool bRefresh = true );

    /*********************************
     * Game-reviewing API.
     *********************************/

    bool DoGameReview_BEGIN();
    bool DoGameReview_PREV( const bool bRefresh = true );
    bool DoGameReview_NEXT( const bool bRefresh = true );
    bool DoGameReview_END();

    /*********************************
     * My event-handlers.
     *********************************/

    void OnPaint( wxPaintEvent& event );
    void OnEraseBackground( wxEraseEvent &event );
    void OnIdle( wxIdleEvent& event );
    void OnSize( wxSizeEvent& event );
    void OnMouseEvent( wxMouseEvent& event );
    void OnMouseCaptureLost( wxMouseCaptureLostEvent& event );

    /******************************************
     * My 'other' (less important) public API
     ******************************************/

    /**
     * Reverse view: Red/Black is at the bottom.
     *
     * @return true if the Board's view is inverted after the action
     *              has been carried out.
     */
    bool ReverseView();

    bool IsViewInverted() const { return m_bViewInverted; }
    void SetGameOver( bool isGameOver = true );

    /**
     * TODO: Hack to adjust the height according to the parent's.
     */
    void SetBestHeightAdjustment( const int adjustHeight )
        { m_nBestHeightAdjustment = adjustHeight; }

protected:
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
                       bool               hightlight = true,
                       const bool         bRefresh = true );

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
     * Attempt to find a Piece a given position. If found, then "capture" it.
     *
     * @return The captured Piece if found. Otherwise, return NULL.
     */
    hoxPiece* _FindAndCapturePieceAt( const hoxPosition& position,
                                      const bool         bRefresh = true );

    /**
     * This API is called when a piece is physically moved by the local player
     * using the mouse.
     *
     * @note This move will trigger referee-validation.
     */
    void _MovePieceToPoint(hoxPiece* piece, const wxPoint& point);

    /**
     * Check if a given piece has the right to move 'next'.
     */
    bool _CanPieceMoveNext( hoxPiece* piece ) const;

    /**
     * Check if this Board is in GAME-REVIEW mode.
     */
    bool _IsBoardInReviewMode() const;

    void   _DrawAllPieces( wxDC& dc );
    void   _DrawAndHighlightPiece( hoxPiece*  piece,
                                   const bool bRefresh = true );
    void   _DrawPiece( const hoxPiece* piece );
    void   _DrawPieceWithDC( wxDC& dc, const hoxPiece* piece );
    wxRect _GetPieceRect( const hoxPiece* piece ) const;

    void      _DoPaint( wxDC& dc );
    void      _ErasePiece( hoxPiece* piece );
    void      _ClearPieces();
    void      _ReloadAllPieceBitmaps();
    hoxPiece* _FindPiece( const wxPoint& point ) const;
    wxPoint   _GetPieceLocation( const hoxPiece* piece ) const;
    bool      _PieceHitTest( const hoxPiece* piece, const wxPoint& pt ) const;
    hoxPosition _PointToPosition( const hoxPiece* piece, const wxPoint& p ) const;

    void      _RecordMove( const hoxMove& move );

    /**
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * TODO: This API exists ONLY to help printing debug-message related to
     *       this Board.  wxLogXXX() is not used because its output can be
     *       hidden and may not be visible to the end-users.
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     */
    void _PrintDebug( const wxString& debugMsg ) const;

private:
    wxString           m_sImage;
    hoxCoreBackground* m_background;
    wxString           m_piecesPath; // The path of all Pieces' bitmaps.

    /* Board's characteristics. */
    bool            m_bViewInverted; // true if Black is at the bottom

    typedef std::list<hoxPiece*> hoxPieceList;
    hoxPieceList     m_pieces;  // List of all Pieces

    hoxIReferee_SPtr m_referee; // The Referee of the game.
    BoardOwner*      m_owner;   // This Board's owner.

    /* Variables used when a piece is dragged by the mouse. */
    int             m_dragMode;
    hoxPiece*       m_draggedPiece;
    wxPoint         m_dragStartPos;
    wxDragImage*    m_dragImage;

    hoxPiece*       m_latestPiece; // piece that last moved.

    /* The History of all Moves 
     * This list is maintained so that the players can review the game.
     */
    hoxMoveVector   m_historyMoves; // All (past) Moves made so far.
    enum HistoryIndex // NOTE: Do not change the constants 'values below.
    {
        HISTORY_INDEX_END   = -2,
        HISTORY_INDEX_BEGIN = -1
    };
    int             m_historyIndex; // Which Move the user is reviewing.

    /* End-Game tracking */
    bool            m_isGameOver;

    /* HACK: To adjust the height according to the parent 's. */
    int             m_nBestHeightAdjustment;

    DECLARE_DYNAMIC_CLASS(hoxCoreBoard)
    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_CORE_BOARD_H__ */
