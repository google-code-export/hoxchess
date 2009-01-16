#pragma once
#include <wx/string.h>
#include <wx/xml/xml.h>
#include "hoxTypes.h"

class hoxSavedTable
{
public:
	hoxSavedTable(const wxString &fileName);
	~hoxSavedTable(void);
	bool Load();
	bool Save();
	void SetTableId(const wxString&);
	const wxString GetTableId(void);
	void SetGameState(hoxPieceInfoList &pieceInfoList, hoxColor &nextColor);
	void GetGameState(hoxPieceInfoList &pieceInfoList, hoxColor &nextColor);
private:
	wxXmlDocument m_doc;
	wxString m_fileName;
};
