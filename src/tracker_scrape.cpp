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

void CTracker :: serverResponseScrape( struct request_t *pRequest, struct response_t *pResponse )
{
	if( !m_bAllowScrape )
	{
		pResponse->strCode = "403 Forbidden";

		return;
	}

	/* =X= */
	// Authorise scrape
	if( m_iAuthScrape != 0 && ( pRequest->user.iAccess & m_iAuthScrape ) )
	{
		pResponse->strCode = "401 Unauthorized";

		pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", gmapMime[".txt"] ) );
		pResponse->mapHeaders.insert( pair<string, string>( "WWW-Authenticate", string( "Basic realm=\"" ) + gstrRealm + "\"" ) );
		
		pResponse->strContent = UTIL_FailureReason( "Tracker authentication failed" );

		return;
	}

	typedef multimap<string,string> stringmap;
	pResponse->strCode = "200 OK";

	pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", gmapMime[".txt"] ) ); /* =X= */

	// retrieve info hash (note that the client sends an actual hash so don't convert it)

	string strInfoHash = pRequest->mapParams["info_hash"];

	CAtomDicti *pScrape = new CAtomDicti( );
	CAtomDicti *pFiles = new CAtomDicti( );
	CAtomDicti *pFlags = new CAtomDicti( );

	pScrape->setItem( "files", pFiles );
	pScrape->setItem( "flags", pFlags );

	pFlags->setItem( "min_request_interval", new CAtomLong( m_iMinRequestInterval ) );

	if ( !pRequest->hasQuery ){
#ifdef BNBT_MYSQL
   if( m_bMySQLOverrideDState && m_iMySQLRefreshStatsInterval > 0 )
   {
      // Modified by =Xotic= 
	  // Harold - Added MySQL fix to CVS
      string strQuery;

      strQuery = "SELECT bseeders,bleechers,bcompleted,bhash FROM torrents";

      CMySQLQuery *pQuery = new CMySQLQuery( strQuery );

      vector<string> vecQuery;

         // full
         while( ( vecQuery = pQuery->nextRow( ) ).size( ) == 4 )
         {
            CAtomDicti *pHuh = new CAtomDicti( );

            pHuh->setItem( "complete", new CAtomInt( atoi( vecQuery[0].c_str( ) ) ) );
            pHuh->setItem( "incomplete", new CAtomInt( atoi( vecQuery[1].c_str( ) ) ) );
            pHuh->setItem( "downloaded", new CAtomInt( atoi( vecQuery[2].c_str( ) ) ) );

            if( m_pAllowed )
            {
               CAtom *pList = m_pAllowed->getItem( vecQuery[3] );

               if( pList && dynamic_cast<CAtomList *>( pList ) )
               {
                  vector<CAtom *> vecTorrent = dynamic_cast<CAtomList *>( pList )->getValue( );

                  if( vecTorrent.size( ) == 6 )
                  {
                     CAtom *pName = vecTorrent[1];

                     if( pName )
                        pHuh->setItem( "name", new CAtomString( pName->toString( ) ) );
                  }
               }
            }

            pFiles->setItem( vecQuery[3], pHuh );
         }
      // ------------------ End of Modification

      delete pQuery;

      pResponse->strContent = Encode( pScrape );

      delete pScrape;

      return;
   }
#endif 
	if( m_pDFile )
	{
		if( GetTime( ) > m_iRefreshFastCacheNext )
		{
			RefreshFastCache( );

			m_iRefreshFastCacheNext = GetTime( ) + m_iRefreshFastCacheInterval;
		}

			//
			// full scrape
			//

			map<string, CAtom *> *pmapDicti = m_pFastCache->getValuePtr( );

			for( map<string, CAtom *> :: iterator i = pmapDicti->begin( ); i != pmapDicti->end( ); i++ )
			{
				if( dynamic_cast<CAtomList *>( (*i).second ) )
				{
					vector<CAtom *> vecList = dynamic_cast<CAtomList *>( (*i).second )->getValue( );

					CAtomDicti *pHuh = new CAtomDicti( );

					pHuh->setItem( "complete", new CAtomInt( *dynamic_cast<CAtomInt *>( vecList[0] ) ) );
					pHuh->setItem( "incomplete", new CAtomInt( *dynamic_cast<CAtomInt *>( vecList[1] ) ) );
					pHuh->setItem( "downloaded", new CAtomInt( *dynamic_cast<CAtomInt *>( vecList[2] ) ) );
					// Harold - Fixed stupid bug here.

					if( m_pAllowed )
					{
						CAtom *pList = m_pAllowed->getItem( (*i).first );

						if( pList && dynamic_cast<CAtomList *>( pList ) )
						{
							vector<CAtom *> vecTorrent = dynamic_cast<CAtomList *>( pList )->getValue( );

							if( vecTorrent.size( ) == 6 )
							{
								CAtom *pName = vecTorrent[1];

								if( pName )
									pHuh->setItem( "name", new CAtomString( pName->toString( ) ) );
							}
						}
					}

					pFiles->setItem( (*i).first, pHuh );
				}
			}
	}

	pResponse->strContent = Encode( pScrape );

	delete pScrape;
}
else
{
		for(stringmap::iterator pos = pRequest->multiParams.lower_bound("info_hash"); pos != pRequest->multiParams.upper_bound("info_hash"); pos++)
		{
#ifdef BNBT_MYSQL
	       if( m_bMySQLOverrideDState && m_iMySQLRefreshStatsInterval > 0 )
			{
				string strQuery;
				// Fix for %00 in infohash bug
			strQuery = "SELECT bseeders,bleechers,bcompleted FROM torrents WHERE bhash=\'" + UTIL_StringToMySQL( (*pos).second ) + "\'";
			
			CMySQLQuery *pQuery = new CMySQLQuery( strQuery ); /* =X= */

			vector<string> vecQuery; /* =X= */
			vecQuery.reserve( 6 ); /* =X= */

         // single
         while( ( vecQuery = pQuery->nextRow( ) ).size( ) == 3 )
         {
            CAtomDicti *pHuh = new CAtomDicti( );

            pHuh->setItem( "complete", new CAtomInt( atoi( vecQuery[0].c_str( ) ) ) );
            pHuh->setItem( "incomplete", new CAtomInt( atoi( vecQuery[1].c_str( ) ) ) );
            pHuh->setItem( "downloaded", new CAtomInt( atoi( vecQuery[2].c_str( ) ) ) );

            if( m_pAllowed )
            {
               CAtom *pList = m_pAllowed->getItem( strInfoHash );

               if( pList && dynamic_cast<CAtomList *>( pList ) )
               {
                  vector<CAtom *> vecTorrent = dynamic_cast<CAtomList *>( pList )->getValue( );

                  if( vecTorrent.size( ) == 6 )
                  {
                     CAtom *pName = vecTorrent[1];

                     if( pName )
                        pHuh->setItem( "name", new CAtomString( pName->toString( ) ) );
                  }
               }
            }

				// Fix for %00 in infohash bug
            pFiles->setItem( (*pos).second, pHuh );
         }
		      delete pQuery;
			}
		   else
		   {
#endif
				// Fix for %00 in infohash bug
				CAtom *pFC = m_pFastCache->getItem( (*pos).second );

			if( pFC && dynamic_cast<CAtomList *>( pFC ) )
			{
				vector<CAtom *> vecList = dynamic_cast<CAtomList *>( pFC )->getValue( );

				CAtomDicti *pHuh = new CAtomDicti( );

				pHuh->setItem( "complete", new CAtomInt( *dynamic_cast<CAtomInt *>( vecList[0] ) ) );
				pHuh->setItem( "incomplete", new CAtomInt( *dynamic_cast<CAtomInt *>( vecList[1] ) ) );
				pHuh->setItem( "downloaded", new CAtomInt( *dynamic_cast<CAtomInt *>( vecList[2] ) ) );

				if( m_pAllowed )
				{
				// Fix for %00 in infohash bug
					CAtom *pList = m_pAllowed->getItem( (*pos).second );

					if( pList && dynamic_cast<CAtomList *>( pList ) )
					{
						vector<CAtom *> vecTorrent = dynamic_cast<CAtomList *>( pList )->getValue( );

						if( vecTorrent.size( ) == 6 )
						{
							CAtom *pName = vecTorrent[1];

							if( pName )
								pHuh->setItem( "name", new CAtomString( pName->toString( ) ) );
						}
					}
				}

				// Fix for %00 in infohash bug
				pFiles->setItem( (*pos).second, pHuh );
			}
#ifdef BNBT_MYSQL
}
#endif
		}
	pResponse->strContent = Encode( pScrape );
	delete pScrape;

	}

}
