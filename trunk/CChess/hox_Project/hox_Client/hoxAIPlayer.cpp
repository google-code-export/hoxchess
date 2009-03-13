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
// Name:            hoxAIPlayer.cpp
// Created:         05/04/2008
//
// Description:     The Artificial Intelligent (AI) Player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxAIPlayer.h"
#include "hoxUtil.h"
#include "hoxReferee.h"
#include "hoxTable.h"
#include "../plugins/common/AIEngineLib.h"

IMPLEMENT_DYNAMIC_CLASS(hoxAIPlayer, hoxPlayer)

BEGIN_EVENT_TABLE(hoxAIPlayer, hoxPlayer)
    EVT_COMMAND(wxID_ANY, hoxEVT_CONNECTION_RESPONSE, hoxAIPlayer::OnConnectionResponse)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// hoxAIPlayer
//-----------------------------------------------------------------------------

hoxAIPlayer::hoxAIPlayer( const wxString& name,
                          hoxPlayerType   type,
                          int             score )
            : hoxPlayer( name, type, score )
            , m_engineAPI( NULL )
{ 
    wxLogDebug("%s: ENTER.", __FUNCTION__);
}

hoxAIPlayer::~hoxAIPlayer()
{ 
    delete m_engineAPI;
}

void 
hoxAIPlayer::Start()
{
    hoxAIConnection* conn = new hoxAIConnection( this );
    hoxConnection_APtr connection( conn );
    this->SetConnection( connection ); // Release control.
    conn->StartAIEngine( m_engineAPI );
}

void 
hoxAIPlayer::OnConnectionResponse( wxCommandEvent& event )
{
    const hoxResponse_APtr apResponse( wxDynamicCast(event.GetEventObject(), hoxResponse) );

    const wxString sType = hoxUtil::RequestTypeToString(apResponse->type);
    const wxString sContent = apResponse->content;

    switch ( apResponse->type )
    {
        case hoxREQUEST_MOVE:
		{
            const wxString sMove = sContent;
            wxLogDebug("%s: Received Move [%s].", __FUNCTION__, sMove.c_str());

            hoxTable_SPtr pTable = this->GetFrontTable();
            wxCHECK_RET( pTable, "No front table found" );
            pTable->OnMove_FromNetwork( sMove );
            break;
        }
        default:
        {
		    wxLogWarning("%s: Failed to handle Request [%s]. Error = [%s].", 
                __FUNCTION__, sType.c_str(), sContent.c_str());
        }
    }
}


// ----------------------------------------------------------------------------
// hoxAIEngine
// ----------------------------------------------------------------------------

hoxAIEngine::hoxAIEngine( wxEvtHandler* player,
                          AIEngineLib*  engineAPI /* = NULL */ )
        : wxThread( wxTHREAD_JOINABLE )
        , m_player( player )
        , m_shutdownRequested( false )
        , m_engineAPI( engineAPI )
{
}

bool
hoxAIEngine::AddRequest( hoxRequest_APtr apRequest )
{
    if ( m_shutdownRequested )
    {
        wxLogDebug("%s: *WARN* Deny request [%s]. The thread is being shutdown.", 
            __FUNCTION__, hoxUtil::RequestTypeToString(apRequest->type).c_str());
        return false;
    }

    m_requests.PushBack( apRequest );
    m_semRequests.Post();  // Notify...
	return true;
}

void*
hoxAIEngine::Entry()
{
    hoxRequest_APtr apRequest;

    wxLogDebug("%s: ENTER.", __FUNCTION__);

    while (   !m_shutdownRequested
            && m_semRequests.Wait() == wxSEMA_NO_ERROR )
    {
        apRequest = _GetRequest();
        if ( apRequest.get() == NULL )
        {
            wxASSERT_MSG( m_shutdownRequested, "This thread must be shutdowning." );
            break;  // Exit the thread.
        }
        wxLogDebug("%s: Processing request Type = [%s]...", 
            __FUNCTION__, hoxUtil::RequestTypeToString(apRequest->type).c_str());

        this->HandleRequest( apRequest );
    }

    wxLogDebug("%s: END.", __FUNCTION__);
    return NULL;
}

void
hoxAIEngine::HandleRequest( hoxRequest_APtr apRequest )
{
    const hoxRequestType requestType = apRequest->type;

    switch( requestType )
    {
        case hoxREQUEST_MOVE:
        {
            return _HandleRequest_MOVE( apRequest );
        }
        default:
        {
            wxLogDebug("%s: *WARN* Unsupported Request [%s].", 
                __FUNCTION__, hoxUtil::RequestTypeToString(requestType).c_str());
        }
    }
}

void
hoxAIEngine::_HandleRequest_MOVE( hoxRequest_APtr apRequest )
{
    const wxString sMove = apRequest->parameters["move"];
    wxLogDebug("%s: Received Move [%s].", __FUNCTION__, sMove.c_str());

    this->OnOpponentMove( sMove );

    const hoxGameStatus gameStatus =
        hoxUtil::StringToGameStatus( apRequest->parameters["status"] );

    if ( hoxIReferee::IsGameOverStatus( gameStatus ) )
        return;

    const wxString sNextMove = this->GenerateNextMove();
    wxLogDebug("%s: Generated next Move = [%s].", __FUNCTION__, sNextMove.c_str());

    /* Notify the Player. */
    const hoxRequestType type = apRequest->type;
    hoxResponse_APtr apResponse( new hoxResponse(type) );
    wxCommandEvent event( hoxEVT_CONNECTION_RESPONSE, type );
    apResponse->code = hoxRC_OK;
    apResponse->content = sNextMove;
    event.SetEventObject( apResponse.release() );  // Caller will de-allocate.
    wxPostEvent( m_player, event );
}

void
hoxAIEngine::OnOpponentMove( const wxString& sMove )
{
    if ( m_engineAPI )
    {
        const std::string stdMove = hoxUtil::wx2std( sMove );
        m_engineAPI->onHumanMove( stdMove );
    }
}

wxString
hoxAIEngine::GenerateNextMove()
{
    if ( m_engineAPI )
    {
        std::string stdMove = m_engineAPI->generateMove();
        return hoxUtil::std2wx( stdMove );
    }
    return ""; // NOTE: An invalid move;
}

hoxRequest_APtr
hoxAIEngine::_GetRequest()
{
    hoxRequest_APtr apRequest = m_requests.PopFront();
    wxCHECK_MSG(apRequest.get() != NULL, apRequest, "At least one request must exist");

    /* Handle SHUTDOWN request here to avoid the possible memory leaks.
     * The reason is that others (timers, for example) may continue to 
     * send requests to this thread while this thread is shutdowning it self. 
     *
     * NOTE: The SHUTDOWN request is (purposely) handled here inside this function 
     *       because the "mutex-lock" is still being held.
     */

    if ( apRequest->type == hoxREQUEST_SHUTDOWN )
    {
        wxLogDebug("%s: A SHUTDOWN requested just received.", __FUNCTION__);
        m_shutdownRequested = true;
    }

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * NOTE: The shutdown-request can come from:
     *       (1) The code segment ABOVE, or
     *       (2) It can be triggered from the outside callers
     *           who could invoke this->Shutdown() to this Thread.
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     */

    if ( m_shutdownRequested )
    {
        wxLogDebug("%s: Shutting down this thread...", __FUNCTION__);
        apRequest.reset(); /* Release memory and signal "no more request" ...
                            * ... to the caller!
                            */
    }

    return apRequest;
}


// ----------------------------------------------------------------------------
// hoxAIConnection
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(hoxAIConnection, hoxConnection)

hoxAIConnection::hoxAIConnection( wxEvtHandler* player )
        : hoxConnection( player )
{
}

void
hoxAIConnection::Shutdown()
{
    wxLogDebug("%s: Request the AI Engine thread to be shutdown...", __FUNCTION__);
    if ( m_aiEngine.get() != NULL )
    {
        wxThread::ExitCode exitCode = m_aiEngine->Wait();
        wxLogDebug("%s: The AI Engine thread shutdown with exit-code = [%d].", __FUNCTION__, exitCode);
    }
}

bool
hoxAIConnection::AddRequest( hoxRequest_APtr apRequest )
{
    wxCHECK_MSG(m_aiEngine.get() != NULL, false, "AI Engine is not yet created");
    return m_aiEngine->AddRequest( apRequest );
}

void
hoxAIConnection::CreateAIEngine( AIEngineLib* engineAPI )
{
    m_aiEngine.reset( new hoxAIEngine( this->GetPlayer(), engineAPI ) );
}

void
hoxAIConnection::StartAIEngine( AIEngineLib* engineAPI )
{
    if ( m_aiEngine && m_aiEngine->IsRunning() )
    {
        wxLogDebug("%s: The AI Engine already started. END.", __FUNCTION__);
        return;
    }

    wxLogDebug("%s: Create the AI Engine Thread...", __FUNCTION__);
    this->CreateAIEngine( engineAPI);

    if ( m_aiEngine->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogDebug("%s: *WARN* Failed to create the AI Engine thread.", __FUNCTION__);
        m_aiEngine.reset();
        return;
    }
    wxASSERT_MSG(!m_aiEngine->IsDetached(), "The AI Engine thread must be joinable");

    m_aiEngine->Run();
}

/************************* END OF FILE ***************************************/
