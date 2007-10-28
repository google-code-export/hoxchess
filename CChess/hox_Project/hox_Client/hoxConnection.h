/////////////////////////////////////////////////////////////////////////////
// Name:            hoxConnection.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/28/2007
//
// Description:     The "base" Connection Thread to help a "network" player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_CONNECTION_H_
#define __INCLUDED_HOX_CONNECTION_H_

#include <wx/wx.h>
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// hoxConnection
// ----------------------------------------------------------------------------

/** !!!!!!!!!!!!!!!!!!!!!!!
 * NOTE: If deriving from wxThread, acquiring mutex (using lock) would fail.
 *       Thus, I have to derive from wxThreadHelper.
 * !!!!!!!!!!!!!!!!!!!!!!! */
class hoxConnection : public wxThreadHelper
{
public:
    hoxConnection( const wxString& sHostname,
                   int             nPort );
    virtual ~hoxConnection();

    // Thread execution starts here
    virtual void* Entry();

public:
    // **** My own public API ****

    virtual void AddRequest( hoxRequest* request ) = 0;

protected:
    wxString              m_sHostname; 
    int                   m_nPort;
};


#endif /* __INCLUDED_HOX_CONNECTION_H_ */
