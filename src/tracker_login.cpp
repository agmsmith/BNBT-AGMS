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
#include "util.h"

void CTracker :: serverResponseLogin( struct request_t *pRequest, struct response_t *pResponse )
{
	struct bnbttv btv = UTIL_CurrentTime( );

	if( pRequest->user.strLogin.empty( ) )
	{
		pResponse->strCode = "401 Unauthorized";

		pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", gmapMime[".txt"] ) ); /* =X= */
		pResponse->mapHeaders.insert( pair<string, string>( "WWW-Authenticate", string( "Basic realm=\"" ) + gstrRealm + "\"" ) );

		pResponse->bCompressOK = false;

		pResponse->strContent += "401 Unauthorized";

		return;
	}

	pResponse->strCode = "200 OK";

	// cookies

	time_t tNow = time( NULL );

	struct tm tmFuture = *gmtime( &tNow );
	tmFuture.tm_mon++;
	mktime( &tmFuture );
	struct tm tmPast = *gmtime( &tNow );
	tmPast.tm_mon--;
	mktime( &tmPast );

	char pTime[256];
	memset( pTime, 0, sizeof( char ) * 256 );

	string strLogout = pRequest->mapParams["logout"];

	if( strLogout == "1" )
		strftime( pTime, sizeof( char ) * 256, "%a, %d-%b-%Y %H:%M:%S GMT", &tmPast );
	else
		strftime( pTime, sizeof( char ) * 256, "%a, %d-%b-%Y %H:%M:%S GMT", &tmFuture );

	pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", string( gmapMime[".html"] + "; charset=" ) + gstrCharSet ) ); /* =X= */
	pResponse->mapHeaders.insert( pair<string, string>( "Pragma", "No-Cache" ) );
	pResponse->mapHeaders.insert( pair<string, string>( "Set-Cookie", "login=\"" + pRequest->user.strLogin + "\"; expires=" + pTime + "; path=/" ) );
	pResponse->mapHeaders.insert( pair<string, string>( "Set-Cookie", "md5=\"" + UTIL_StringToEscaped( pRequest->user.strMD5 ) + "\"; expires=" + pTime + "; path=/" ) );

	pResponse->strContent += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n"; /* =X= */
	pResponse->strContent += "<html lang=\"en\">\n";
	pResponse->strContent += "<head>\n";
	pResponse->strContent += "<title>BNBT Login</title>\n";

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
	pResponse->strContent += "<META http-equiv=\"Pragma\" content=\"No-Cache\">\n";
	pResponse->strContent += "<META name=\"Generator\" content=\"BNBT " + string( BNBT_VER ) + "\">\n";
	pResponse->strContent += "<META name=\"MSSmartTagsPreventParsing\" content=\"true\">\n";
	pResponse->strContent += "<META name=\"Set-Cookie\" content=\'login=\"" + pRequest->user.strLogin + "\"; expires=" + pTime + "; path=/\'>\n";
	pResponse->strContent += "<META name=\"Set-Cookie\" content=\'md5=\"" + UTIL_StringToEscaped( pRequest->user.strMD5 ) + "\"; expires=" + pTime + "; path=/\'>\n";

	pResponse->strContent += "</head>\n";
	pResponse->strContent += "<body>\n\n";

	/* =X= */
	// Display a message if javascript not supported by browser
	pResponse->strContent += "<noscript>\n";    
	pResponse->strContent += "<p class=\"js_warning\">Please enable JavaScript support or upgrade your browser.</p>\n";   
	pResponse->strContent += "</noscript>\n\n"; 

	/* =X= */
	if( pRequest->user.strLogin.empty( ) )
		pResponse->strContent += "<p class=\"login1_signup\">You are not logged in. Click <a href=\"/login.html\">here</a> to login.</p>\n";
	else
		pResponse->strContent += "<p class=\"login2_signup\">You are logged in as <span class=\"username\">" + UTIL_RemoveHTML( pRequest->user.strLogin ) + "</span>. Click <a href=\"/login.html?logout=1\">here</a> to logout.</p>\n";

	pResponse->strContent += m_strStaticHeader; /* =X= */

	pResponse->strContent += "<h3>BNBT Login - " + UTIL_RemoveHTML( pRequest->user.strLogin ) + "</h3>\n";

	if( strLogout == "1" )
		pResponse->strContent += "<p>Logging out... You may need to close your browser window to completely logout.</p>\n";
	else
	{
		pResponse->strContent += "<ul>\n";
		pResponse->strContent += "<li>You signed up on " + pRequest->user.strCreated + ".</li>\n";
		pResponse->strContent += "<li>Click <a href=\"/login.html?logout=1\">here</a> to logout.</li>\n";
		pResponse->strContent += "<li>Click <a href=\"/index.html\">here</a> to return to the tracker.</li>\n";
		pResponse->strContent += "</ul>\n";

		if( m_bDeleteOwnTorrents )
		{
			if( pRequest->mapParams.find( "del" ) != pRequest->mapParams.end( ) )
			{
				string strDelHashString = pRequest->mapParams["del"];
				string strDelHash = UTIL_StringToHash( strDelHashString );
				string strOK = pRequest->mapParams["ok"];

				if( strDelHash.empty( ) )
				{
					pResponse->strContent += "<p>Unable to delete torrent " + strDelHashString + ". The info hash is invalid. Click <a href=\"/login.html\">here</a> to return to the login page.</p>\n";
					pResponse->strContent += m_strStaticFooter; /* =X= */

					if( m_bGen )
						pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

					pResponse->strContent += "</body>\n";
					pResponse->strContent += "</html>\n";

					return;
				}
				else
				{
					if( strOK == "1" )
					{
						if( m_pTags )
						{
							CAtom *pTagInfo = m_pTags->getItem( strDelHash );

							if( pTagInfo && pTagInfo->isDicti( ) )
							{
								CAtom *pUploader = ( (CAtomDicti *)pTagInfo )->getItem( "uploader" );

								string strUploader;

								if( pUploader )
									strUploader = pUploader->toString( );

								if( strUploader != pRequest->user.strLogin )
								{
									pResponse->strContent += "<p>Unable to delete torrent " + strDelHashString + ". You didn't upload that torrent. Click <a href=\"/login.html\">here</a> to return to the login page.</p>\n";
									pResponse->strContent += m_strStaticFooter; /* =X= */

									if( m_bGen )
										pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

									pResponse->strContent += "</body>\n";
									pResponse->strContent += "</html>\n";

									return;
								}
							}
						}

						if( m_pAllowed )
						{
							// delete from disk

							CAtom *pList = m_pAllowed->getItem( strDelHash );

							if( pList && dynamic_cast<CAtomList *>( pList ) )
							{
								vector<CAtom *> vecTorrent = dynamic_cast<CAtomList *>( pList )->getValue( );

								if( vecTorrent.size( ) == 6 )
								{
									CAtom *pFileName = vecTorrent[0];

									if( pFileName )
									{
										if( m_strArchiveDir.empty( ) )
											UTIL_DeleteFile( ( m_strAllowedDir + pFileName->toString( ) ).c_str( ) );
										else
											UTIL_MoveFile( ( m_strAllowedDir + pFileName->toString( ) ).c_str( ), ( m_strArchiveDir + pFileName->toString( ) ).c_str( ) );
									}
								}
							}

							m_pAllowed->delItem( strDelHash );
							m_pDFile->delItem( strDelHash );
							deleteTag( strDelHash );

							pResponse->strContent += "<p>Deleted torrent " + strDelHashString + ". Click <a href=\"/login.html\">here</a> to return to the login page.</p>\n";
							pResponse->strContent += m_strStaticFooter; /* =X= */

							if( m_bGen )
								pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

							pResponse->strContent += "</body>\n";
							pResponse->strContent += "</html>\n";

							return;
						}
					}
					else
					{
						pResponse->strContent += "<p>Are you sure you want to delete the torrent " + strDelHashString + "?</p>\n";
						pResponse->strContent += "<p><a href=\"/login.html?del=" + strDelHashString + "&ok=1\">OK</a></p>\n";
						pResponse->strContent += m_strStaticFooter; /* =X= */

						if( m_bGen )
							pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
						
						pResponse->strContent += "</body>\n";
						pResponse->strContent += "</html>\n";

						return;
					}
				}
			}
		}

		if( m_pTags )
		{
			bool bFound = false;

			map<string, CAtom *> *pmapDicti = m_pTags->getValuePtr( );

			for( map<string, CAtom *> :: iterator i = pmapDicti->begin( ); i != pmapDicti->end( ); i++ )
			{
				if( (*i).second->isDicti( ) )
				{
					CAtom *pUploader = ( (CAtomDicti *)(*i).second )->getItem( "uploader" );

					string strUploader;

					if( pUploader )
						strUploader = pUploader->toString( );

					if( strUploader != pRequest->user.strLogin )
						continue;

					if( !bFound )
					{
						pResponse->strContent += "<p>Your Torrents</p>\n";
						pResponse->strContent += "<ul>\n";

						bFound = true;
					}

					pResponse->strContent += "<li><a href=\"/stats.html?info_hash=" + UTIL_HashToString( (*i).first ) + "\">";

					string strName = UTIL_HashToString( (*i).first );

					if( m_pAllowed )
					{
						CAtom *pTorrent = m_pAllowed->getItem( (*i).first );

						if( pTorrent && dynamic_cast<CAtomList *>( pTorrent ) )
						{
							vector<CAtom *> vecTorrent = dynamic_cast<CAtomList *>( pTorrent )->getValue( );

							if( vecTorrent.size( ) == 6 )
							{
								if( vecTorrent[1] )
									strName = vecTorrent[1]->toString( );
							}
						}
					}

					pResponse->strContent += strName + "</a>";

					if( m_bDeleteOwnTorrents )
						pResponse->strContent += " [<a href=\"/login.html?del=" + UTIL_HashToString( (*i).first ) + "\">DELETE</a>]";

					pResponse->strContent += "</li>\n";
				}
			}

			if( bFound )
				pResponse->strContent += "</ul>\n";
		}
	}

	pResponse->strContent += m_strStaticFooter; /* =X= */

	if( m_bGen )
		pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

	pResponse->strContent += "</body>\n";
	pResponse->strContent += "</html>\n";
}
