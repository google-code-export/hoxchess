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
    /**
     * Providing exlusive access to the socket input by disabling 
     * wxSOCKET_INPUT event and re-enabling it upon destruction.
     */
    class SocketInputLock
    {
    public:
        SocketInputLock( wxSocketBase* sock );
        ~SocketInputLock();
    private:
        wxSocketBase* m_sock;
    };


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

    hoxResult ParseNetworkTables( const wxString&          responseStr,
                                  hoxNetworkTableInfoList& tableList );

    hoxResult ParseNewNetworkTable( const wxString&  responseStr,
                                    wxString&        newTableId );

    hoxResult ParseJoinNetworkTable( const wxString&      responseStr,
                                     hoxNetworkTableInfo& tableInfo );

    hoxResult ParseNetworkEvents( const wxString&      tablesStr,
                                  hoxNetworkEventList& networkEvents );

    /* PRIVATE */
    hoxResult _ParseNetworkTableInfoString( const wxString&      tableInfoStr,
                                            hoxNetworkTableInfo& tableInfo );

    /* PRIVATE */
    hoxResult _ParseNetworkEventString( const wxString&  eventStr,
                                        hoxNetworkEvent& networkEvent );

    /**
     * Convert a given socket-event to a (human-readable) string.
     */
    const wxString SocketEventToString( const wxSocketNotify socketEvent );

    /**
     * Convert a given socket-error to a (human-readable) string.
     */
    const wxString SocketErrorToString( const wxSocketError socketError );


} /* namespace hoxNetwork */

#endif /* __INCLUDED_HOX_NETWORK_API_H_ */