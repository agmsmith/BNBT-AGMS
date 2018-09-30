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
#include "server.h"
#include "util.h"

void CTracker :: serverResponseAdmin( struct request_t *pRequest, struct response_t *pResponse )
{
	struct bnbttv btv = UTIL_CurrentTime( );
	
	time_t tNow = time( NULL ) + m_iRefreshFastCacheInterval;
	char *szTime = asctime( gmtime( &tNow ) );
	szTime[strlen( szTime ) - 1] = '\0';

	pResponse->strCode = "200 OK";

	pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", string( gmapMime[".html"] + "; charset=" ) + gstrCharSet ) );  /* =X= */
	pResponse->mapHeaders.insert( pair<string, string>( "Expires", string( szTime ) + " GMT" ) );

	pResponse->strContent += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n"; /* =X= */
	pResponse->strContent += "<html lang=\"en\">\n";
	pResponse->strContent += "<head>\n";
	pResponse->strContent += "<title>BNBT Admin Panel</title>\n";

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

	/* =X= */
	// Display a message if javascript not supported by browser
	pResponse->strContent += "<noscript>\n";    
	pResponse->strContent += "<p class=\"js_warning\">Please enable JavaScript support or upgrade your browser.</p>\n";   
	pResponse->strContent += "</noscript>\n\n"; 

	if( pRequest->user.strLogin.empty( ) )
		pResponse->strContent += "<p class=\"login1_admin\">You are not logged in. Click <a href=\"/login.html\">here</a> to login.</p>\n";
	else
		pResponse->strContent += "<p class=\"login2_admin\">You are logged in as <span class=\"username\">" + UTIL_RemoveHTML( pRequest->user.strLogin ) + "</span>. Click <a href=\"/login.html?logout=1\">here</a> to logout.</p>\n";

	pResponse->strContent += m_strStaticHeader; /* =X= */

	pResponse->strContent += "<h3>BNBT Admin Panel</h3>\n";
	
	if( pRequest->user.iAccess & ACCESS_ADMIN )
	{
		//
		// kill tracker
		//

		if( pRequest->mapParams["ap_kill"] == "1" )
		{
			gpServer->Kill( );

			return;
		}

		if( m_bCountUniquePeers && pRequest->mapParams["ap_recount"] == "1" )
		{
			gpTracker->CountUniquePeers( );

			pResponse->strContent += "<p>Counting unique peers. Click <a href=\"/admin.html\">here</a> to return to the admin page.</p>\n";
			pResponse->strContent += m_strStaticFooter; /* =X= */

			if( m_bGen )
				pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

			pResponse->strContent += "</body>\n";
			pResponse->strContent += "</html>\n";

			return;
		}

		// addition by labarks

		if( !m_strDumpRSSFile.empty( ) && pRequest->mapParams["ap_rss"] == "1" )
		{
			if( m_iDumpRSSFileMode == 0 || m_iDumpRSSFileMode == 2 )
			{
				if( gbDebug )
					UTIL_LogPrint( "tracker - dumping RSS\n" );

				saveRSS( );
			}

			if( m_iDumpRSSFileMode == 1 || m_iDumpRSSFileMode == 2 )
			{
				for( vector< pair<string, string> > :: iterator i = m_vecTags.begin( ); i != m_vecTags.end( ); i++ )
				{
					string strTag = (string)(*i).first;

					if( gbDebug )
						UTIL_LogPrint( "tracker - dumping RSS for %s\n", strTag.c_str( ) );

					saveRSS( strTag );
				}

				if( !m_vecTags.size( ) && m_iDumpRSSFileMode == 1 )
					UTIL_LogPrint( "tracker warning - no tags to dump RSS files per category, try changing to mode 0 or 2\n" );
			}

			pResponse->strContent += "<p>Updated RSS file(s). Click <a href=\"/admin.html\">here</a> to return to the admin page.</p>\n";
			pResponse->strContent += m_strStaticFooter; /* =X= */

			if( m_bGen )
				pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

			pResponse->strContent += "</body>\n";
			pResponse->strContent += "</html>\n";

			return;
		}

		// end addition

		//
		// clients
		//

		pResponse->strContent += "<p>Currently serving ";
		pResponse->strContent += CAtomInt( (int)gpServer->m_vecClients.size( ) ).toString( );
		pResponse->strContent += " clients (including you)!</p>\n";

		//
		// kill tracker
		//

		pResponse->strContent += "<p><a href=\"/admin.html?ap_kill=1\">Stop Tracker</a></p>\n";
		pResponse->strContent += "<p>If you stop the tracker your connection will be dropped and no response will be sent to your browser.</p>\n";

		//
		// count unique peers
		//

		if( m_bCountUniquePeers )
			pResponse->strContent += "<p><a href=\"/admin.html?ap_recount=1\">Count Unique Peers</a></p>\n";

		// addition by labarks

		if( !m_strDumpRSSFile.empty( ) )
			pResponse->strContent += "<p><a href=\"/admin.html?ap_rss=1\">Update RSS file(s)</a></p>\n";

		// end addition
	}
	else
		pResponse->strContent += "<p class=\"denied\">You are not authorized to view this page.</p>\n";
		
	pResponse->strContent += m_strStaticFooter; /* =X= */

	if( m_bGen )
		pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

	pResponse->strContent += "</body>\n";
	pResponse->strContent += "</html>\n";
}
