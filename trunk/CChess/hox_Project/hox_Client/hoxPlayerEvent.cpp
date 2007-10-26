/////////////////////////////////////////////////////////////////////////////
// Name:            hoxPlayerEvent.cpp
// Program's Name:  Huy's Open Xiangqi
// Created:         10/10/2007
//
// Description:     The Player-Event.
/////////////////////////////////////////////////////////////////////////////

#include "hoxPlayerEvent.h"
#include "hoxEnums.h"

//-----------------------------------------------------------------------------
// hoxPlayerEvent
//-----------------------------------------------------------------------------

hoxPlayerEvent::hoxPlayerEvent( wxEventType commandType, 
                                int         id )
        : wxNotifyEvent( commandType, id )
{
    m_pPiece = NULL;
}

wxEvent*
hoxPlayerEvent::Clone() const 
{ 
    hoxPlayerEvent* newEvent = new hoxPlayerEvent(*this);
    newEvent->m_pPiece = m_pPiece;
    newEvent->m_position = m_position;
    return newEvent; 
}


/************************* END OF FILE ***************************************/
