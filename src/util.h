//
// Copyright (C) 2003-2005 Trevor Hogan
//

#ifndef UTIL_H
 #define UTIL_H

void UTIL_LogPrint( const char *format, ... )
#ifdef __GNUC__
 __attribute__ ((format(printf,1,2)))
#endif
;

void UTIL_AccessLogPrint( string strIP, string strUser, string strRequest, int iStatus, int iBytes );

// addition by labarks

string UTIL_AddedToDate( string strAdded );

// end addition

const char *UTIL_ErrorToString( int v );

//
// shamelessly stolen from wget source code
//

struct bnbttv
{
#ifdef WIN32
	ULARGE_INTEGER wintime;
#else
	long int sec;
	long int usec;
#endif
};

struct bnbttv UTIL_CurrentTime( );
long UTIL_ElapsedTime( struct bnbttv btvStart, struct bnbttv btvEnd );
string UTIL_ElapsedTimeStr( struct bnbttv btvStart, struct bnbttv btvEnd );

// string conversion routines

string UTIL_EscapedToString( const string &strEscape );
string UTIL_HashToString( const string &strHash );
string UTIL_BytesToString( int64 iBytes );
string UTIL_SecondsToString( unsigned long iSeconds );
string UTIL_StringToEscaped( const string &strString );
string UTIL_StringToEscapedStrict( const string &strString );
string UTIL_StringToHash( const string &strString );
string UTIL_AccessToString( int iAccess );

int64 UTIL_StringTo64( const char *sz64 );

string UTIL_InfoHash( CAtom *pTorrent );

// file routines

bool UTIL_CheckFile( const char *szFile );
void UTIL_MakeFile( const char *szFile, string strContents );
void UTIL_DeleteFile( const char *szFile );
void UTIL_MoveFile( const char *szFile, const char *szDest );
string UTIL_ReadFile( const char *szFile );
unsigned long UTIL_SizeFile( const char *szFile );

// string routines

string UTIL_ToLower( string strUpper );
string UTIL_StripPath( string strPath );
string UTIL_RemoveHTML( string strHTML );

// misc routines

string UTIL_FailureReason( string strFailureReason );

CAtomList *UTIL_DecodeHTTPPost( string strPost );

#endif
