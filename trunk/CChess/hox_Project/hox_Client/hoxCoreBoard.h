/////////////////////////////////////////////////////////////////////////////
// Name:            hoxCoreBoard.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/05/2007
//
// Description:     The "core" Board.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_CORE_BOARD_H_
#define __INCLUDED_HOX_CORE_BOARD_H_

#include "wx/wx.h"

// Under Windows, change this to 1
// to use wxGenericDragImage

//#define wxUSE_GENERIC_DRAGIMAGE 1

#if wxUSE_GENERIC_DRAGIMAGE
#include "wx/generic/dragimgg.h"
#define wxDragImage wxGenericDragImage
#else
#include "wx/dragimag.h"
#endif

#include "wx/image.h"

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
    hoxCoreBoard( wxWindow*    parent,
                  hoxIReferee* referee = new hoxNaiveReferee() );
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
     * Load all the pieces on board.
     *
     * @note The pieces' images are located in the path set by SetPiecesPath().
     */
    //void LoadNewPieces();

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
     * Set the table which this Board will inform of new Moves.
     *
     */
    void SetTable( hoxTable* table );

    /**
     * This API is called by Table.
     * Usually, the Move is coming an external source (e.g., over the network).
     * Thus, the Table should already validate the Move before 
     * invoking this API.
     *
     * @note This move will BY PASS referee-validation.
     */
    bool MovePieceToPosition(hoxPiece* piece, const hoxPosition& newPosition);


    /*********************************
     * override base class virtuals
     *********************************/

    void OnPaint(wxPaintEvent &WXUNUSED(event));
    void OnEraseBackground( wxEraseEvent &event );
    void OnIdle(wxIdleEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);

    /******************************************
     * My 'other' (less important) public API
     ******************************************/

    hoxPiece* GetPieceAt(const hoxPosition& pos) const;
    void ToggleViewSide();  // toggle view side: Red/Black is at the bottom.
    bool IsViewInverted() const { return m_bViewInverted; }

private:
    /**
     * This API is called when a piece is physically moved by the local player
     * using the mouse.
     *
     * @note This move will trigger referee-validation.
     */
    void _MovePieceToPoint(hoxPiece* piece, const wxPoint& point);

    void _OnPieceMoved(hoxPiece* piece, const hoxPosition& newPos);


    void   DrawBoard(wxDC& dc);
    void   DrawWorkSpace( wxDC& dc );
    void   DrawAllPieces(wxDC& dc);
    bool   DrawPiece(wxDC& dc, const hoxPiece* piece, int op = wxCOPY);
    bool   DrawPiece(const hoxPiece* piece, int op = wxCOPY); 
    wxRect GetPieceRect(const hoxPiece* piece) const;

    void      DoPaint(wxDC& dc);
    void      ErasePiece(hoxPiece* piece, wxDC& dc);
    void      ClearPieces();
    hoxPiece* FindPiece(const wxPoint& pt) const;
    wxPoint   GetPiecePosition(const hoxPiece* piece) const;
    bool      PieceHitTest(const hoxPiece* piece, const wxPoint& pt) const;
    hoxPosition PointToPosition(const hoxPiece* piece, const wxPoint& p) const;


private:
    // Board's characteristics.
    wxCoord         m_borderX;  // X-position from the border
    wxCoord         m_borderY;  // Y-position from the border
    wxCoord         m_cellS;    // size of each cell.
    bool            m_bViewInverted; // true if Black is at the bottom

    hoxPieceList    m_pieces;  // list of all pieces

    wxWindow*       m_parent;  // the parent window

    hoxIReferee*    m_referee;  // who will referee the game.
    hoxTable*       m_table;  // which table this board belongs to.

    // Variables used when a piece is dragged by the mouse.
    int             m_dragMode;
    hoxPiece*       m_draggedPiece;
    wxPoint         m_dragStartPos;
    wxDragImage*    m_dragImage;

    hoxPiece*       m_latestPiece; // piece that last moved.


    DECLARE_CLASS(hoxCoreBoard)
    DECLARE_EVENT_TABLE()
};


#endif /* __INCLUDED_HOX_CORE_BOARD_H_ */
