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
// Name:            hoxSavedTable.cpp
// Created:         01/16/2009
//
// Description:     A Table implementing the "Save Table" feature.
// Created by:      darickdle
/////////////////////////////////////////////////////////////////////////////

#include "hoxSavedTable.h"
#include "hoxPiece.h"
#include "hoxUtil.h"

hoxSavedTable::hoxSavedTable( const wxString& fileName )
        : m_fileName( fileName )
{
    wxASSERT_MSG(!m_fileName.empty(), "Filename must be set");
}

bool
hoxSavedTable::Load()
{
    return m_doc.Load( m_fileName );
}

bool
hoxSavedTable::Save()
{
    return m_doc.Save( m_fileName );
}

void
hoxSavedTable::SetTableId( const wxString& tableId )
{
	wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, "TABLE" );
	root->AddAttribute("id", tableId);
	m_doc.SetRoot(root);
}

const wxString
hoxSavedTable::GetTableId() const
{
	wxXmlNode* root = m_doc.GetRoot();
	if ( root && root->GetName() == "TABLE" )
    {
		return root->GetAttribute("id");
    }

    return ""; // Wrong file.
}

void
hoxSavedTable::SetGameState( const hoxPieceInfoList& pieceInfoList,
                             hoxColor&               nextColor )
{
	if ( !m_doc.IsOk() ) return; //error

    wxXmlNode* root = m_doc.GetRoot();
    if ( !root || root->GetName() != "TABLE" )
	{
        wxLogError("%s: Need to save game id first as a root.", __FUNCTION__);
        return;
	}

	root->AddAttribute("NextColor", hoxUtil::ColorToString(nextColor));
	for ( hoxPieceInfoList::const_iterator it = pieceInfoList.begin();
                                           it != pieceInfoList.end(); 
	                                     ++it )
	{
		hoxPiece* piece = new hoxPiece( (*it) );

		wxXmlNode* pieceNode = new wxXmlNode( wxXML_ELEMENT_NODE,
                                              hoxUtil::TypeToString(piece->GetType()) );

		pieceNode->AddAttribute("Color",hoxUtil::ColorToString(piece->GetColor()));
		pieceNode->AddAttribute("Row",wxString::Format("%d",piece->GetPosition().x));
		pieceNode->AddAttribute("Col",wxString::Format("%d",piece->GetPosition().y));			
		root->AddChild(pieceNode);
	}
}

void
hoxSavedTable::GetGameState( hoxPieceInfoList& pieceInfoList,
                             hoxColor&         nextColor)
{
	if ( !m_doc.IsOk() ) return; // Error: no file is loaded.

    wxXmlNode* root = m_doc.GetRoot();
    if ( !root || root->GetName() != "TABLE" )
	{
        wxLogError("%s: Need to save game id first as a root.", __FUNCTION__);
        return;
	}
	
    nextColor = hoxUtil::StringToColor(root->GetAttribute("NextColor"));
	wxXmlNode* pieceNodes = root->GetChildren();
	wxString sType, sColor, xRow, yCol;
	hoxPieceType type;
	hoxColor color;
	hoxPosition pos;
	pieceInfoList.clear();    // Clear the old info, if exists.
	while (pieceNodes)
	{
		sType = pieceNodes->GetName();
		type = hoxPIECE_INVALID;
		type = hoxUtil::StringToType(sType);

		sColor = pieceNodes->GetAttribute("Color");
		color = hoxUtil::StringToColor(sColor);

		xRow = pieceNodes->GetAttribute("Row");
		yCol = pieceNodes->GetAttribute("Col");
		long x,y;
		xRow.ToLong(&x);
		yCol.ToLong(&y);
		pos.x = x; pos.y = y;
		
		pieceInfoList.push_back( hoxPieceInfo( type, color, pos ) );

		/* Next Child */
		pieceNodes = pieceNodes->GetNext();
	}
}

/************************* END OF FILE ***************************************/
