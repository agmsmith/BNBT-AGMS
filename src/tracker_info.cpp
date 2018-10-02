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
#include "tracker.h"
// added /info.bencode support - DWK
#include "bencode.h"
#include "util.h"

void CTracker :: serverResponseInfo( struct request_t *pRequest, struct response_t *pResponse )
{
	struct bnbttv btv = UTIL_CurrentTime( );
	
	time_t tNow = time( NULL ) + m_iRefreshFastCacheInterval;
	char *szTime = asctime( gmtime( &tNow ) );
	szTime[strlen( szTime ) - 1] = '\0';

	pResponse->strCode = "200 OK";

	pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", string( gmapMime[".html"] + "; charset=" ) + gstrCharSet ) ); /* =X= */
	pResponse->mapHeaders.insert( pair<string, string>( "Expires", string( szTime ) + " GMT" ) );

	pResponse->strContent += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n"; /* =X= */
	pResponse->strContent += "<html lang=\"en\">\n";
	pResponse->strContent += "<head>\n";
	pResponse->strContent += "<title>BNBT Tracker Info</title>\n";

	if( !gstrStyle.empty( ) )
		pResponse->strContent += "<link rel=\"stylesheet\" title=\"external\" type=\"" + gmapMime[".css"] + "\" href=\"" + gstrStyle + "\">\n"; /* =X= */

	/* =X= */
	// Embeded Styles for browser bugfixes
	// The style tags are visible on Pre-HTML 3.2 browsers
	if( pRequest->user.iAccess & ACCESS_VIEW )
	{
		pResponse->strContent += "<style type=\"" + gmapMime[".css"] + "\" title=\"internal\">\n";
		pResponse->strContent += "<!--\n";
		pResponse->strContent += "html{overflow-x:auto}\n"; /* IE6 scroll bar bugfix */
		pResponse->strContent += "td{empty-cells:show}\n"; /* Empty cells bugfix */
		pResponse->strContent += "-->\n";
		pResponse->strContent += "</style>\n";
	}

	/* =X= */
	if( !m_strFavicon.empty( ) )
	{
		string strExt = string( );

		string :: size_type iExt = m_strFaviconFile.rfind( "." );

		if( iExt != string :: npos )
			strExt = m_strFaviconFile.substr( iExt );

		if( strExt == ".ico" )
			pResponse->strContent += "<link rel=\"Shortcut Icon\" type=\"" + gmapMime[".ico"] + "\" href=\"favicon.ico\">\n";
		else if( strExt == ".png" )
			pResponse->strContent += "<link rel=\"Shortcut Icon\" type=\"" + gmapMime[".png"] + "\" href=\"favicon.ico\">\n";
		else if( strExt == ".gif" )
			pResponse->strContent += "<link rel=\"Shortcut Icon\" type=\"" + gmapMime[".gif"] + "\" href=\"favicon.ico\">\n";
	}

	/* =X= */
	// META information
	pResponse->strContent += "<META http-equiv=\"Content-Type\" content=\"text/html; charset=" + gstrCharSet + "\">\n";
	pResponse->strContent += "<META http-equiv=\"Content-Script-Type\" content=\"text/javascript\">\n";
	pResponse->strContent += "<META http-equiv=\"Expires\" content=\"" + string( szTime ) + " GMT\">\n";
	pResponse->strContent += "<META name=\"Generator\" content=\"BNBT " + string( BNBT_VER ) + "\">\n";
	pResponse->strContent += "<META name=\"MSSmartTagsPreventParsing\" content=\"true\">\n";

	pResponse->strContent += "</head>\n";
	pResponse->strContent += "<body>\n\n";

	if( pRequest->user.strLogin.empty( ) )
		pResponse->strContent += "<p class=\"login1_info\">You are not logged in. Click <a href=\"/login.html\">here</a> to login.</p>\n";
	else
		pResponse->strContent += "<p class=\"login2_info\">You are logged in as <span class=\"username\">" + UTIL_RemoveHTML( pRequest->user.strLogin ) + "</span>. Click <a href=\"/login.html?logout=1\">here</a> to logout.</p>\n";

	pResponse->strContent += m_strStaticHeader; /* =X= */

	pResponse->strContent += "<h3>BNBT Tracker Info</h3>\n";

	if( pRequest->user.iAccess & ACCESS_VIEW )
	{
		time_t tNowLocal = time( NULL );
		char *szTimeLocal = asctime( localtime( &tNowLocal ) );
		szTimeLocal[strlen( szTimeLocal ) - 1] = '\0';

		pResponse->strContent += "<ul>\n";
		pResponse->strContent += "<li><strong>Tracker Version:</strong> BNBT " + string( BNBT_VER ) + "</li>\n";
		pResponse->strContent += "<li><strong>Server Time:</strong> " + string( szTimeLocal ) + "</li>\n";
		pResponse->strContent += "<li><strong>Uptime:</strong> " + UTIL_SecondsToString( GetTime( ) ) + "</li>\n";

		if( m_pDFile )
		{
			pResponse->strContent += "<li><strong>Tracking " + CAtomLong( m_pDFile->getValuePtr( )->size( ) ).toString( ) + " Files, ";

			unsigned long iPeers = 0;

			map<string, CAtom *> *pmapDicti = m_pDFile->getValuePtr( );

			for( map<string, CAtom *> :: iterator i = pmapDicti->begin( ); i != pmapDicti->end( ); i++ )
			{
				if( (*i).second->isDicti( ) )
					iPeers += (unsigned long)( (CAtomDicti *)(*i).second )->getValuePtr( )->size( );
			}

			pResponse->strContent += CAtomLong( iPeers ).toString( ) + " Peers";

			if( m_bCountUniquePeers )
				pResponse->strContent += ", " + CAtomLong( m_pIPs->getValuePtr( )->size( ) ).toString( ) + " Unique";

			pResponse->strContent += "</strong></li>\n";
		}

		if( m_pUsers )
			pResponse->strContent += "<li><strong>" + CAtomLong( m_pUsers->getValuePtr( )->size( ) ).toString( ) + " Users</strong></li>\n";

		pResponse->strContent += "</ul>\n";
	}
	else
		pResponse->strContent += "<p class=\"denied\">You are not authorized to view this page.</p>\n";

	pResponse->strContent += m_strStaticFooter; /* =X= */

	if( m_bGen )
		pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

	pResponse->strContent += "</body>\n";
	pResponse->strContent += "</html>\n";
}

// Added /info.bencode support- DWK
void CTracker :: serverResponseBencodeInfo( struct request_t *pRequest, struct response_t *pResponse )
{
	(void) pRequest; // Unused variable, make compiler not warn about that.
	pResponse->strCode = "200 OK";
	pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", string( "text/plain" ) ) );

	CAtomDicti *pData = new CAtomDicti( );
	pData->setItem("version", new CAtomString( string( BNBT_VER ) ) );

	if( m_pDFile )
		{
		pData->setItem("files", new CAtomLong( m_pDFile->getValuePtr( )->size( ) ) );
		unsigned long iPeers = 0;

		map<string, CAtom *> *pmapDicti = m_pDFile->getValuePtr( );

		for( map<string, CAtom *> :: iterator i = pmapDicti->begin( ); i != pmapDicti->end( ); i++ )
		{
			if( (*i).second->isDicti( ) )
				iPeers += (unsigned long)( (CAtomDicti *)(*i).second )->getValuePtr( )->size( );
		}

		pData->setItem("peers", new CAtomLong( iPeers ) );

		if( m_bCountUniquePeers )
			pData->setItem("unique", new CAtomLong( m_pIPs->getValuePtr( )->size( ) ) );
	}

	pResponse->strContent = Encode( pData );
	pResponse->bCompressOK = false;

	delete pData;
}
