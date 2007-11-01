/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPlayerMgr.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/06/2007
//
// Description:     The manager that manages ALL the players in this server.
/////////////////////////////////////////////////////////////////////////////

#include "hoxPlayerMgr.h"


// Declare the single instance.
hoxPlayerMgr* hoxPlayerMgr::m_instance = NULL;

hoxPlayerMgr*
hoxPlayerMgr::GetInstance()
{
    if ( m_instance == NULL )
    {
        m_instance = new hoxPlayerMgr();
    }
        
    return m_instance;
}

hoxPlayerMgr::hoxPlayerMgr()
{
}

hoxPlayerMgr::~hoxPlayerMgr()
{
    for ( hoxPlayerList::iterator it = m_players.begin();
                                  it != m_players.end(); ++it )
    {
        delete (*it);
    }
}

hoxHostPlayer*
hoxPlayerMgr::CreateHostPlayer( const wxString& name,
                                int             score /* = 1500 */)
{
    hoxHostPlayer* player 
        = new hoxHostPlayer( name, hoxPLAYER_TYPE_HOST, score );
    m_players.push_back( player );

    return player;
}

hoxHttpPlayer*
hoxPlayerMgr::CreateHTTPPlayer( const wxString& name,
                                int             score /* = 1500 */)
{
    hoxHttpPlayer* player 
        = new hoxHttpPlayer( name, hoxPLAYER_TYPE_LOCAL, score );
    m_players.push_back( player );

    return player;
}

hoxMyPlayer*
hoxPlayerMgr::CreateMyPlayer( const wxString& name,
                              int             score /* = 1500 */)
{
    hoxMyPlayer* player 
        = new hoxMyPlayer( name, hoxPLAYER_TYPE_LOCAL, score );
    m_players.push_back( player );

    return player;
}

hoxNetworkPlayer*
hoxPlayerMgr::CreateNetworkPlayer( const wxString& name,
                                   int             score /* = 1500 */)
{
    hoxNetworkPlayer* player 
        = new hoxNetworkPlayer( name, hoxPLAYER_TYPE_NETWORK, score );
    m_players.push_back( player );

    return player;
}

hoxPlayer*
hoxPlayerMgr::CreatePlayer( const wxString& name,
                            hoxPlayerType   type,
                            int             score /* = 1500 */)
{
    const char* FNAME = "hoxPlayerMgr::CreatePlayer";
    wxLogDebug("%s: Creating player [%s]...", FNAME, name);

    hoxPlayer* player = new hoxPlayer( name, type, score );
    m_players.push_back( player );

    return player;
}

void 
hoxPlayerMgr::DeletePlayer( hoxPlayer* player )
{
    const char* FNAME = "hoxPlayerMgr::DeletePlayer";

    wxCHECK_RET( player != NULL, "The player should not be NULL." );
    
    wxLogDebug("%s: Deleting player [%s]...", FNAME, player->GetName());

    delete player;
    m_players.remove( player );
}

hoxPlayer* 
hoxPlayerMgr::FindPlayer( const wxString& playerId )
{
    for ( hoxPlayerList::iterator it = m_players.begin();
                                  it != m_players.end(); ++it )
    {
        if ( playerId == (*it)->GetName() )
        {
            return (*it);
        }
    }
    
    return NULL;
}

/************************* END OF FILE ***************************************/
