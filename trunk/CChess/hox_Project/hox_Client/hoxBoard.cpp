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
#include "hoxTable.h"

#define hoxID_BOARD_WALL_INPUT   (wxID_HIGHEST+1)

/* Define my custom events */
DEFINE_EVENT_TYPE(hoxEVT_BOARD_PLAYER_JOIN)
DEFINE_EVENT_TYPE(hoxEVT_BOARD_PLAYER_LEAVE)
DEFINE_EVENT_TYPE(hoxEVT_BOARD_WALL_OUTPUT)

BEGIN_EVENT_TABLE(hoxSimpleBoard, wxPanel)
  EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_PLAYER_JOIN, hoxSimpleBoard::OnPlayerJoin)
  EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_PLAYER_LEAVE, hoxSimpleBoard::OnPlayerLeave)
  EVT_COMMAND(wxID_ANY, hoxEVT_BOARD_WALL_OUTPUT, hoxSimpleBoard::OnWallOutput)

  EVT_TEXT_ENTER(hoxID_BOARD_WALL_INPUT, hoxSimpleBoard::OnWallInputEnter)
END_EVENT_TABLE()


// ----------------------------------------------------------------------------
// hoxSimpleBoard
// ----------------------------------------------------------------------------

hoxSimpleBoard::hoxSimpleBoard( wxWindow*       parent,
                                const wxString& piecesPath,
                                hoxIReferee*    referee,
                                const wxPoint&  pos  /* = wxDefaultPosition */, 
                                const wxSize&   size /* = wxDefaultSize */)
        : wxPanel( parent, wxID_ANY, 
                   pos, 
                   size,
                   wxFULL_REPAINT_ON_RESIZE )
        , m_coreBoard( NULL )
        , m_table( NULL )
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
hoxSimpleBoard::OnPlayerJoin( wxCommandEvent &event )
{
    const char* FNAME = "hoxSimpleBoard::OnPlayerJoin";

    hoxPlayer* player = wx_reinterpret_cast(hoxPlayer*, event.GetEventObject());
    wxCHECK_RET(player, "Player cannot be NULL.");

    if ( event.GetInt() == hoxPIECE_COLOR_RED )
    {
        SetRedInfo( player );
    } 
    else if ( event.GetInt() == hoxPIECE_COLOR_BLACK )
    {
        SetBlackInfo( player );
    }

    _AddPlayerToList( player->GetName(), player->GetScore() );
}

void 
hoxSimpleBoard::OnPlayerLeave( wxCommandEvent &event )
{
    const char* FNAME = "hoxSimpleBoard::OnPlayerLeave";

    hoxPlayer* player = wx_reinterpret_cast(hoxPlayer*, event.GetEventObject());
    wxCHECK_RET(player, "Player cannot be NULL.");

    const wxString playerId = player->GetName();

    if ( playerId == m_redId )     // Check RED
    {
        m_redInfo->SetLabel( "*" );

    }
    else if ( playerId == m_blackId ) // Check BLACK
    {
        m_blackInfo->SetLabel( "*" ); // Check Observers
    }
    else
    {
        m_observerIds.remove( playerId );
    }

    _RemovePlayerFromList( playerId );
}

void 
hoxSimpleBoard::OnWallOutput( wxCommandEvent &event )
{
    const wxString eventString = event.GetString();
    const wxString who = eventString.BeforeFirst(' ');
    const wxString msg = eventString.AfterFirst(' ');

    //wxString wallMsg;
    //wallMsg.Printf("[%s] %s\n", who, msg);

    m_wallOutput->SetDefaultStyle(wxTextAttr(*wxBLACK));
    m_wallOutput->AppendText( wxString::Format("[%s] ", who.c_str()) );
    m_wallOutput->SetDefaultStyle(wxTextAttr(*wxBLUE));
    m_wallOutput->AppendText( wxString::Format("%s\n", msg.c_str()) );
}

void 
hoxSimpleBoard::OnWallInputEnter( wxCommandEvent &event )
{
    m_wallInput->Clear();
    m_table->OnMessage_FromBoard( event.GetString() );
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
hoxSimpleBoard::SetRedInfo( const hoxPlayer* player )
{
    m_redId = player->GetName();

    if ( this->IsShown() )
    {
        const wxString info = wxString::Format("%s (%d)", 
                player->GetName().c_str(), player->GetScore());
        m_redInfo->SetLabel( info );
    }
}

void 
hoxSimpleBoard::SetBlackInfo( const hoxPlayer* player )
{
    m_blackId = player->GetName();

    if ( this->IsShown() )
    {
        const wxString info = wxString::Format("%s (%d)", 
                player->GetName().c_str(), player->GetScore());
        m_blackInfo->SetLabel( info );
    }
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

    /*********************************
     * Create Wall's contents.
     *********************************/

    m_playerListBox = new wxListBox( boardPanel, wxID_ANY );

    m_wallOutput = new wxTextCtrl( boardPanel, wxID_ANY, _T(""),
                                   wxDefaultPosition, wxDefaultSize,
                                   wxTE_MULTILINE | wxRAISED_BORDER | wxTE_READONLY 
                                   | wxHSCROLL | wxTE_RICH /* needed for Windows */ );
    m_wallInput  = new wxTextCtrl( boardPanel, hoxID_BOARD_WALL_INPUT, _T(""),
                                   wxDefaultPosition, wxDefaultSize,
                                   wxTE_PROCESS_ENTER | wxSUNKEN_BORDER );

    /****************************************
     * Arrange the players' info + timers 
     ****************************************/

    // Sizers
    m_mainSizer  = new wxBoxSizer( wxHORIZONTAL );
    m_boardSizer = new wxBoxSizer( wxVERTICAL );
    m_wallSizer  = new wxBoxSizer( wxVERTICAL );
    
    m_blackSizer = new wxBoxSizer( wxHORIZONTAL );
    m_redSizer = new wxBoxSizer( wxHORIZONTAL );

    // Add Black player-info
    m_blackSizer->Add(
        m_blackInfo,
        1,            // make vertically stretchable
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    m_blackSizer->Add(
        m_blackGameTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    m_blackSizer->Add(
        m_blackMoveTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    m_blackSizer->Add(
        m_blackFreeTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    // Add Red player-info
    m_redSizer->Add(
        m_redInfo,
        1,            // make vertically stretchable
        wxEXPAND |    // make horizontally stretchable
        wxBOTTOM|wxRIGHT|wxLEFT,  //   and make border
        1 );         // set border width

    m_redSizer->Add(
        m_redGameTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    m_redSizer->Add(
        m_redMoveTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    m_redSizer->Add(
        m_redFreeTime,
        0,            // make vertically fixed
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    // Invert view if required.

    bool viewInverted = m_coreBoard->IsViewInverted();
    _LayoutBoardPanel( viewInverted);

    // Setup the Wall.

    m_wallSizer->Add(
        m_playerListBox,
        1,            // fixed-size vertically
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT, // and make border
        1 );         // set border width

    m_wallSizer->Add(
        m_wallOutput,
        3,            // fixed-size vertically
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT, // and make border
        1 );         // set border width

    m_wallSizer->Add(
        m_wallInput,
        0,            // fixed-size vertically
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT, // and make border
        1 );         // set border width

    // Setup main sizer.

    m_mainSizer->Add(
        m_boardSizer,
        0,            // fixed size
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    m_mainSizer->Add(
        m_wallSizer,
        1,            // proportion
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT|wxTOP,  //   and make border
        1 );         // set border width

    /* Setup the main size */
    boardPanel->SetSizer( m_mainSizer );      // use the sizer for layout
    //m_mainSizer->SetSizeHints( boardPanel );   // set size hints to honour minimum size
}

void 
hoxSimpleBoard::_LayoutBoardPanel( bool viewInverted )
{
    wxSizer* topSizer = NULL;
    wxSizer* bottomSizer = NULL;

    if ( ! viewInverted ) // normal view?
    {
        topSizer = m_blackSizer;
        bottomSizer = m_redSizer;
    }
    else                  // inverted view?
    {
        topSizer = m_redSizer;
        bottomSizer = m_blackSizer;
    }

    // Add the top-sizer to the main-sizer
    m_boardSizer->Add(
        topSizer,
        0,            // fixed-size vertically
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT, // and make border
        1 );         

    // Add the main board to the main-sizer.
    m_boardSizer->Add(
        m_coreBoard,
        1,            // make vertically stretchable
        wxEXPAND |    // make horizontally stretchable
        wxALL,        // and make border all around
        1 );          // set border width

    // Add the bottom-sizer to the main-sizer
    m_boardSizer->Add(
        bottomSizer,
        0,            // fixed-size vertically
        wxEXPAND |    // make horizontally stretchable
        wxRIGHT|wxLEFT,  // and make border
        1 );          // set border width
}

void 
hoxSimpleBoard::SetTable( hoxTable* table ) 
{ 
    m_table = table;
    m_coreBoard->SetTable( m_table ); 
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
        wxLogDebug("%s: View not yet created. Do nothing for info-panels.", FNAME);
        return;
    }

    /* Detach the sizers */

    bool found;

    found = m_boardSizer->Detach( m_redSizer );
    wxASSERT( found );
    found = m_boardSizer->Detach( m_coreBoard );
    wxASSERT( found );
    found = m_boardSizer->Detach( m_blackSizer );
    wxASSERT( found );

    /* Invert */

    bool viewInverted = m_coreBoard->IsViewInverted();
    _LayoutBoardPanel( viewInverted );

    // Force the layout update (just to make sure!).
    m_boardSizer->Layout();
}

bool 
hoxSimpleBoard::DoMove( const hoxMove& move )
{
    const char* FNAME = "hoxSimpleBoard::DoMove";
    hoxPiece* piece = m_coreBoard->GetPieceAt( move.piece.position );
    wxASSERT( piece != NULL );
    
    if ( ! this->GetReferee()->ValidateMove( move ) )
    {
        wxLogWarning("%s: Move is not valid.", FNAME);
        return false;
    }

    return m_coreBoard->MovePieceToPosition( piece, move.newPosition );
}

void 
hoxSimpleBoard::_AddPlayerToList( const wxString& playerId,
                                  int             playerScore )
{
    const char* FNAME = "hoxSimpleBoard::_AddPlayerToList";

    if ( ! this->IsShown() )
    {
        wxLogDebug("%s: Board is not shown. Do nothing.", FNAME);
        return;
    }

    const wxString info = wxString::Format("%s (%d)", playerId.c_str(), playerScore);
    m_playerListBox->Append( info );
}

void 
hoxSimpleBoard::_RemovePlayerFromList( const wxString& playerId )
{
    const char* FNAME = "hoxSimpleBoard::_RemovePlayerFromList";

    if ( ! this->IsShown() )
    {
        wxLogDebug("%s: Board is not shown. Do nothing.", FNAME);
        return;
    }

    int idCount = m_playerListBox->GetCount();

    for ( int i = 0; i < idCount; ++i )
    {
        if ( m_playerListBox->GetString(i).StartsWith(playerId) )
        {
            m_playerListBox->Delete( i );
            break;
        }
    }
}

/************************* END OF FILE ***************************************/
