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
// Name:            hoxSavedTable.h
// Created:         01/16/2009
//
// Description:     A Table implementing the "Save Table" feature.
// Created by:      darickdle
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_SAVED_TABLE_H__
#define __INCLUDED_HOX_SAVED_TABLE_H__

#include <wx/string.h>
#include <wx/xml/xml.h>
#include "hoxTypes.h"

/**
 * NOTE: Added 2 new functions Save Table (worked for both local and
 *       remote and Open Table for local plays ie. practice with computer.
 *       Table is saved in xml format. Saved tables must have "NextColor"
 *       value equal to "Red" or we have to save table only after 
 *       computer completes its turn to have a valid saved table.
 */

class hoxSavedTable
{
public:
    hoxSavedTable( const wxString& fileName );
    virtual ~hoxSavedTable() {}

    bool Load();
    bool Save();

    void SetTableId( const wxString& tableId );
    const wxString GetTableId() const;

    void SetGameState( const hoxPieceInfoList& pieceInfoList,
                       hoxColor&               nextColor );

    void GetGameState( hoxPieceInfoList& pieceInfoList,
                       hoxColor&         nextColor );

private:
    const wxString  m_fileName;
    wxXmlDocument   m_doc;
};

#endif /* __INCLUDED_HOX_SAVED_TABLE_H__ */
