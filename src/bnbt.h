//
// Copyright (C) 2003-2005 Trevor Hogan
//

#ifndef BNBT_H
 #define BNBT_H

#include <time.h>

#ifndef WIN32
 #include <errno.h>
#endif

#ifdef WIN32
 #define WIN32_LEAN_AND_MEAN
#endif

//
// SOLARIS USERS - IF YOUR SYSTEM IS LITTLE ENDIAN, REMOVE THE NEXT 3 LINES
//  also see sha1.h
//

#if defined( __APPLE__ ) || defined( __SOLARIS__ )
 #define BNBT_BIG_ENDIAN
#endif

// large integers

#ifdef WIN32
 typedef __int64 int64;
 typedef unsigned __int64 uint64;
#else
 typedef long long int64;
 typedef unsigned long long uint64;
#endif

// stl

#ifdef WIN32
 #pragma warning( disable : 4786 )
#endif

// tphogan - these new includes may have broken BSD support

#include <cstring>
#include <iostream>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <utility>

// tphogan - eliminate namespace pollution

using std :: endl;
using std :: ifstream;
using std :: ofstream;
using std :: map;
using std :: multimap;
using std :: pair;
using std :: string;
using std :: vector;

// path seperator

#ifdef WIN32
 #define PATH_SEP '\\'
#else
 #define PATH_SEP '/'
#endif

// this fixes MSVC loop scoping issues

/*

#ifdef WIN32
 #define for if( 0 ) { } else for
#endif

*/

// time stuff

unsigned long GetTime( );

#ifdef WIN32
 #define MILLISLEEP( x ) Sleep( x )
#else
 #define MILLISLEEP( x ) usleep( ( x ) * 1000 )
#endif

// network

#ifdef WIN32
 #include <winsock.h>

 #define EADDRINUSE WSAEADDRINUSE
 #define EADDRNOTAVAIL WSAEADDRNOTAVAIL
 #define EAFNOSUPPORT WSAEAFNOSUPPORT
 #define EALREADY WSAEALREADY
 #define ECONNABORTED WSAECONNABORTED
 #define ECONNREFUSED WSAECONNREFUSED
 #define ECONNRESET WSAECONNRESET
 #define EDESTADDRREQ WSAEDESTADDRREQ
 #define EDQUOT WSAEDQUOT
 #define EHOSTDOWN WSAEHOSTDOWN
 #define EHOSTUNREACH WSAEHOSTUNREACH
 #define EINPROGRESS WSAEINPROGRESS
 #define EISCONN WSAEISCONN
 #define ELOOP WSAELOOP
 #define EMSGSIZE WSAEMSGSIZE
 // #define ENAMETOOLONG WSAENAMETOOLONG
 #define ENETDOWN WSAENETDOWN
 #define ENETRESET WSAENETRESET
 #define ENETUNREACH WSAENETUNREACH
 #define ENOBUFS WSAENOBUFS
 #define ENOPROTOOPT WSAENOPROTOOPT
 #define ENOTCONN WSAENOTCONN
 // #define ENOTEMPTY WSAENOTEMPTY
 #define ENOTSOCK WSAENOTSOCK
 #define EOPNOTSUPP WSAEOPNOTSUPP
 #define EPFNOSUPPORT WSAEPFNOSUPPORT
 #define EPROTONOSUPPORT WSAEPROTONOSUPPORT
 #define EPROTOTYPE WSAEPROTOTYPE
 #define EREMOTE WSAEREMOTE
 #define ESHUTDOWN WSAESHUTDOWN
 #define ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
 #define ESTALE WSAESTALE
 #define ETIMEDOUT WSAETIMEDOUT
 #define ETOOMANYREFS WSAETOOMANYREFS
 #define EUSERS WSAEUSERS
 #define EWOULDBLOCK WSAEWOULDBLOCK
#else
 #include <arpa/inet.h>
 #include <netdb.h>
 #include <netinet/in.h>
 #include <sys/ioctl.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <unistd.h>

 typedef int SOCKET;

 #define INVALID_SOCKET -1
 #define SOCKET_ERROR -1

 #define closesocket close

 extern int GetLastError( );
#endif

extern const char *GetLastErrorString( );

#ifdef __APPLE__
 typedef int socklen_t;
 typedef int sockopt_len_t;
#endif

/*

#ifdef FreeBSD
 #include <sys/stat.h>
#endif

*/

#ifndef INADDR_NONE
 #define INADDR_NONE -1
#endif

#ifndef MSG_NOSIGNAL
 #define MSG_NOSIGNAL 0
#endif

class CAtom;
class CAtomInt;
class CAtomLong;
class CAtomString;
class CAtomList;
class CAtomDicti;

class CServer;
class CTracker;
class CClient;

struct response_t
{
	string strCode;
	multimap<string, string> mapHeaders;
	string strContent;
	bool bCompressOK;
};

// user access levels

#define ACCESS_VIEW				( 1 << 0 )		// 1
#define ACCESS_DL				( 1 << 1 )		// 2
#define ACCESS_COMMENTS			( 1 << 2 )		// 4
#define ACCESS_UPLOAD			( 1 << 3 )		// 8
#define ACCESS_EDIT				( 1 << 4 )		// 16
#define ACCESS_ADMIN			( 1 << 5 )		// 32
#define ACCESS_SIGNUP			( 1 << 6 )		// 64

struct user_t
{
	string strLogin;
	string strLowerLogin;
	string strMD5;
	string strMail;
	string strLowerMail;
	string strCreated;
	int iAccess;
};

struct request_t
{
	struct sockaddr_in sin;
	string strMethod;
	string strURL;
	// Harold - Adding Support for Multiple scrapes in a single /scrape call
	bool hasQuery;
	multimap<string, string> multiParams;
	map<string, string> mapParams;
	map<string, string> mapHeaders;
	map<string, string> mapCookies;
	struct user_t user;
};

// current version

#define BNBT_VER "Beta 8.5 CVS (Unstable)"

#ifdef WIN32
 #define BNBT_SERVICE_NAME "BNBT Service"
#endif

extern CServer *gpServer;
extern CTracker *gpTracker;
extern string gstrErrorLogDir;
extern string gstrErrorLogFile;
extern FILE *gpErrorLog;
extern string gstrAccessLogDir;
extern string gstrAccessLogFile;
extern FILE *gpAccessLog;
extern unsigned long giErrorLogCount;
extern unsigned long giAccessLogCount;
extern int giFlushInterval;
extern bool gbDebug;
extern unsigned int giMaxConns;
extern unsigned int giMaxRecvSize;
extern string gstrStyle;
extern string gstrCharSet;
extern string gstrRealm;

// this is basically the old main( ), but it's here to make the NT Service code neater

extern int bnbtmain( );

#endif
