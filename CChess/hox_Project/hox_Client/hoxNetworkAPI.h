/////////////////////////////////////////////////////////////////////////////
// Name:            hoxNetworkAPI.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/26/2007
//
// Description:     Containing network related APIs specific to this project.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_NETWORK_API_H_
#define __INCLUDED_HOX_NETWORK_API_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include "hoxEnums.h"
#include "hoxTypes.h"

namespace hoxNetworkAPI
{
    hoxResult SendMove( wxSocketBase*   sock, 
                        const wxString& commandInput );

    hoxResult HandleMove( wxSocketBase* sock,
                          hoxCommand&   command );

    hoxResult HandleLeave( wxSocketBase*  sock,
                           hoxCommand&    command );

    hoxResult ParseCommand( const wxString& commandStr, 
                            hoxCommand&     command );

    hoxResult ParseSimpleResponse( const wxString& responseStr,
                                   int&            returnCode,
                                   wxString&       returnMsg );


} /* namespace hoxNetwork */

#endif /* __INCLUDED_HOX_NETWORK_API_H_ */