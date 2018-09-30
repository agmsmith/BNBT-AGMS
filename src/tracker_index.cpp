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
#include "sort.h"
#include "tracker.h"
#include "util.h"

void CTracker :: serverResponseIndex( struct request_t *pRequest, struct response_t *pResponse )
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

	// addition by labarks

	if( !m_strDumpRSSFile.empty( ) )
	{
		if( m_iDumpRSSFileMode == 0 || m_iDumpRSSFileMode == 2 )
			pResponse->strContent += "<link rel=\"alternate\" type=\"" + gmapMime[".rss"] + "\" title=\"RSS\" href=\"" + m_strDumpRSSFileDir + m_strDumpRSSFile + "\">\n"; /* =X= */
	}

	// end addition

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

	/* =X= */
	// For internalised mouseover feature
	if( m_bUseMouseovers && ( pRequest->user.iAccess & ACCESS_VIEW ) )
	{
		int iImage = 1;

		pResponse->strContent += "<script type=\"text/javascript\">\n\n";
		pResponse->strContent += "<!--\n";

		if( !m_vecTags.empty( ) && !m_vecTagsMouse.empty( ) )
		{
			vector< pair< string, string > > :: iterator j = m_vecTagsMouse.begin( );

			int iTag = 1;

			for( vector< pair< string, string > > :: iterator i = m_vecTags.begin( ); i != m_vecTags.end( ); i++ )
			{
				if( (*i).second.empty( ) || (*j).second.empty( ) )
				{
					iTag++;
					j++;

					continue;
				}

				pResponse->strContent += "imagetag" + CAtomInt( iImage ).toString( ) + " = new Image();\n";
				pResponse->strContent += "imagetag" + CAtomInt( iImage ).toString( ) + ".src = \"" + (*j).second + "\";\n";
				pResponse->strContent += "function hoverOffTag" + CAtomInt( iTag ).toString( ) + "() {\n";
				pResponse->strContent += "	document.bnbt_tag" + CAtomInt( iTag ).toString( ) + ".src = \"" + (*j).second + "\";\n";
				pResponse->strContent += "}\n\n";

				iImage++;

				pResponse->strContent += "imagetag" + CAtomInt( iImage ).toString( ) + " = new Image();\n";
				pResponse->strContent += "imagetag" + CAtomInt( iImage ).toString( ) + ".src = \"" + (*i).second + "\";\n";
				pResponse->strContent += "function hoverOnTag" + CAtomInt( iTag ).toString( ) + "() {\n";
				pResponse->strContent += "	document.bnbt_tag" + CAtomInt( iTag ).toString( ) + ".src = \"" + (*i).second + "\";\n";
				pResponse->strContent += "}\n\n";

				iImage++;

				iTag++;
				j++;
			}
		}

		pResponse->strContent += "//-->\n";
		pResponse->strContent += "</script>\n\n";
	}

	if( pRequest->user.strLogin.empty( ) )
		pResponse->strContent += "<p class=\"login1_index\">You are not logged in. Click <a href=\"/login.html\">here</a> to login.</p>\n";
	else
		pResponse->strContent += "<p class=\"login2_index\">You are logged in as <span class=\"username\">" + UTIL_RemoveHTML( pRequest->user.strLogin ) + "</span>. Click <a href=\"/login.html?logout=1\">here</a> to logout.</p>\n";

	if( pRequest->user.iAccess & ACCESS_VIEW )
	{
		//
		// delete torrent
		//

		if( pRequest->user.iAccess & ACCESS_EDIT )
		{
			if( pRequest->mapParams.find( "del" ) != pRequest->mapParams.end( ) )
			{
				string strDelHashString = pRequest->mapParams["del"];
				string strDelHash = UTIL_StringToHash( strDelHashString );
				string strOK = pRequest->mapParams["ok"];

				if( strDelHash.empty( ) )
				{
					pResponse->strContent += "<p>Unable to delete torrent " + strDelHashString + ". The info hash is invalid. Click <a href=\"/index.html\">here</a> to return to the tracker.</p>\n";
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

							pResponse->strContent += "<p>Deleted torrent " + strDelHashString + ". Click <a href=\"/index.html\">here</a> to return to the tracker.</p>\n";
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
						pResponse->strContent += "<p><a href=\"/index.html?del=" + strDelHashString + "&amp;ok=1\">OK</a></p>\n";
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

		pResponse->strContent += m_strStaticHeader;

		if( m_pDFile )
		{
			if( m_pDFile->isEmpty( ) )
			{
				pResponse->strContent += "<p>Not tracking any files yet!</p>\n";
				pResponse->strContent += m_strStaticFooter; /* =X= */

				if( m_bGen )
					pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

				pResponse->strContent += "</body>\n";
				pResponse->strContent += "</html>\n";

				return;
			}

			map<string, CAtom *> *pmapDicti = m_pDFile->getValuePtr( );

			unsigned long iKeySize = (unsigned long)pmapDicti->size( );

			// add the torrents into this structure one by one and sort it afterwards

			struct torrent_t *pTorrents = new struct torrent_t[iKeySize];

			unsigned long torrent_iter = 0;

			for( map<string, CAtom *> :: iterator it = pmapDicti->begin( ); it != pmapDicti->end( ); it++ )
			{
				pTorrents[torrent_iter].strInfoHash = (*it).first;
				pTorrents[torrent_iter].strName = "unknown";
				pTorrents[torrent_iter].strLowerName = "unknown";
				pTorrents[torrent_iter].iSeeders = 0;
				pTorrents[torrent_iter].iLeechers = 0;
				pTorrents[torrent_iter].iCompleted = 0;
				pTorrents[torrent_iter].iTransferred = 0;
				pTorrents[torrent_iter].iSize = 0;
				pTorrents[torrent_iter].iFiles = 0;
				pTorrents[torrent_iter].iComments = 0;
				pTorrents[torrent_iter].iAverageLeft = 0;
				pTorrents[torrent_iter].iAverageLeftPercent = 0;
				pTorrents[torrent_iter].iMinLeft = 0;
				pTorrents[torrent_iter].iMaxiLeft = 0;

				if( (*it).second->isDicti( ) )
				{
					// grab data from m_pAllowed

					if( m_pAllowed )
					{
						CAtom *pList = m_pAllowed->getItem( pTorrents[torrent_iter].strInfoHash );

						if( pList && dynamic_cast<CAtomList *>( pList ) )
						{
							vector<CAtom *> vecTorrent = dynamic_cast<CAtomList *>( pList )->getValue( );

							if( vecTorrent.size( ) == 6 )
							{
								CAtom *pFileName = vecTorrent[0];
								CAtom *pName = vecTorrent[1];
								CAtom *pAdded = vecTorrent[2];
								CAtom *pSize = vecTorrent[3];
								CAtom *pFiles = vecTorrent[4];

								if( pFileName )
									pTorrents[torrent_iter].strFileName = pFileName->toString( );

								if( pName )
								{
									// stick a lower case version in strNameLower for non case sensitive searching and sorting

									pTorrents[torrent_iter].strName = pName->toString( );
									pTorrents[torrent_iter].strLowerName = UTIL_ToLower( pTorrents[torrent_iter].strName );
								}

								if( pAdded )
									pTorrents[torrent_iter].strAdded = pAdded->toString( );

								if( pSize && dynamic_cast<CAtomLong *>( pSize ) )
									pTorrents[torrent_iter].iSize = dynamic_cast<CAtomLong *>( pSize )->getValue( );

								if( pFiles && dynamic_cast<CAtomInt *>( pFiles ) )
									pTorrents[torrent_iter].iFiles = (unsigned int)dynamic_cast<CAtomInt *>( pFiles )->getValue( );
							}
						}

						if( m_bAllowComments )
						{
							if( m_pComments )
							{
								CAtom *pCommentList = m_pComments->getItem( pTorrents[torrent_iter].strInfoHash );

								if( pCommentList && dynamic_cast<CAtomList *>( pCommentList ) )
									pTorrents[torrent_iter].iComments = (unsigned int)dynamic_cast<CAtomList *>( pCommentList )->getValuePtr( )->size( );
							}
						}
					}

					// grab data from m_pTags

					if( m_pTags )
					{
						CAtom *pDicti = m_pTags->getItem( pTorrents[torrent_iter].strInfoHash );

						if( pDicti && pDicti->isDicti( ) )
						{
							CAtom *pTag = ( (CAtomDicti *)pDicti )->getItem( "tag" );
							CAtom *pName = ( (CAtomDicti *)pDicti )->getItem( "name" );
							CAtom *pUploader = ( (CAtomDicti *)pDicti )->getItem( "uploader" );
							CAtom *pInfoLink = ( (CAtomDicti *)pDicti )->getItem( "infolink" );

							if( pTag )
								pTorrents[torrent_iter].strTag = pTag->toString( );

							if( pName )
							{
								// this will overwrite the previous name, i.e. the filename

								pTorrents[torrent_iter].strName = pName->toString( );
								pTorrents[torrent_iter].strLowerName = UTIL_ToLower( pTorrents[torrent_iter].strName );
							}

							if( pUploader )
								pTorrents[torrent_iter].strUploader = pUploader->toString( );

							if( pInfoLink )
								pTorrents[torrent_iter].strInfoLink = pInfoLink->toString( );
						}
					}

					// grab data from m_pFastCache

					if( GetTime( ) > m_iRefreshFastCacheNext )
					{
						RefreshFastCache( );

						m_iRefreshFastCacheNext = GetTime( ) + m_iRefreshFastCacheInterval;
					}

					CAtom *pFC = m_pFastCache->getItem( (*it).first );

					if( pFC && dynamic_cast<CAtomList *>( pFC ) )
					{
						vector<CAtom *> vecList = dynamic_cast<CAtomList *>( pFC )->getValue( );

						pTorrents[torrent_iter].iSeeders = dynamic_cast<CAtomInt *>( vecList[0] )->getValue( );
						pTorrents[torrent_iter].iLeechers = dynamic_cast<CAtomInt *>( vecList[1] )->getValue( );
						pTorrents[torrent_iter].iCompleted = dynamic_cast<CAtomInt *>( vecList[2] )->getValue( );

						if( pTorrents[torrent_iter].iLeechers > 0 )
							pTorrents[torrent_iter].iAverageLeft = dynamic_cast<CAtomLong *>( vecList[3] )->getValue( ) / pTorrents[torrent_iter].iLeechers;

						pTorrents[torrent_iter].iMinLeft = dynamic_cast<CAtomLong *>( vecList[4] )->getValue( );
						pTorrents[torrent_iter].iMaxiLeft = dynamic_cast<CAtomLong *>( vecList[5] )->getValue( );
					}

					// misc calculations

					if( m_pAllowed && m_bShowTransferred )
						pTorrents[torrent_iter].iTransferred = pTorrents[torrent_iter].iCompleted * pTorrents[torrent_iter].iSize;

					if( pTorrents[torrent_iter].iSize > 0 )
						pTorrents[torrent_iter].iAverageLeftPercent = (unsigned int)( ( (float)pTorrents[torrent_iter].iAverageLeft / pTorrents[torrent_iter].iSize ) * 100 );
				}

				torrent_iter++;
			}

			string strSort = pRequest->mapParams["sort"];

			if( m_bSort )
			{
				if( !strSort.empty( ) )
				{
					int iSort = atoi( strSort.c_str( ) );

					if( iSort == SORT_ANAME )
					{
						if( m_pAllowed && m_bShowNames )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortByName );
					}
					else if( iSort == SORT_ACOMPLETE )
						qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortByComplete );
					else if( iSort == SORT_AINCOMPLETE )
						qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortByDL );
					else if( iSort == SORT_AADDED )
					{
						if( m_pAllowed && m_bShowAdded )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortByAdded );
					}
					else if( iSort == SORT_ASIZE )
					{
						if( m_pAllowed && m_bShowSize )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortBySize );
					}
					else if( iSort == SORT_AFILES )
					{
						if( m_pAllowed && m_bShowNumFiles )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortByFiles );
					}
					else if( iSort == SORT_ACOMMENTS )
					{
						if( m_pAllowed && m_bAllowComments )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortByComments );
					}
					else if( iSort == SORT_AAVGLEFT )
					{
						if( m_bShowAverageLeft )
						{
							if( m_pAllowed )
							{
								if( m_bShowLeftAsProgress )
									qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByAvgLeftPercent );
								else
									qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortByAvgLeftPercent );
							}
							else
								qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortByAvgLeft );
						}
					}
					else if( iSort == SORT_ACOMPLETED )
					{
						if( m_bShowCompleted )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortByCompleted );
					}
					else if( iSort == SORT_ATRANSFERRED )
					{
						if( m_pAllowed && m_bShowTransferred )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortByTransferred );
					}
					else if( iSort == SORT_ATAG ) /* =X= */
					{
						if( !m_vecTags.empty( ) )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortByTag );
					}
					else if( iSort == SORT_AUPLOADER ) /* =X= */
					{
						if( m_bShowUploader )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortByUploader );
					}
					else if( iSort == SORT_DNAME )
					{
						if( m_pAllowed && m_bShowNames )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByName );
					}
					else if( iSort == SORT_DCOMPLETE )
						qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByComplete );
					else if( iSort == SORT_DINCOMPLETE )
						qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByDL );
					else if( iSort == SORT_DADDED )
					{
						if( m_pAllowed && m_bShowAdded )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByAdded );
					}
					else if( iSort == SORT_DSIZE )
					{
						if( m_pAllowed && m_bShowSize )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortBySize );
					}
					else if( iSort == SORT_DFILES )
					{
						if( m_pAllowed && m_bShowNumFiles )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByFiles );
					}
					else if( iSort == SORT_DCOMMENTS )
					{
						if( m_pAllowed && m_bAllowComments )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByComments );
					}
					else if( iSort == SORT_DAVGLEFT )
					{
						if( m_bShowAverageLeft )
						{
							if( m_pAllowed )
							{
								if( m_bShowLeftAsProgress )
									qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), asortByAvgLeftPercent );
								else
									qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByAvgLeftPercent );
							}
							else
								qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByAvgLeft );
						}
					}
					else if( iSort == SORT_DCOMPLETED )
					{
						if( m_bShowCompleted )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByCompleted );
					}
					else if( iSort == SORT_DTRANSFERRED )
					{
						if( m_pAllowed && m_bShowTransferred )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByTransferred );
					}
					else if( iSort == SORT_DTAG ) /* =X= */
					{
						if( !m_vecTags.empty( ) )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByTag );
					}
					else if( iSort == SORT_DUPLOADER ) /* =X= */
					{
						if( m_bShowUploader )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByUploader );
					}
					else
					{
						// default action is to sort by added if we can

						if( m_pAllowed && m_bShowAdded )
							qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByAdded );
					}
				}
				else
				{
					// default action is to sort by added if we can

					if( m_pAllowed && m_bShowAdded )
						qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByAdded );
				}
			}
			else
			{
				// sort is disabled, but default action is to sort by added if we can

				if( m_pAllowed && m_bShowAdded )
					qsort( pTorrents, iKeySize, sizeof( struct torrent_t ), dsortByAdded );
			}

			// some preliminary search crap

			string strSearch = pRequest->mapParams["search"];
			string strLowerSearch = UTIL_ToLower( strSearch );
			string strSearchResp = UTIL_StringToEscaped( strSearch );

			if( !strSearch.empty( ) && m_pAllowed && m_bShowNames && m_bSearch )
				pResponse->strContent += "<p class=\"search_results\">Search results for \"" + UTIL_RemoveHTML( strSearch ) + "\".</p>\n";

			// filters

			string strFilter = pRequest->mapParams["filter"];

			vector< pair< string, string > > :: iterator j = m_vecTagsMouse.begin( ); /* =X= */

			int iTag = 1; /* =X= */

			if( !m_vecTags.empty( ) )
			{
				pResponse->strContent += "<p style=\"text-align:center\">";

				bool bFound = false;

				for( vector< pair<string, string> > :: iterator i = m_vecTags.begin( ); i != m_vecTags.end( ); i++ )
				{
					if( !bFound )
						pResponse->strContent += "<a href=\"/index.html\">Clear Filter</a><br><br>\n";

					pResponse->strContent += "<a title=\"" + (*i).first + "\" href=\"/index.html?filter=" + UTIL_StringToEscaped( (*i).first );

					if( !strSort.empty( ) )
						pResponse->strContent += "&amp;amp;sort=" + strSort;

					if( !strSearch.empty( ) )
						pResponse->strContent += "&amp;amp;search=" + strSearchResp;

					/* =X= */
					// Assigns functions to onMouseOver and onMouseOut for each Tag Image
					// Activated by setting "bnbt_use_mouseovers" to 1
					// Generates code that validates with HTML 4.01 Strict
					pResponse->strContent += "\"";

					if( m_bUseMouseovers && !(*i).first.empty( ) && !(*i).second.empty( ) && !(*j).second.empty( ) )
					{
						pResponse->strContent += " onMouseOver=\"hoverOnTag" + CAtomInt( iTag ).toString( ) + "(); return true\"";
						pResponse->strContent += " onMouseOut=\"hoverOffTag" + CAtomInt( iTag ).toString( ) + "(); return true\"";
					}

					pResponse->strContent += ">\n";

					if( !(*i).first.empty( ) && !(*i).second.empty( ) )
						// Sets a TITLE parameter for each Tag Image, so that a TOOLTIP box will popup when
						// the user's mouse pointer hovers over the Tag Image.
						pResponse->strContent += "<img class=\"tag\" style=\"border:0\" src=\"" + (*i).second + "\" alt=\"[" + (*i).first + "]\" title=\"" + (*i).first + "\" name=\"bnbt_tag" + CAtomInt( iTag ).toString( ) + "\">";
					else
						pResponse->strContent += UTIL_RemoveHTML( (*i).first );

					pResponse->strContent += "</a>\n\n";

					// addition by labarks

					if( ( m_iDumpRSSFileMode == 1 || m_iDumpRSSFileMode == 2 ) && !m_strDumpRSSFileDir.empty( ) )
					{
						string :: size_type iExt = m_strDumpRSSFile.rfind( "." );

						string strExt;

						if( iExt != string :: npos )
							strExt = m_strDumpRSSFile.substr( iExt );

						string strRSSFile = UTIL_StripPath( m_strDumpRSSFile );

						strRSSFile = m_strDumpRSSFileDir + strRSSFile.substr( 0, strRSSFile.length( ) - strExt.length( ) ) + "-" + (*i).first + strExt;

						pResponse->strContent += "<span class=\"dash\">&nbsp;-&nbsp;</span><a class=\"rss\" href=\"" + strRSSFile + "\">RSS</a>";
					}

					// end addition

					if( i + 1 != m_vecTags.end( ) )
						pResponse->strContent += " <span class=\"pipe\">|</span> ";

					bFound = true;

					iTag++; /* =X= */
					j++; /* =X= */
				}

				pResponse->strContent += "</p>\n";
			}

			// which page are we viewing

			unsigned int iStart = 0;

			if( m_iPerPage > 0 )
			{
				string strPage = pRequest->mapParams["page"];

				if( !strPage.empty( ) )
					iStart = atoi( strPage.c_str( ) ) * m_iPerPage;

				pResponse->strContent += "<p class=\"pagenum_top\">Page " + CAtomInt( ( iStart / m_iPerPage ) + 1 ).toString( ) + "</p>\n";
			}

			bool bFound = false;

			int iAdded = 0;
			int iSkipped = 0;

			// for correct page numbers after searching

			int iFound = 0;

			for( unsigned long i = 0; i < iKeySize; i++ )
			{
				if( !strFilter.empty( ) )
				{
					// only display entries that match the filter

					if( pTorrents[i].strTag != strFilter )
						continue;
				}

				if( !strSearch.empty( ) )
				{
					// only display entries that match the search

					if( pTorrents[i].strLowerName.find( strLowerSearch ) == string :: npos )
						continue;
				}

				iFound++;

				if( m_iPerPage == 0 || iAdded < m_iPerPage )
				{
					if( !bFound )
					{
						// output table headers

						pResponse->strContent += "<table summary=\"files\">\n";
						pResponse->strContent += "<tr>";

						// <th> tag

						if( !m_vecTags.empty( ) ) /* =X= */
						{
							pResponse->strContent += "<th class=\"tag\">Tag";

							if( m_bSort )
							{
								pResponse->strContent += "<br><a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_ATAG;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_DTAG;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">Z</a>";
							}

							pResponse->strContent += "</th>";
						}

						// <th> info hash

						if( m_bShowInfoHash )
							pResponse->strContent += "<th class=\"hash\">Info Hash</th>";

						// <th> name

						if( m_pAllowed && m_bShowNames )
						{
							pResponse->strContent += "<th class=\"name\">Name";

							if( m_bSort )
							{
								pResponse->strContent += "<br><a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_ANAME;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_DNAME;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">Z</a>";
							}

							pResponse->strContent += "</th>";
						}

						// <th> torrent

						if( m_pAllowed && m_bAllowTorrentDownloads && ( pRequest->user.iAccess & ACCESS_DL ) )
							pResponse->strContent += "<th class=\"download\">Torrent</th>\n";

						// <th> comments

						if( m_pAllowed && m_bAllowComments )
						{
							if( m_pComments )
							{
								pResponse->strContent += "<th class=\"number\">Comments";

								if( m_bSort )
								{
									pResponse->strContent += "<br><a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_ACOMMENTS;

									if( !strSearch.empty( ) )
										pResponse->strContent += "&amp;search=" + strSearchResp;

									if( !strFilter.empty( ) )
										pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

									pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_DCOMMENTS;

									if( !strSearch.empty( ) )
										pResponse->strContent += "&amp;search=" + strSearchResp;

									if( !strFilter.empty( ) )
										pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

									pResponse->strContent += "\">Z</a>";
								}

								pResponse->strContent += "</th>";
							}
						}

						// <th> added

						if( m_pAllowed && m_bShowAdded )
						{
							pResponse->strContent += "<th class=\"date\">Added";
							
							if( m_bSort )
							{
								pResponse->strContent += "<br><a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_AADDED;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_DADDED;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">Z</a>";
							}

							pResponse->strContent += "</th>";
						}

						// <th> size

						if( m_pAllowed && m_bShowSize )
						{
							pResponse->strContent += "<th class=\"bytes\">Size";

							if( m_bSort )
							{
								pResponse->strContent += "<br><a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_ASIZE;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_DSIZE;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">Z</a>";
							}

							pResponse->strContent += "</th>";
						}

						// <th> files

						if( m_pAllowed && m_bShowNumFiles )
						{
							pResponse->strContent += "<th class=\"number\">Files";

							if( m_bSort )
							{
								pResponse->strContent += "<br><a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_AFILES;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_DFILES;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">Z</a>";
							}

							pResponse->strContent += "</th>";
						}

						// <th> seeders

						pResponse->strContent += "<th class=\"number\">Seeders";
						
						if( m_bSort )
						{
							pResponse->strContent += "<br><a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_ACOMPLETE;

							if( !strSearch.empty( ) )
								pResponse->strContent += "&amp;search=" + strSearchResp;

							if( !strFilter.empty( ) )
								pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

							pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_DCOMPLETE;

							if( !strSearch.empty( ) )
								pResponse->strContent += "&amp;search=" + strSearchResp;

							if( !strFilter.empty( ) )
								pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

							pResponse->strContent += "\">Z</a>";
						}

						pResponse->strContent += "</th>";

						// <th> leechers
						
						pResponse->strContent += "<th class=\"number\">Leechers";
						
						if( m_bSort )
						{
							pResponse->strContent += "<br><a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_AINCOMPLETE;

							if( !strSearch.empty( ) )
								pResponse->strContent += "&amp;search=" + strSearchResp;

							if( !strFilter.empty( ) )
								pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

							pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_DINCOMPLETE;

							if( !strSearch.empty( ) )
								pResponse->strContent += "&amp;search=" + strSearchResp;

							if( !strFilter.empty( ) )
								pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

							pResponse->strContent += "\">Z</a>";
						}

						pResponse->strContent += "</th>";

						// <th> completed

						if( m_bShowCompleted )
						{
							pResponse->strContent += "<th class=\"number\">Completed";

							if( m_bSort )
							{
								pResponse->strContent += "<br><a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_ACOMPLETED;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_DCOMPLETED;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">Z</a>";
							}

							pResponse->strContent += "</th>";
						}

						// <th> transferred

						if( m_pAllowed && m_bShowTransferred )
						{
							pResponse->strContent += "<th class=\"bytes\">Transferred";

							if( m_bSort )
							{
								pResponse->strContent += "<br><a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_ATRANSFERRED;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_DTRANSFERRED;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">Z</a>";
							}

							pResponse->strContent += "</th>";
						}

						// <th> min left

						if( m_bShowMinLeft )
						{
							if( m_pAllowed && m_bShowLeftAsProgress )
								pResponse->strContent += "<th class=\"percent\">Min Progress</th>";
							else
								pResponse->strContent += "<th class=\"percent\">Min Left</th>";
						}

						// <th> average left

						if( m_bShowAverageLeft )
						{
							if( m_pAllowed && m_bShowLeftAsProgress )
								pResponse->strContent += "<th class=\"percent\">Average Progress";
							else
								pResponse->strContent += "<th class=\"percent\">Average Left";

							if( m_bSort )
							{
								pResponse->strContent += "<br><a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_AAVGLEFT;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_DAVGLEFT;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">Z</a>";
							}

							pResponse->strContent += "</th>";
						}

						// <th> maxi left

						if( m_bShowMaxiLeft )
						{
							if( m_pAllowed && m_bShowLeftAsProgress )
								pResponse->strContent += "<th class=\"percent\">Max Progress</th>";
							else
								pResponse->strContent += "<th class=\"percent\">Max Left</th>";
						}

						// <th> uploader

						if( m_bShowUploader ) /* =X= */
						{
							pResponse->strContent += "<th class=\"name\">Uploader";

							if( m_bSort )
							{
								pResponse->strContent += "<br><a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_AUPLOADER;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/index.html?sort=" + SORTSTR_DUPLOADER;

								if( !strSearch.empty( ) )
									pResponse->strContent += "&amp;search=" + strSearchResp;

								if( !strFilter.empty( ) )
									pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

								pResponse->strContent += "\">Z</a>";
							}

							pResponse->strContent += "</th>";
						}

						// <th> info link

						if( m_bAllowInfoLink )
							pResponse->strContent += "<th class=\"infolink\">Info Link</th>";

						// <th> admin

						if( pRequest->user.iAccess & ACCESS_EDIT )
						{
							if( m_pAllowed )
								pResponse->strContent += "<th>Admin</th>";
						}

						pResponse->strContent += "</tr>\n";

						bFound = true;
					}

					if( iSkipped == (int)iStart )
					{
						// output table rows

						if( iAdded % 2 )
							pResponse->strContent += "<tr class=\"even\">";
						else
							pResponse->strContent += "<tr class=\"odd\">";

						// <td> tag

						if( !m_vecTags.empty( ) )
						{
							string strTemp = pTorrents[i].strTag;

							for( vector< pair<string, string> > :: iterator j = m_vecTags.begin( ); j != m_vecTags.end( ); j++ )
							{
								if( (*j).first == pTorrents[i].strTag && !(*j).second.empty( ) )
									pTorrents[i].strTag = "<img class=\"tag\" style=\"border:0\" alt=\"[" + pTorrents[i].strTag + "]\" src=\"" + (*j).second + "\">";
							}

							if( pTorrents[i].strTag == strTemp )
								pTorrents[i].strTag = UTIL_RemoveHTML( pTorrents[i].strTag );

							pResponse->strContent += "<td class=\"tag\"><a title=\"" + strTemp + "\" href=\"/index.html?filter=" + UTIL_StringToEscaped( strTemp ) + "\">" + pTorrents[i].strTag + "</a></td>";
						}

						// <td> info hash

						if( m_bShowInfoHash )
						{
							pResponse->strContent += "<td class=\"hash\">";

							if( m_bShowStats )
								pResponse->strContent += "<a class=\"stats\" href=\"/stats.html?info_hash=" + UTIL_HashToString( pTorrents[i].strInfoHash ) + "\">";

							pResponse->strContent += UTIL_HashToString( pTorrents[i].strInfoHash );

							if( m_bShowStats )
								pResponse->strContent += "</a>";

							pResponse->strContent += "</td>";
						}

						// <td> name

						if( m_pAllowed && m_bShowNames )
						{
							pResponse->strContent += "<td class=\"name\">";

							if( m_bShowStats )
								pResponse->strContent += "<a class=\"stats\" href=\"/stats.html?info_hash=" + UTIL_HashToString( pTorrents[i].strInfoHash ) + "\">";

							pResponse->strContent += UTIL_RemoveHTML( pTorrents[i].strName );

							if( m_bShowStats )
								pResponse->strContent += "</a>";

							pResponse->strContent += "</td>";
						}

						// <td> torrent

						if( m_pAllowed && m_bAllowTorrentDownloads && ( pRequest->user.iAccess & ACCESS_DL ) )
						{
							pResponse->strContent += "<td class=\"download\"><a class=\"download\" href=\"";

							if( m_strExternalTorrentDir.empty( ) )
								pResponse->strContent += "/torrents/" + UTIL_HashToString( pTorrents[i].strInfoHash ) + ".torrent";
							else
								pResponse->strContent += m_strExternalTorrentDir + UTIL_StringToEscapedStrict( pTorrents[i].strFileName );

							pResponse->strContent += "\">DL</a></td>";
						}

						// <td> comments

						if( m_pAllowed && m_bAllowComments )
						{
							if( m_pComments )
								pResponse->strContent += "<td class=\"number\"><a href=\"/comments.html?info_hash=" + UTIL_HashToString( pTorrents[i].strInfoHash ) + "\">" + CAtomInt( pTorrents[i].iComments ).toString( ) + "</a></td>";
						}

						// <td> added

						if( m_pAllowed && m_bShowAdded )
						{
							pResponse->strContent += "<td class=\"date\">";

							if( !pTorrents[i].strAdded.empty( ) )
							{
								// strip year and seconds from time

								pResponse->strContent += pTorrents[i].strAdded.substr( 5, pTorrents[i].strAdded.size( ) - 8 );
							}

							pResponse->strContent += "</td>";
						}

						// <td> size

						if( m_pAllowed && m_bShowSize )
							pResponse->strContent += "<td class=\"bytes\">" + UTIL_BytesToString( pTorrents[i].iSize ) + "</td>";

						// <td> files

						if( m_pAllowed && m_bShowNumFiles )
							pResponse->strContent += "<td class=\"number\">" + CAtomInt( pTorrents[i].iFiles ).toString( ) + "</td>";

						// <td> seeders

						pResponse->strContent += "<td class=\"number_";

						if( pTorrents[i].iSeeders == 0 )
							pResponse->strContent += "red\">";
						else if( pTorrents[i].iSeeders < 5 )
							pResponse->strContent += "yellow\">";
						else
							pResponse->strContent += "green\">";

						pResponse->strContent += CAtomInt( pTorrents[i].iSeeders ).toString( ) + "</td>";

						// <td> leechers

						pResponse->strContent += "<td class=\"number_";

						if( pTorrents[i].iLeechers == 0 )
							pResponse->strContent += "red\">";
						else if( pTorrents[i].iLeechers < 5 )
							pResponse->strContent += "yellow\">";
						else
							pResponse->strContent += "green\">";

						pResponse->strContent += CAtomInt( pTorrents[i].iLeechers ).toString( ) + "</td>";

						// <td> completed

						if( m_bShowCompleted )
							pResponse->strContent += "<td class=\"number\">" + CAtomInt( pTorrents[i].iCompleted ).toString( ) + "</td>";

						// <td> transferred

						if( m_pAllowed && m_bShowTransferred )
							pResponse->strContent += "<td class=\"bytes\">" + UTIL_BytesToString( pTorrents[i].iTransferred ) + "</td>";

						// <td> min left

						if( m_bShowMinLeft )
						{
							pResponse->strContent += "<td class=\"percent\">";

							if( pTorrents[i].iLeechers == 0 )
								pResponse->strContent += "N/A";
							else
							{
								if( m_pAllowed )
								{
									int iPercent = 0;

									if( pTorrents[i].iSize > 0 )
									{
										if( m_bShowLeftAsProgress )
											iPercent = 100 - (int)( ( (float)pTorrents[i].iMaxiLeft / pTorrents[i].iSize ) * 100 );
										else
											iPercent = (int)( ( (float)pTorrents[i].iMinLeft / pTorrents[i].iSize ) * 100 );
									}

									pResponse->strContent += CAtomInt( iPercent ).toString( ) + "%</td>";
								}
								else
									pResponse->strContent += UTIL_BytesToString( pTorrents[i].iMinLeft );
							}
						}

						// <td> average left

						if( m_bShowAverageLeft )
						{
							pResponse->strContent += "<td class=\"percent\" style=\"white-space:nowrap\">";

							if( pTorrents[i].iLeechers == 0 )
								pResponse->strContent += "N/A";
							else
							{
								if( m_pAllowed )
								{
									int iPercent;

									if( m_bShowLeftAsProgress )
										iPercent = 100 - pTorrents[i].iAverageLeftPercent;
									else
										iPercent = pTorrents[i].iAverageLeftPercent;

									pResponse->strContent += CAtomInt( iPercent ).toString( ) + "%";

									if( !m_strImageBarFill.empty( ) && !m_strImageBarTrans.empty( ) )
									{
										pResponse->strContent += "<br>";

										if( iPercent > 0 )
											pResponse->strContent += "<img style=\"\" alt=\"[Completed]\" src=\"" + m_strImageBarFill + "\" width=" + CAtomInt( iPercent ).toString( ) + " height=8>";

										if( iPercent < 100 )
											pResponse->strContent += "<img style=\"\" alt=\"[Remaining]\" src=\"" + m_strImageBarTrans + "\" width=" + CAtomInt( 100 - iPercent ).toString( ) + " height=8>";
									}
								}
								else
									pResponse->strContent += UTIL_BytesToString( pTorrents[i].iAverageLeft );

								pResponse->strContent += "</td>";
							}
						}

						// <td> maxi left

						if( m_bShowMaxiLeft )
						{
							pResponse->strContent += "<td class=\"percent\">";

							if( pTorrents[i].iLeechers == 0 )
								pResponse->strContent += "N/A";
							else
							{
								if( m_pAllowed )
								{
									int iPercent = 0;

									if( pTorrents[i].iSize > 0 )
									{
										if( m_bShowLeftAsProgress )
											iPercent = 100 - (int)( ( (float)pTorrents[i].iMinLeft / pTorrents[i].iSize ) * 100 );
										else
											iPercent = (int)( ( (float)pTorrents[i].iMaxiLeft / pTorrents[i].iSize ) * 100 );
									}

									pResponse->strContent += CAtomInt( iPercent ).toString( ) + "%</td>";
								}
								else
									pResponse->strContent += UTIL_BytesToString( pTorrents[i].iMaxiLeft );
							}
						}

						// <td> uploader

						if( m_bShowUploader )
							pResponse->strContent += "<td class=\"name\">" + UTIL_RemoveHTML( pTorrents[i].strUploader ) + "</td>";

						// <td> info link

						if( m_bAllowInfoLink )
						{
							pResponse->strContent += "<td class=\"infolink\">";

							if( !pTorrents[i].strInfoLink.empty( ) )
								pResponse->strContent += "<a href=\"" + UTIL_RemoveHTML( pTorrents[i].strInfoLink ) + "\">Link</a>";

							pResponse->strContent += "</td>";
						}

						// <td> admin

						if( pRequest->user.iAccess & ACCESS_EDIT )
						{
							if( m_pAllowed )
								pResponse->strContent += "<td>[<a href=\"/index.html?del=" + UTIL_HashToString( pTorrents[i].strInfoHash ) + "\">Delete</a>]</td>";
						}

						pResponse->strContent += "</tr>\n";

						iAdded++;
					}
					else
						iSkipped++;
				}
			}

			delete [] pTorrents;

			// some finishing touches

			if( bFound )
				pResponse->strContent += "</table>\n";

			if( m_pAllowed && m_bShowNames && m_bSearch )
			{
				pResponse->strContent += "<form class=\"search_index\" name=\"bottomsearch\" method=\"get\" action=\"/index.html\">\n";

				if( !strSort.empty( ) )
					pResponse->strContent += "<div><input name=\"sort\" type=hidden value=\"" + strSort + "\"></div>\n";

				if( !strFilter.empty( ) )
					pResponse->strContent += "<div><input name=\"filter\" type=hidden value=\"" + strFilter + "\"></div>\n";

				pResponse->strContent += "<div><label for=\"bottomtorrentsearch\">Search</label> <input name=\"search\" id=\"bottomtorrentsearch\" type=text size=40> <a href=\"/index.html\">Clear Search</a></div>\n";
				pResponse->strContent += "</form>\n";
			}

			// page numbers

			if( m_iPerPage > 0 )
			{
				pResponse->strContent += "<p class=\"pagenum_bottom\">";

				for( unsigned long i = 0; i < (unsigned int)iFound; i += m_iPerPage )
				{
					pResponse->strContent += " ";

					// don't link to current page

					if( i != iStart )
					{
						pResponse->strContent += "<a href=\"/index.html?page=" + CAtomInt( i / m_iPerPage ).toString( );

						if( !strSort.empty( ) )
							pResponse->strContent += "&amp;sort=" + strSort;

						if( !strSearch.empty( ) )
							pResponse->strContent += "&amp;search=" + strSearchResp;

						if( !strFilter.empty( ) )
							pResponse->strContent += "&amp;filter=" + UTIL_StringToEscaped( strFilter );

						pResponse->strContent += "\">";
					}

					pResponse->strContent += CAtomInt( ( i / m_iPerPage ) + 1 ).toString( );

					if( i != iStart )
						pResponse->strContent += "</a>";

					pResponse->strContent += " ";

					// don't display a bar after the last page

					if( i + (unsigned int)m_iPerPage < (unsigned int)iFound )
						pResponse->strContent += "|";
				}

				pResponse->strContent += "</p>\n";
			}
		}

		pResponse->strContent += m_strStaticFooter;

		// don't even think about removing this :)

		pResponse->strContent += "<p style=\"text-align:center\">POWERED BY BNBT " + string( BNBT_VER ) + "</p>\n";

		/* =X= */
		// For internalised mouseover feature
		if( m_bUseMouseovers && ( pRequest->user.iAccess & ACCESS_VIEW ) )
		{
			pResponse->strContent += "<script type=\"text/javascript\">\n";
			pResponse->strContent += "<!--\n";

			if( !m_vecTags.empty( ) && !m_vecTagsMouse.empty( ) )
			{
				vector< pair< string, string > > :: iterator j = m_vecTags.begin( );

				int iTag = 1;

				for( vector< pair< string, string > > :: iterator i = m_vecTagsMouse.begin( ); i != m_vecTagsMouse.end( ); i++ )
				{
					if( (*i).second.empty( ) || (*j).second.empty( ) )
					{
						iTag++;
						j++;

						continue;
					}

					pResponse->strContent += "document.bnbt_tag" + CAtomInt( iTag ).toString( ) + ".src = \"" + (*i).second + "\";\n";

					iTag++;
					j++;
				}
			}

			pResponse->strContent += "//-->\n";
			pResponse->strContent += "</script>\n\n";
		}
	}
	else
	{
		pResponse->strContent += m_strStaticHeader; /* =X= */
		pResponse->strContent += "<p class=\"denied\">You are not authorized to view this page.";

		if( pRequest->user.iAccess & ACCESS_SIGNUP )
			pResponse->strContent += " Click <a href=\"/signup.html\">here</a> to sign up for an account!";

		pResponse->strContent += "</p>\n";
		pResponse->strContent += m_strStaticFooter; /* =X= */
	}

	if( m_bGen )
		pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

	pResponse->strContent += "</body>\n";
	pResponse->strContent += "</html>\n";
}
