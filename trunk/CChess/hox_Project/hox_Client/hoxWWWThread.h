/////////////////////////////////////////////////////////////////////////////
// Name:            hoxWWWThread.h
// Program's Name:  Huy's Open Xiangqi
// Created:         10/21/2007
//
// Description:     The WWW Thread to help the WWW player.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_WWW_THREAD_H_
#define __INCLUDED_HOX_WWW_THREAD_H_

#include "wx/wx.h"
#include "hoxEnums.h"
#include "hoxTypes.h"

// ----------------------------------------------------------------------------
// WWW worker thread
// ----------------------------------------------------------------------------

/** !!!!!!!!!!!!!!!!!!!!!!!
 * NOTE: If deriving from wxThread, acquiring mutex (using lock) would fail.
 *       Thus, I have to derive from wxThreadHelper.
 * !!!!!!!!!!!!!!!!!!!!!!! */
class hoxWWWThread : public wxThreadHelper
{
public:
    hoxWWWThread( const wxString& sHostname,
                  int             nPort );
    ~hoxWWWThread();

    // Thread execution starts here
    virtual void* Entry();

public:
    // **** My own public API ****

    void AddRequest( hoxRequest* request );

private:
    hoxRequest* _GetRequest();         
    void        _HandleRequest( hoxRequest* request );
    hoxResult   _SendRequest( const wxString& request, wxString& response );

public:  /* Public static API */
    static hoxResult 
    parse_string_for_simple_response( const wxString& responseStr,
                                      int&            returnCode,
                                      wxString&       returnMsg );
    static hoxResult 
    parse_string_for_network_tables( const wxString& responseStr,
                                     hoxNetworkTableInfoList& tableList );

    static hoxResult 
    parse_string_for_new_network_table( const wxString&  responseStr,
                                        wxString&        newTableId );

    static hoxResult 
    parse_string_for_join_network_table( const wxString&      responseStr,
                                         hoxNetworkTableInfo& tableInfo );
    
    static hoxResult 
    parse_string_for_network_events( const wxString& tablesStr,
                                     hoxNetworkEventList& networkEvents );

private:  /* Private static API */
    static hoxResult
    _parse_network_table_info_string( const wxString&      tableInfoStr,
                                      hoxNetworkTableInfo& tableInfo );
    static hoxResult 
    _parse_network_event_string( const wxString& eventStr,
                                 hoxNetworkEvent& networkEvent );

private:
    wxString              m_sHostname; 
    int                   m_nPort;

    bool                  m_shutdownRequested;
                /* Has a shutdown-request been received? */

    wxSemaphore           m_semRequests;
    wxMutex               m_mutexRequests;
    hoxRequestList        m_requests;
};


#endif /* __INCLUDED_HOX_WWW_THREAD_H_ */
