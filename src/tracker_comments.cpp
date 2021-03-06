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

void CTracker :: serverResponseComments( struct request_t *pRequest, struct response_t *pResponse )
{
	struct bnbttv btv = UTIL_CurrentTime( );

	if( !m_pAllowed || !m_bAllowComments )
	{
		pResponse->strCode = "403 Forbidden";

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
	pResponse->strContent += "<title>BNBT Comments</title>\n";

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

	// assorted scripts (thanks SA)

	pResponse->strContent += "<script language=\"javascript\">\n";
	pResponse->strContent += "function validate( theform ) {\n";
	pResponse->strContent += "if( theform.comment.value == \"\" ) {\n";
	pResponse->strContent += "	alert( \"You must fill in all the fields.\" );\n";
	pResponse->strContent += "	return false; }\n";
	pResponse->strContent += "if( theform.comment.value.length > " + CAtomInt( m_iCommentLength ).toString( ) + " ) {\n";
	pResponse->strContent += "	alert( \"Your message is too long.\\nReduce your message to " + CAtomInt( m_iCommentLength ).toString( ) + " characters.\\nIt is currently \" + theform.comment.value.length + \" characters long.\" );\n";
	pResponse->strContent += "	return false; }\n";
	pResponse->strContent += "else { return true; }\n";
	pResponse->strContent += "}\n";
	pResponse->strContent += "function checklength( theform ) {\n";
	pResponse->strContent += "alert( \"Your message is \" + theform.comment.value.length + \" characters long.\" );\n";
	pResponse->strContent += "}\n";
	pResponse->strContent += "</script>\n";

	if( pRequest->user.strLogin.empty( ) )
		pResponse->strContent += "<p class=\"login1_comments\">You are not logged in. Click <a href=\"/login.html\">here</a> to login.</p>\n";
	else
		pResponse->strContent += "<p class=\"login2_comments\">You are logged in as <span class=\"username\">" + UTIL_RemoveHTML( pRequest->user.strLogin ) + "</span>. Click <a href=\"/login.html?logout=1\">here</a> to logout.</p>\n";

	pResponse->strContent += "<h3>BNBT Comments</h3>\n";

	if( pRequest->user.iAccess & ACCESS_VIEW )
	{
		string strHashString = pRequest->mapParams["info_hash"];
		string strHash = UTIL_StringToHash( strHashString );

		if( !strHash.empty( ) )
		{
			//
			// delete comment
			//

			if( pRequest->user.iAccess & ACCESS_EDIT )
			{
				string strDelAll = pRequest->mapParams["delall"];
				string strDel = pRequest->mapParams["del"];

				if( strDelAll == "1" )
				{
					m_pComments->delItem( strHash );

					saveComments( );

					pResponse->strContent += "<p>Deleted all comments. Click <a href=\"/comments.html?info_hash=" + strHashString + "\">here</a> to return to the comments page.</p>\n";

					if( m_bGen )
						pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

					pResponse->strContent += "</body>\n";
					pResponse->strContent += "</html>\n";

					return;
				}
				else if( !strDel.empty( ) )
				{
					unsigned int iDel = (unsigned int)( atoi( strDel.c_str( ) ) - 1 );

					CAtom *pComments = m_pComments->getItem( strHash );

					if( pComments && dynamic_cast<CAtomList *>( pComments ) )
					{
						vector<CAtom *> vecComments = dynamic_cast<CAtomList *>( pComments )->getValue( );

						if( iDel < vecComments.size( ) )
						{
							dynamic_cast<CAtomList *>( pComments )->delItem( vecComments[iDel] );

							saveComments( );

							pResponse->strContent += "<p>Deleted comment " + strDel + ". Click <a href=\"/comments.html?info_hash=" + strHashString + "\">here</a> to return to the comments page.</p>\n";

							if( m_bGen )
								pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

							pResponse->strContent += "</body>\n";
							pResponse->strContent += "</html>\n";

							return;
						}
						else
						{
							pResponse->strContent += "<p>Unable to delete comment " + strDel + ". The comment number is invalid. Click <a href=\"/comments.html?info_hash=" + strHashString + "\">here</a> to return to the comments page.</p>\n";

							if( m_bGen )
								pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

							pResponse->strContent += "</body>\n";
							pResponse->strContent += "</html>\n";

							return;
						}
					}
				}
			}

			// display torrent information list

			if( m_pAllowed )
			{
				CAtom *pTorrent = m_pAllowed->getItem( strHash );

				if( pTorrent && dynamic_cast<CAtomList *>( pTorrent ) )
				{
					vector<CAtom *> vecTorrent = dynamic_cast<CAtomList *>( pTorrent )->getValue( );

					if( vecTorrent.size( ) == 6 )
					{
						CAtom *pName = vecTorrent[1];
						CAtom *pAdded = vecTorrent[2];
						CAtom *pSize = vecTorrent[3];
						CAtom *pFiles = vecTorrent[4];
						CAtom *pComment = vecTorrent[5];

						pResponse->strContent += "<p>File Information</p>\n";
						pResponse->strContent += "<ul>\n";

						if( pName )
							pResponse->strContent += "<li><strong>Name:</strong> " + UTIL_RemoveHTML( pName->toString( ) ) + "</li>\n";

						pResponse->strContent += "<li><strong>Info Hash:</strong> " + strHashString + "</li>\n";

						if( pAdded )
							pResponse->strContent += "<li><strong>Added:</strong> " + pAdded->toString( ) + "</li>\n";

						if( pSize && dynamic_cast<CAtomLong *>( pSize ) )
							pResponse->strContent += "<li><strong>Size:</strong> " + UTIL_BytesToString( dynamic_cast<CAtomLong *>( pSize )->getValue( ) ) + "</li>\n";

						if( pFiles && dynamic_cast<CAtomInt *>( pFiles ) )
							pResponse->strContent += "<li><strong>Files:</strong> " + pFiles->toString( ) + "</li>\n";

						pResponse->strContent += "</ul>\n";

						if( pComment )
						{
							if( m_bShowFileComment )
							{
								pResponse->strContent += "<p>File Comment</p>\n";
								pResponse->strContent += "<table summary=\"file comment\">\n";
								pResponse->strContent += "<tr><td class=\"com_body\"><code>" + UTIL_RemoveHTML( pComment->toString( ) ) + "</code></td></tr>\n";
								pResponse->strContent += "</table>\n";
							}
						}
					}
				}
			}

			if( !m_pComments->getItem( strHash ) )
				m_pComments->setItem( strHash, new CAtomList( ) );

			CAtom *pComments = m_pComments->getItem( strHash );

			if( pComments && dynamic_cast<CAtomList *>( pComments ) )
			{
				if( pRequest->user.iAccess & ACCESS_COMMENTS )
				{
					if( pRequest->mapParams.find( "comment" ) != pRequest->mapParams.end( ) )
					{
						string strComment = pRequest->mapParams["comment"].substr( 0, m_iCommentLength );

						if( strComment.empty( ) )
						{
							pResponse->strContent += "<p>You must fill in all the fields. Click <a href=\"/comments.html?info_hash=" + strHashString + "\">here</a> to return to the comments page.</p>\n";

							if( m_bGen )
								pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

							pResponse->strContent += "</body>\n";
							pResponse->strContent += "</html>\n";

							return;
						}

						CAtomDicti *pNew = new CAtomDicti( );

						pNew->setItem( "ip", new CAtomString( inet_ntoa( pRequest->sin.sin_addr ) ) );

						if( !pRequest->user.strLogin.empty( ) )
							pNew->setItem( "name", new CAtomString( pRequest->user.strLogin ) );

						pNew->setItem( "comment", new CAtomString( strComment ) );

						time_t tNowLocal = time( NULL );
						char *szTimeLocal = asctime( localtime( &tNowLocal ) );
						szTimeLocal[strlen( szTimeLocal ) - 1] = '\0';
						pNew->setItem( "time", new CAtomString( szTimeLocal ) );

						dynamic_cast<CAtomList *>( pComments )->addItem( pNew );

						saveComments( );

						pResponse->strContent += "<p>Your comment has been posted. DO NOT REFRESH THIS PAGE OR YOUR COMMENT WILL BE POSTED TWICE. Click <a href=\"/comments.html?info_hash=" + strHashString + "\">here</a> to return to the comments page.</p>\n";

						if( m_bGen )
							pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

						pResponse->strContent += "</body>\n";
						pResponse->strContent += "</html>\n";

						return;
					}
				}

				vector<CAtom *> *pvecList = dynamic_cast<CAtomList *>( pComments )->getValuePtr( );

				bool bFound = false;

				unsigned long i = 0;

				for( vector<CAtom *> :: iterator it = pvecList->begin( ); it != pvecList->end( ); it++ )
				{
					if( (*it)->isDicti( ) )
					{
						CAtomDicti *pCommentDicti = (CAtomDicti *)(*it);

						CAtom *pIP = pCommentDicti->getItem( "ip" );
						CAtom *pName = pCommentDicti->getItem( "name" );
						CAtom *pComText = pCommentDicti->getItem( "comment" );
						CAtom *pTime = pCommentDicti->getItem( "time" );

						if( pIP && pComText && pTime )
						{
							if( !bFound )
							{
								pResponse->strContent += "<p>Comments";

								if( pRequest->user.iAccess & ACCESS_EDIT )
									pResponse->strContent += " [<a href=\"/comments.html?info_hash=" + strHashString + "&delall=1\">Delete All</a>]";

								pResponse->strContent += "</p>\n";
								pResponse->strContent += "<table summary=\"comments\">\n";

								bFound = true;
							}

							string strIP = pIP->toString( );
							string strName;

							if( pName )
								strName = pName->toString( );

							string strComText = pComText->toString( );
							string strTime = pTime->toString( );

							if( strName.empty( ) )
							{
								// strip ip

								string :: size_type iStart = strIP.rfind( "." );

								if( iStart != string :: npos )
								{
									// don't strip ip for mods

									if( !( pRequest->user.iAccess & ACCESS_EDIT ) )
										strIP = strIP.substr( 0, iStart + 1 ) + "xxx";
								}
							}
							else
							{
								if( !( pRequest->user.iAccess & ACCESS_EDIT ) )
									strIP = "HIDDEN";
							}

							//
							// header
							//

							pResponse->strContent += "<tr class=\"com_header\"><td class=\"com_header\"><code>Comment " + CAtomInt( (int) i + 1 ).toString( ) + " posted by ";

							if( !strName.empty( ) )
								pResponse->strContent += "<strong>" + UTIL_RemoveHTML( strName ) + "</strong> (";

							pResponse->strContent += strIP;

							if( !strName.empty( ) )
								pResponse->strContent += ")";

							pResponse->strContent += " on " + strTime;

							if( pRequest->user.iAccess & ACCESS_EDIT )
								pResponse->strContent += " [<a href=\"/comments.html?info_hash=" + strHashString + "&del=" + CAtomInt( (int) i + 1 ).toString( ) + "\">Delete</a>]";

							pResponse->strContent += "</code></td></tr>\n";

							//
							// body
							//

							pResponse->strContent += "<tr class=\"com_body\"><td class=\"com_body\"><code>" + UTIL_RemoveHTML( strComText ) + "</code></td></tr>\n";
						}
					}

					i++;
				}

				if( bFound )
					pResponse->strContent += "</table>\n";
				else
					pResponse->strContent += "<p>No Comments Posted</p>\n";
			}

			if( pRequest->user.iAccess & ACCESS_COMMENTS )
			{
				pResponse->strContent += "<p>Post A Comment</p>\n";
				pResponse->strContent += "<form method=\"get\" action=\"/comments.html\" name=\"form\" onSubmit=\"return validate( this )\">\n";
				pResponse->strContent += "<ul>\n";
				pResponse->strContent += "<li>Comments must be less than " + CAtomInt( m_iCommentLength ).toString( ) + " characters long</li>\n";
				pResponse->strContent += "<li>No HTML</li>\n";
				pResponse->strContent += "</ul>\n";
				pResponse->strContent += "<input name=\"info_hash\" type=hidden value=\"" + strHashString + "\">\n";
				pResponse->strContent += "<textarea name=\"comment\" rows=8 cols=64></textarea><br><br>\n";
				pResponse->strContent += "<a href=\"javascript:checklength( document.form );\">[check message length]</a><br><br>\n";
				pResponse->strContent += "<input type=submit value=\"Submit\">\n";
				pResponse->strContent += "</form>\n";
			}
			else
				pResponse->strContent += "<p class=\"denied\">You are not authorized to post comments.</p>\n";
		}
	}
	else
		pResponse->strContent += "<p class=\"denied\">You are not authorized to view this page.</p>\n";

	if( m_bGen )
		pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

	pResponse->strContent += "</body>\n";
	pResponse->strContent += "</html>\n";
}
