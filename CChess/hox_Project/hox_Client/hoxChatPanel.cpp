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
// Name:            hoxChatPanel.cpp
// Created:         12/14/2009
//
// Description:     The panel to hold either a "Table"-Chat session
//                  or a "Private"-Chat session.
/////////////////////////////////////////////////////////////////////////////

#include "hoxChatPanel.h"
#include "hoxUtil.h"

/* UI-related IDs. */
enum
{
    ID_BOARD_WALL_INPUT = hoxUI_ID_RANGE_CHAT,
    ID_BOARD_INPUT_BUTTON,
};

//-----------------------------------------------------------------------------
// hoxChatPanel
//-----------------------------------------------------------------------------

wxDEFINE_EVENT(hoxEVT_CHAT_INPUT_ENTER, wxCommandEvent);

BEGIN_EVENT_TABLE(hoxChatPanel, wxPanel)
    EVT_TEXT_ENTER(ID_BOARD_WALL_INPUT, hoxChatPanel::OnWallInputEnter)
    EVT_BUTTON(ID_BOARD_INPUT_BUTTON, hoxChatPanel::OnWallInputEnter)
END_EVENT_TABLE()

hoxChatPanel::hoxChatPanel( wxWindow* parent, wxWindowID id,
                            const wxString& sCaption,
                            const wxString& ownerId )
            : wxPanel( parent, id )
            , m_sCaption( sCaption )
            , m_sOwnerId( ownerId )
            , m_inPrivateMode( false )
{
    wxBoxSizer* topSizer  = new wxBoxSizer( wxVERTICAL );
    SetSizer( topSizer ); // Use this sizer for layout.

    m_wallOutput = new hoxWallOutput( this, wxID_ANY, m_sCaption );
    topSizer->Add( m_wallOutput, wxSizerFlags(1).Expand() );

    wxBoxSizer* inputSizer  = new wxBoxSizer( wxHORIZONTAL );
    m_wallInput = new hoxInputTextCtrl( this, ID_BOARD_WALL_INPUT );
    inputSizer->Add( m_wallInput, wxSizerFlags(1).Expand() );
    wxBitmapButton* goButton = new wxBitmapButton( this, ID_BOARD_INPUT_BUTTON,
                                                   hoxUtil::LoadImage("go-jump.png") );
    goButton->SetFocus();
    inputSizer->Add( goButton );
    topSizer->Add( inputSizer, wxSizerFlags().Expand() );
}

void
hoxChatPanel::OnMessageFrom( const wxString& senderId,
                             const wxString& sMessage,
                             bool            bPrivate /* = false */ )
{
    m_wallOutput->AppendMessage( senderId, sMessage, bPrivate );
}

void 
hoxChatPanel::OnWallInputEnter( wxCommandEvent& event )
{
    const wxString sText = m_wallInput->GetValue();
    if ( ! sText.empty() )
    {
        m_wallInput->Clear();
        this->OnMessageFrom( m_sOwnerId, sText, m_inPrivateMode );

        /* Notify the parent. */
        wxCommandEvent* event = new wxCommandEvent( hoxEVT_CHAT_INPUT_ENTER );
        event->SetString( sText );
        wxQueueEvent( this->GetParent(), event );
    }
}

//-----------------------------------------------------------------------------
// hoxChatWindow
//-----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(hoxChatWindow, wxDialog)
    EVT_COMMAND(wxID_ANY, hoxEVT_CHAT_INPUT_ENTER, hoxChatWindow::OnChatInputEnter)
    EVT_CLOSE(hoxChatWindow::OnClose)
END_EVENT_TABLE()

hoxChatWindow::hoxChatWindow( wxWindow*       parent,
                              const wxString& title,
                              hoxLocalPlayer* localPlayer,
                              wxString        sOtherId )
        : wxDialog( parent, wxID_ANY, title )
        , m_localPlayer( localPlayer )
        , m_sOtherId( sOtherId )
{
    wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( topSizer ); // use the sizer for layout

    m_chatPanel = new hoxChatPanel( this, wxID_ANY, _("Messages"),
                                    localPlayer->GetId() );
    m_chatPanel->SetPrivateMode( true );
    topSizer->Add( m_chatPanel, wxSizerFlags(1).Border(wxALL, 10).Expand());
}

void
hoxChatWindow::OnMessageFrom( const wxString& senderId,
                              const wxString& sMessage )
{
    m_chatPanel->OnMessageFrom( (senderId == m_sOtherId ? senderId
                                                        : "***" + senderId),
                                sMessage, true /* Private */ );
}

void 
hoxChatWindow::OnChatInputEnter( wxCommandEvent& event )
{
    const wxString sText = event.GetString();
    m_localPlayer->SendPrivateMessage( m_sOtherId, sText );
}

void 
hoxChatWindow::OnClose( wxCloseEvent& event )
{
    m_localPlayer->OnPrivateChatWindowClosed();
    event.Skip(); // let the search for the event handler continue...
}

//-----------------------------------------------------------------------------
// hoxWallOutput
//-----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(hoxWallOutput, wxPanel)
    EVT_BUTTON(wxID_ANY, hoxWallOutput::OnClearButton)
END_EVENT_TABLE()

hoxWallOutput::hoxWallOutput( wxWindow* parent, wxWindowID id,
                              const wxString& sCaption  )
            : wxPanel( parent, id )
            , m_sCaption( sCaption )
            , m_wall( NULL )
{
    m_wall = new wxTextCtrl( this, wxID_ANY, wxEmptyString,
                             wxDefaultPosition, wxDefaultSize,
                             wxTE_MULTILINE | wxRAISED_BORDER | wxTE_READONLY 
                                | wxTE_RICH2 /* Windows only */ );
    _CreateUI();
}

void
hoxWallOutput::_CreateUI()
{
    /* Reference: Color constants from:
     *       http://www.colorschemer.com/online.html
     */

    wxBoxSizer* mainSizer = new wxBoxSizer( wxVERTICAL );
    this->SetSizer( mainSizer );    

    wxBoxSizer* headerSizer = new wxBoxSizer( wxHORIZONTAL );
    wxStaticText* captionText = new wxStaticText( this, wxID_ANY, m_sCaption );
    captionText->SetBackgroundColour( wxColor(87,87,87) ) ;
    captionText->SetForegroundColour( wxColor(*wxWHITE) ) ;
    headerSizer->Add( captionText,
        wxSizerFlags(1).Expand().Align(wxALIGN_CENTER_VERTICAL) );
    wxBitmapButton* clearButton = new wxBitmapButton( this, wxID_ANY,
                                        hoxUtil::LoadImage("edit-clear.png"));
    clearButton->SetToolTip( _("Clear All") );
    headerSizer->Add( clearButton );

    mainSizer->Add( headerSizer,
        wxSizerFlags().Border(wxTOP|wxLEFT|wxRIGHT,1) );
    mainSizer->Add( m_wall,
        wxSizerFlags(1).Expand().Border(wxRIGHT|wxLEFT|wxBOTTOM,1) );
}

void 
hoxWallOutput::AppendMessage( const wxString& who,
                              const wxString& sMessage,
                              bool            bPrivate /* = false */ )
{
    if ( !who.empty() )
    {
        wxTextAttr newStyle( bPrivate ? *wxBLUE : *wxBLACK );
        newStyle.SetFontWeight( wxFONTWEIGHT_BOLD );
        m_wall->SetDefaultStyle( newStyle );
        m_wall->AppendText( who + ": " );
        newStyle.SetFontWeight( wxFONTWEIGHT_NORMAL );
        m_wall->SetDefaultStyle( newStyle );
    }
    m_wall->SetDefaultStyle( wxTextAttr(*wxBLACK) );
    const wxString displayMsg = sMessage + "\n";

    /* NOTE:
     *    Make sure that the last line is at the bottom of the wxTextCtrl
     *    so that new messages are visiable to the Player.
     *    This technique was learned from the following site:
     *        http://wiki.wxwidgets.org/WxTextCtrl#Scrolling
     *
     * HACK: Under Windows (using wxTE_RICH2) we have trouble ensuring that the last
     * entered line is really at the bottom of the screen. We jump through some
     * hoops to get this working.
     */
 
    // Count number of lines.
    int lines = 0;
    for ( wxString::const_iterator it = displayMsg.begin();
                                   it != displayMsg.end(); ++it )
    {
        const wchar_t ch = *it;
        if( ch == '\n' ) ++lines;
    }

    m_wall->Freeze();                 // Freeze the window to prevent scrollbar jumping
    m_wall->AppendText( displayMsg ); // Add the text
    m_wall->ScrollLines( lines + 1 ); // Scroll down correct number of lines + one (the extra line is important for some cases!)
    m_wall->ShowPosition( m_wall->GetLastPosition() ); // Ensure the last line is shown at the very bottom of the window
    m_wall->Thaw();                   // Allow the window to redraw

}

void
hoxWallOutput::OnClearButton( wxCommandEvent& event )
{
    m_wall->Clear();
}

// ----------------------------------------------------------------------------
// hoxInputTextCtrl
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(hoxInputTextCtrl, wxTextCtrl)
    EVT_MOUSE_EVENTS( hoxInputTextCtrl::OnMouseEvent  )
END_EVENT_TABLE()

hoxInputTextCtrl::hoxInputTextCtrl( wxWindow* parent, wxWindowID id,
                                    const wxString& value /* = wxEmptyString */ )
            : wxTextCtrl( parent, id, value,
                          wxDefaultPosition, wxDefaultSize,
                          wxTE_PROCESS_ENTER | wxSUNKEN_BORDER
                            | wxTE_RICH2 /* Windows only */ )
            , m_bFirstEnter( true )
{
    const wxTextAttr defaultStyle = GetDefaultStyle();
    SetDefaultStyle(wxTextAttr(wxNullColour, *wxLIGHT_GREY));
    AppendText( _("[Type your message here]") );
    SetDefaultStyle(defaultStyle);
}

void
hoxInputTextCtrl::OnMouseEvent( wxMouseEvent& event )
{
    event.Skip();
    if ( event.LeftUp() )
    {
        if ( m_bFirstEnter )
        {
            m_bFirstEnter = false;
            Clear();
        }
        else if ( ! IsEmpty() )
        {
            SelectAll();
        }
    }
}

/************************* END OF FILE ***************************************/
