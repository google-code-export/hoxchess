/////////////////////////////////////////////////////////////////////////////
// Name:            hoxConnection.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         11/05/2007
//
// Description:     The Connection which is the base for all connections.
/////////////////////////////////////////////////////////////////////////////

#include "hoxConnection.h"
#include "hoxEnums.h"

IMPLEMENT_ABSTRACT_CLASS(hoxConnection, wxObject)

//-----------------------------------------------------------------------------
// hoxConnection
//-----------------------------------------------------------------------------

hoxConnection::hoxConnection()
{
    const char* FNAME = "hoxConnection::hoxConnection";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxConnection::~hoxConnection()
{
    const char* FNAME = "hoxConnection::~hoxConnection";
    wxLogDebug("%s: ENTER.", FNAME);
}


/************************* END OF FILE ***************************************/
