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
#include "bnbt_mysql.h"
#include "atom.h"
#include "bencode.h"
#include "tracker.h"
#include "util.h"

void CTracker :: serverResponseAnnounce( struct request_t *pRequest, struct response_t *pResponse )
{
	/* =X= */
	// Authorise client announce
	if( m_iAuthAnnounce != 0 && ( pRequest->user.iAccess & m_iAuthAnnounce ) )
	{
		pResponse->strCode = "401 Unauthorized";

		pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", gmapMime[".txt"] ) );
		pResponse->mapHeaders.insert( pair<string, string>( "WWW-Authenticate", string( "Basic realm=\"" ) + gstrRealm + "\"" ) );
		
		pResponse->strContent = UTIL_FailureReason( "Tracker authentication failed" );

		return;
	}

	pResponse->strCode = "200 OK";

	pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", gmapMime[".txt"] ) ); /* =X= */

	// retrieve info hash

	string strInfoHash = pRequest->mapParams["info_hash"];

	if( strInfoHash.empty( ) )
	{
		pResponse->strContent = UTIL_FailureReason( "info hash missing" );
		pResponse->bCompressOK = false;

		return;
	}

	if( m_pAllowed )
	{
		if( !m_pAllowed->getItem( strInfoHash ) )
		{
			pResponse->strContent = UTIL_FailureReason( "requested download is not authorized for use with this tracker" );
			pResponse->bCompressOK = false;

			return;
		}
	}

	// retrieve ip

	string strIP = inet_ntoa( pRequest->sin.sin_addr );
	string strTempIP = pRequest->mapParams["ip"];
	// Harold - NAT IP handling
	string strIPConv = strIP.c_str( );

	// Harold - This prevents NAT IPs from being manually specified in the tracker.
	if( m_iBlockNATedIP == 1 && ( !strTempIP.empty( ) && ( strTempIP.substr(0,8) == "192.168." || strTempIP.substr(0,8) == "169.254." || strTempIP.substr(0,3) == "10." || strTempIP.substr(0,7) == "172.16." || strTempIP.substr(0,7) == "172.17." || strTempIP.substr(0,7) == "172.18." || strTempIP.substr(0,7) == "172.19." || strTempIP.substr(0,7) == "172.20." || strTempIP.substr(0,7) == "172.21." || strTempIP.substr(0,7) == "172.22." || strTempIP.substr(0,7) == "172.23." || strTempIP.substr(0,7) == "172.24." || strTempIP.substr(0,7) == "172.25." || strTempIP.substr(0,7) == "172.26." || strTempIP.substr(0,7) == "172.27." || strTempIP.substr(0,7) == "172.28." || strTempIP.substr(0,7) == "172.29." || strTempIP.substr(0,7) == "172.30." || strTempIP.substr(0,7) == "172.31." || strTempIP.substr(0,7) == "172.32." || strTempIP == "127.0.0.1" ) ) )
		strTempIP = "";

	// Harold - this code prevents non-NAT IPs from having their specified IP override their actual IP.
	if( m_iLocalOnly == 1 && ( !strIPConv.empty( ) && ( strIPConv.substr(0,8) == "192.168." || strIPConv.substr(0,8) == "169.254." || strIPConv.substr(0,3) == "10." || strIPConv.substr(0,7) == "172.16." || strIPConv.substr(0,7) == "172.17." || strIPConv.substr(0,7) == "172.18." || strIPConv.substr(0,7) == "172.19." || strIPConv.substr(0,7) == "172.20." || strIPConv.substr(0,7) == "172.21." || strIPConv.substr(0,7) == "172.22." || strIPConv.substr(0,7) == "172.23." || strIPConv.substr(0,7) == "172.24." || strIPConv.substr(0,7) == "172.25." || strIPConv.substr(0,7) == "172.26." || strIPConv.substr(0,7) == "172.27." || strIPConv.substr(0,7) == "172.28." || strIPConv.substr(0,7) == "172.29." || strIPConv.substr(0,7) == "172.30." || strIPConv.substr(0,7) == "172.31." || strIPConv.substr(0,7) == "172.32." || strIPConv == "127.0.0.1" ) ) )
	{
		if( !strTempIP.empty( ) && strTempIP.find_first_not_of( "1234567890." ) == string :: npos )
			strIP = strTempIP;
	}
	else if( m_iLocalOnly == 0 )
	{
		if( !strTempIP.empty( ) && strTempIP.find_first_not_of( "1234567890." ) == string :: npos )
			strIP = strTempIP;
	}

	// Harold - End NAT IP Handling
	// retrieve a ton of parameters

	string strEvent = pRequest->mapParams["event"];
	string strPort = pRequest->mapParams["port"];
	string strUploaded = pRequest->mapParams["uploaded"];
	string strDownloaded = pRequest->mapParams["downloaded"];
	string strLeft = pRequest->mapParams["left"];
	string strPeerID = pRequest->mapParams["peer_id"];
	string strNumWant = pRequest->mapParams["numwant"]; // DWK - Fixed numwant values.
	string strNoPeerID = pRequest->mapParams["no_peer_id"];
	string strCompact = pRequest->mapParams["compact"];
	string strKey = pRequest->mapParams["key"]; /* =X= */

	if( !strEvent.empty( ) && strEvent != "started" && strEvent != "completed" && strEvent != "stopped" )
	{
		pResponse->strContent = UTIL_FailureReason( "invalid event" );
		pResponse->bCompressOK = false;

		return;
	}

	if( strPort.empty( ) )
	{
		pResponse->strContent = UTIL_FailureReason( "port missing" );
		pResponse->bCompressOK = false;

		return;
	}

	if( strUploaded.empty( ) )
	{
		pResponse->strContent = UTIL_FailureReason( "uploaded missing" );
		pResponse->bCompressOK = false;

		return;
	}

	if( strDownloaded.empty( ) )
	{
		pResponse->strContent = UTIL_FailureReason( "downloaded missing" );
		pResponse->bCompressOK = false;

		return;
	}

	if( strLeft.empty( ) )
	{
		pResponse->strContent = UTIL_FailureReason( "left missing" );
		pResponse->bCompressOK = false;

		return;
	}

	if( strPeerID.size( ) != 20 )
	{
		pResponse->strContent = UTIL_FailureReason( "peer id not of length 20" );
		pResponse->bCompressOK = false;

		return;
	}

	struct announce_t ann;

	ann.strInfoHash = strInfoHash;
	ann.strIP = strIP;
	ann.strEvent = strEvent;
	ann.iPort = (unsigned int)atoi( strPort.c_str( ) );
	ann.iUploaded = UTIL_StringTo64( strUploaded.c_str( ) );
	ann.iDownloaded = UTIL_StringTo64( strDownloaded.c_str( ) );
	ann.iLeft = UTIL_StringTo64( strLeft.c_str( ) );
	ann.strPeerID = strPeerID;
	ann.strKey = strKey; /* =X= */

	Announce( ann );

	unsigned int iRSize = m_iResponseSize;

	if( !strNumWant.empty( ) )
		iRSize = atoi( strNumWant.c_str( ) );

	if( iRSize > (unsigned int)m_iMaxGive )
		iRSize = (unsigned int)m_iMaxGive;

	// populate cache

	if( !m_pCached->getItem( ann.strInfoHash ) )
		m_pCached->setItem( ann.strInfoHash, new CAtomList( ) );

	CAtom *pCache = m_pCached->getItem( ann.strInfoHash );

	if( pCache && dynamic_cast<CAtomList *>( pCache ) )
	{
		CAtomList *pCacheList = dynamic_cast<CAtomList *>( pCache );

		if( pCacheList->getValuePtr( )->size( ) < iRSize )
		{
#ifdef BNBT_MYSQL
			if( m_bMySQLOverrideDState )
			{
				CMySQLQuery *pQuery = new CMySQLQuery( "SELECT bid,bip,bport FROM dstate WHERE bhash=\'" + UTIL_StringToMySQL( ann.strInfoHash ) + "\'" );

				vector<string> vecQuery;

				while( ( vecQuery = pQuery->nextRow( ) ).size( ) == 3 )
				{
					CAtomDicti *pAdd = new CAtomDicti( );

					pAdd->setItem( "peer id", new CAtomString( vecQuery[0] ) );
					pAdd->setItem( "ip", new CAtomString( vecQuery[1] ) );
					pAdd->setItem( "port", new CAtomLong( atoi( vecQuery[2].c_str( ) ) ) );

					pCacheList->addItem( pAdd );
				}

				delete pQuery;
			}
			else
			{
#endif
				if( m_pDFile )
				{
					if( !m_pDFile->getItem( ann.strInfoHash ) )
						m_pDFile->setItem( ann.strInfoHash, new CAtomDicti( ) );

					CAtom *pPeers = m_pDFile->getItem( ann.strInfoHash );

					if( pPeers && pPeers->isDicti( ) )
					{
						map<string, CAtom *> *pmapPeersDicti = ( (CAtomDicti *)pPeers )->getValuePtr( );

						for( map<string, CAtom *> :: iterator i = pmapPeersDicti->begin( ); i != pmapPeersDicti->end( ); i++ )
						{
							if( (*i).second->isDicti( ) )
							{
								CAtomDicti *pAdd = new CAtomDicti( );

								CAtom *pIP = ( (CAtomDicti *)(*i).second )->getItem( "ip" );
								CAtom *pPort = ( (CAtomDicti *)(*i).second )->getItem( "port" );
								
								// check for oneself and remove from the list
								if( pIP && pIP->toString( ) == strIP && pPort && pPort->toString( ) == strPort )
								{
									continue;
								}
								
								// remove seeds if announce is from a seed
								if( ann.iLeft == 0 )
								{
									CAtom *pLeft = ( (CAtomDicti *)(*i).second )->getItem( "left" );

									if( pLeft && atol( pLeft->toString( ).c_str( ) ) == 0 )
									{
										continue;
									}
								}


								pAdd->setItem( "peer id", new CAtomString( (*i).first ) );

								if( pIP )
									pAdd->setItem( "ip", new CAtomString( pIP->toString( ) ) );

								if( pPort && dynamic_cast<CAtomLong *>( pPort ) )
									pAdd->setItem( "port", new CAtomLong( *dynamic_cast<CAtomLong *>( pPort ) ) );

								pCacheList->addItem( pAdd );
							}
						}
					}
				}

#ifdef BNBT_MYSQL
			}
#endif

			pCacheList->Randomize( );
		}
	}

	// clamp response

	if( pCache && dynamic_cast<CAtomList *>( pCache ) )
	{
		CAtomList *pCacheList = dynamic_cast<CAtomList *>( pCache );

		CAtom *pPeers = NULL;

		if( strCompact == "1" )
		{
			// compact announce

			string strPeers;

			vector<CAtom *> *pvecList = pCacheList->getValuePtr( );

			for( vector<CAtom *> :: iterator i = pvecList->begin( ); i != pvecList->end( ); )
			{
				if( (*i)->isDicti( ) )
				{
					if( strPeers.size( ) / 6 >= iRSize )
						break;

					CAtom *pIP = ( (CAtomDicti *)(*i) )->getItem( "ip" );
					CAtom *pPort = ( (CAtomDicti *)(*i) )->getItem( "port" );

					if( pIP && pPort && dynamic_cast<CAtomLong *>( pPort ) )
					{
						// tphogan - this is a much more efficient version of UTIL_Compact

						bool bOK = true;
						char pCompact[6];
						char szIP[18]; // Two extra to avoid a compiler warning for strncpy.
						char *szCur = szIP;
						char *pSplit;
						strncpy( szIP, pIP->toString( ).c_str( ), 16 );

						// first three octets

						for( int iThree = 0; iThree < 3; iThree++ )
						{
							if( (pSplit = strstr( szCur, "." )) )
							{
								*pSplit = '\0';
								pCompact[iThree] = (char)atoi( szCur );
								szCur = pSplit + 1;
							}
							else
							{
								bOK = false;

								break;
							}
						}

						if( bOK )
						{
							// fourth octet

							pCompact[3] = (char)atoi( szCur );

							// port

							unsigned int iPort = (unsigned int)dynamic_cast<CAtomLong *>( pPort )->getValue( );

#ifdef BNBT_BIG_ENDIAN
							pCompact[5] = (char)( ( iPort & 0xFF00 ) >> 8 );
							pCompact[4] = (char)( iPort & 0xFF );
#else
							pCompact[4] = (char)( ( iPort & 0xFF00 ) >> 8 );
							pCompact[5] = (char)( iPort & 0xFF );
#endif

							strPeers += string( pCompact, 6 );
						}
					}

					delete *i;

					i = pvecList->erase( i );
				}
				else
					i++;
			}

			pPeers = new CAtomString( strPeers );

			// don't compress

			pResponse->bCompressOK = false;
		}
		else
		{
			// regular announce

			CAtomList *pPeersList = new CAtomList( );

			vector<CAtom *> *pvecList = pCacheList->getValuePtr( );

			for( vector<CAtom *> :: iterator i = pvecList->begin( ); i != pvecList->end( ); )
			{
				if( (*i)->isDicti( ) )
				{
					if( pPeersList->getValuePtr( )->size( ) >= iRSize )
						break;

					if( strNoPeerID == "1" )
						( (CAtomDicti *)(*i) )->delItem( "peer id" );

					pPeersList->addItem( new CAtomDicti( *(CAtomDicti *)(*i) ) );

					delete *i;

					i = pvecList->erase( i );
				}
				else
					i++;
			}

			pPeers = pPeersList;
		}

		CAtomDicti *pData = new CAtomDicti( );

		// DWK - addition to support private tracker definitions.
		pData->setItem( "private", new CAtomLong( m_iPrivateTracker ) );

		pData->setItem( "interval", new CAtomLong( m_iAnnounceInterval ) );
		pData->setItem( "peers", pPeers );

		// Added by =Xotic=
		// Harold - Added patch to CVS.
		CAtom *pFC = m_pFastCache->getItem( strInfoHash );

		if( pFC && dynamic_cast<CAtomList *>( pFC ) )
		{
			vector<CAtom *> vecList = dynamic_cast<CAtomList *>( pFC )->getValue( );

			pData->setItem( "complete", new CAtomInt( *dynamic_cast<CAtomInt *>( vecList[0] ) ) );
			pData->setItem( "incomplete", new CAtomInt( *dynamic_cast<CAtomInt *>( vecList[1] ) ) );
		}
		// ----------------------- End of Addition 

		pResponse->strContent = Encode( pData );

		delete pData;
	}
}
