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
// Name:            hoxNetworkAPI.cpp
// Created:         10/26/2007
//
// Description:     Containing network related APIs specific to this project.
/////////////////////////////////////////////////////////////////////////////

#include "hoxNetworkAPI.h"
#include "hoxUtil.h"

#include <wx/tokenzr.h>

//-----------------------------------------------------------------------------
// hoxNetworkAPI namespace
//-----------------------------------------------------------------------------

hoxResult
hoxNetworkAPI::ParseCommand( const wxMemoryBuffer& data,
                             hoxCommand&     command )
{
    /* TODO: Force to convert the buffer to a string. */

    const wxString commandStr =
        wxString::FromUTF8( (const char*) data.GetData(), data.GetDataLen() );
    if ( data.GetDataLen() > 0 && commandStr.empty() ) // failed?
    {
        wxLogDebug("%s: *WARN* Fail to convert [%d] data to string.", 
            __FUNCTION__, data.GetDataLen());
        return hoxRC_ERR;
    }

    wxStringTokenizer tkz( commandStr, "&" );

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();

        size_t foundIndex = token.find( '=' );
        
        if ( foundIndex == wxNOT_FOUND )
            continue;  // ignore this 'error'.

        wxString paramName;
        wxString paramValue;

        paramName = token.substr( 0, foundIndex );
        paramValue = token.substr( foundIndex+1 );

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

hoxResult 
hoxNetworkAPI::ParseOneNetworkTable( const wxString&      tableStr,
                                     hoxNetworkTableInfo &tableInfo )
{
    const char* FNAME = __FUNCTION__;
    hoxResult  result = hoxRC_ERR;

    wxLogDebug("%s: ENTER.", FNAME);
	tableInfo.Clear();

	wxStringTokenizer tkz( tableStr, ";" );
    int i = 0;

    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        switch (i++)
        {
            case 0:  // Id
                tableInfo.id = token; 
                break;

            case 1:  // Group
				tableInfo.group = (  token == "0" 
					               ? hoxGAME_GROUP_PUBLIC 
								   : hoxGAME_GROUP_PRIVATE ); 
                break;

            case 2:  // Type
				tableInfo.gameType = (  token == "0" 
					               ? hoxGAME_TYPE_RATED 
								   : hoxGAME_TYPE_NONRATED ); 
                break;

            case 3:  // Initial-Time
				tableInfo.initialTime = hoxUtil::StringToTimeInfo( token );
                break;

            case 4:  // RED-Time
				tableInfo.redTime = hoxUtil::StringToTimeInfo( token );
                break;

            case 5:  // BLACK-Time
				tableInfo.blackTime = hoxUtil::StringToTimeInfo( token );
                break;

            case 6:  // RED-Id
                tableInfo.redId = token; 
                break;

            case 7:  // RED-Score
				tableInfo.redScore = token; 
                break;

            case 8:  // BLACK-Id
                tableInfo.blackId = token;
                break;

            case 9:  // BLACK-Score
				tableInfo.blackScore = token;
                break;

			default:
				// Ignore the rest
				break;
        }
    }

    return hoxRC_OK;
}

/************************* END OF FILE ***************************************/
