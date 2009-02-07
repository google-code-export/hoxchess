/***************************************************************************
 *  Copyright 2007-2009 Huy Phan  <huyphan@playxiangqi.com>                *
 *                      Bharatendra Boddu (bharathendra at yahoo dot com)  *
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
// Name:            hoxXQWLightPlayer.h
// Created:         10/11/2008
//
// Description:     The AI Player based on the open-source (?) Xiangqi Engine
//                  called XQWLight written by Huang Chen at
//                  www.elephantbase.net
//
//  (Original Chinese URL)
//        http://www.elephantbase.net/computer/stepbystep1.htm
//
//  (Translated English URL using Goold Translate)
//       http://74.125.93.104/translate_c?hl=en&langpair= \
//       zh-CN|en&u=http://www.elephantbase.net/computer/stepbystep1.htm& \
//       usg=ALkJrhj7W0v3J1P-xmbufsWzYq7uKciL1w
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_XQWLIGHT_PLAYER_H__
#define __INCLUDED_HOX_XQWLIGHT_PLAYER_H__

#include "hoxAIPlayer.h"

/**
 * The XQWLight AI player.
 */
class hoxXQWLightPlayer :  public hoxAIPlayer
{
public:
    hoxXQWLightPlayer() {} // DUMMY default constructor required for event handler.
    hoxXQWLightPlayer( const wxString& name,
                       hoxPlayerType   type,
                       int             score );

    virtual ~hoxXQWLightPlayer() {}

    // **** Override the parent's API ****
    virtual void Start();

private:

    DECLARE_DYNAMIC_CLASS(hoxXQWLightPlayer)
};

// ----------------------------------------------------------------------------
// hoxXQWLightEngine
// ----------------------------------------------------------------------------

class hoxXQWLightEngine : public hoxAIEngine
{
public:
    hoxXQWLightEngine( wxEvtHandler*   player,
                       const wxString& sSavedFile );
    virtual ~hoxXQWLightEngine() {}

protected:
    virtual void     OnOpponentMove( const wxString& sMove );
    virtual wxString GenerateNextMove();

private:
	void _hoxPcsPos2XQWLight( unsigned char pcsPos[10][9] );
};

// ----------------------------------------------------------------------------
// hoxXQWLightConnection
// ----------------------------------------------------------------------------

/* Typedef(s) */
typedef boost::shared_ptr<hoxXQWLightEngine> hoxXQWLightEngine_SPtr;

/**
 * The connection to an XQWLight AI Engine.
 */
class hoxXQWLightConnection : public hoxAIConnection
{
public:
    hoxXQWLightConnection() {} // DUMMY default constructor required for RTTI info.
    hoxXQWLightConnection( wxEvtHandler*   player,
                           const wxString& sSavedFile = "" );
    virtual ~hoxXQWLightConnection() {}

protected:
    virtual void CreateAIEngine();

private:
    const wxString  m_sSavedFile; // Containing a previously saved table.

    DECLARE_DYNAMIC_CLASS(hoxXQWLightConnection)
};

#endif /* __INCLUDED_HOX_XQWLIGHT_PLAYER_H__ */
