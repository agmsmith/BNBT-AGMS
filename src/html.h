//
// Copyright (C) 2003-2005 Trevor Hogan
//

#ifndef HTML_H
 #define HTML_H

// tphogan - reference http://www.greenend.org.uk/rjk/2003/03/inline.html

static inline string HTML_MakeURLFromFQDN( string &strFQDN, const char *szURL )
{
	if( strFQDN.empty( ) )
		return string( szURL );
	else
		return string( "http://" + strFQDN + szURL );
}

/*

static inline string HTML_MakeURL( const char *szURL )
{
	return HTML_MakeURLFromFQDN( gstrFQDN, szURL );
}

*/

#endif
