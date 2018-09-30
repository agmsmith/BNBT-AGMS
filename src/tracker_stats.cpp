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
#include "sort.h"
#include "tracker.h"
#include "util.h"

void CTracker :: serverResponseStats( struct request_t *pRequest, struct response_t *pResponse )
{
	struct bnbttv btv = UTIL_CurrentTime( );

	if( !m_bShowStats )
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
	pResponse->strContent += "<title>BNBT File Info</title>\n";

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
		pResponse->strContent += "<p class=\"login1_stats\">You are not logged in. Click <a href=\"/login.html\">here</a> to login.</p>\n";
	else
		pResponse->strContent += "<p class=\"login2_stats\">You are logged in as <span class=\"username\">" + UTIL_RemoveHTML( pRequest->user.strLogin ) + "</span>. Click <a href=\"/login.html?logout=1\">here</a> to logout.</p>\n";

	pResponse->strContent += m_strStaticHeader; /* =X= */

	pResponse->strContent += "<h3>BNBT File Info</h3>\n";

	if( pRequest->user.iAccess & ACCESS_VIEW )
	{
		string strHashString = pRequest->mapParams["info_hash"];
		string strHash = UTIL_StringToHash( strHashString );

		if( !strHash.empty( ) )
		{
			//
			// admin
			//

			if( pRequest->user.iAccess & ACCESS_EDIT )
			{
				if( m_pTags )
				{
					string strName = pRequest->mapParams["name"];
					string strTag = pRequest->mapParams["tag"];
					string strUploader = pRequest->mapParams["uploader"];
					string strInfoLink = pRequest->mapParams["infolink"];
					string strOK = pRequest->mapParams["ok"];

					if( !strName.empty( ) || !strTag.empty( ) || !strUploader.empty( ) || !strInfoLink.empty( ) )
					{
						if( strOK == "1" )
						{
							addTag( strHash, strTag, strName, strUploader, strInfoLink );

							pResponse->strContent += "<ul>\n";
							pResponse->strContent += "<li>Changed name to \"" + UTIL_RemoveHTML( strName ) + "\" (blank values mean no change).</li>\n";
							pResponse->strContent += "<li>Changed tag to \"" + UTIL_RemoveHTML( strTag ) + "\" (blank values mean no change).</li>\n";
							pResponse->strContent += "<li>Changed uploader to \"" + UTIL_RemoveHTML( strUploader ) + "\" (blank values mean no change).</li>\n";
							pResponse->strContent += "<li>Changed info link to \"" + UTIL_RemoveHTML( strInfoLink ) + "\" (blank values mean no change).</li>\n";
							pResponse->strContent += "</ul>\n";
							pResponse->strContent += "<p>Click <a href=\"/stats.html?info_hash=" + strHashString + "\">here</a> to return to the stats page.</p>\n";

							if( m_bGen )
								pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

							pResponse->strContent += "</body>\n";
							pResponse->strContent += "</html>\n";

							return;
						}
						else
						{
							pResponse->strContent += "<p>Are you sure you want to change the torrent's info?</p>\n";
							pResponse->strContent += "<p><a href=\"/stats.html?info_hash=" + strHashString + "&name=" + UTIL_StringToEscaped( strName ) + "&uploader=" + UTIL_StringToEscaped( strUploader ) + "&infolink=" + UTIL_StringToEscaped( strInfoLink ) + "&tag=" + UTIL_StringToEscaped( strTag ) + "&ok=1\">OK</a></p>\n";

							if( m_bGen )
								pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

							pResponse->strContent += "</body>\n";
							pResponse->strContent += "</html>\n";

							return;
						}
					}

					pResponse->strContent += "<hr width=\"80%\">\n";
					pResponse->strContent += "<p>Change Info</p>\n";
					pResponse->strContent += "<form method=\"get\" action=\"/stats.html\">\n";
					pResponse->strContent += "<input name=\"info_hash\" type=hidden value=\"" + strHashString + "\">\n";
					pResponse->strContent += "<input name=\"name\" type=text size=64 maxlength=" + CAtomInt( MAX_FILENAME_LEN ).toString( ) + "> New Name (blank values mean no change)<br><br>\n";
					pResponse->strContent += "<input name=\"uploader\" type=text size=64 maxlength=" + CAtomInt( m_iNameLength ).toString( ) + "> New Uploader (blank values mean no change)<br><br>\n";
					pResponse->strContent += "<input name=\"infolink\" type=text size=64 maxlength=" + CAtomInt( MAX_INFO_LINK_LEN ).toString( ) + "> New Info Link (blank values mean no change)<br><br>\n";

					string strCur;

					CAtom *pTagInfo = m_pTags->getItem( strHash );

					if( pTagInfo && pTagInfo->isDicti( ) )
					{
						CAtom *pCur = ( (CAtomDicti *)pTagInfo )->getItem( "tag" );

						if( pCur )
							strCur = pCur->toString( );
					}

					if( !m_vecTags.empty( ) )
						pResponse->strContent += "<select name=\"tag\">\n";

					for( vector< pair<string, string> > :: iterator i = m_vecTags.begin( ); i != m_vecTags.end( ); i++ )
					{
						pResponse->strContent += "<option";

						if( (*i).first == strCur )
							pResponse->strContent += " selected";

						pResponse->strContent += ">" + (*i).first + "\n";
					}

					if( !m_vecTags.empty( ) )
						pResponse->strContent += "</select> New Tag<br><br>\n";

					pResponse->strContent += "<input type=submit value=\"Change Info\">\n";
					pResponse->strContent += "</form>\n";
					pResponse->strContent += "<hr width=\"80%\">\n";
				}
			}

			// display torrent information list

			string strFileName;

			int64 iSize = 0;
			int iFiles = 1;

			if( m_pAllowed )
			{
				CAtom *pTorrent = m_pAllowed->getItem( strHash );

				if( pTorrent && dynamic_cast<CAtomList *>( pTorrent ) )
				{
					vector<CAtom *> vecTorrent = dynamic_cast<CAtomList *>( pTorrent )->getValue( );

					if( vecTorrent.size( ) == 6 )
					{
						CAtom *pFileName = vecTorrent[0];
						CAtom *pName = vecTorrent[1];
						CAtom *pAdded = vecTorrent[2];
						CAtom *pSize = vecTorrent[3];
						CAtom *pFiles = vecTorrent[4];
						CAtom *pComment = vecTorrent[5];

						pResponse->strContent += "<p>File Information</p>\n";
						pResponse->strContent += "<ul>\n";

						if( pFileName )
							strFileName = pFileName->toString( );

						if( pName )
							pResponse->strContent += "<li><strong>Name:</strong> " + UTIL_RemoveHTML( pName->toString( ) ) + "</li>\n";

						pResponse->strContent += "<li><strong>Info Hash:</strong> " + strHashString + "</li>\n";

						if( pAdded )
							pResponse->strContent += "<li><strong>Added:</strong> " + pAdded->toString( ) + "</li>\n";

						if( pSize && dynamic_cast<CAtomLong *>( pSize ) )
						{
							// cache iSize

							iSize = dynamic_cast<CAtomLong *>( pSize )->getValue( );

							pResponse->strContent += "<li><strong>Size:</strong> " + UTIL_BytesToString( iSize ) + "</li>\n";
						}

						if( pFiles && dynamic_cast<CAtomInt *>( pFiles ) )
						{
							// cache iFiles

							iFiles = (int)dynamic_cast<CAtomInt *>( pFiles )->getValue( );

							pResponse->strContent += "<li><strong>Files:</strong> " + pFiles->toString( ) + "</li>\n";
						}

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

			if( m_pAllowed && m_bAllowTorrentDownloads && ( pRequest->user.iAccess & ACCESS_DL ) )
			{
				pResponse->strContent += "<p><a class=\"download\" href=\"";

				if( m_strExternalTorrentDir.empty( ) )
					pResponse->strContent += "/torrents/" + strHashString + ".torrent";
				else
					pResponse->strContent += m_strExternalTorrentDir + UTIL_StringToEscapedStrict( strFileName );

				pResponse->strContent += "\">DOWNLOAD TORRENT</a></p>";
			}

			if( m_bShowFileContents && iFiles > 1 )
			{
				CAtom *pDecoded = DecodeFile( ( m_strAllowedDir + strFileName ).c_str( ) );

				if( pDecoded && pDecoded->isDicti( ) )
				{
					CAtom *pInfo = ( (CAtomDicti *)pDecoded )->getItem( "info" );

					if( pInfo && pInfo->isDicti( ) )
					{
						CAtom *pFiles = ( (CAtomDicti *)pInfo )->getItem( "files" );

						if( pFiles && dynamic_cast<CAtomList *>( pFiles ) )
						{
							bool bFound = false;

							int iAdded = 0;

							vector<CAtom *> *pvecFiles = dynamic_cast<CAtomList *>( pFiles )->getValuePtr( );

							for( vector<CAtom *> :: iterator i = pvecFiles->begin( ); i != pvecFiles->end( ); i++ )
							{
								if( (*i)->isDicti( ) )
								{
									CAtom *pPath = ( (CAtomDicti *)(*i) )->getItem( "path" );
									CAtom *pLen = ( (CAtomDicti *)(*i) )->getItem( "length" );

									if( pPath && dynamic_cast<CAtomList *>( pPath ) )
									{
										if( !bFound )
										{
											pResponse->strContent += "<p>Contents</p>\n";
											pResponse->strContent += "<table summary=\"contents\">\n";

											bFound = true;
										}

										string strPath;

										vector<CAtom *> *pvecPath = dynamic_cast<CAtomList *>( pPath )->getValuePtr( );

										for( vector<CAtom *> :: iterator j = pvecPath->begin( ); j != pvecPath->end( ); j++ )
										{
											if( j != pvecPath->begin( ) )
												strPath += '/';

											strPath += (*j)->toString( );
										}

										if( !strPath.empty( ) )
										{
											if( iAdded % 2 )
												pResponse->strContent += "<tr class=\"even\">";
											else
												pResponse->strContent += "<tr class=\"odd\">";

											pResponse->strContent += "<td class=\"path\">" + UTIL_RemoveHTML( strPath ) + "</td>";

											if( pLen && dynamic_cast<CAtomLong *>( pLen ) )
												pResponse->strContent += "<td class=\"bytes\">" + UTIL_BytesToString( dynamic_cast<CAtomLong *>( pLen )->getValue( ) ) + "</td>";

											pResponse->strContent += "</tr>\n";

											iAdded++;
										}
									}
								}
							}

							if( bFound )
								pResponse->strContent += "</table>\n";
						}
					}
				}

				if( pDecoded )
					delete pDecoded;
			}

			if( m_pDFile )
			{
				CAtom *pPeers = m_pDFile->getItem( strHash );

				if( pPeers && pPeers->isDicti( ) )
				{
					map<string, CAtom *> *pmapPeersDicti = ( (CAtomDicti *)pPeers )->getValuePtr( );

					// add the peers into this structure one by one and sort it afterwards

					struct peer_t *pPeersT = new struct peer_t[pmapPeersDicti->size( )];

					unsigned long i = 0;

					for( map<string, CAtom *> :: iterator it = pmapPeersDicti->begin( ); it != pmapPeersDicti->end( ); it++ )
					{
						pPeersT[i].iUpped = 0;
						pPeersT[i].iDowned = 0;
						pPeersT[i].iLeft = 0;
						pPeersT[i].iConnected = 0;
						pPeersT[i].flShareRatio = 0.0;

						if( (*it).second->isDicti( ) )
						{
							CAtomDicti *pPeerDicti = (CAtomDicti *)(*it).second;

							CAtom *pIP = pPeerDicti->getItem( "ip" );
							CAtom *pUpped = pPeerDicti->getItem( "uploaded" );
							CAtom *pDowned = pPeerDicti->getItem( "downloaded" );
							CAtom *pLef = pPeerDicti->getItem( "left" );
							CAtom *pConn = pPeerDicti->getItem( "connected" );

							if( pIP )
							{
								pPeersT[i].strIP = pIP->toString( );

								// strip ip

								string :: size_type iStart = pPeersT[i].strIP.rfind( "." );

								if( iStart != string :: npos )
								{
									// don't strip ip for mods

									if( !( pRequest->user.iAccess & ACCESS_EDIT ) )
										pPeersT[i].strIP = pPeersT[i].strIP.substr( 0, iStart + 1 ) + "xxx";
								}
							}

							if( pUpped && dynamic_cast<CAtomLong *>( pUpped ) )
								pPeersT[i].iUpped = dynamic_cast<CAtomLong *>( pUpped )->getValue( );

							if( pDowned && dynamic_cast<CAtomLong *>( pDowned ) )
							{
								pPeersT[i].iDowned = dynamic_cast<CAtomLong *>( pDowned )->getValue( );

								if( m_bShowShareRatios )
								{
									if( pPeersT[i].iDowned > 0 )
										pPeersT[i].flShareRatio = (float)pPeersT[i].iUpped / (float)pPeersT[i].iDowned;
									else if( pPeersT[i].iUpped == 0 )
										pPeersT[i].flShareRatio = 0.0;
									else
										pPeersT[i].flShareRatio = -1.0;
								}
							}

							if( pLef && dynamic_cast<CAtomLong *>( pLef ) )
								pPeersT[i].iLeft = dynamic_cast<CAtomLong *>( pLef )->getValue( );

							if( pConn && dynamic_cast<CAtomLong *>( pConn ) )
								pPeersT[i].iConnected = GetTime( ) - (unsigned long)dynamic_cast<CAtomLong *>( pConn )->getValue( );
						}

						i++;
					}

					string strSort = pRequest->mapParams["sort"];

					if( m_bSort )
					{
						if( !strSort.empty( ) )
						{
							int iSort = atoi( strSort.c_str( ) );

							if( iSort == SORTP_AUPPED )
								qsort( pPeersT, pmapPeersDicti->size( ), sizeof( struct peer_t ), asortpByUpped );
							else if( iSort == SORTP_ADOWNED )
								qsort( pPeersT, pmapPeersDicti->size( ), sizeof( struct peer_t ), asortpByDowned );
							else if( iSort == SORTP_ALEFT )
								qsort( pPeersT, pmapPeersDicti->size( ), sizeof( struct peer_t ), asortpByLeft );
							else if( iSort == SORTP_ACONNECTED )
								qsort( pPeersT, pmapPeersDicti->size( ), sizeof( struct peer_t ), asortpByConnected );
							else if( iSort == SORTP_ASHARERATIO )
							{
								if( m_bShowShareRatios )
									qsort( pPeersT, pmapPeersDicti->size( ), sizeof( struct peer_t ), asortpByShareRatio );
							}
							else if( iSort == SORTP_DUPPED )
								qsort( pPeersT, pmapPeersDicti->size( ), sizeof( struct peer_t ), dsortpByUpped );
							else if( iSort == SORTP_DDOWNED )
								qsort( pPeersT, pmapPeersDicti->size( ), sizeof( struct peer_t ), dsortpByDowned );
							else if( iSort == SORTP_DLEFT )
								qsort( pPeersT, pmapPeersDicti->size( ), sizeof( struct peer_t ), dsortpByLeft );
							else if( iSort == SORTP_DCONNECTED )
								qsort( pPeersT, pmapPeersDicti->size( ), sizeof( struct peer_t ), dsortpByConnected );
							else if( iSort == SORTP_DSHARERATIO )
							{
								if( m_bShowShareRatios )
									qsort( pPeersT, pmapPeersDicti->size( ), sizeof( struct peer_t ), dsortpByShareRatio );
							}
							else
							{
								// default action is to sort by left

								qsort( pPeersT, pmapPeersDicti->size( ), sizeof( struct peer_t ), asortpByLeft );
							}
						}
						else
						{
							// default action is to sort by left

							qsort( pPeersT, pmapPeersDicti->size( ), sizeof( struct peer_t ), asortpByLeft );
						}
					}
					else
					{
						// sort is disabled, but default action is to sort by left

						qsort( pPeersT, pmapPeersDicti->size( ), sizeof( struct peer_t ), asortpByLeft );
					}

					bool bFound = false;

					int iAdded = 0;

					//
					// seeders
					//

					string :: size_type iCountGoesHere = string :: npos;

					for( unsigned long peer_iter = 0; peer_iter < pmapPeersDicti->size( ); peer_iter++ )
					{
						if( iAdded >= m_iMaxPeersDisplay )
							break;

						if( pPeersT[peer_iter].iLeft == 0 )
						{
							if( !bFound )
							{
								// output table headers

								pResponse->strContent += "<p>Seeders</p>\n";

								// to save on calculations, we're going to insert the number of seeders later, keep track of where

								iCountGoesHere = pResponse->strContent.size( ) - strlen( "</p>\n" );

								pResponse->strContent += "<table summary=\"seeders\">\n";
								pResponse->strContent += "<tr><th class=\"ip\">Peer IP</th><th class=\"bytes\">Uploaded";

								if( m_bSort )
								{
									pResponse->strContent += "<br><a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_AUPPED;
									pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_DUPPED;
									pResponse->strContent += "\">Z</a>";
								}

								pResponse->strContent += "</th><th class=\"bytes\">Downloaded";

								if( m_bSort )
								{
									pResponse->strContent += "<br><a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_ADOWNED;
									pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_DDOWNED;
									pResponse->strContent += "\">Z</a>";
								}

								pResponse->strContent += "</th><th class=\"connected\">Connected";

								if( m_bSort )
								{
									pResponse->strContent += "<br><a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_ACONNECTED;
									pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_DCONNECTED;
									pResponse->strContent += "\">Z</a>";
								}

								pResponse->strContent += "</th>";

								if( m_bShowShareRatios )
								{
									pResponse->strContent += "<th class=\"number\">Share Ratio";

									if( m_bSort )
									{
										pResponse->strContent += "<br><a class=\"sort\" href=\"/stats.html?info_hash=";
										pResponse->strContent += strHashString;
										pResponse->strContent += "&sort=";
										pResponse->strContent += SORTPSTR_ASHARERATIO;
										pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/stats.html?info_hash=";
										pResponse->strContent += strHashString;
										pResponse->strContent += "&sort=";
										pResponse->strContent += SORTPSTR_DSHARERATIO;
										pResponse->strContent += "\">Z</a>";
									}

									pResponse->strContent += "</th>";
								}

								if( m_bShowAvgULRate )
									pResponse->strContent += "<th class=\"number\">Average Upload Rate</th>";

								pResponse->strContent += "</tr>\n";

								bFound = true;
							}

							// output table rows

							if( iAdded % 2 )
								pResponse->strContent += "<tr class=\"even\">";
							else
								pResponse->strContent += "<tr class=\"odd\">";

							pResponse->strContent += "<td class=\"ip\">" + pPeersT[peer_iter].strIP + "</td>";
							pResponse->strContent += "<td class=\"bytes\">" + UTIL_BytesToString( pPeersT[peer_iter].iUpped ) + "</td>";
							pResponse->strContent += "<td class=\"bytes\">" + UTIL_BytesToString( pPeersT[peer_iter].iDowned ) + "</td>";
							pResponse->strContent += "<td class=\"connected\">" + UTIL_SecondsToString( pPeersT[peer_iter].iConnected ) + "</td>";

							if( m_bShowShareRatios )
							{
								pResponse->strContent += "<td class=\"number_";

								if( pPeersT[peer_iter].flShareRatio == -1.0 )
									pResponse->strContent += "green\">";
								else if( pPeersT[peer_iter].flShareRatio < 0.8 )
									pResponse->strContent += "red\">";
								else if( pPeersT[peer_iter].flShareRatio < 1.2 )
									pResponse->strContent += "yellow\">";
								else
									pResponse->strContent += "green\">";

								// turn the share ratio into a string

								if( pPeersT[peer_iter].flShareRatio == -1.0 )
									pResponse->strContent += "Perfect";
								else
								{
									char szFloat[16];
									memset( szFloat, 0, sizeof( char ) * 16 );
									sprintf( szFloat, "%0.3f", pPeersT[peer_iter].flShareRatio );

									pResponse->strContent += szFloat;
								}

								pResponse->strContent += "</td>";
							}

							if( m_bShowAvgULRate )
							{
								pResponse->strContent += "<td class=\"number\">";

								if( pPeersT[peer_iter].iConnected > 0 )
									pResponse->strContent += UTIL_BytesToString( pPeersT[peer_iter].iUpped / pPeersT[peer_iter].iConnected ) + "/sec";
								else
									pResponse->strContent += "N/A";

								pResponse->strContent += "</td>";
							}

							pResponse->strContent += "</tr>\n";

							iAdded++;
						}
					}

					// insert the number of seeders

					string strTemp = " (" + CAtomInt( iAdded ).toString( ) + ")";

					if( iCountGoesHere != string :: npos )
						pResponse->strContent.insert( iCountGoesHere, strTemp );

					iCountGoesHere = string :: npos;

					if( bFound )
						pResponse->strContent += "</table>\n";

					bFound = false;

					iAdded = 0;

					//
					// leechers
					//

					for( unsigned long leecher_iter = 0; leecher_iter < pmapPeersDicti->size( ); leecher_iter++ )
					{
						if( iAdded >= m_iMaxPeersDisplay )
							break;

						if( pPeersT[leecher_iter].iLeft != 0 )
						{
							if( !bFound )
							{
								// output table headers

								pResponse->strContent += "<p>Leechers</p>\n";

								// to save on calculations, we're going to insert the number of leechers later, keep track of where

								iCountGoesHere = pResponse->strContent.size( ) - strlen( "</p>\n" );

								pResponse->strContent += "<table summary=\"leechers\">\n";
								pResponse->strContent += "<tr><th class=\"ip\">Peer IP</th><th class=\"bytes\">Uploaded";

								if( m_bSort )
								{
									pResponse->strContent += "<br><a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_AUPPED;
									pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_DUPPED;
									pResponse->strContent += "\">Z</a>";
								}

								pResponse->strContent += "</th><th class=\"bytes\">Downloaded";

								if( m_bSort )
								{
									pResponse->strContent += "<br><a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_ADOWNED;
									pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_DDOWNED;
									pResponse->strContent += "\">Z</a>";
								}

								if( m_pAllowed && m_bShowLeftAsProgress )
									pResponse->strContent += "</th><th class=\"bytes\">Progress";
								else
									pResponse->strContent += "</th><th class=\"bytes\">Left";

								if( m_bSort )
								{
									pResponse->strContent += "<br><a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_ALEFT;
									pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_DLEFT;
									pResponse->strContent += "\">Z</a>";
								}

								pResponse->strContent += "</th><th class=\"connected\">Connected";

								if( m_bSort )
								{
									pResponse->strContent += "<br><a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_ACONNECTED;
									pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/stats.html?info_hash=";
									pResponse->strContent += strHashString;
									pResponse->strContent += "&sort=";
									pResponse->strContent += SORTPSTR_DCONNECTED;
									pResponse->strContent += "\">Z</a>";
								}

								pResponse->strContent += "</th>";

								if( m_bShowShareRatios )
								{
									pResponse->strContent += "<th class=\"number\">Share Ratio";

									if( m_bSort )
									{
										pResponse->strContent += "<br><a class=\"sort\" href=\"/stats.html?info_hash=";
										pResponse->strContent += strHashString;
										pResponse->strContent += "&sort=";
										pResponse->strContent += SORTPSTR_ASHARERATIO;
										pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/stats.html?info_hash=";
										pResponse->strContent += strHashString;
										pResponse->strContent += "&sort=";
										pResponse->strContent += SORTPSTR_DSHARERATIO;
										pResponse->strContent += "\">Z</a>";
									}

									pResponse->strContent += "</th>";
								}

								if( m_bShowAvgULRate )
									pResponse->strContent += "<th class=\"number\">Average Upload Rate</th>";

								if( m_bShowAvgDLRate )
									pResponse->strContent += "<th class=\"number\">Average Download Rate</th>";

								pResponse->strContent += "</tr>\n";

								bFound = true;
							}

							// output table rows

							if( iAdded % 2 )
								pResponse->strContent += "<tr class=\"even\">";
							else
								pResponse->strContent += "<tr class=\"odd\">";

							pResponse->strContent += "<td class=\"ip\">" + pPeersT[leecher_iter].strIP + "</td>";
							pResponse->strContent += "<td class=\"bytes\">" + UTIL_BytesToString( pPeersT[leecher_iter].iUpped ) + "</td>";
							pResponse->strContent += "<td class=\"bytes\">" + UTIL_BytesToString( pPeersT[leecher_iter].iDowned ) + "</td>";
							pResponse->strContent += "<td class=\"percent\">";

							if( m_pAllowed && m_bShowLeftAsProgress )
								pResponse->strContent += UTIL_BytesToString( iSize - pPeersT[leecher_iter].iLeft );
							else
								pResponse->strContent += UTIL_BytesToString( pPeersT[leecher_iter].iLeft );

							if( m_pAllowed )
							{
								pResponse->strContent += " (";

								int iPercent = 0;

								if( iSize > 0 )
								{
									if( m_bShowLeftAsProgress )
										iPercent = 100 - (int)( ( (float)pPeersT[leecher_iter].iLeft / iSize ) * 100 );
									else
										iPercent = (int)( ( (float)pPeersT[leecher_iter].iLeft / iSize ) * 100 );
								}

								pResponse->strContent += CAtomInt( iPercent ).toString( ) + "%)";

								if( !m_strImageBarFill.empty( ) && !m_strImageBarTrans.empty( ) )
								{
									pResponse->strContent += "<br>";

									if( iPercent > 0 )
										pResponse->strContent += "<img src=\"" + m_strImageBarFill + "\" width=" + CAtomInt( iPercent ).toString( ) + " height=8>";

									if( iPercent < 100 )
										pResponse->strContent += "<img src=\"" + m_strImageBarTrans + "\" width=" + CAtomInt( 100 - iPercent ).toString( ) + " height=8>";
								}
							}

							pResponse->strContent += "</td><td class=\"connected\">" + UTIL_SecondsToString( pPeersT[leecher_iter].iConnected ) + "</td>";

							if( m_bShowShareRatios )
							{
								pResponse->strContent += "<td class=\"number_";

								if( pPeersT[leecher_iter].flShareRatio == -1.0 )
									pResponse->strContent += "green\">";
								else if( pPeersT[leecher_iter].flShareRatio < 0.8 )
									pResponse->strContent += "red\">";
								else if( pPeersT[leecher_iter].flShareRatio < 1.2 )
									pResponse->strContent += "yellow\">";
								else
									pResponse->strContent += "green\">";

								// turn the share ratio into a string

								if( pPeersT[leecher_iter].flShareRatio == -1.0 )
									pResponse->strContent += "Perfect";
								else
								{
									char szFloat[16];
									memset( szFloat, 0, sizeof( char ) * 16 );
									sprintf( szFloat, "%0.3f", pPeersT[leecher_iter].flShareRatio );

									pResponse->strContent += szFloat;
								}

								pResponse->strContent += "</td>";
							}

							if( m_bShowAvgULRate )
							{
								pResponse->strContent += "<td class=\"number\">";

								if( pPeersT[leecher_iter].iConnected > 0 )
									pResponse->strContent += UTIL_BytesToString( pPeersT[leecher_iter].iUpped / pPeersT[leecher_iter].iConnected ) + "/sec";
								else
									pResponse->strContent += "N/A";

								pResponse->strContent += "</td>";
							}

							if( m_bShowAvgDLRate )
							{
								pResponse->strContent += "<td class=\"number\">";

								if( pPeersT[leecher_iter].iConnected > 0 )
									pResponse->strContent += UTIL_BytesToString( pPeersT[leecher_iter].iDowned / pPeersT[leecher_iter].iConnected ) + "/sec";
								else
									pResponse->strContent += "N/A";

								pResponse->strContent += "</td>";
							}

							pResponse->strContent += "</tr>\n";

							iAdded++;
						}
					}

					// insert the number of leechers

					strTemp = " (" + CAtomInt( iAdded ).toString( ) + ")";

					if( iCountGoesHere != string :: npos )
						pResponse->strContent.insert( iCountGoesHere, strTemp );

					delete [] pPeersT;

					if( bFound )
						pResponse->strContent += "</table>\n";
				}
			}
		}
	}
	else
		pResponse->strContent += "<p class=\"denied\">You are not authorized to view this page.</p>\n";

	pResponse->strContent += m_strStaticFooter; /* =X= */

	if( m_bGen )
		pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

	pResponse->strContent += "</body>\n";
	pResponse->strContent += "</html>\n";
}
