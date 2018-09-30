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
#include "tracker.h"
#include "util.h"

void CTracker :: serverResponseTorrent( struct request_t *pRequest, struct response_t *pResponse )
{
	if( !m_pAllowed || !m_bAllowTorrentDownloads || !( pRequest->user.iAccess & ACCESS_DL ) )
	{
		pResponse->strCode = "403 Forbidden";

		return;
	}

	// rip apart URL of the form "/torrents/<hash>.torrent"

	string strTorrent = UTIL_EscapedToString( pRequest->strURL.substr( 10 ) );

	string :: size_type iExt = strTorrent.rfind( "." );

	if( iExt != string :: npos )
		strTorrent = strTorrent.substr( 0, iExt );

	CAtom *pTorrent = m_pAllowed->getItem( UTIL_StringToHash( strTorrent ) );

	if( pTorrent && dynamic_cast<CAtomList *>( pTorrent ) )
	{
		vector<CAtom *> vecTorrent = dynamic_cast<CAtomList *>( pTorrent )->getValue( );

		if( vecTorrent.size( ) == 6 )
		{
			CAtom *pFileName = vecTorrent[0];

			if( pFileName )
			{
				string strPath = m_strAllowedDir + pFileName->toString( );

				if( UTIL_CheckFile( strPath.c_str( ) ) )
				{
					pResponse->strCode = "200 OK";

					pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", string( gmapMime[".torrent"] + "; name=\"" ) + pFileName->toString( ) + "\"" ) ); /* =X= */
					pResponse->mapHeaders.insert( pair<string, string>( "Content-Disposition", string( "attachment; filename=\"" ) + pFileName->toString( ) + "\"" ) );

					// cache for awhile

					time_t tNow = time( NULL ) + m_iFileExpires * 60;
					char *szTime = asctime( gmtime( &tNow ) );
					szTime[strlen( szTime ) - 1] = '\0';

					pResponse->mapHeaders.insert( pair<string, string>( "Expires", string( szTime ) + " GMT" ) );

					pResponse->bCompressOK = false;

					string strData = UTIL_ReadFile( strPath.c_str( ) );

					if( !m_strForceAnnounceURL.empty( ) && m_bForceAnnounceOnDL )
					{
						CAtom *pData = Decode( strData );

						if( pData && pData->isDicti( ) )
						{
							( (CAtomDicti *)pData )->setItem( "announce", new CAtomString( m_strForceAnnounceURL ) );

							pResponse->strContent = Encode( pData );
						}

						delete pData;
					}
					else
						pResponse->strContent = strData;
				}
				else
					serverResponseNotFound( pRequest, pResponse ); /* =X= */
			}
			else
				serverResponseNotFound( pRequest, pResponse ); /* =X= */
		}
		else
			serverResponseNotFound( pRequest, pResponse ); /* =X= */
	}
	else
		serverResponseNotFound( pRequest, pResponse ); /* =X= */
}
