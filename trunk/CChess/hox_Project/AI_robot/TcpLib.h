/***************************************************************************
 *  Copyright 2007, 2008 Huy Phan  <huyphan@playxiangqi.com>               *
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
// Name:            TcpLib.h
// Created:         10/04/2008
//
// Description:     The TCP API module.
/////////////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_HOX_TCP_LIB_H__
#define __INCLUDED_HOX_TCP_LIB_H__

#include <string>

#define HOX_INVALID_SOCKET  (-1)    /* Invalid socket descriptor */

// ---------------------------------------------------------
// Constants
// ---------------------------------------------------------

#define HOX_CHAR_CR       0x0d    /* CR */
#define HOX_CHAR_LF       0x0a    /* LF */
#define HOX_CMD_LINE_MAX  65536   /* Maxium length of a command line */

/////////////////////////////////////////////////

namespace HOX
{
    int
    tcp_initialize();

    int
    tcp_deinitialize();

    /**
     * Establish a connection a given server = {host:port}.
     *
     * @param sHost - [IN] The remote host.
     * @param nPort - [IN] The remote port.
     * @param s     - [OUT] The output socket.
     * @return 0   - if success
     */
    int
    tcp_connect( const std::string&       sHost,
                 const unsigned short int nPort,
                 int&                     s );

    /**
     * Close (disconnect) a connection.
     *
     * @param s  - [IN] The socket being closed.
     * @return 0   - if success
     */
    int
    tcp_close( int s );

    /**
     * @return 0   - if the connection is closed.
     *         A positive number - if successfully read.
     *         -1  - If max count read (over the specific max/limit).
     *         -2  - If other error
     */
    int
    tcp_read_line( int          s /* socket */,
                   std::string& sLine,
                   unsigned int nMax = HOX_CMD_LINE_MAX );

    int
    tcp_read_nbytes( int          s /* socket */,
                     unsigned int nBytes,
                     std::string& sOutput );

    int
    tcp_read_until_all( int                s /* socket */,
                        const std::string& sWanted,
                        std::string&       sOutput );

    int
    tcp_send_data( int                s /* socket */,
                   const std::string& sData );

    int
    tcp_send_line( int                s /* socket */,
                   const std::string& sData );

    /**
     *   Socket (based on TCP socket).
     */
    class Socket
    {
    public:
        Socket() : m_sock( HOX_INVALID_SOCKET ) {}
        virtual ~Socket();

        /**
         * Establish a connection a given server = {host:port}.
         *
         * @param sHost - [IN] The remote host.
         * @param nPort - [IN] The remote port.
         * @return 0   - if success
         */
        virtual int Connect( const std::string&       sHost,
                             const unsigned short int nPort );

        /**
         * Close (disconnect) a connection.
         *
         * @return 0   - if success
         */
        virtual int Close();

        virtual int ReadLine( std::string& sLine,
                              unsigned int nMax = HOX_CMD_LINE_MAX );
        virtual int ReadNBytes( unsigned int  nBytes,
                                std::string&  sOutput );
        virtual int ReadUntilAll( const std::string& sWanted,
                                  std::string&       sOutput );
        virtual int SendData( const std::string& sData );
        virtual int SendLine( const std::string& sData );

    protected:
        int    m_sock;   // The socket descriptor.
    };

} // namespace HOX

#endif /* __INCLUDED_HOX_TCP_LIB_H__ */
