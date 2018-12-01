/***
*
* BNBT Beta 8.5 - A C++ BitTorrent Tracker
* Copyright (C) 2003-2005 Trevor Hogan
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
***/

#include "bnbt.h"
#include "atom.h"
#include "client.h"
#include "config.h"
#include "server.h"
#include "tracker.h"
#include "util.h"

#ifdef WIN32
 #include "util_ntservice.h"
#endif

CServer :: CServer( )
{
	m_bKill = false;

	m_iSocketTimeOut = CFG_GetInt( "socket_timeout", 15 );
	m_strBind = CFG_GetString( "bind", string( ) );
	m_iCompression = CFG_GetInt( "bnbt_compression_level", 6 );

	// clamp compression

	if( m_iCompression > 9 )
		m_iCompression = 9;

	struct sockaddr_in sin;

	memset( &sin, 0, sizeof( sin ) );

	sin.sin_family = AF_INET;

	if( !m_strBind.empty( ) )
	{
		// bind to m_strBind

		if( gbDebug )
			UTIL_LogPrint( "server - binding to %s\n", m_strBind.c_str( ) );

		if( ( sin.sin_addr.s_addr = inet_addr( m_strBind.c_str( ) ) ) == INADDR_NONE )
		{
			UTIL_LogPrint( "server error - unable to bind to %s\n", m_strBind.c_str( ) );

			exit( 1 );
		}
	}
	else
	{
		// bind to all available addresses

		if( gbDebug )
			UTIL_LogPrint( "server - binding to all available addresses\n" );

		sin.sin_addr.s_addr = INADDR_ANY;
	}

	// tphogan - legacy support, check for "port" config entry
	// by doing this here "port" will always be required
	// so in fact this isn't entirely legacy support since it'll continue to be used

	if( ( sin.sin_port = htons( (u_short)CFG_GetInt( "port", 6969 ) ) ) == 0 )
		UTIL_LogPrint( "server warning - invalid port %d (\"port\"), ignoring\n", CFG_GetInt( "port", 6969 ) );
	else if( !AddListener( sin ) )
		UTIL_LogPrint( "server warning - unable to add listener on port %d (\"port\")\n", CFG_GetInt( "port", 6969 ) );
	else
		UTIL_LogPrint( "server - listening on port %d (\"port\")\n", CFG_GetInt( "port", 6969 ) );

	// tphogan - retrieve list of ports from config file for multiport listeners
	// do we want to support multiple bindings as well?
	// this code will bind every socket to the same address

	int iPort = 1;

	string strName = "port" + CAtomInt( iPort ).toString( );
	string strPort = CFG_GetString( strName, string( ) );

	while( !strPort.empty( ) )
	{
		if( ( sin.sin_port = htons( (u_short)atoi( strPort.c_str( ) ) ) ) == 0 )
			UTIL_LogPrint( "server warning - invalid port %d (\"%s\"), ignoring\n", atoi( strPort.c_str( ) ), strName.c_str( ) );
		else if( !AddListener( sin ) )
			UTIL_LogPrint( "server warning - unable to add listener on port %d (\"%s\")\n", atoi( strPort.c_str( ) ), strName.c_str( ) );
		else
			UTIL_LogPrint( "server - listening on port %d (\"%s\")\n", atoi( strPort.c_str( ) ), strName.c_str( ) );

		strName = "port" + CAtomInt( ++iPort ).toString( );
		strPort = CFG_GetString( strName, string( ) );
	}

	// tphogan - we didn't exit on invalid ports above
	// so make sure we're listening on at least one valid port
	// however, since "port" is always forced greater than zero in CFG_SetDefaults
	// this should only happen if a valid port is denied for "port"
	// for example, address in use or not enough privs for ports < 1024

	if( m_vecListeners.empty( ) )
	{
		UTIL_LogPrint( "server error - not listening on any ports" );

		exit( 1 );
	}

	UTIL_LogPrint( "server - start\n" );
}

CServer :: ~CServer( )
{
	for( vector<SOCKET> :: iterator i = m_vecListeners.begin( ); i != m_vecListeners.end( ); i++ )
		closesocket( *i );

	for( vector<CClient *> :: iterator j = m_vecClients.begin( ); j != m_vecClients.end( ); j++ )
		delete *j;

	m_vecListeners.clear( );
	m_vecClients.clear( );

	UTIL_LogPrint( "server - exit\n" );
}

void CServer :: Kill( )
{
	m_bKill = true;
}

bool CServer :: isDying( )
{
	return m_bKill;
}

bool CServer :: Update( bool bBlock )
{
	if( m_vecClients.size( ) < giMaxConns )
	{
		// tphogan - check every listener for new connections

		for( vector<SOCKET> :: iterator i = m_vecListeners.begin( ); i != m_vecListeners.end( ); i++ )
		{
			fd_set fdServer;

			FD_ZERO( &fdServer );
			FD_SET( *i, &fdServer );

			// tphogan - only block on the first listener
			// this is actually a bit of a hack but I don't feel like doing it "right" :)

			struct timeval tv;

			if( bBlock && i == m_vecListeners.begin( ) )
			{
				// block for 100 ms to keep from eating up all cpu time

				tv.tv_sec = 0;
				tv.tv_usec = 100000;
			}
			else
			{
				tv.tv_sec = 0;
				tv.tv_usec = 0;
			}

#ifdef WIN32
			if( select( 1, &fdServer, NULL, NULL, &tv ) == SOCKET_ERROR )
#else
			if( select( *i + 1, &fdServer, NULL, NULL, &tv ) == SOCKET_ERROR )
#endif
			{
				UTIL_LogPrint( "server warning - select error (error %s)\n", GetLastErrorString( ) );

				FD_ZERO( &fdServer );
			}

			if( FD_ISSET( *i, &fdServer ) )
			{
				struct sockaddr_in adrFrom;

				int iAddrLen = sizeof( adrFrom );

				SOCKET sckClient;

#ifdef WIN32
				if( ( sckClient = accept( *i, (struct sockaddr *)&adrFrom, &iAddrLen ) ) == INVALID_SOCKET )
#else
				if( ( sckClient = accept( *i, (struct sockaddr *)&adrFrom, (socklen_t *)&iAddrLen ) ) == INVALID_SOCKET )
#endif
					UTIL_LogPrint( "server warning - accept error (error %s)\n", GetLastErrorString( ) );
				else
					m_vecClients.push_back( new CClient( sckClient, adrFrom, m_iSocketTimeOut, m_iCompression ) );
			}
		}
	}
	else
	{
		// maximum connections reached

		// tphogan - reduced from 100 ms to 10 ms
		// it's very difficult to tell if the backlog is due to legitimate load or hung clients
		// hung clients don't eat CPU time so the server's CPU usage will skyrocket
		// but if it's due to load then sleeping for 100 ms is a terrible idea!
		// someone should take a look at this and rewrite it eventually

		MILLISLEEP( 10 );
	}

	// process

	for( vector<CClient *> :: iterator i = m_vecClients.begin( ); i != m_vecClients.end( ); )
	{
		if( (*i)->Update( ) )
		{
			delete *i;

			i = m_vecClients.erase( i );
		}
		else
			i++;
	}

	return m_bKill;
}

bool CServer :: AddListener( struct sockaddr_in sin )
{
	SOCKET sckListener;

	// map protocol name to protocol number

	struct protoent *pPE;

	if( ( pPE = getprotobyname( "tcp" ) ) == 0 )
	{
		UTIL_LogPrint( "server warning - unable to get tcp protocol entry (error %s)\n", GetLastErrorString( ) );

		return false;
	}

	// allocate socket

	if( ( sckListener = socket( PF_INET, SOCK_STREAM, pPE->p_proto ) ) == INVALID_SOCKET )
	{
		UTIL_LogPrint( "server warning - unable to allocate socket (error %s)\n", GetLastErrorString( ) );

		return false;
	}

	// bind socket

	int optval = 1;

#ifdef WIN32
	setsockopt( sckListener, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof( int ) );
#else
	setsockopt( sckListener, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof( int ) );
#endif

	if( bind( sckListener, (struct sockaddr *)&sin, sizeof( sin ) ) == SOCKET_ERROR )
	{
		UTIL_LogPrint( "server warning - unable to bind socket (error %s)\n", GetLastErrorString( ) );

		return false;
	}

	// listen

	if( listen( sckListener, 16 /* pending connections queue size */ ) == SOCKET_ERROR )
	{
		UTIL_LogPrint( "server warning - unable to listen (error %s)\n", GetLastErrorString( ) );

		return false;
	}

	m_vecListeners.push_back( sckListener );

	return true;
}
