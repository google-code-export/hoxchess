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
#include "hoxUtil.h"
#include <wx/tokenzr.h>

hoxSavedTable::hoxSavedTable( const wxString& fileName )
        : m_fileName( fileName )
{
    wxASSERT_MSG(!m_fileName.empty(), "Filename must be set");
}

bool
hoxSavedTable::SaveGameState( const wxString&         tableId,
                              const hoxMoveList&      moveList, 
                              const hoxPieceInfoList& pieceInfoList,
                              const hoxColor          nextColor )
{
    /* Save the table-ID. */
  	wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, "TABLE" );
	root->AddAttribute("id", tableId);
	m_doc.SetRoot(root);

    /* Save the 'past' Moves. */
    wxString sMoves;
    for ( hoxMoveList::const_iterator it = moveList.begin();
                                      it != moveList.end(); ++it )
    {
        if ( !sMoves.empty() ) sMoves += '/';
        sMoves += it->ToString();
    }
    wxXmlNode* node = new wxXmlNode( wxXML_ELEMENT_NODE, "Moves" );
    node->AddAttribute("value", sMoves);
	root->AddChild( node );

    /* Save the 'next' color. */
    node = new wxXmlNode( wxXML_ELEMENT_NODE, "NextColor" );
    node->AddAttribute("value", hoxUtil::ColorToString(nextColor));
	root->AddChild( node );

    /* Save the current pieces. */
	wxXmlNode* piecesNode = new wxXmlNode( wxXML_ELEMENT_NODE, "Pieces" );
	root->AddChild(piecesNode);

	for ( hoxPieceInfoList::const_iterator it = pieceInfoList.begin();
                                           it != pieceInfoList.end(); ++it )
	{
		wxXmlNode* pieceNode = new wxXmlNode( wxXML_ELEMENT_NODE,
                                              hoxUtil::TypeToString(it->type) );

		pieceNode->AddAttribute("Color", hoxUtil::ColorToString(it->color));
		pieceNode->AddAttribute("Row", wxString::Format("%d", it->position.x));
		pieceNode->AddAttribute("Col", wxString::Format("%d", it->position.y));			
		piecesNode->AddChild(pieceNode);
	}

    return m_doc.Save( m_fileName ); // Save the entire file to disk.
}

bool
hoxSavedTable::LoadGameState( hoxStringList&    pastMoves,
                              hoxPieceInfoList& pieceInfoList,
                              hoxColor&         nextColor )
{
    if ( !m_doc.Load( m_fileName ) )
    {
        wxLogWarning("%s: Fail to load the content of file [%s].",
            __FUNCTION__, m_fileName.c_str());
        return false;
    }

    const wxXmlNode* root = m_doc.GetRoot();
    wxASSERT( root != NULL );
    if ( root->GetName() != "TABLE" )
	{
        wxLogWarning("%s: Need to save game id first as a root.", __FUNCTION__);
        return false;
	}
	
    pieceInfoList.clear(); // Clear the old info, if exists.
    pastMoves.clear();

    bool bGotMoves     = false;
    bool bGotNextColor = false;
    bool bGotPieces    = false;

    for ( const wxXmlNode* child = root->GetChildren();
                           child != NULL;
                           child = child->GetNext() )
	{
        if ( child->GetName() == "Moves" )
        {
            bGotMoves = _LoadMoves( child, pastMoves );
        }
        else if ( child->GetName() == "NextColor" )
        {
            nextColor = hoxUtil::StringToColor(child->GetAttribute("value"));
            bGotNextColor = true;
        }
        else if ( child->GetName() == "Pieces" )
        {
            bGotPieces = _LoadPieces( child, pieceInfoList );
        }
    }

    return ( bGotMoves && bGotNextColor && bGotPieces );
}

bool
hoxSavedTable::_LoadPieces( const wxXmlNode*  parentNode,
                            hoxPieceInfoList& pieceInfoList)
{
    wxCHECK_MSG(parentNode, false, "The parent node cannot be NULL");

    long  lVal;  // Just a numeric value holder.

    for ( const wxXmlNode* node = parentNode->GetChildren();
                           node != NULL;
                           node = node->GetNext() )
	{
        hoxPieceInfo pieceInfo;

		pieceInfo.type = hoxUtil::StringToType( node->GetName() );
		pieceInfo.color = hoxUtil::StringToColor( node->GetAttribute("Color") );

        node->GetAttribute("Row").ToLong( &lVal );
        pieceInfo.position.x = (char) lVal;
        node->GetAttribute("Col").ToLong( &lVal );
        pieceInfo.position.y = (char) lVal;
		
		pieceInfoList.push_back( pieceInfo );
	}

    return true; // success.
}

bool
hoxSavedTable::_LoadMoves( const wxXmlNode* parentNode,
                           hoxStringList&   pastMoves )
{
    const wxString sMoves = parentNode->GetAttribute("value");

	wxStringTokenizer tkz( sMoves, "/", wxTOKEN_STRTOK );
	while ( tkz.HasMoreTokens() )
	{
        pastMoves.push_back( tkz.GetNextToken() );
	}		

    return true; // success.
}

/************************* END OF FILE ***************************************/
