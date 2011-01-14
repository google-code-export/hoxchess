/***************************************************************************
 *  Copyright 2010-2011 Huy Phan  <huyphan@playxiangqi.com>                *
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
// Name:            hoxMessage.cpp
// Created:         01/09/2011
//
// Description:     Messages that are used to communicate between
//                  CLient / Server.
/////////////////////////////////////////////////////////////////////////////

#include "hoxMessage.h"
#include <boost/algorithm/string.hpp>  // trim_right()
#include "common/hoxUtil.h"

namespace hox {

// ----------------------------------------------------------------------------
// Message
// ----------------------------------------------------------------------------

const std::string
Message::toString() const
{
    std::string result;

    if (m_type.empty())
    {
        return result;
    }

    result += "op=" + m_type;

    for ( Parameters::const_iterator it = m_parameters.begin();
                                     it != m_parameters.end(); ++it )
    {
        result += "&" + it->first + "=" + it->second;
    }

    return result;
}

void
Message::clear()
{
    m_type = "";
    m_parameters.clear();
}

/* static */ void
Message::string_to_message( const std::string& sInput,
                            Message&           message )
{
    message.clear();

    Separator sep("&");
    Tokenizer tok(sInput, sep);

    for ( Tokenizer::iterator it = tok.begin(); it != tok.end(); ++it )
    {
        const std::string token = (*it);

        size_t sepIndex = token.find_first_of( '=' );

        if ( sepIndex == std::string::npos )
        {
            continue;  // NOTE: Ignore this 'error' token.
        }

        std::string paramName  = token.substr( 0, sepIndex );
        std::string paramValue = token.substr( sepIndex + 1 );

        if ( paramName == "op" ) // Special case for "op" param.
        {
            message.m_type = paramValue;
        }
        else
        {
            boost::trim_right( paramValue );
            message.m_parameters[paramName] = paramValue;
        }
    }
}

/* static */ void
Message::parse_inCommand_LOGIN( const std::string& sInput,
                                std::string&       pid,
                                int&               nRating )
{
    Tokenizer tok( sInput, Separator(";") );
    Tokenizer::iterator it = tok.begin();
    pid = *it++;
    nRating = ::atoi( (*it++).c_str() );
}

/* static */ void
Message::parse_inCommand_I_PLAYERS( const std::string& sInput,
                                    StringList&        players )
{
    players.clear();

    Tokenizer tok( sInput, Separator("\n") );

    std::string playerId;
    for ( Tokenizer::iterator it = tok.begin(); it != tok.end(); ++it )
    {
        const std::string token = (*it);
        // NOTE: Only extract player-ID for now (i.e., ignore score...)
        std::string::size_type foundIndex = token.find_first_of(';');
        if ( foundIndex != std::string::npos )
        {
            playerId = token.substr(0, foundIndex);
            players.push_back( playerId );
        }
    }
}

/* static */ void
Message::parse_inCommand_LIST( const std::string& sInput,
                               TableList&         tables )
{
    tables.clear();

    Tokenizer tok( sInput, Separator("\n") );
    for ( Tokenizer::iterator it = tok.begin(); it != tok.end(); ++it )
    {
        TableInfo_SPtr pTableInfo( new TableInfo );
        Message::parse_one_table( *it, *pTableInfo );
        tables.push_back( pTableInfo );
    }
}

/* static */ void
Message::parse_inCommand_E_JOIN( const std::string& sInput,
                                 std::string&       tableId,
                                 std::string&       playerId,
                                 int&               nPlayerScore,
                                 ColorEnum&         color )
{
    nPlayerScore = 0;
    color        = HC_COLOR_NONE; // Default = observer.

    Tokenizer tok(sInput, Separator(";"));

    int i = 0;
    for ( Tokenizer::iterator it = tok.begin(); it != tok.end(); ++it )
    {
        const std::string token = (*it);
        switch ( i++ )
        {
            case 0: tableId = token;  break;
            case 1: playerId = token;  break;
            case 2: nPlayerScore = ::atoi( token.c_str() ); break;
            case 3: color = util::stringToColor( token ); break;
            default: /* Ignore the rest. */ break;
        }
    }
}

/* static */ void
Message::parse_inCommand_INVITE( const std::string& sInput,
                                 std::string&       inviterId )
{
    inviterId = "";

    Tokenizer tok(sInput, Separator(";"));

    int i = 0;
    for ( Tokenizer::const_iterator it = tok.begin(); it != tok.end(); ++it )
    {
        const std::string token = (*it);
        switch ( i++ )
        {
            case 0: inviterId  = token;  break;
            case 1: /* nInviterScore = token */;  break;
            case 2: /*inviteeId  = token*/;  break;
            default: /* Ignore the rest. */ break;
        }
    }
}

/* static */ void
Message::parse_inCommand_MOVE( const std::string& sInput,
                               std::string&       tableId,
                               std::string&       playerId,
                               std::string&       sMove,
                               GameStatusEnum&    gameStatus)
{
    tableId    = "";
    playerId   = "";
    sMove      = "";
    gameStatus = HC_GAME_STATUS_UNKNOWN;

    Tokenizer tok(sInput, Separator(";"));

    int i = 0;
    for ( Tokenizer::iterator it = tok.begin(); it != tok.end(); ++it )
    {
        const std::string token = (*it);
        switch ( i++ )
        {
            case 0: tableId  = token;  break;
            case 1: playerId = token;  break;
            case 2: sMove    = token;  break;
            case 3: gameStatus = util::stringToGameStatus(token);  break;
            default: /* Ignore the rest. */ break;
        }
    }
}

/* static */ void
Message::parse_inCommand_E_END( const std::string& sInput,
                                std::string&       tableId,
                                GameStatusEnum&     gameStatus,
                                std::string&       sReason )
{
    tableId    = "";
    gameStatus = HC_GAME_STATUS_UNKNOWN;
    sReason    = "";

    Tokenizer tok(sInput, Separator(";"));

    int i = 0;
    for ( Tokenizer::iterator it = tok.begin(); it != tok.end(); ++it )
    {
        const std::string token = (*it);
        switch (i++)
        {
            case 0: tableId  = token;  break;
            case 1: gameStatus = util::stringToGameStatus(token);  break;
            case 2: sReason  = token;  break;
            default: /* Ignore the rest. */ break;
        }
    }
}

/* static */ void
Message::parse_inCommand_DRAW( const std::string& sInput,
                               std::string&       tableId,
                               std::string&       playerId )
{
    tableId    = "";
    playerId   = "";

    Tokenizer tok(sInput, Separator(";"));

    int i = 0;
    for ( Tokenizer::iterator it = tok.begin(); it != tok.end(); ++it )
    {
        const std::string token = (*it);
        switch (i++)
		{
			case 0: tableId  = token;  break;
            case 1: playerId = token;  break;
			default: /* Ignore the rest. */ break;
		}
	}
}

/* static */ void
Message::parse_one_table( const std::string& sInput,
                          TableInfo&         tableInfo )
{
   tableInfo.clear();

    Separator sep(";", "" /* kept_delims */, boost::keep_empty_tokens);
    Tokenizer tok(sInput, sep);
    Tokenizer::iterator it = tok.begin();

    tableInfo.id = *it++;
    it++; // Skip this PUBLIC/PRIVATE field.
    tableInfo.rated = ( (*it++) == "0" );
    tableInfo.initialTime = hox::util::stringToTimeInfo(*it++);
    tableInfo.redTime = hox::util::stringToTimeInfo(*it++);
    tableInfo.blackTime = hox::util::stringToTimeInfo(*it++);
    tableInfo.redId = *it++;
    tableInfo.redRating = *it++;
    tableInfo.blackId = *it++;
    tableInfo.blackRating = *it++;
}

} // namespace hox

/************************* END OF FILE ***************************************/
