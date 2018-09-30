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

#include <signal.h>

#include "bnbt.h"
#include "bnbt_mysql.h"
#include "config.h"
#include "server.h"
#include "tracker.h"

#ifdef WIN32
 #include "util_ntservice.h"
#endif

#include "util.h"

#ifndef WIN32
 int GetLastError( ) { return errno; }
#endif

const char *GetLastErrorString( )
{
#ifdef WIN32
	return UTIL_ErrorToString( GetLastError( ) );
#else
	return UTIL_ErrorToString( errno );
#endif
}

time_t giStartTime;

unsigned long GetTime( )
{
	return (unsigned long)( time( NULL ) - giStartTime );
}

CServer *gpServer;
CTracker *gpTracker;
string gstrErrorLogDir;
string gstrErrorLogFile;
FILE *gpErrorLog;
string gstrAccessLogDir;
string gstrAccessLogFile;
FILE *gpAccessLog;
unsigned long giErrorLogCount;
unsigned long giAccessLogCount;
int giFlushInterval;
bool gbDebug;
unsigned int giMaxConns;
unsigned int giMaxRecvSize;
string gstrStyle;
string gstrCharSet;
string gstrRealm;

void sigCatcher( int sig )
{
	signal( SIGABRT, sigCatcher );
	signal( SIGINT, sigCatcher );
	signal( SIGTERM, sigCatcher ); /* =X= */

	switch( sig )
	{
	case SIGABRT:
		UTIL_LogPrint( "BNBT caught SIGABRT\n" );

		break;
	case SIGINT:
		UTIL_LogPrint( "BNBT caught SIGINT\n" );

		break;
	case SIGTERM:
		UTIL_LogPrint( "BNBT caught SIGTERM\n" );

		break;
	default:
		UTIL_LogPrint( "BNBT caught unknown signal\n" );
	} /* =X= */

	if( gpServer )
	{
		if( gpServer->isDying( ) )
			exit( 1 );
		else
			gpServer->Kill( );
	}
	else
		exit( 1 );
}

int main( int argc, char *argv[] )
{
#ifdef WIN32
	if( argv[0] )
	{
		char *szEndPos = strrchr( argv[0], '\\' );

		if( szEndPos )
		{
			char *szEXEPath = new char[szEndPos - argv[0] + 1];
			memcpy( szEXEPath, argv[0], szEndPos - argv[0] );
			szEXEPath[szEndPos - argv[0]] = '\0';

			SetCurrentDirectory( szEXEPath );

			delete [] szEXEPath;
		}
	}

	if( argc > 1 )
	{
		if( _stricmp( argv[1], "-i" ) == 0 )
		{
			// install service

			if( UTIL_NTServiceTest( ) )
				printf( "BNBT Service is already installed!\n" );
			else
			{
				if( UTIL_NTServiceInstall( ) )
					printf( "BNBT Service installed.\n" );
				else
					printf( "BNBT Service failed to install (error %d).\n", GetLastError( ) );
			}

			return 0;
		}
		else if( _stricmp( argv[1], "-u" ) == 0 )
		{
			// uninstall service

			if( !UTIL_NTServiceTest( ) )
				printf( "BNBT Service is not installed!\n" );
			else
			{
				if( UTIL_NTServiceUninstall( ) )
					printf( "BNBT Service uninstalled.\n" );
				else
					printf( "BNBT Service failed to uninstall (error %d).\n", GetLastError( ) );
			}

			return 0;
		}
		else if( _stricmp( argv[1], "-start" ) == 0 )
		{
			// start

			if( !UTIL_NTServiceTest( ) )
				printf( "BNBT Service is not installed!\n" );
			else
			{
				printf( "Starting BNBT Service.\n" );

				if( !UTIL_NTServiceStart( ) )
					printf( "BNBT Service failed to start (error %d).\n", GetLastError( ) );
			}

			return 0;
		}
		else if( _stricmp( argv[1], "-stop" ) == 0 )
		{
			// stop

			if( !UTIL_NTServiceTest( ) )
				printf( "BNBT Service is not installed!\n" );
			else
			{
				printf( "Stopping BNBT Service.\n" );

				if( !UTIL_NTServiceStop( ) )
					printf( "BNBT Service failed to stop (error %d).\n", GetLastError( ) );
			}

			return 0;
		}
		else if( _stricmp( argv[1], "-s" ) == 0 )
		{
			// internal start

			SERVICE_TABLE_ENTRY st[] = {
				{ BNBT_SERVICE_NAME, NTServiceMain },
				{ NULL, NULL }
			};

			StartServiceCtrlDispatcher( st );

			return 0;
		}
	}
#else
	// disable SIGPIPE since some systems like OS X don't define MSG_NOSIGNAL

	signal( SIGPIPE, SIG_IGN );
#endif

	// catch SIGABRT and SIGINT

	signal( SIGABRT, sigCatcher );
	signal( SIGINT, sigCatcher );
	signal( SIGTERM, sigCatcher ); /* =X= */

	return bnbtmain( );
}

int bnbtmain( )
{
	srand( (unsigned int)time( NULL ) );

	giStartTime = time( NULL );

	CFG_Open( CFG_FILE );
	CFG_SetDefaults( );
	CFG_Close( CFG_FILE );

	gstrErrorLogDir = CFG_GetString( "bnbt_error_log_dir", string( ) );

	if( !gstrErrorLogDir.empty( ) && gstrErrorLogDir[gstrErrorLogDir.size( ) - 1] != PATH_SEP )
		gstrErrorLogDir += PATH_SEP;

	gpErrorLog = NULL;
	gstrAccessLogDir = CFG_GetString( "bnbt_access_log_dir", string( ) );

	if( !gstrAccessLogDir.empty( ) && gstrAccessLogDir[gstrAccessLogDir.size( ) - 1] != PATH_SEP )
		gstrAccessLogDir += PATH_SEP;

	gpAccessLog = NULL;
	giErrorLogCount = 0;
	giAccessLogCount = 0;
	giFlushInterval = CFG_GetInt( "bnbt_flush_interval", 100 );
	gbDebug = CFG_GetInt( "bnbt_debug", 1 ) == 0 ? false : true;
	giMaxConns = CFG_GetInt( "bnbt_max_conns", 64 );
	giMaxRecvSize = CFG_GetInt( "bnbt_max_recv_size", 524288 );
	gstrStyle = CFG_GetString( "bnbt_style_sheet", string( ) );
	gstrCharSet = CFG_GetString( "bnbt_charset", "iso-8859-1" );
	gstrRealm = CFG_GetString( "bnbt_realm", "BNBT" );

	// start winsock

#ifdef WIN32
	WSADATA wsadata;

	if( WSAStartup( MAKEWORD( 2, 0 ), &wsadata ) != 0 )
	{
		UTIL_LogPrint( "error - unable to start winsock (error %s)\n", GetLastErrorString( ) );

		if( gpAccessLog )
			fclose( gpAccessLog );

		if( gpErrorLog )
			fclose( gpErrorLog );

		return 1;
	}
#endif

	// start mysql

#ifdef BNBT_MYSQL
	if( !( gpMySQL = mysql_init( NULL ) ) )
	{
		UTIL_LogPrint( "mysql error - %s\n", mysql_error( gpMySQL ) );

		if( gpAccessLog )
			fclose( gpAccessLog );

		if( gpErrorLog )
			fclose( gpErrorLog );

		return 1;
	}

	gstrMySQLHost = CFG_GetString( "mysql_host", string( ) );
	gstrMySQLDatabase = CFG_GetString( "mysql_database", "bnbt" );
	gstrMySQLUser = CFG_GetString( "mysql_user", string( ) );
	gstrMySQLPassword = CFG_GetString( "mysql_password", string( ) );
	giMySQLPort = CFG_GetInt( "mysql_port", 0 );

	if( !( mysql_real_connect( gpMySQL, gstrMySQLHost.c_str( ), gstrMySQLUser.c_str( ), gstrMySQLPassword.c_str( ), gstrMySQLDatabase.c_str( ), giMySQLPort, NULL, 0 ) ) )
	{
		UTIL_LogPrint( "mysql error - %s\n", mysql_error( gpMySQL ) );

		if( gpAccessLog )
			fclose( gpAccessLog );

		if( gpErrorLog )
			fclose( gpErrorLog );

		return 1;
	}

	UTIL_LogPrint( "mysql - connected\n" );

	// CMySQLQuery mq( "REPLACE INTO bnbt_status (status_key,status_datetime_value) VALUES('bnbt_start_time',NOW())" );
#endif

	// Fix for bugs relating to clients connecting before the tracker processor is constructed - DWK in consultation with Trevor
	gpServer = NULL;
	gpTracker = new CTracker( );
	gpServer = new CServer( );

	while( 1 )
	{
		if( gpServer->Update( true ) )
		{
			delete gpServer;
			delete gpTracker;

			gpServer = NULL;
			gpTracker = NULL;

			break;
		}

		// tphogan - untied CTracker from CServer so don't forget to update on frame
		// note that CTracker updates are just long interval updates
		// stuff like parsing the allowed dir and refreshing the static files
		// this can't fail so no need to check as with CServer above

		gpTracker->Update( );
	}

#ifdef BNBT_MYSQL
	mysql_close( gpMySQL );
#endif

	if( gpAccessLog )
		fclose( gpAccessLog );

	if( gpErrorLog )
		fclose( gpErrorLog );

#ifdef WIN32
	WSACleanup( );
#endif

	return 0;
}
