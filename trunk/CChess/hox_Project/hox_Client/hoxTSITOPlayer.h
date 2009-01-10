/***************************************************************************
 *  Copyright 2007, 2008, 2009 Huy Phan  <huyphan@playxiangqi.com>         *
 *                                                                         * 
 *  This file is part of HOXChess.                                         *
 *                                                                         *
 *  HOXChess is free software: you can redistribute it and/or modify       *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  HOXChess is distributed in the hope that it will be useful,            *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with HOXChess.  If not, see <http://www.gnu.org/licenses/>.      *
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// Name:            hoxTSITOPlayer.h
// Created:         09/07/2008
//
// Description:     The AI Player based on the open-source Xiangqi Engine
//                  called TSITO 
//                     http://xiangqi-engine.sourceforge.net/tsito.html
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_TSITO_PLAYER_H__
#define __INCLUDED_HOX_TSITO_PLAYER_H__

#include "hoxAIPlayer.h"

/**
 * The TSITO AI player.
 */
class hoxTSITOPlayer :  public hoxAIPlayer
{
public:
    hoxTSITOPlayer() {} // DUMMY default constructor required for event handler.
    hoxTSITOPlayer( const wxString& name,
                    hoxPlayerType   type,
                    int             score );

    virtual ~hoxTSITOPlayer() {}

    // **** Override the parent's API ****
    virtual void Start();

private:

    DECLARE_DYNAMIC_CLASS(hoxTSITOPlayer)
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// hoxTSITOEngine
// ----------------------------------------------------------------------------

class hoxTSITOEngine : public hoxAIEngine
{
public:
    hoxTSITOEngine( wxEvtHandler* player );
    virtual ~hoxTSITOEngine();

protected:
    virtual void     OnOpponentMove( const wxString& sMove );
    virtual wxString GenerateNextMove();

private:
    class TSITO_Engine;
    TSITO_Engine*  m_tsito_engine;
        /* NOTE: I cannot use std::auto_ptr<...> here because
         *       it generates a compiler error
         */
};

// ----------------------------------------------------------------------------
// hoxTSITOConnection
// ----------------------------------------------------------------------------

/* Typedef(s) */
typedef boost::shared_ptr<hoxTSITOEngine> hoxTSITOEngine_SPtr;

/**
 * The connection to an TSITO AI Engine.
 */
class hoxTSITOConnection : public hoxAIConnection
{
public:
    hoxTSITOConnection() {} // DUMMY default constructor required for RTTI info.
    hoxTSITOConnection( wxEvtHandler* player );
    virtual ~hoxTSITOConnection() {}

protected:
    virtual void CreateAIEngine();

private:

    DECLARE_DYNAMIC_CLASS(hoxTSITOConnection)
};

#endif /* __INCLUDED_HOX_TSITO_PLAYER_H__ */
