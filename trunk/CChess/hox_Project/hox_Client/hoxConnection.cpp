/////////////////////////////////////////////////////////////////////////////
// Name:            hoxConnection.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/28/2007
//
// Description:     The "base" Connection Thread to help a "network" player.
/////////////////////////////////////////////////////////////////////////////

#include "hoxConnection.h"
#include "hoxEnums.h"

//-----------------------------------------------------------------------------
// hoxConnection
//-----------------------------------------------------------------------------


hoxConnection::hoxConnection( const wxString&  sHostname,
                              int              nPort )
        : wxThreadHelper()
        , m_sHostname( sHostname )
        , m_nPort( nPort )
{
    const char* FNAME = "hoxConnection::hoxConnection";
    wxLogDebug("%s: ENTER.", FNAME);
}

hoxConnection::~hoxConnection()
{
    const char* FNAME = "hoxConnection::~hoxConnection";
    wxLogDebug("%s: ENTER.", FNAME);
}

void*
hoxConnection::Entry()
{
    const char* FNAME = "hoxConnection::Entry";
    hoxRequest* request = NULL;

    wxLogDebug("%s: ENTER. *** Do nothing ***", FNAME);
    return NULL;
}


/************************* END OF FILE ***************************************/
