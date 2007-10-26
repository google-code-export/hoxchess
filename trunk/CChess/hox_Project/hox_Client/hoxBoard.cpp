/////////////////////////////////////////////////////////////////////////////
// Name:            hoxBoard.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/05/2007
//
// Description:     The "simple" Board with player-info(s) + timers.
/////////////////////////////////////////////////////////////////////////////

#include "hoxBoard.h"
#include "hoxCoreBoard.h"
#include "hoxEnums.h"
#include "hoxUtility.h"
#include "hoxReferee.h"
#include "hoxTypes.h"

/* Define my custom events */
DEFINE_EVENT_TYPE(hoxEVT_BOARD_PLAYER_INFO)

BEGIN_EVENT_TABLE(hoxSimpleBoard, wxPanel)
  EVT_COMMAND  (wxID_ANY, hoxEVT_BOARD_PLAYER_INFO, hoxSimpleBoard::OnPlayerInfoEvent)
END_EVENT_TABLE()


// ----------------------------------------------------------------------------
// hoxSimpleBoard
// ----------------------------------------------------------------------------

hoxSimpleBoard::hoxSimpleBoard( wxWindow*       parent,
                                const wxString& piecesPath,
                                hoxIReferee*    referee )
        : wxPanel( parent, wxID_ANY, 
                   wxDefaultPosition, 
                   wxDefaultSize,
                   wxFULL_REPAINT_ON_RESIZE )
        , m_coreBoard( NULL )
{
    wxASSERT( referee != NULL );

    /* Create the core board. */
    m_coreBoard = new hoxCoreBoard( this, referee );
    m_coreBoard->SetPiecesPath( piecesPath );

    // *** NOTE: By default, the Board is NOT visible.
    wxPanel::Show( false );  // invoke the parent's API.
}

hoxSimpleBoard::~hoxSimpleBoard()
{
    delete m_coreBoard;
}

void 
hoxSimpleBoard::OnPlayerInfoEvent( wxCommandEvent &event )
{
    const char* FNAME = "hoxSimpleBoard::OnPlayerInfoEvent";

    hoxPlayer* player = wx_reinterpret_cast(hoxPlayer*, event.GetEventObject());
    wxASSERT_MSG(player != NULL, wxString::Format("%s: Player cannot be NULL.", FNAME));

    if ( event.GetInt() == hoxPIECE_COLOR_RED )
    {
        SetRedInfo( wxString::Format(_("%s (%d)"), 
                                player->GetName(), player->GetScore()) );
    } 
    else if ( event.GetInt() == hoxPIECE_COLOR_BLACK )
    {
        SetBlackInfo( wxString::Format(_("%s (%d)"), 
                                player->GetName(), player->GetScore()) );
    }
}

bool 
hoxSimpleBoard::Show(bool show /* = true */)
{
    if ( !this->IsShown() && show ) // hidden -> shown?
    {
        /* Ask the board to display the pieces. */
        m_coreBoard->LoadPieces();

        /* Create the whole panel with player-info + timers */
        this->_CreateBoardPanel();

        // Set its background color.
        wxColour bgPanelCol = wxTheColourDatabase->Find(_T("SKY BLUE"));
        if ( bgPanelCol.Ok() ) {
            this->SetBackgroundColour(bgPanelCol);
        }
    }

    return wxPanel::Show( show );  // invoke the parent's API.
}

void 
hoxSimpleBoard::SetRedInfo( const wxString& info )
{
    if ( this->IsShown() )
        m_redInfo->SetLabel( info );
}

void 
hoxSimpleBoard::SetBlackInfo( const wxString& info )
{
    if ( this->IsShown() )
        m_blackInfo->SetLabel( info );
}

/*
 * Create panel with the core board + player-info(s) + timers
 */
void
hoxSimpleBoard::_CreateBoardPanel()
{
    wxPanel* boardPanel = this;

    /*********************************
     * Create players' info + timers 
     *********************************/

    // Create players' info.
    m_blackInfo = new wxStaticText( boardPanel, wxID_ANY, "*", 
        wxDefaultPosition, wxSize(200,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);
    m_redInfo = new wxStaticText( boardPanel, wxID_ANY, "*", 
        wxDefaultPosition, wxSize(200,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);

    // Create players' game-time.
    m_blackGameTime = new wxStaticText( boardPanel, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);
    m_redGameTime = new wxStaticText( boardPanel, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);

    m_blackMoveTime = new wxStaticText( boardPanel, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);
    m_redMoveTime = new wxStaticText( boardPanel, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);

    m_blackFreeTime = new wxStaticText( boardPanel, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);
    m_redFreeTime = new wxStaticText( boardPanel, wxID_ANY, "00:00", 
        wxDefaultPosition, wxSize(50,20), 
        wxBORDER_SIMPLE|wxALIGN_CENTER|wxST_NO_AUTORESIZE);

    /****************************************
     * Arrange the players' info + timers 
     ****************************************/

    // Sizers
    wxBoxSizer *mainSizer = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *blackSizer = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer *redSizer = new wxBoxSizer( wxHORIZONTAL );

    // Add Black player-info
    blackSizer->Add(
        m_blackInfo,
        1,            // make vertically stretchable
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    blackSizer->Add(
        m_blackGameTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    blackSizer->Add(
        m_blackMoveTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    blackSizer->Add(
        m_blackFreeTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    // Add Red player-info
    redSizer->Add(
        m_redInfo,
        1,            // make vertically stretchable
        wxEXPAND |    // make horizontally stretchable
        wxBOTTOM|wxRIGHT|wxLEFT,  //   and make border
        1 );         // set border width

    redSizer->Add(
        m_redGameTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    redSizer->Add(
        m_redMoveTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    redSizer->Add(
        m_redFreeTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    // Invert view if required.

    bool viewInverted = m_coreBoard->IsViewInverted();
    _LayoutBoardPanel( mainSizer, redSizer, blackSizer, viewInverted);

    boardPanel->SetSizer( mainSizer );      // use the sizer for layout
    mainSizer->SetSizeHints( boardPanel );   // set size hints to honour minimum size
}

void 
hoxSimpleBoard::_LayoutBoardPanel( wxSizer* mainSizer,
                                   wxSizer* redSizer, 
                                   wxSizer* blackSizer,
                                   bool     viewInverted)
{
    wxSizer* topSizer = NULL;
    wxSizer* bottomSizer = NULL;

    if ( ! viewInverted ) // normal view?
    {
        topSizer = blackSizer;
        bottomSizer = redSizer;
    }
    else                  // inverted view?
    {
        topSizer = redSizer;
        bottomSizer = blackSizer;
    }

    // Add the top-sizer to the main-sizer
    mainSizer->Add(
        topSizer,
        0,            // fixed-size vertically
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT, // and make border
        1 );         

    // Add the main board to the main-sizer.
    mainSizer->Add(
        m_coreBoard,
        1,            // make vertically stretchable
        wxEXPAND |    // make horizontally stretchable
        wxALL,        // and make border all around
        1 );          // set border width

    // Add the bottom-sizer to the main-sizer
    mainSizer->Add(
        bottomSizer,
        0,            // fixed-size vertically
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT,  // and make border
        1 );          // set border width
}

void 
hoxSimpleBoard::SetTable( hoxTable* table ) 
{ 
    m_coreBoard->SetTable( table ); 
}

hoxIReferee* 
hoxSimpleBoard::GetReferee() const 
{ 
    return m_coreBoard->GetReferee(); 
}

void 
hoxSimpleBoard::ToggleViewSide()
{
    const char* FNAME = "hoxSimpleBoard::ToggleViewSide";

    m_coreBoard->ToggleViewSide();

    /* Invert view if the view has been displayed. 
     * Right now, assume that if red-info text has been created,
     * then the view has been displayed.
     */

    if ( m_redInfo == NULL )
    {
        wxLogDebug(wxString::Format("%s: View not yet created. Do nothing for info-panels.", FNAME));
        return;
    }

    /* Find out about the our sizers */

    wxSizer* mainSizer = m_coreBoard->GetContainingSizer();
    wxASSERT_MSG(mainSizer != NULL, "Main sizer must have been created.");

    wxSizer* redSizer = m_redInfo->GetContainingSizer();
    wxASSERT_MSG(redSizer != NULL, "Red sizer must have been created.");
    wxSizer* blackSizer = m_blackInfo->GetContainingSizer();
    wxASSERT_MSG(blackSizer != NULL, "Black sizer must have been created.");

    /* Detach the sizers */

    bool found;

    found = mainSizer->Detach( redSizer );
    wxASSERT( found );
    found = mainSizer->Detach( m_coreBoard );
    wxASSERT( found );
    found = mainSizer->Detach( blackSizer );
    wxASSERT( found );

    /* Invert */

    bool viewInverted = m_coreBoard->IsViewInverted();
    _LayoutBoardPanel( mainSizer, redSizer, blackSizer, viewInverted);

    // Force the layout update (just to make sure!).
    mainSizer->Layout();
}

bool 
hoxSimpleBoard::DoMove( const hoxMove& move )
{
    const char* FNAME = "hoxSimpleBoard::DoMove";
    hoxPiece* piece = m_coreBoard->GetPieceAt( move.piece.position );
    wxASSERT( piece != NULL );
    
    if ( ! this->GetReferee()->ValidateMove( move ) )
    {
        wxLogWarning(wxString::Format("Move is not valid.", FNAME));
        return false;
    }

    return m_coreBoard->MovePieceToPosition( piece, move.newPosition );
}

/************************* END OF FILE ***************************************/
