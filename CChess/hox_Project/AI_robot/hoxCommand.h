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
// Name:            hoxCommand.h
// Created:         10/04/2008
//
// Description:     Containing simple types commonly used through out 
//                  the project.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_COMMAND_H__
#define __INCLUDED_HOX_COMMAND_H__

#include <string>
#include <map>
#include "hoxCommon.h"

/**
 * A list of key-value pairs (of strings).
 * The main usage is to contain parameters of a Command exchanged
 * between Client / Server.
 */
typedef std::map<std::string, std::string> hoxParameters;

/**
 * Command exchanged between Client / Server.
 */
class hoxCommand
{
public:
    std::string    m_type;
    hoxParameters  m_parameters;

    // --- API
    hoxCommand( std::string t = "" ) : m_type( t ) {}

    const std::string ToString() const;

    std::string& operator[]( const std::string& key )
        { return m_parameters[key]; }

    void Clear();  // Clear all data.

public:
    static void
    String_To_Command( const std::string& sInput, 
                       hoxCommand&        command );

    static void
    Parse_InCommand_E_JOIN( const std::string& sInput,
                            std::string&       tableId,
                            std::string&       playerId,
                            int&               nPlayerScore,
                            hoxColor&          color );

    static void
    Parse_InCommand_MOVE( const std::string& sInput,
                          std::string&       tableId,
                          std::string&       playerId,
                          std::string&       sMove,
                          hoxGameStatus&     gameStatus );

    static void
    Parse_InCommand_E_END( const std::string& sInput,
                           std::string&       tableId,
                           hoxGameStatus&     gameStatus,
                           std::string&       sReason );

    static void
    Parse_InCommand_DRAW( const std::string& sInput,
                          std::string&       tableId,
                          std::string&       playerId );

};

#endif /* __INCLUDED_HOX_COMMAND_H__ */
