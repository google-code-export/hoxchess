/////////////////////////////////////////////////////////////////////////////
// Name:            hoxConnection.h
// Program's Name:  Huy's Open Xiangqi
// Created:         11/05/2007
//
// Description:     The Connection which is the base for all connections.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_CONNECTION_H_
#define __INCLUDED_HOX_CONNECTION_H_

#include <wx/wx.h>
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// hoxConnection
// ----------------------------------------------------------------------------

class hoxConnection : public wxObject
{
public:
    hoxConnection();
    virtual ~hoxConnection();

    virtual void AddRequest( hoxRequest* request ) = 0;
};


#endif /* __INCLUDED_HOX_CONNECTION_H_ */
