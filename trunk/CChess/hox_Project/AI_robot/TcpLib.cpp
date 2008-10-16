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
// Name:            TcpLib.cpp
// Created:         10/04/2008
//
// Description:     The TCP API module.
/////////////////////////////////////////////////////////////////////////////

/* CREDITS:
 *
 * Source code obtained from:
 * http://sage.mc.yu.edu/kbeen/teaching/networking/resources/sockets.html
 */

#ifdef WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <netdb.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <errno.h>
#endif

#include <iostream>
#include "TcpLib.h"

#if WIN32
int
initilize_windows_socket_library()
{
    /**
     * CODE obtained from:
     *   http://msdn.microsoft.com/en-us/library/ms742213(VS.85).aspx
     *
     * The following code fragment demonstrates how an application that supports
     * only version 2.2 of Windows Sockets makes a WSAStartup call.
     */

    WORD    wVersionRequested;
    WSADATA wsaData;
    int     err;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }

    /* Confirm that the WinSock DLL supports 2.2.        */
    /* Note that if the DLL supports versions greater    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested.                                        */

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        printf("Could not find a useable version of Winsock.dll\n");
        WSACleanup();
        return 1;
    }
    else
        printf("The Winsock 2.2 dll was found okay\n");
        
    return 0; // success
}

int
deinitilize_windows_socket_library()
{
    WSACleanup();  /* ... done using the Winsock dll */
    return 0;
}
#endif /* WIN32 */

int
get_last_socket_error()
{
#ifdef WIN32
    return WSAGetLastError();
#endif
    return errno;
}

/**
 * Initilize the TCP Library.
 *
 * @return 0 if success.
 */
int
HOX::tcp_initialize()
{
    int iResult = 0; // Default: success

#ifdef WIN32
    iResult = initilize_windows_socket_library();
#endif

    return iResult;
}

/**
 * De-initilize the TCP Library.
 *
 * @return 0 if success.
 */
int
HOX::tcp_deinitialize()
{
    int iResult = 0; // Default: success

#ifdef WIN32
    iResult = deinitilize_windows_socket_library();
#endif

    return iResult;
}

int
HOX::tcp_connect( const std::string&       sHost,
                  const unsigned short int nPort,
                  int&                     s )
{
    int                 iResult = 0;
    struct sockaddr_in  serverAddress;
    struct hostent*     hostInfo;

    s = -1;   // Default: invalid socket

    // gethostbyname() takes a host name or ip address in "numbers and
    // dots" notation, and returns a pointer to a hostent structure,
    // which we'll need later.  It's not important for us what this
    // structure is actually composed of.
    hostInfo = gethostbyname( sHost.c_str() );
    if (hostInfo == NULL)
    {
        std::cout << "problem interpreting host: " << sHost << "\n";
        return 1;
    }

    // Create a socket.  "AF_INET" means it will use the IPv4 protocol.
    // "SOCK_STREAM" means it will be a reliable connection (i.e., TCP;
    // for UDP use SOCK_DGRAM), and I'm not sure what the 0 for the last
    // parameter means, but it seems to work.
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        std::cout << "cannot create socket\n";
        return 2;
    }

    // Connect to server.  First we have to set some fields in the
    // serverAddress structure.  The system will assign me an arbitrary
    // local port that is not in use.
    serverAddress.sin_family = hostInfo->h_addrtype;
    memcpy((char *) &serverAddress.sin_addr.s_addr,
           hostInfo->h_addr_list[0], hostInfo->h_length);
    serverAddress.sin_port = htons( nPort);

    iResult = connect( s,
                       (struct sockaddr *) &serverAddress,
                       sizeof(serverAddress) );
    if ( iResult < 0 )
    {
        std::cout << "cannot connect\n";
        return 3;
    }

    return 0;  // success
}

int
HOX::tcp_close( int s )
{
#ifdef WIN32
  closesocket(s);
#else
  close(s);
#endif
  return 0;
}

int
HOX::tcp_read_line( int          s /* socket */,
                    std::string& sLine,
                    unsigned int nMax /* = HOX_CMD_LINE_MAX*/ )
{
    const char* FNAME = __FUNCTION__;

    sLine.clear();

	/* Read a line until seeing CRLF */

    char c;
    char last = 0;  // anything as long as it is not CR nor LF
    int  iResult;
    int  nTotal = 0; // Total bytes received so far.

    /* Windows socket sample code obtained from:
     * http://msdn.microsoft.com/en-us/library/ms740121(VS.85).aspx
     */

    /* Receive a line until seeing CRLF 
     * or until the peer closes the connection
     */
    for (;;)
    {
        iResult = recv( s, &c, sizeof(c), 0 /* flags */ );
        if ( iResult > 0 )
        {
            //printf("Bytes received: %d\n", iResult);
            sLine += c;
			if ( last == HOX_CHAR_CR && c == HOX_CHAR_LF )  // got CRLF?
			{
                nTotal = sLine.size();
                sLine = sLine.substr(0, nTotal-2); // chop off CRLF
				break;   // *** Done (success)
			}
            // Impose some limit.
            else if ( sLine.size() >= nMax )
            {
                printf("%s: *** WARN *** Maximum message's size [%d] reached."
                       "Likely to be an error.", FNAME, nMax);
                return -1;  // Return "over limit/max error" code
            }
            last = c;
        }
        else if ( iResult == 0 )
        {
            //printf("%s: Connection closed\n", FNAME);
            return 0;  //  Return "connection closed" code
        }
        else /* if ( iResult == SOCKET_ERROR ) */
        {
            printf("%s: recv failed: %d\n", FNAME, get_last_socket_error());
            return -2;  //  Return "other error" code
        }
    }

    return nTotal; // Return the number of bytes received.
}

int
HOX::tcp_read_nbytes( int          s /* socket */,
                      unsigned int nBytes,
                      std::string& sOutput )
{
    const char* FNAME = __FUNCTION__;

    sOutput.clear();

    char c;
    int iResult;
    int nTotal = 0; // Total bytes received so far.

    /* Receive a line until seeing the required number of bytes
     * or until the peer closes the connection
     */
    while ( nTotal < (int) nBytes )
    {
        iResult = recv( s, &c, sizeof(c), 0 /* flags */ );
        if ( iResult > 0 )
        {
            //printf("Bytes received: %d\n", iResult);
            sOutput += c;
            nTotal += sizeof(c);
        }
        else if ( iResult == 0 )
        {
            //printf("%s: Connection closed\n", FNAME);
            return 0;  //  Return "connection closed" code
        }
        else /* if ( iResult == SOCKET_ERROR ) */
        {
            printf("%s: recv failed: %d\n", FNAME, get_last_socket_error());
            return -2;  //  Return "other error" code
        }
    }

    return nTotal; // Return the number of bytes received.
}

int
HOX::tcp_read_until_all( int                s /* socket */,
                         const std::string& sWanted,
                         std::string&       sOutput )
{
    const unsigned int nMax = 10 * 1024;  // 10-K limit

    sOutput.clear();

	/* Read a line until seeing CRLF */

    char         c;         // The character just received.
    const size_t requiredSeen = sWanted.size();
    size_t       currentSeen  = 0;
    int          iResult      = 0;
    int          nTotal       = 0; // Total bytes received so far.

    /* Receive data until seeing all characters in the "wanted" string
     * or until the peer closes the connection
     */
    while ( currentSeen < requiredSeen )
    {
        iResult = recv( s, &c, sizeof(c), 0 /* flags */ );
        if ( iResult > 0 )
        {
            sOutput += c;
			if ( c == sWanted[currentSeen] ) // seen the next char?
			{
                ++currentSeen;
                continue;
			}

            currentSeen = 0;  // Reset "what we have seen".

            // Impose some limit.
            if ( sOutput.size() >= nMax )
            {
                printf("%s: *** WARN *** Maximum message's size [%d] reached."
                       "Likely to be an error.", __FUNCTION__, nMax);
                return -1;  // Return "over limit/max error" code
            }
        }
        else if ( iResult == 0 ) // Connection closed?
        {
            return 0;  //  Return "connection closed" code
        }
        else // Some other socket error?
        {
            printf("%s: recv failed: %d\n", __FUNCTION__, get_last_socket_error());
            return -2;  //  Return "other error" code
        }
    }

    /* Chop off 'want' string. */
	if ( currentSeen == requiredSeen && requiredSeen > 0 )
	{
        nTotal = sOutput.size();
        sOutput = sOutput.substr(0, nTotal - requiredSeen);
	}

    return nTotal; // Return the number of bytes received.
}

int
HOX::tcp_send_data( int                s /* socket */,
                    const std::string& sData )
{
    int iResult;

    iResult = send( s, sData.c_str(), sData.size(), 0);
    if ( iResult < 0) /* ( iResult == SOCKET_ERROR ) */
    {
        printf("%s: cannot send data. Error = [%d]\n", __FUNCTION__, get_last_socket_error());
        return 1; // error
    }

    return 0;
}

int
HOX::tcp_send_line( int                s /* socket */,
                    const std::string& sData )
{
    std::string sLine;
    int iResult;

    sLine = sData;
    sLine += HOX_CHAR_CR;
    sLine += HOX_CHAR_LF;

    iResult = send( s, sLine.c_str(), sLine.size(), 0);
    if ( iResult < 0) /* ( iResult == SOCKET_ERROR ) */
    {
        printf("%s: cannot send data. Error = [%d]\n", __FUNCTION__, get_last_socket_error());
        return 1; // error
    }

    return 0;
}

namespace HOX
{

Socket::~Socket()
{
    this->Close();
}

int
Socket::Connect( const std::string&       sHost,
                 const unsigned short int nPort )
{
    return tcp_connect( sHost, nPort, m_sock );
}

int
Socket::Close()
{
    int iResult = 0;
    if ( m_sock != HOX_INVALID_SOCKET )
    {
        iResult = tcp_close( m_sock );
        m_sock = HOX_INVALID_SOCKET;
    }
    return iResult;
}

int
Socket::ReadLine( std::string& sLine,
                  unsigned int nMax /* = HOX_CMD_LINE_MAX */ )
{
    return tcp_read_line( m_sock, sLine, nMax );
}

int
Socket::ReadNBytes( unsigned int  nBytes,
                    std::string&  sOutput )
{
    return tcp_read_nbytes( m_sock, nBytes, sOutput );
}

int
Socket::ReadUntilAll( const std::string& sWanted,
                      std::string&       sOutput )
{
    return tcp_read_until_all( m_sock, sWanted, sOutput );
}

int
Socket::SendData( const std::string& sData )
{
    return tcp_send_data( m_sock, sData );
}

int
Socket::SendLine( const std::string& sData )
{
    return tcp_send_line( m_sock, sData );
}

} /* namespace HOX */

/////////////////////// END OF FILE ///////////////////////////////////////////
