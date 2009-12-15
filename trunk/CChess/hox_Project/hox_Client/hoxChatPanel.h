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
// Name:            hoxChatPanel.h
// Created:         12/14/2009
//
// Description:     The panel to hold either a "Table"-Chat session
//                  or a "Private"-Chat session.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_CHAT_PANEL_H__
#define __INCLUDED_HOX_CHAT_PANEL_H__

#include "hoxTypes.h"
#include "hoxLocalPlayer.h"

class hoxWallOutput;
class hoxInputTextCtrl;

// ----------------------------------------------------------------------------
// The Chat-Panel class
// ----------------------------------------------------------------------------

wxDECLARE_EVENT(hoxEVT_CHAT_INPUT_ENTER, wxCommandEvent);

class hoxChatPanel : public wxPanel
{
public:
    hoxChatPanel( wxWindow* parent, wxWindowID id,
                  const wxString& sCaption,
                  const wxString& ownerId );

    void OnMessageFrom( const wxString& senderId,
                        const wxString& sMessage,
                        bool            bPrivate = false );

    void SetPrivateMode( bool bPrivate ) { m_inPrivateMode = bPrivate; }

protected:
    void OnWallInputEnter( wxCommandEvent& event );

private:
    const wxString    m_sCaption;
    wxString          m_sOwnerId; // The Local Player's ID.
    bool              m_inPrivateMode;

    hoxWallOutput*    m_wallOutput;
    wxTextCtrl*       m_wallInput;

    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// The Chat-Window class
// ----------------------------------------------------------------------------

class hoxChatWindow : public wxDialog
{
public:
    hoxChatWindow( wxWindow*       parent,
                   const wxString& title,
                   hoxLocalPlayer* localPlayer,
                   wxString        sOtherId );

    void OnMessageFrom( const wxString& senderId,
                        const wxString& sMessage );

    wxString GetOtherId() const { return m_sOtherId; }

    void OnChatInputEnter( wxCommandEvent& event );
    void OnClose( wxCloseEvent& event );

private:
    hoxChatPanel*    m_chatPanel;
    hoxLocalPlayer*  m_localPlayer;
    wxString         m_sOtherId; // The other Player's ID.

    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// hoxWallOutput
// ----------------------------------------------------------------------------

class hoxWallOutput : public wxPanel
{
public:
    hoxWallOutput( wxWindow* parent, wxWindowID id,
                   const wxString& sCaption  );

    void AppendMessage( const wxString& who,
                        const wxString& sMessage,
                        bool            bPrivate = false );

protected:
    void OnClearButton( wxCommandEvent& event );

private:
    void _CreateUI();

private:
    const wxString   m_sCaption;
    wxTextCtrl*      m_wall;

    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// hoxInputTextCtrl
// ----------------------------------------------------------------------------

class hoxInputTextCtrl : public wxTextCtrl
{
public:
    hoxInputTextCtrl(wxWindow *parent, wxWindowID id,
                     const wxString& value = wxEmptyString );

protected:
    void OnMouseEvent( wxMouseEvent& event );

private:
    bool  m_bFirstEnter;

    DECLARE_EVENT_TABLE()
};

#endif /* __INCLUDED_HOX_CHAT_PANEL_H__ */
