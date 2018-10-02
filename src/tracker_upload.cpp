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
#include "bencode.h"
#include "client.h"
#include "tracker.h"
#include "util.h"

void CTracker :: serverResponseUploadGET( struct request_t *pRequest, struct response_t *pResponse )
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
	pResponse->strContent += "<title>BNBT File Uploader</title>\n";

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
		pResponse->strContent += "<p class=\"login1_upload\">You are not logged in. Click <a href=\"/login.html\">here</a> to login.</p>\n";
	else
		pResponse->strContent += "<p class=\"login2_upload\">You are logged in as <span class=\"username\">" + UTIL_RemoveHTML( pRequest->user.strLogin ) + "</span>. Click <a href=\"/login.html?logout=1\">here</a> to logout.</p>\n";

	pResponse->strContent += m_strStaticHeader; /* =X= */

	pResponse->strContent += "<h3>BNBT File Uploader</h3>\n";

	if( pRequest->user.iAccess & ACCESS_UPLOAD )
	{
		if( m_strUploadDir.empty( ) )
		{
			pResponse->strContent += "<p class=\"denied\">This tracker does not allow file uploads. Click <a href=\"/index.html\">here</a> to return to the tracker.</p>\n";

			pResponse->strContent += m_strStaticFooter; /* =X= */

			if( m_bGen )
				pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

			pResponse->strContent += "</body>\n";
			pResponse->strContent += "</html>\n";

			return;
		}
		else if( m_iMaxTorrents > 0 )
		{
			if( m_pAllowed && m_pAllowed->getValuePtr( )->size( ) >= (unsigned int)m_iMaxTorrents )
			{
				pResponse->strContent += "<p class=\"denied\">This tracker has reached its torrent limit. Click <a href=\"/index.html\">here</a> to return to the tracker.</p>\n";

				pResponse->strContent += m_strStaticFooter; /* =X= */

				if( m_bGen )
					pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

				pResponse->strContent += "</body>\n";
				pResponse->strContent += "</html>\n";

				return;
			}
		}

		pResponse->strContent += "<form method=\"post\" action=\"/upload.html\" enctype=\"multipart/form-data\">\n";
		pResponse->strContent += "<input name=\"torrent\" type=file size=24> Torrent<br><br>\n";
		pResponse->strContent += "<input name=\"name\" type=text size=64 maxlength=" + CAtomInt( MAX_FILENAME_LEN ).toString( ) + "> Name (optional - if blank, the name will be taken from the filename)<br><br>\n";

		if( m_bAllowInfoLink )
			pResponse->strContent += "<input name=\"infolink\" type=text size=64 maxlength=" + CAtomInt( MAX_INFO_LINK_LEN ).toString( ) + "> Info Link (optional)<br><br>\n";

		if( !m_vecTags.empty( ) )
			pResponse->strContent += "<select name=\"tag\">\n";

		for( vector< pair<string, string> > :: iterator i = m_vecTags.begin( ); i != m_vecTags.end( ); i++ )
			pResponse->strContent += "<option>" + (*i).first + "\n";

		if( !m_vecTags.empty( ) )
			pResponse->strContent += "</select> Tag\n";

		pResponse->strContent += "<ul>\n";
		pResponse->strContent += "<li>Names must be less than " + CAtomInt( MAX_FILENAME_LEN ).toString( ) + " characters long</li>\n";
		pResponse->strContent += "<li>No HTML</li>\n";
		pResponse->strContent += "<li><strong>Max. File Size:</strong> " + UTIL_BytesToString( giMaxRecvSize ) + "</li>\n";

		if( !m_strForceAnnounceURL.empty( ) )
			pResponse->strContent += "<li><strong>Auto Announce URL:</strong> " + UTIL_RemoveHTML( m_strForceAnnounceURL ) + "</li>\n";

		pResponse->strContent += "</ul>\n";
		pResponse->strContent += "<input type=submit value=\"Upload\">\n";
		pResponse->strContent += "</form>\n";
	}
	else
		pResponse->strContent += "<p class=\"denied\">You are not authorized to view this page.</p>\n";

	pResponse->strContent += m_strStaticFooter; /* =X= */

	if( m_bGen )
		pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

	pResponse->strContent += "</body>\n";
	pResponse->strContent += "</html>\n";
}

void CTracker :: serverResponseUploadPOST( struct request_t *pRequest, struct response_t *pResponse, CAtomList *pPost )
{
	struct bnbttv btv = UTIL_CurrentTime( );

	if( m_strUploadDir.empty( ) || !( pRequest->user.iAccess & ACCESS_UPLOAD ) )
	{
		pResponse->strCode = "403 Forbidden";

		return;
	}

	if( m_iMaxTorrents > 0 )
	{
		if( m_pAllowed && m_pAllowed->getValuePtr( )->size( ) >= (unsigned int)m_iMaxTorrents )
		{
			pResponse->strCode = "403 Forbidden";

			return;
		}
	}

	string strFile;
	string strTorrent;
	string strTag;
	string strPostedName;
	string strPostedInfoLink;

	if( pPost )
	{
		vector<CAtom *> vecSegs = pPost->getValue( );

		for( unsigned long i = 0; i < vecSegs.size( ); i++ )
		{
			if( vecSegs[i]->isDicti( ) )
			{
				CAtomDicti *pSeg = (CAtomDicti *)vecSegs[i];

				CAtom *pDisp = pSeg->getItem( "disposition" );
				CAtom *pDat = pSeg->getItem( "data" );

				if( pDisp && pDisp->isDicti( ) && pDat )
				{
					CAtom *pName = ( (CAtomDicti *)pDisp )->getItem( "name" );

					if( pName )
					{
						string strName = pName->toString( );

						if( strName == "torrent" )
						{
							CAtom *pFile = ( (CAtomDicti *)pDisp )->getItem( "filename" );

							if( pFile )
							{
								// the path is local to the peer

								strFile = UTIL_StripPath( pFile->toString( ) );

								strTorrent = pDat->toString( );
							}
							else
							{
								pResponse->strCode = "400 Bad Request";

								return;
							}
						}
						else if( strName == "tag" )
							strTag = pDat->toString( );
						else if( strName == "name" )
							strPostedName = pDat->toString( ).substr( 0, MAX_FILENAME_LEN );
						else if( strName == "infolink" )
							strPostedInfoLink = pDat->toString( ).substr( 0, MAX_INFO_LINK_LEN );
					}
					else
					{
						pResponse->strCode = "400 Bad Request";

						return;
					}
				}
			}
		}
	}
	else
	{
		pResponse->strCode = "400 Bad Request";

		return;
	}

	time_t tNow = time( NULL ) + m_iRefreshFastCacheInterval;
	char *szTime = asctime( gmtime( &tNow ) );
	szTime[strlen( szTime ) - 1] = '\0';

	pResponse->strCode = "200 OK";

	pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", string( gmapMime[".html"] + "; charset=" ) + gstrCharSet ) ); /* =X= */
	pResponse->mapHeaders.insert( pair<string, string>( "Expires", string( szTime ) + " GMT" ) );

	pResponse->strContent += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n"; /* =X= */
	pResponse->strContent += "<html lang=\"en\">\n";
	pResponse->strContent += "<head>\n";
	pResponse->strContent += "<title>BNBT File Uploader</title>\n";

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
		pResponse->strContent += "<p class=\"login1_upload\">You are not logged in. Click <a href=\"/login.html\">here</a> to login.</p>\n";
	else
		pResponse->strContent += "<p class=\"login2_upload\">You are logged in as <span class=\"username\">" + UTIL_RemoveHTML( pRequest->user.strLogin ) + "</span>. Click <a href=\"/login.html?logout=1\">here</a> to logout.</p>\n";

	pResponse->strContent += m_strStaticHeader; /* =X= */

	string strPath = m_strUploadDir + strFile;

	string :: size_type iExt = strFile.rfind( "." );

	string strExt;

	if( iExt != string :: npos )
		strExt = strFile.substr( iExt );

	if( strTorrent.empty( ) )
	{
		pResponse->strContent += "<h3>Upload Failed</h3>\n";
		pResponse->strContent += "<p>The uploaded file is corrupt or invalid. Click <a href=\"/upload.html\">here</a> to return to the upload page.</p>\n";
	}
	else if( strExt != ".torrent" )
	{
		pResponse->strContent += "<h3>Upload Failed</h3>\n";
		pResponse->strContent += "<p>The uploaded file is not a .torrent file. Click <a href=\"/upload.html\">here</a> to return to the upload page.</p>\n";
	}
	else if( !checkTag( strTag ) )
	{
		pResponse->strContent += "<h3>Upload Failed</h3>\n";
		pResponse->strContent += "<p>The file tag is invalid. Click <a href=\"/upload.html\">here</a> to return to the upload page.</p>\n";
	}
	else if( UTIL_CheckFile( strPath.c_str( ) ) )
	{
		pResponse->strContent += "<h3>Upload Failed</h3>\n";
		pResponse->strContent += "<p>The uploaded file already exists. Click <a href=\"/upload.html\">here</a> to return to the upload page.</p>\n";
	}
	else
	{
		CAtom *pTorrent = Decode( strTorrent );

		if( pTorrent && pTorrent->isDicti( ) )
		{
			string strInfoHash = UTIL_InfoHash( pTorrent );

			if( !strInfoHash.empty( ) )
			{
				if( m_pDFile->getItem( strInfoHash ) )
				{
					pResponse->strContent += "<h3>Upload Failed</h3>\n";
					pResponse->strContent += "<p>A file with the uploaded file's info hash already exists. Click <a href=\"/upload.html\">here</a> to return to the upload page.</p>\n";
				}
				else
				{
					if( !m_strForceAnnounceURL.empty( ) )
						( (CAtomDicti *)pTorrent )->setItem( "announce", new CAtomString( m_strForceAnnounceURL ) );

					UTIL_MakeFile( strPath.c_str( ), Encode( pTorrent ) );

					addTag( strInfoHash, strTag, strPostedName, pRequest->user.strLogin, strPostedInfoLink );

					pResponse->strContent += "<h3>Upload Successful</h3>\n";

					if( m_bParseOnUpload )
					{
						if( m_pAllowed )
							parseTorrent( strPath.c_str( ) );

						pResponse->strContent += "<p>The uploaded file is ready. You should start seeding it now. Click <a href=\"/index.html\">here</a> to return to the tracker.</p>\n";
					}
					else
					{
						pResponse->strContent += "<p>The uploaded file will be ready in " + CAtomInt( m_iParseAllowedInterval ).toString( );

						if( m_iParseAllowedInterval == 1 )
							pResponse->strContent += " minute. You should start seeding it as soon as possible. Click <a href=\"/index.html\">here</a> to return to the tracker.</p>\n";
						else
							pResponse->strContent += " minutes. You should start seeding it as soon as possible. Click <a href=\"/index.html\">here</a> to return to the tracker.</p>\n";
					}

					UTIL_LogPrint( "%s uploaded %s\n", inet_ntoa( pRequest->sin.sin_addr ), strFile.c_str( ) );
				}
			}
			else
			{
				pResponse->strContent += "<h3>Upload Failed</h3>\n";
				pResponse->strContent += "<p>The uploaded file is corrupt or invalid. Click <a href=\"/upload.html\">here</a> to return to the upload page.</p>\n";
			}
		}
		else
		{
			pResponse->strContent += "<h3>Upload Failed</h3>\n";
			pResponse->strContent += "<p>The uploaded file is corrupt or invalid. Click <a href=\"/upload.html\">here</a> to return to the upload page.</p>\n";
		}

		if( pTorrent )
			delete pTorrent;
	}

	pResponse->strContent += m_strStaticFooter; /* =X= */

	if( m_bGen )
		pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

	pResponse->strContent += "</body>\n";
	pResponse->strContent += "</html>\n";
}
