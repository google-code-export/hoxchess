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
// Name:            hoxMessage.h
// Created:         01/09/2011
//
// Description:     Messages that are used to communicate between
//                  CLient / Server.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_MESSAGE_H__
#define __INCLUDED_HOX_MESSAGE_H__

#include <string>
#include <map>
#include "enums.h"
#include "types.h"

namespace hox {

// ----------------------------------------------------------------- //
//                                                                   //
//                Typedefs                                           //
//                                                                   //
// ----------------------------------------------------------------- //

/**
 * A list of key-value pairs (of strings).
 * The main usage is to contain parameters of a Command exchanged
 * between Client / Server.
 */
typedef std::map<std::string, std::string> Parameters;

typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
typedef boost::char_separator<char>                    Separator;

// ----------------------------------------------------------------- //
//                                                                   //
//                Message                                           //
//                                                                   //
// ----------------------------------------------------------------- //

/**
 * Messages exchanged between Client / Server.
 */
class Message
{
public:
    std::string  m_type;
    Parameters   m_parameters;

    // --- API
    Message( std::string t = "" ) : m_type( t ) {}

    const std::string toString() const;

    std::string& operator[]( const std::string& key )
        { return m_parameters[key]; }

    void clear();  // Clear all data.

public:
    static void
    string_to_message( const std::string& sInput,
                       Message&           message );

    static void
    parse_inCommand_LOGIN( const std::string& sInput,
                           std::string&       pid,
                           int&               nRating );

    static void
    parse_inCommand_I_PLAYERS( const std::string& sInput,
                               StringList&     players );

    static void
    parse_inCommand_LIST( const std::string& sInput,
                          TableList&         tables );

    static void
    parse_inCommand_E_JOIN( const std::string& sInput,
                            std::string&       tableId,
                            std::string&       playerId,
                            int&               nPlayerScore,
                            ColorEnum&         color );

    static void
    parse_inCommand_INVITE( const std::string& sInput,
                            std::string&       inviterId );

    static void
    parse_inCommand_MOVE( const std::string& sInput,
                          std::string&       tableId,
                          std::string&       playerId,
                          std::string&       sMove,
                          GameStatusEnum&    gameStatus );

    static void
    parse_inCommand_E_END( const std::string& sInput,
                           std::string&       tableId,
                           GameStatusEnum&    gameStatus,
                           std::string&       sReason );

    static void
    parse_inCommand_DRAW( const std::string& sInput,
                          std::string&       tableId,
                          std::string&       playerId );

private:
    static void
    parse_one_table( const std::string& sInput,
                     TableInfo&         tableInfo );

};

} // namespace hox

#endif /* __INCLUDED_HOX_MESSAGE_H__ */
