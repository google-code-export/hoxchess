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
// Name:            hoxAIPlayer.h
// Created:         05/04/2008
//
// Description:     The Artificial Intelligent (AI) Player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_AI_PLAYER_H__
#define __INCLUDED_HOX_AI_PLAYER_H__

#include "hoxPlayer.h"
#include "hoxTypes.h"
#include "hoxConnection.h"

/* Forward declaration */
class AIEngineLib;

/**
 * The AI player.
 */
class hoxAIPlayer :  public hoxPlayer
{
public:
    hoxAIPlayer() {} // DUMMY default constructor required for event handler.
    hoxAIPlayer( const wxString& name,
                 hoxPlayerType   type,
                 int             score );

    virtual ~hoxAIPlayer();

    // **** Override the parent's API ****
    virtual void Start();

    /*******************************
     * Connection-event handlers
     *******************************/

    void OnConnectionResponse( wxCommandEvent& event ); 

     /*******************************
     * Other API
     *******************************/

    void SetEngineAPI( AIEngineLib*  engineAPI ) { m_engineAPI = engineAPI; }

protected:
    AIEngineLib*  m_engineAPI;

private:

    DECLARE_DYNAMIC_CLASS(hoxAIPlayer)
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// hoxAIEngine
// ----------------------------------------------------------------------------

class hoxAIEngine : public wxThread
{
public:
    hoxAIEngine( wxEvtHandler* player,
                 AIEngineLib*  engineAPI = NULL );
    virtual ~hoxAIEngine() {}

    bool AddRequest( hoxRequest_APtr apRequest );

protected:
    virtual void* Entry();  // Entry point for the thread

    virtual void HandleRequest( hoxRequest_APtr apRequest );

    virtual void    OnOpponentMove( const wxString& sMove );
    virtual wxString GenerateNextMove();

private:
    void            _HandleRequest_MOVE( hoxRequest_APtr apRequest );
    hoxRequest_APtr _GetRequest();

protected:
    wxEvtHandler*           m_player;

    /* Storage to hold pending outgoing request. */
    wxSemaphore             m_semRequests;
    hoxRequestQueue         m_requests;

    bool                    m_shutdownRequested;
                /* Has a shutdown-request been received? */

    AIEngineLib*             m_engineAPI;
};

// ----------------------------------------------------------------------------
// hoxAIConnection
// ----------------------------------------------------------------------------

/* Typedef(s) */
typedef boost::shared_ptr<hoxAIEngine> hoxAIEngine_SPtr;

/**
 * The connection to an AI Engine.
 */
class hoxAIConnection : public hoxConnection
{
public:
    hoxAIConnection() {} // DUMMY default constructor required for RTTI info.
    hoxAIConnection( wxEvtHandler* player );
    virtual ~hoxAIConnection() {}

    // **** Override the parent's API ****
    virtual void Start() {}
    virtual void Shutdown();
    virtual bool AddRequest( hoxRequest_APtr apRequest );
    virtual bool IsConnected() const { return true; }

    // *** My own.
    virtual void StartAIEngine( AIEngineLib* engineAPI );

protected:
    virtual void CreateAIEngine( AIEngineLib* engineAPI );

protected:
    hoxAIEngine_SPtr  m_aiEngine; // The AI Engine thread.

    DECLARE_DYNAMIC_CLASS(hoxAIConnection)
};

#endif /* __INCLUDED_HOX_AI_PLAYER_H__ */
