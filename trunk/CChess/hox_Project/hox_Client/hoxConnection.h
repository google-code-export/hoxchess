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

    virtual void Start() = 0;
    virtual void Shutdown() = 0;
    virtual void AddRequest( hoxRequest* request ) = 0;

    virtual void       SetPlayer(hoxPlayer* player) {}
    virtual hoxPlayer* GetPlayer()           { return NULL; }

    DECLARE_ABSTRACT_CLASS(hoxConnection)
};


#endif /* __INCLUDED_HOX_CONNECTION_H_ */
