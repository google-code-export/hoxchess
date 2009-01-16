#include "hoxSavedTable.h"
#include "hoxPiece.h"
#include "hoxUtil.h"

hoxSavedTable::hoxSavedTable(const wxString &fileName):m_fileName(fileName)
{
}

hoxSavedTable::~hoxSavedTable(void)
{
}

bool hoxSavedTable::Load()
{
//	m_fileName = wxFileSelector("Choose a file to open", "../SAVEDTABLES", "table.xml", 0, "Extension Markup Language (*.xml)|*.xml");
	if (!m_fileName.IsEmpty())
		return m_doc.Load(m_fileName);
	return false;
}

bool hoxSavedTable::Save()
{
//	wxFileDialog * openFileDialog = new wxFileDialog(NULL, "Choose a File","", "table.xml", "Extensible Markup Language(*.xml)|*.xml", wxFD_SAVE );
//	openFileDialog->SetDirectory("../test2");
//	m_fileName = wxFileSelector("Enter a file name to save", "../SAVEDTABLES", "table.xml", 0, "Extension Markup Language (*.xml)|*.xml");
	if (!m_fileName.IsEmpty()){
	//	m_fileName = openFileDialog->GetPath();
		return m_doc.Save(m_fileName);
	}
	return false;
}

void hoxSavedTable::SetTableId(const wxString &tableId)
{
	wxXmlNode *root = new wxXmlNode(wxXML_ELEMENT_NODE,"TABLE");
	root->AddProperty("id", tableId);
	m_doc.SetRoot(root);
	//m_Table = root;
}
const wxString hoxSavedTable::GetTableId(void)
{
	wxXmlNode* root=m_doc.GetRoot();
	if (root && root->GetName() == "TABLE")
		return root->GetPropVal("id","");
	else
		//wrong file
		return "";
}

void hoxSavedTable::SetGameState(hoxPieceInfoList &pieceInfoList, hoxColor &nextColor)
{
	if (!m_doc.IsOk()) return; //error
	wxXmlNode* root = m_doc.GetRoot();
	if(root->GetName() == "TABLE")
	{
		root->AddProperty("NextColor", hoxUtil::ColorToString(nextColor));
		for ( hoxPieceInfoList::const_iterator it = pieceInfoList.begin();
                                           it != pieceInfoList.end(); 
		                                   ++it )
		{
			hoxPiece* piece = new hoxPiece( (*it) );

			wxXmlNode* pieceNode = new wxXmlNode(wxXML_ELEMENT_NODE, hoxUtil::TypeToString(piece->GetType()));

			pieceNode->AddProperty("Color",hoxUtil::ColorToString(piece->GetColor()));
			pieceNode->AddProperty("Row",wxString::Format("%d",piece->GetPosition().x));
			pieceNode->AddProperty("Col",wxString::Format("%d",piece->GetPosition().y));			
			root->AddChild(pieceNode);
		}
	}
	else
	{
		//wxMessageDialog *dial = new wxMessageDialog(NULL, wxT("Error creating file"), wxT("Error"), wxOK | wxICON_ERROR);
		//dial->ShowModal();

		//need to save game id first as a root
	}
}

void hoxSavedTable::GetGameState(hoxPieceInfoList &pieceInfoList, hoxColor &nextColor)
{
	if (!m_doc.IsOk()) return; //error: no file is loaded.
	wxXmlNode* root = m_doc.GetRoot();
	if(root->GetName() != "TABLE")
		return; //error: wrong file is loaded
	nextColor = hoxUtil::StringToColor(root->GetPropVal("NextColor",""));
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

		sColor = pieceNodes->GetPropVal("Color","");
		color = hoxUtil::StringToColor(sColor);

		xRow = pieceNodes->GetPropVal("Row","");
		yCol = pieceNodes->GetPropVal("Col","");
		long x,y;
		xRow.ToLong(&x);
		yCol.ToLong(&y);
//		pos = 
		pos.x = x; pos.y = y;
		
		pieceInfoList.push_back( hoxPieceInfo( type, color, pos ) );

		/* Next Child */
		pieceNodes = pieceNodes->GetNext();
	}
}
