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
// Name:            hoxMyPlayer.cpp
// Created:         10/28/2007
//
// Description:     The LOCAL Player specialized to login to
//                  my "games.PlayXiangqi.com" server
/////////////////////////////////////////////////////////////////////////////

#include <asio.hpp>
#include "hoxMyPlayer.h"
#include "hoxSocketConnection.h"
#include "hoxSite.h"
#include "hoxUtil.h"

#include <wx/tokenzr.h>

IMPLEMENT_DYNAMIC_CLASS(hoxMyPlayer, hoxLocalPlayer)

BEGIN_EVENT_TABLE(hoxMyPlayer, hoxLocalPlayer)
    EVT_COMMAND(hoxREQUEST_PLAYER_DATA, hoxEVT_CONNECTION_RESPONSE, hoxMyPlayer::OnConnectionResponse_PlayerData)
    EVT_COMMAND(wxID_ANY, hoxEVT_CONNECTION_RESPONSE, hoxMyPlayer::OnConnectionResponse)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// _ContentTokenizer
//-----------------------------------------------------------------------------

class _ContentTokenizer : public wxStringTokenizer
{
public:
    _ContentTokenizer( const wxString& sContent )
        : wxStringTokenizer( sContent, ";", wxTOKEN_RET_EMPTY )
        {}
    const int GetNextInt() { return ::atoi( GetNextToken().c_str() ); }
};

//-----------------------------------------------------------------------------
// hoxMyPlayer
//-----------------------------------------------------------------------------

hoxMyPlayer::hoxMyPlayer( const wxString& name,
                          hoxPlayerType   type,
                          int             score )
            : hoxLocalPlayer( name, type, score )
            , m_bLoginSuccess( false )
{ 
}

void
hoxMyPlayer::Start()
{
    wxASSERT_MSG(m_site, "Site must be set first");
    hoxServerAddress address = m_site->GetAddress();
    hoxConnection_APtr connection( new hoxSocketConnection( address, this ) );
    this->SetConnection( connection );
}

void 
hoxMyPlayer::OnConnectionResponse_PlayerData( wxCommandEvent& event )
{
    const hoxResponse_APtr apResponse( wxDynamicCast(event.GetEventObject(), hoxResponse) );

    /* Handle error-code. */

    if ( apResponse->code != hoxRC_OK )
    {
        wxLogDebug("%s: *INFO* Received response-code [%s].", 
            __FUNCTION__, hoxUtil::ResultToStr(apResponse->code));

        if ( m_bLoginSuccess )
        {
            this->LeaveAllTables();
            this->DisconnectFromServer();
            m_site->OnShutdownReadyFromLocalPlayer();
            wxLogDebug("%s: END (exception).", __FUNCTION__);
        }
        return;  // *** Exit immediately.
    }

    /* Handle other type of data. */

    hoxCommand  command;
    if ( hoxRC_OK != _ParseCommand( apResponse->data, command ) )
    {
        wxLogError("%s: Failed to parse command-data.", __FUNCTION__);
        return;
    }

    const wxString sType    = hoxUtil::RequestTypeToString(command.type);
    const wxString sCode    = command.parameters["code"];
    const wxString sTableId = command.parameters["tid"];
    const wxString sContent = command.parameters["content"];

    wxLogDebug("%s: Received a command [%s].", __FUNCTION__, sType.c_str());

    /* Lookup Table if the Table-Id is provided.
     * NOTE: It is possible that the table is not found.
     *       That is the case when, for example, other player is inviting you
     *       to his/her table.
     */
    hoxTable_SPtr  pTable;
    if ( ! sTableId.empty() )
    {
        pTable = m_site->FindTable( sTableId );
    }

    /* Handle the error-code. */
    if ( sCode != "0" ) // failed?
    {
        const wxString sMessage = "Request " + sType + " failed with code = " + sCode;
        wxLogDebug("%s: %s.", __FUNCTION__, sMessage.c_str());

        // TODO: Should we post to the 'active' table?
        if ( pTable )
        {
            pTable->PostBoardMessage( sMessage );
        }
        // *** Allow to continue
    }

    switch ( command.type )
    {
        case hoxREQUEST_LOGIN:       return _HandleEvent_LOGIN( sCode, sContent, apResponse );
        case hoxREQUEST_LOGOUT:      return _HandleEvent_LOGOUT( sContent );
        case hoxREQUEST_LIST:        return _HandleEvent_LIST( sContent );
        case hoxREQUEST_I_PLAYERS:   return _HandleEvent_I_PLAYERS( sContent );
        case hoxREQUEST_I_TABLE:     return _HandleEvent_I_TABLE( sContent );
        case hoxREQUEST_LEAVE:       return _HandleEvent_LEAVE( sContent );
        case hoxREQUEST_UPDATE:      return _HandleEvent_UPDATE( sContent );
        case hoxREQUEST_E_JOIN:      return _HandleEvent_E_JOIN( sContent );
        case hoxREQUEST_MSG:         return _HandleEvent_MSG( sTableId, sContent, pTable );
        case hoxREQUEST_MOVE:        return _HandleEvent_MOVE( sContent );
        case hoxREQUEST_DRAW:        return _HandleEvent_DRAW( sContent );
        case hoxREQUEST_RESET:       return _HandleEvent_RESET( sContent );
        case hoxREQUEST_E_END:       return _HandleEvent_E_END( sContent );
        case hoxREQUEST_E_SCORE:     return _HandleEvent_E_SCORE( sContent );
        case hoxREQUEST_I_MOVES:     return _HandleEvent_I_MOVES( sContent );
        case hoxREQUEST_INVITE:      return _HandleEvent_INVITE( sTableId, sContent );
        case hoxREQUEST_PLAYER_INFO: return _HandleEvent_PLAYER_INFO( sContent );
        default:
        {
		    wxLogDebug("%s: *WARN* Unexpected command [%s].", __FUNCTION__, sType.c_str());
        }
    } // switch()
}

void 
hoxMyPlayer::OnConnectionResponse( wxCommandEvent& event )
{
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     *
     * NOTE: Currently, this function exists to handle errors only.
     *
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    const hoxResponse_APtr apResponse( wxDynamicCast(event.GetEventObject(), hoxResponse) );

    const wxString sType = hoxUtil::RequestTypeToString(apResponse->type);
    const wxString sContent = apResponse->content;

    switch ( apResponse->type )
    {
        case hoxREQUEST_LOGIN:
		{
            if ( apResponse->code != hoxRC_OK )
            {
                wxLogDebug("%s: *WARN* Failed to login [%s].", __FUNCTION__, sContent.c_str());
                _OnLoginFailure( apResponse );
            }
            break;
        }
        default:
        {
            wxLogWarning("%s: Unsupported Request [%s].", __FUNCTION__, sType.c_str());
        }
    }
}

hoxResult
hoxMyPlayer::_ParseCommand( const wxMemoryBuffer& data,
                            hoxCommand&           command ) const
{
    /* TODO: Force to convert the buffer to a string. */
    const size_t nDataLen = data.GetDataLen();
    const wxString sCommand = wxString::FromUTF8( (const char*) data.GetData(),
                                                   nDataLen );
    if ( nDataLen > 0 && sCommand.empty() ) // failed?
    {
        wxLogWarning("%s: Fail to convert [%d] bytes of data to string.", __FUNCTION__, nDataLen);
        return hoxRC_ERR;
    }

    wxStringTokenizer tkz( sCommand, "&" );
    wxString          token;
    wxString          paramName;
    wxString          paramValue;

    while ( tkz.HasMoreTokens() )
    {
        token = tkz.GetNextToken();
        paramName = token.BeforeFirst( '=' );
        paramValue = token.AfterFirst( '=' );

        // Special case for "op" param-name.
        if ( paramName == "op" )
        {
            command.type = hoxUtil::StringToRequestType( paramValue );
            if ( command.type == hoxREQUEST_UNKNOWN )
            {
                wxLogError("%s: Unsupported command-type = [%s].", __FUNCTION__, paramValue.c_str());
                return hoxRC_NOT_SUPPORTED;
            }
        }
        else
        {
			paramValue.Trim();
            command.parameters[paramName] = paramValue;
        }
    }

    return hoxRC_OK;
}

void 
hoxMyPlayer::_HandleEvent_LOGIN( const wxString&         sCode,
                                 const wxString&         sContent,
                                 const hoxResponse_APtr& apResponse )
{
    if ( sCode != "0" )
    {
        wxLogDebug("%s: *WARN* LOGIN error [%s: %s].", __FUNCTION__, sCode.c_str(), sContent.c_str());
        apResponse->code = hoxRC_ERR;
        _OnLoginFailure( apResponse );
        return;
    }

    /* Handle data. */

    _ContentTokenizer tkz( sContent );
    const wxString sPlayerId = tkz.GetNextToken();
	const int      nScore    = tkz.GetNextInt();

    wxLogDebug("%s: LOGIN from [%s %d].", __FUNCTION__, sPlayerId.c_str(), nScore);

    if ( sPlayerId == this->GetId() )
    {
        m_bLoginSuccess = true;  // *** Record this LOGIN event.

        wxLogDebug("%s: Set my score = [%d].", __FUNCTION__, nScore);
        this->SetScore( nScore );

        apResponse->code = hoxRC_OK;
        m_site->OnResponse_LOGIN( apResponse );
    }

    m_site->OnPlayerLoggedIn( sPlayerId, nScore );
}

void 
hoxMyPlayer::_HandleEvent_LOGOUT( const wxString& sContent )
{
    const wxString sPlayerId = sContent;
    wxLogDebug("%s: LOGOUT from [%s].", __FUNCTION__, sPlayerId.c_str());
    m_site->OnPlayerLoggedOut( sPlayerId );
}

void
hoxMyPlayer::_HandleEvent_LIST( const wxString& sContent )
{
    hoxNetworkTableInfoList tableList;

	wxStringTokenizer tkz( sContent, "\n" );
    hoxNetworkTableInfo tableInfo;

	while ( tkz.HasMoreTokens() )
	{
        const wxString token = tkz.GetNextToken();

        _ParseOneNetworkTable(token, tableInfo);
		tableList.push_back( tableInfo );
    }

    m_site->DisplayListOfTables( tableList );
}

void
hoxMyPlayer::_HandleEvent_I_PLAYERS( const wxString& sContent )
{
	wxStringTokenizer mainTkz( sContent, "\n" );
    wxString          sPlayerId;
    int               nScore = 0;

	while ( mainTkz.HasMoreTokens() )
	{
        _ContentTokenizer tkz( mainTkz.GetNextToken() );
        sPlayerId = tkz.GetNextToken();
        nScore    = tkz.GetNextInt();

        wxLogDebug("%s: ... PLAYER [%s %d].", __FUNCTION__, sPlayerId.c_str(), nScore);
        m_site->OnPlayerLoggedIn( sPlayerId, nScore );
    }
}

void
hoxMyPlayer::_HandleEvent_I_TABLE( const wxString& sContent )
{
    hoxNetworkTableInfo tableInfo;

    _ParseOneNetworkTable( sContent, tableInfo );
    m_site->JoinLocalPlayerToTable( tableInfo );
}

void
hoxMyPlayer::_HandleEvent_LEAVE( const wxString& sContent )
{
    _ContentTokenizer tkz( sContent );
    const wxString tableId = tkz.GetNextToken();
    const wxString leaveId = tkz.GetNextToken();

    wxLogDebug("%s: Player [%s] left Table [%s].", __FUNCTION__, 
        leaveId.c_str(), tableId.c_str());

    hoxTable_SPtr pTable = m_site->FindTable( tableId );
    if ( ! pTable )
    {
        wxLogDebug("%s: Table [%s] not found.", __FUNCTION__, tableId.c_str());
        return;
    }

    hoxPlayer* leavePlayer = m_site->FindPlayer( leaveId );
    if ( ! leavePlayer ) 
    {
        wxLogDebug("%s: Player [%s] not found.", __FUNCTION__, leaveId.c_str());
        return;
    }

    pTable->OnLeave_FromNetwork( leavePlayer );
}

void
hoxMyPlayer::_HandleEvent_UPDATE( const wxString& sContent )
{
    _ContentTokenizer tkz( sContent );
    const wxString    tableId  = tkz.GetNextToken();
    const wxString    playerId = tkz.GetNextToken();
    const hoxGameType gameType = (  tkz.GetNextToken() == "1"
                                  ? hoxGAME_TYPE_RATED
                                  : hoxGAME_TYPE_NONRATED );
    const hoxTimeInfo newTimeInfo = hoxUtil::StringToTimeInfo( tkz.GetNextToken() );

    wxLogDebug("%s: Player [%s] updated Timers to [%s] in Table [%s].", __FUNCTION__, 
        playerId.c_str(), hoxUtil::TimeInfoToString(newTimeInfo).c_str(),
        tableId.c_str());

    hoxTable_SPtr pTable = m_site->FindTable( tableId );
    if ( ! pTable )
    {
        wxLogDebug("%s: Table [%s] not found.", __FUNCTION__, tableId.c_str());
        return;
    }

    hoxPlayer* player = m_site->FindPlayer( playerId );
    if ( ! player ) 
    {
        wxLogDebug("%s: Player [%s] not found.", __FUNCTION__, playerId.c_str());
        return;
    }

    pTable->OnUpdate_FromPlayer( player, gameType, newTimeInfo );
}

void
hoxMyPlayer::_HandleEvent_E_JOIN( const wxString& sContent )
{
    _ContentTokenizer tkz( sContent );
    const wxString tableId  = tkz.GetNextToken();
    const wxString playerId = tkz.GetNextToken();
    const int      nScore   = tkz.GetNextInt();
    const hoxColor color    = hoxUtil::StringToColor( tkz.GetNextToken() );

    wxLogDebug("%s: Player [%s %d] joining Table [%s] as [%d].", __FUNCTION__,
        playerId.c_str(), nScore, tableId.c_str(), color);

    if ( hoxRC_OK != m_site->OnPlayerJoined( tableId, 
                                             playerId, nScore,
                                             color ) )
    {
        wxLogDebug("%s: *WARN* Failed to ask table to join as color [%d].",
            __FUNCTION__, color);
        return;
    }
}

void
hoxMyPlayer::_HandleEvent_MSG( const wxString&      sTableId,
                               const wxString&      sContent,
                               const hoxTable_SPtr& pTable )
{
    _ContentTokenizer tkz( sContent );
    const wxString senderId = tkz.GetNextToken();
    const wxString message  = tkz.GetNextToken();

    /* NOTE: For now, just assume that if no 'table' was specified,
     *       then this is a "private" message.
     */
    const bool bPublic = !sTableId.empty();
    wxLogDebug("%s: Player [%s] sent message [%s].", __FUNCTION__,
        senderId.c_str(), message.c_str());

    hoxTable_SPtr pReceiveTable = pTable;
    if ( ! pReceiveTable )
    {
        pReceiveTable = this->GetActiveTable();
    }

    if ( pReceiveTable )
    {
        pReceiveTable->OnMessage_FromNetwork( senderId, message, bPublic );
    }
    else
    {
        const wxString sCaption =
            (   bPublic ? _("Message from Player")
                        : wxString::Format( _("A private message from [%s]"),
                                            senderId.c_str()) );
        ::wxMessageBox( message, sCaption, wxOK|wxICON_INFORMATION );
    }
}

void
hoxMyPlayer::_HandleEvent_MOVE( const wxString& sContent )
{
    _ContentTokenizer tkz( sContent );
    const wxString tableId  = tkz.GetNextToken();
    const wxString playerId = tkz.GetNextToken(); // Who sent the Move?
    const wxString sMove    = tkz.GetNextToken();

    wxLogDebug("%s: Player [%s] sent move [%s] in Table [%s].", __FUNCTION__, 
        playerId.c_str(), sMove.c_str(), tableId.c_str());

    hoxTable_SPtr pTable = m_site->FindTable( tableId );
    if ( ! pTable )
    {
        wxLogDebug("%s: Table [%s] not found.", __FUNCTION__, tableId.c_str());
        return;
    }

    pTable->OnMove_FromNetwork( sMove );
}

void
hoxMyPlayer::_HandleEvent_DRAW( const wxString& sContent )
{
    _ContentTokenizer tkz( sContent );
    const wxString tableId  = tkz.GetNextToken();
    const wxString playerId = tkz.GetNextToken();

    wxLogDebug("%s: Player [%s] requested 'Draw' in Table [%s].", 
        __FUNCTION__, playerId.c_str(), tableId.c_str());

    hoxTable_SPtr pTable = m_site->FindTable( tableId );
    if ( ! pTable )
    {
        wxLogDebug("%s: Table [%s] not found.", __FUNCTION__, tableId.c_str());
        return;
    }

    hoxPlayer* player = m_site->FindPlayer( playerId );
    if ( ! player ) 
    {
        wxLogDebug("%s: Player [%s] not found.", __FUNCTION__, playerId.c_str());
        return;
    }

    pTable->OnDrawRequest_FromNetwork( player );
}

void
hoxMyPlayer::_HandleEvent_RESET( const wxString& sContent )
{
    const wxString tableId = sContent;
    wxLogDebug("%s: RESET Table [%s].", __FUNCTION__, tableId.c_str());

    hoxTable_SPtr pTable = m_site->FindTable( tableId );
    if ( ! pTable )
    {
        wxLogDebug("%s: Table [%s] not found.", __FUNCTION__, tableId.c_str());
        return;
    }

    pTable->OnGameReset_FromNetwork();
}

void
hoxMyPlayer::_HandleEvent_E_END( const wxString& sContent )
{
    _ContentTokenizer tkz( sContent );
    const wxString      tableId = tkz.GetNextToken();
    const hoxGameStatus status  = hoxUtil::StringToGameStatus( tkz.GetNextToken() );
    const wxString      sReason = tkz.GetNextToken();

    wxLogDebug("%s: Table [%s] ended. Status = [%d]. Reason = [%s]",
        __FUNCTION__, tableId.c_str(), status, sReason.c_str());

    hoxTable_SPtr pTable = m_site->FindTable( tableId );
    if ( ! pTable )
    {
        wxLogDebug("%s: Table [%s] not found.", __FUNCTION__, tableId.c_str());
        return;
    }

    pTable->OnGameOver_FromNetwork( status );
}

void
hoxMyPlayer::_HandleEvent_E_SCORE( const wxString& sContent )
{
    _ContentTokenizer tkz( sContent );
    const wxString tableId  = tkz.GetNextToken();
    const wxString playerId = tkz.GetNextToken();
    const int      nScore   = tkz.GetNextInt();

    wxLogDebug("%s: Inform table [%s] of player [%s] new Score [%d].", 
        __FUNCTION__, tableId.c_str(), playerId.c_str(), nScore);

    hoxTable_SPtr pTable = m_site->FindTable( tableId );
    if ( ! pTable )
    {
        wxLogDebug("%s: Table [%s] not found.", __FUNCTION__, tableId.c_str());
        return;
    }

    hoxPlayer* player = m_site->FindPlayer( playerId );
    if ( ! player ) 
    {
        wxLogDebug("%s: Player [%s] not found.", __FUNCTION__, playerId.c_str());
        return;
    }

    player->SetScore( nScore );
    pTable->OnScore_FromNetwork( player );
}

void
hoxMyPlayer::_HandleEvent_I_MOVES( const wxString& sContent )
{
    _ContentTokenizer tkz( sContent );
    const wxString tableId = tkz.GetNextToken();
    const wxString sMoves  = tkz.GetNextToken();

    wxLogDebug("%s: Inform Table [%s] of past Moves [%s].", __FUNCTION__, 
        tableId.c_str(), sMoves.c_str());

    hoxTable_SPtr pTable = m_site->FindTable( tableId );
    if ( ! pTable )
    {
        wxLogDebug("%s: Table [%s] not found.", __FUNCTION__, tableId.c_str());
        return;
    }

    hoxStringList moves;
    _ParseMovesString( sMoves, moves );

    pTable->OnPastMoves_FromNetwork( moves );
}

void
hoxMyPlayer::_HandleEvent_INVITE( const wxString& sTableId,
                                  const wxString& sContent )
{
    _ContentTokenizer tkz( sContent );
    const wxString inviterId     = tkz.GetNextToken();
    const int      nInviterScore = tkz.GetNextInt();
    const wxString inviteeId     = tkz.GetNextToken(); 
            /* TODO: Not used now but can be used for verification! */
    
    const wxString sInvitorTable = ( sTableId.empty() ? "?" : sTableId );

    const wxString sMessage =
        wxString::Format(_("*INVITE from [%s (%d)] to Table [%s]"),
        inviterId.c_str(), nInviterScore, sInvitorTable.c_str());

    const hoxTable_SPtr pActiveTable = this->GetActiveTable();
    if ( pActiveTable )
    {
        pActiveTable->PostBoardMessage( sMessage );
    }
    else
    {
        ::wxMessageBox( sMessage, _("Invitation from Player"),
                        wxOK | wxICON_INFORMATION );
    }
}

void
hoxMyPlayer::_HandleEvent_PLAYER_INFO( const wxString& sContent )
{
    hoxPlayerStats  playerStats;

    _ContentTokenizer tkz( sContent );
    playerStats.id    = tkz.GetNextToken();
    playerStats.score = tkz.GetNextInt();

    hoxTable_SPtr pTable = this->GetActiveTable();

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // TODO: HOXServer does not support player's statistics yet.
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    const wxString sMessage = wxString::Format("*INFO: %s %d W%d D%d L%d",
        playerStats.id.c_str(), playerStats.score,
        playerStats.wins, playerStats.draws, playerStats.losses);
    if ( pTable )
    {
        pTable->PostBoardMessage( sMessage );
    }
    else
    {
        ::wxMessageBox( sMessage, _("Information of Player"),
                        wxOK | wxICON_INFORMATION );
    }
}

void
hoxMyPlayer::_OnLoginFailure( const hoxResponse_APtr& apResponse )
{
    wxLogDebug("%s: ENTER.", __FUNCTION__);
    m_site->OnResponse_LOGIN( apResponse );

    /* NOTE:
     *   PlayXiangqi server automatically closes the connection
     *   after a login-failure.
     */
    m_site->OnShutdownReadyFromLocalPlayer();
}

void
hoxMyPlayer::_ParseOneNetworkTable( const wxString&      tableStr,
                                    hoxNetworkTableInfo& tableInfo )
{
	tableInfo.Clear();

    _ContentTokenizer tkz( tableStr );
    tableInfo.id    = tkz.GetNextToken();
	tableInfo.group = ( tkz.GetNextToken() == "0" ? hoxGAME_GROUP_PUBLIC 
						                          : hoxGAME_GROUP_PRIVATE );
	tableInfo.gameType = ( tkz.GetNextToken() == "0" ? hoxGAME_TYPE_RATED 
						                             : hoxGAME_TYPE_NONRATED );
    tableInfo.initialTime = hoxUtil::StringToTimeInfo( tkz.GetNextToken() );
    tableInfo.redTime     = hoxUtil::StringToTimeInfo( tkz.GetNextToken() );
    tableInfo.blackTime   = hoxUtil::StringToTimeInfo( tkz.GetNextToken() );
    tableInfo.redId       = tkz.GetNextToken();
    tableInfo.redScore    = tkz.GetNextToken();
    tableInfo.blackId     = tkz.GetNextToken();
    tableInfo.blackScore  = tkz.GetNextToken();
}

hoxResult
hoxMyPlayer::_ParseMovesString( const wxString& sMoves,
                                hoxStringList&  moves )
{
	wxStringTokenizer tkz( sMoves, "/", wxTOKEN_STRTOK /* no emtpy token */ );
	wxString token;

	while ( tkz.HasMoreTokens() )
	{
		token = tkz.GetNextToken();
        moves.push_back( token );
	}		

	return hoxRC_OK;
}

/************************* END OF FILE ***************************************/
