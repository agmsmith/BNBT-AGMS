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

#ifndef WIN32
 #include <sys/stat.h>
 #include <dirent.h>
#endif

#include "bnbt.h"
#include "bnbt_mysql.h"
#include "atom.h"
#include "bencode.h"
#include "config.h"
#include "md5.h"
#include "server.h"
#include "sort.h"
#include "tracker.h"
#include "util.h"

map<string, string> gmapMime;

CTracker :: CTracker( )
{
	m_strAllowedDir = CFG_GetString( "allowed_dir", string( ) );
	m_strUploadDir = CFG_GetString( "bnbt_upload_dir", string( ) );
	m_strExternalTorrentDir = CFG_GetString( "bnbt_external_torrent_dir", string( ) );
	m_strArchiveDir = CFG_GetString( "bnbt_archive_dir", string( ) );
	m_strFileDir = CFG_GetString( "bnbt_file_dir", string( ) );

	if( !m_strAllowedDir.empty( ) && m_strAllowedDir[m_strAllowedDir.size( ) - 1] != PATH_SEP )
		m_strAllowedDir += PATH_SEP;

	if( !m_strUploadDir.empty( ) && m_strUploadDir[m_strUploadDir.size( ) - 1] != PATH_SEP )
		m_strUploadDir += PATH_SEP;

	// DWK - Added support for external torrent.php and similar scripts that end with a http get paramater check
	if( !m_strExternalTorrentDir.empty( ) && m_strExternalTorrentDir[m_strExternalTorrentDir.size( ) - 1] != '/' && m_strExternalTorrentDir[m_strExternalTorrentDir.size( ) - 1] != '=' )
		m_strExternalTorrentDir += '/';

	if( !m_strArchiveDir.empty( ) && m_strArchiveDir[m_strArchiveDir.size( ) - 1] != PATH_SEP )
		m_strArchiveDir += PATH_SEP;

	if( !m_strFileDir.empty( ) && m_strFileDir[m_strFileDir.size( ) - 1] != PATH_SEP )
		m_strFileDir += PATH_SEP;

	m_strDFile = CFG_GetString( "dfile", "dstate.bnbt" );
	m_strCommentsFile = CFG_GetString( "bnbt_comments_file", string( ) );
	m_strTagFile = CFG_GetString( "bnbt_tag_file", "tags.bnbt" );
	m_strUsersFile = CFG_GetString( "bnbt_users_file", "users.bnbt" );
	m_strStaticHeaderFile = CFG_GetString( "bnbt_static_header", string( ) );
	m_strStaticFooterFile = CFG_GetString( "bnbt_static_footer", string( ) );
	m_strRobotsFile = CFG_GetString( "bnbt_robots_txt", string( ) );

	// addition by labarks

	m_strDumpRSSFile = CFG_GetString( "bnbt_rss_file", string( ) );
	m_strDumpRSSFileDir = CFG_GetString( "bnbt_rss_online_dir", string( ) );
	m_iDumpRSSFileMode = CFG_GetInt( "bnbt_rss_file_mode", 0 );
	m_strDumpRSSTitle = CFG_GetString( "bnbt_rss_channel_title", "My BNBT RSS Feed" );
	m_strDumpRSSDescription = CFG_GetString( "bnbt_rss_channel_description", "BitTorrent RSS Feed for BNBT" );
	m_iDumpRSS_TTL = CFG_GetInt( "bnbt_rss_channel_ttl", 60 );
	m_strDumpRSSLanguage = CFG_GetString( "bnbt_rss_channel_language", "en-us" );
	m_strDumpRSSImageURL = CFG_GetString( "bnbt_rss_channel_image_url", string( ) );
	m_iDumpRSSImageWidth = CFG_GetInt( "bnbt_rss_channel_image_width", 0 );
	m_iDumpRSSImageHeight = CFG_GetInt( "bnbt_rss_channel_image_height", 0 );
	m_strDumpRSSCopyright = CFG_GetString( "bnbt_rss_channel_copyright", string( ) );
	m_iDumpRSSLimit = CFG_GetInt( "bnbt_rss_limit", 25 );

	// backwards compatibility

	if( m_strDumpRSSImageURL.substr( 0, 7 ) == "/files/" )
		UTIL_LogPrint( "tracker warning - bnbt_rss_channel_image_url starts with /files/\n" );

	// end addition

	m_strImageBarFill = CFG_GetString( "image_bar_fill", string( ) );
	m_strImageBarTrans = CFG_GetString( "image_bar_trans", string( ) );
	m_strTrackerURL = CFG_GetString( "bnbt_rss_tracker_url", "http://localhost:6969/" );
	m_strForceAnnounceURL = CFG_GetString( "bnbt_force_announce_url", string( ) );
	m_bForceAnnounceOnDL = CFG_GetInt( "bnbt_force_announce_on_download", 0 ) == 0 ? false : true;
	m_iParseAllowedInterval = CFG_GetInt( "parse_allowed_interval", 0 );
	m_iSaveDFileInterval = CFG_GetInt( "save_dfile_interval", 300 );
	m_iDownloaderTimeOutInterval = CFG_GetInt( "downloader_timeout_interval", 2700 );
	m_iRefreshStaticInterval = CFG_GetInt( "bnbt_refresh_static_interval", 10 );
	m_iDumpRSSInterval = CFG_GetInt( "bnbt_rss_interval", 30 );
	m_iMySQLRefreshAllowedInterval = CFG_GetInt( "mysql_refresh_allowed_interval", 300 );
	m_iMySQLRefreshStatsInterval = CFG_GetInt( "mysql_refresh_stats_interval", 600 );
	m_iRefreshFastCacheInterval = CFG_GetInt( "bnbt_refresh_fast_cache_interval", 30 );
	m_iParseAllowedNext = GetTime( ) + m_iParseAllowedInterval * 60;
	m_iSaveDFileNext = GetTime( ) + m_iSaveDFileInterval;
	m_iPrevTime = 1;
	m_iDownloaderTimeOutNext = GetTime( ) + m_iDownloaderTimeOutInterval;
	m_iRefreshStaticNext = 0;
	m_iDumpRSSNext = 0;
	m_iMySQLRefreshAllowedNext = 0;
	m_iMySQLRefreshStatsNext = 0;
	m_iRefreshFastCacheNext = 0;
	m_iAnnounceInterval = CFG_GetInt( "announce_interval", 1800 );
	m_iMinRequestInterval = CFG_GetInt( "min_request_interval", 18000 );
	m_iResponseSize = CFG_GetInt( "response_size", 50 );
	m_iMaxGive = CFG_GetInt( "max_give", 200 );
	m_bKeepDead = CFG_GetInt( "keep_dead", 0 ) == 0 ? false : true;
	m_bAllowScrape = CFG_GetInt( "bnbt_allow_scrape", 1 ) == 0 ? false : true;
	m_bCountUniquePeers = CFG_GetInt( "bnbt_count_unique_peers", 1 ) == 0 ? false : true;
	m_bDeleteInvalid = CFG_GetInt( "bnbt_delete_invalid", 0 ) == 0 ? false : true;
	m_bParseOnUpload = CFG_GetInt( "bnbt_parse_on_upload", 1 ) == 0 ? false : true;
	m_iMaxTorrents = CFG_GetInt( "bnbt_max_torrents", 0 );
	m_bShowInfoHash = CFG_GetInt( "bnbt_show_info_hash", 0 ) == 0 ? false : true;
	m_bShowNames = CFG_GetInt( "show_names", 1 ) == 0 ? false : true;
	m_bShowStats = CFG_GetInt( "bnbt_show_stats", 1 ) == 0 ? false : true;
	m_bAllowTorrentDownloads = CFG_GetInt( "bnbt_allow_torrent_downloads", 1 ) == 0 ? false : true;
	m_bAllowComments = CFG_GetInt( "bnbt_allow_comments", 0 ) == 0 ? false : true;
	m_bShowAdded = CFG_GetInt( "bnbt_show_added", 1 ) == 0 ? false : true;
	m_bShowSize = CFG_GetInt( "bnbt_show_size", 1 ) == 0 ? false : true;
	m_bShowNumFiles = CFG_GetInt( "bnbt_show_num_files", 1 ) == 0 ? false : true;
	m_bShowCompleted = CFG_GetInt( "bnbt_show_completed", 0 ) == 0 ? false : true;
	m_bShowTransferred = CFG_GetInt( "bnbt_show_transferred", 0 ) == 0 ? false : true;
	m_bShowMinLeft = CFG_GetInt( "bnbt_show_min_left", 0 ) == 0 ? false : true;
	m_bShowAverageLeft = CFG_GetInt( "bnbt_show_average_left", 0 ) == 0 ? false : true;
	m_bShowMaxiLeft = CFG_GetInt( "bnbt_show_max_left", 0 ) == 0 ? false : true;
	m_bShowLeftAsProgress = CFG_GetInt( "bnbt_show_left_as_progress", 1 ) == 0 ? false : true;
	m_bShowUploader = CFG_GetInt( "bnbt_show_uploader", 0 ) == 0 ? false : true;
	m_bAllowInfoLink = CFG_GetInt( "bnbt_allow_info_link", 0 ) == 0 ? false : true;
	m_bSearch = CFG_GetInt( "bnbt_allow_search", 1 ) == 0 ? false : true;
	m_bSort = CFG_GetInt( "bnbt_allow_sort", 1 ) == 0 ? false : true;
	m_bShowFileComment = CFG_GetInt( "bnbt_show_file_comment", 1 ) == 0 ? false : true;
	m_bShowFileContents = CFG_GetInt( "bnbt_show_file_contents", 0 ) == 0 ? false : true;
	m_bShowShareRatios = CFG_GetInt( "bnbt_show_share_ratios", 1 ) == 0 ? false : true;
	m_bShowAvgDLRate = CFG_GetInt( "bnbt_show_average_dl_rate", 0 ) == 0 ? false : true;
	m_bShowAvgULRate = CFG_GetInt( "bnbt_show_average_ul_rate", 0 ) == 0 ? false : true;
	m_bDeleteOwnTorrents = CFG_GetInt( "bnbt_delete_own_torrents", 1 ) == 0 ? false : true;
	m_bGen = CFG_GetInt( "bnbt_show_gen_time", 1 ) == 0 ? false : true;
	m_bMySQLOverrideDState = CFG_GetInt( "mysql_override_dstate", 0 ) == 0 ? false : true;
	m_iPerPage = CFG_GetInt( "bnbt_per_page", 20 );
	m_iUsersPerPage = CFG_GetInt( "bnbt_users_per_page", 50 );
	m_iMaxPeersDisplay = CFG_GetInt( "bnbt_max_peers_display", 500 );
	m_iGuestAccess = CFG_GetInt( "bnbt_guest_access", ACCESS_VIEW | ACCESS_DL | ACCESS_SIGNUP );
	m_iMemberAccess = CFG_GetInt( "bnbt_member_access", ACCESS_VIEW | ACCESS_DL | ACCESS_COMMENTS | ACCESS_UPLOAD | ACCESS_SIGNUP );
	m_iFileExpires = CFG_GetInt( "bnbt_file_expires", 180 );
	m_iNameLength = CFG_GetInt( "bnbt_name_length", 32 );
	m_iCommentLength = CFG_GetInt( "bnbt_comment_length", 800 );

	// Harold - XML Dump
	m_strDumpXMLFile = CFG_GetString( "bnbt_dump_xml_file", string( ) );
	m_iDumpXMLInterval = CFG_GetInt( "bnbt_dump_xml_interval", 600 );
	m_bDumpXMLPeers = CFG_GetInt( "bnbt_dump_xml_peers", 1 ) == 0 ? false : true;
	m_iDumpXMLNext = GetTime( ) + m_iDumpXMLInterval;

	// Harold - NAT IP Configuation
	m_iBlockNATedIP = CFG_GetInt( "bnbt_block_private_ip", 0 );
	// adopted official tracker config valuse
	m_iLocalOnly = CFG_GetInt( "only_local_override_ip", 0 );
	// Private Flag for announce replies
	m_iPrivateTracker = CFG_GetInt( "bnbt_private_tracker_flag", 0 );

	/* =X= */
	// Favicon support
	m_strFaviconFile = CFG_GetString( "favicon", string( ) ); 
	m_strFavicon = string( );

	/* =X= */
	// Internalised mouseover
	m_bUseMouseovers = CFG_GetInt( "bnbt_use_mouseovers", 0 ) == 0 ? false : true;

	/* =X= */
	// Custom scrape and announce strings
	m_strCustomAnnounce = CFG_GetString( "bnbt_custom_announce", string( ) ); 
	m_strCustomScrape = CFG_GetString( "bnbt_custom_scrape", string( ) ); 

	/* =X= */
	// Announce and scrape authorisation
	m_iAuthAnnounce = CFG_GetInt( "bnbt_announce_access_required", 0 );
	m_iAuthScrape = CFG_GetInt( "bnbt_scrape_access_required", 0 );

	// tags

	int iTag = 1;

	string strName = "bnbt_tag" + CAtomInt( iTag ).toString( );
	string strTag = CFG_GetString( strName, string( ) );
	
	m_vecTags.reserve(8);
	m_vecTagsMouse.reserve(8);

	while( !strTag.empty( ) )
	{
		string :: size_type iSplit = strTag.find( "|" );
		string :: size_type iSplitMouse = strTag.find( "|", iSplit + 1 );

		strName = "bnbt_tag" + CAtomInt( ++iTag ).toString( );

		if( iSplit == string :: npos ) 
		{
			if( iSplitMouse == string :: npos )
			{
				m_vecTags.push_back( pair<string, string>( strTag, string( ) ) );
				m_vecTagsMouse.push_back( pair<string, string>( strTag, string( ) ) );
			}
			else
			{
				m_vecTags.push_back( pair<string, string>( strTag.substr( 0, iSplitMouse ), string( ) ) );
				m_vecTagsMouse.push_back( pair<string, string>( strTag.substr( 0, iSplitMouse ), strTag.substr( iSplitMouse + 1) ) );
			}
		}
		else
		{
			if( iSplitMouse == string :: npos )
			{
				m_vecTags.push_back( pair<string, string>( strTag.substr( 0, iSplit ), strTag.substr( iSplit + 1 ) ) );
				m_vecTagsMouse.push_back( pair<string, string>( strTag.substr( 0, iSplit ), string( ) ) );
			}
			else
			{
				m_vecTags.push_back( pair<string, string>( strTag.substr( 0, iSplit ), strTag.substr( iSplit + 1, iSplitMouse - iSplit - 1 ) ) );
				m_vecTagsMouse.push_back( pair<string, string>( strTag.substr( 0, iSplit ), strTag.substr( iSplitMouse + 1 ) ) );
			}
		}   
		strTag = CFG_GetString( strName, string( ) );
	}

	m_pAllowed = NULL;
	m_pTimeDicti = new CAtomDicti( );
	m_pCached = new CAtomDicti( );
	m_pIPs = new CAtomDicti( );

	// decode dfile

	CAtom *pState = DecodeFile( m_strDFile.c_str( ) );

	if( pState && pState->isDicti( ) )
	{
		m_pState = (CAtomDicti *)pState;

		if( m_pState->getItem( "peers" ) && m_pState->getItem( "completed" ) )
		{
			CAtom *pDFile = m_pState->getItem( "peers" );

			if( pDFile && pDFile->isDicti( ) )
				m_pDFile = (CAtomDicti *)pDFile;

			CAtom *pCompleted = m_pState->getItem( "completed" );

			if( pCompleted && pCompleted->isDicti( ) )
				m_pCompleted = (CAtomDicti *)pCompleted;
		}
		else
		{
			if( pState )
				delete pState;

			m_pState = new CAtomDicti( );
			m_pDFile = new CAtomDicti( );
			m_pCompleted = new CAtomDicti( );

			m_pState->setItem( "peers", m_pDFile );
			m_pState->setItem( "completed", m_pCompleted );
		}

		// populate time dicti

		map<string, CAtom *> *pmapDicti = m_pDFile->getValuePtr( );

		for( map<string, CAtom *> :: iterator i = pmapDicti->begin( ); i != pmapDicti->end( ); i++ )
		{
			CAtomDicti *pTS = new CAtomDicti( );

			if( (*i).second->isDicti( ) )
			{
				CAtomDicti *pPeersDicti = (CAtomDicti *)(*i).second;

				map<string, CAtom *> *pmapPeersDicti = pPeersDicti->getValuePtr( );

				for( map<string, CAtom *> :: iterator j = pmapPeersDicti->begin( ); j != pmapPeersDicti->end( ); j++ )
					pTS->setItem( (*j).first, new CAtomLong( 0 ) );

				// reset connected times

				for( map<string, CAtom *> :: iterator j = pmapPeersDicti->begin( ); j != pmapPeersDicti->end( ); j++ )
				{
					if( (*j).second->isDicti( ) )
					{
						CAtomDicti *pPeerDicti = (CAtomDicti *)(*j).second;

						if( pPeerDicti )
							pPeerDicti->setItem( "connected", new CAtomLong( 0 ) );
					}
				}
			}

			m_pTimeDicti->setItem( (*i).first, pTS );
		}
	}
	else
	{
		if( pState )
			delete pState;

		m_pState = new CAtomDicti( );
		m_pDFile = new CAtomDicti( );
		m_pCompleted = new CAtomDicti( );

		m_pState->setItem( "peers", m_pDFile );
		m_pState->setItem( "completed", m_pCompleted );
	}

	// decode comments file

	if( m_strCommentsFile.empty( ) )
		m_pComments = new CAtomDicti( );
	else
	{
		CAtom *pComments = DecodeFile( m_strCommentsFile.c_str( ) );

		if( pComments && pComments->isDicti( ) )
			m_pComments = (CAtomDicti *)pComments;
		else
		{
			if( pComments )
				delete pComments;

			m_pComments = new CAtomDicti( );
		}
	}

	// decode tag file

	if( m_strTagFile.empty( ) )
		m_pTags = new CAtomDicti( );
	else
	{
		CAtom *pTags = DecodeFile( m_strTagFile.c_str( ) );

		if( pTags && pTags->isDicti( ) )
			m_pTags = (CAtomDicti *)pTags;
		else
		{
			if( pTags )
				delete pTags;

			m_pTags = new CAtomDicti( );
		}
	}

	// decode users file

	if( m_strUsersFile.empty( ) )
		m_pUsers = new CAtomDicti( );
	else
	{
		CAtom *pUsers = DecodeFile( m_strUsersFile.c_str( ) );

		if( pUsers && pUsers->isDicti( ) )
			m_pUsers = (CAtomDicti *)pUsers;
		else
		{
			if( pUsers )
				delete pUsers;

			m_pUsers = new CAtomDicti( );
		}
	}

	if( m_bCountUniquePeers )
		CountUniquePeers( );

	m_pFastCache = new CAtomDicti( );

	// parse allowed dir

	if( !m_strAllowedDir.empty( ) )
		parseTorrents( m_strAllowedDir.c_str( ) );

	// mime

	gmapMime[".hqx"]		= "application/mac-binhex40";
	gmapMime[".exe"]		= "application/octet-stream";
	gmapMime[".pdf"]		= "application/pdf";
	gmapMime[".rss"]		= "application/rss+xml";
	gmapMime[".torrent"]	= "application/x-bittorrent";
	gmapMime[".gtar"]		= "application/x-gtar";
	gmapMime[".gz"]			= "application/x-gzip";
	gmapMime[".js"]			= "application/x-javascript";
	gmapMime[".swf"]		= "application/x-shockwave-flash";
	gmapMime[".sit"]		= "application/x-stuffit";
	gmapMime[".tar"]		= "application/x-tar";
	gmapMime[".xml"]		= "application/xml";
	gmapMime[".zip"]		= "application/zip";
	gmapMime[".bmp"]		= "image/bmp";
	gmapMime[".gif"]		= "image/gif";
	gmapMime[".jpg"]		= "image/jpeg";
	gmapMime[".jpeg"]		= "image/jpeg";
	gmapMime[".jpe"]		= "image/jpeg";
	gmapMime[".png"]		= "image/png";
	gmapMime[".tiff"]		= "image/tiff";
	gmapMime[".tif"]		= "image/tiff";
	gmapMime[".ico"]		= "image/vnd.microsoft.icon"; /* =X= */
	gmapMime[".css"]		= "text/css";
	gmapMime[".html"]		= "text/html";
	gmapMime[".htm"]		= "text/html";
	gmapMime[".txt"]		= "text/plain";
	gmapMime[".rtf"]		= "text/rtf";
}

CTracker :: ~CTracker( )
{
	delete m_pAllowed;
	delete m_pState;
	delete m_pTimeDicti;
	delete m_pCached;
	delete m_pComments;
	delete m_pTags;
	delete m_pUsers;
	delete m_pIPs;
	delete m_pFastCache;

	m_pAllowed = NULL;
	m_pState = NULL;
	m_pDFile = NULL;		// don't delete
	m_pCompleted = NULL;	// don't delete
	m_pTimeDicti = NULL;
	m_pCached = NULL;
	m_pComments = NULL;
	m_pTags = NULL;
	m_pUsers = NULL;
	m_pIPs = NULL;
	m_pFastCache = NULL;
}

void CTracker :: saveDFile( )
{
	string strData = Encode( m_pState );

	FILE *pFile = NULL;

	if( ( pFile = fopen( m_strDFile.c_str( ), "wb" ) ) == NULL )
	{
		UTIL_LogPrint( "tracker warning - unable to open %s for writing\n", m_strDFile.c_str( ) );

		return;
	}

	fwrite( strData.c_str( ), sizeof( char ), strData.size( ), pFile );
	fclose( pFile );
}

void CTracker :: saveComments( )
{
	if( m_strCommentsFile.empty( ) )
		return;

	string strData = Encode( m_pComments );

	FILE *pFile = NULL;

	if( ( pFile = fopen( m_strCommentsFile.c_str( ), "wb" ) ) == NULL )
	{
		UTIL_LogPrint( "tracker warning - unable to open %s for writing\n", m_strCommentsFile.c_str( ) );

		return;
	}

	fwrite( strData.c_str( ), sizeof( char ), strData.size( ), pFile );
	fclose( pFile );
}

void CTracker :: saveTags( )
{
	string strData = Encode( m_pTags );

	FILE *pFile = NULL;

	if( ( pFile = fopen( m_strTagFile.c_str( ), "wb" ) ) == NULL )
	{
		UTIL_LogPrint( "tracker warning - unable to open %s for writing\n", m_strTagFile.c_str( ) );

		return;
	}

	fwrite( strData.c_str( ), sizeof( char ), strData.size( ), pFile );
	fclose( pFile );
}

void CTracker :: saveUsers( )
{
	string strData = Encode( m_pUsers );

	FILE *pFile = NULL;

	if( ( pFile = fopen( m_strUsersFile.c_str( ), "wb" ) ) == NULL )
	{
		UTIL_LogPrint( "tracker warning - unable to open %s for writing\n", m_strUsersFile.c_str( ) );

		return;
	}

	fwrite( strData.c_str( ), sizeof( char ), strData.size( ), pFile );
	fclose( pFile );
}

// DWKnight - Readding XML
void CTracker :: saveXML( )
{
	string strData;
	string tmpData;
	int iDL;
	int iComplete;
	int iCompleted;

	strData += "<?xml version=\"1.0\" encoding=\"" + gstrCharSet + "\"?>\n";

	strData += "<torrents>\n";

	if( m_pDFile )
	{
		map<string, CAtom *> *pmapDicti = m_pDFile->getValuePtr( );

		for( map<string, CAtom *> :: iterator i = pmapDicti->begin( ); i != pmapDicti->end( ); i++ )
		{
			strData += "<torrent hash=\"";
			strData += UTIL_HashToString( (*i).first );
			strData += "\"";

			if( m_pAllowed )
			{
				CAtom *pList = m_pAllowed->getItem( (*i).first );

				if( pList && dynamic_cast< CAtomList *>(pList) )
				{
					vector<CAtom *> vecTorrent = ( (CAtomList *)pList )->getValue( );

					if( vecTorrent.size( ) == 6 )
					{
						CAtom *pFileName = vecTorrent[0];
						CAtom *pName = vecTorrent[1];
						CAtom *pAdded = vecTorrent[2];
						CAtom *pSize = vecTorrent[3];
						CAtom *pFiles = vecTorrent[4];

						if( pFileName )
							strData += " filename=\"" + UTIL_RemoveHTML( pFileName->toString( ) ) + "\"";

						if( pName )
							strData += " name=\"" + UTIL_RemoveHTML( pName->toString( ) ) + "\"";

						if( pAdded )
							strData += " added=\"" + pAdded->toString( ) + "\"";

						if( pSize )
							strData += " size=\"" + pSize->toString( ) + "\"";

						if( pFiles )
							strData += " files=\"" + pFiles->toString( ) + "\"";
					}
				}
			}

			iCompleted = 0;

			if( m_pCompleted )
			{
				CAtom *pCompleted = m_pCompleted->getItem( (*i).first );

				if( pCompleted && dynamic_cast< CAtomLong *>(pCompleted) )
					iCompleted = (int)( (CAtomLong *)pCompleted )->getValue( );
			}
			strData += " completed=\"" + CAtomInt( iCompleted ).toString( ) + "\"";

			if( m_pTags )
			{
				CAtom *pDicti = m_pTags->getItem( (*i).first );

				if( pDicti && pDicti->isDicti( ) )
				{
					CAtom *pTag = ( (CAtomDicti *)pDicti )->getItem( "tag" );
					CAtom *pName = ( (CAtomDicti *)pDicti )->getItem( "name" );
					CAtom *pUploader = ( (CAtomDicti *)pDicti )->getItem( "uploader" );

					if( pTag )
						strData += " tag=\"" + UTIL_RemoveHTML( pTag->toString( ) ) + "\"";

					if( pName )
						strData += " uploadname=\"" + UTIL_RemoveHTML( pName->toString( ) ) + "\"";

					if( pUploader )
						strData += " uploader=\"" + UTIL_RemoveHTML( pUploader->toString( ) ) + "\"";
				}
			}

			tmpData = "";
			iDL = 0;
			iComplete = 0;

			if( (*i).second->isDicti( ) )
			{
				map<string, CAtom *> *pmapPeersDicti = ( (CAtomDicti *)(*i).second )->getValuePtr( );

				if( pmapPeersDicti->empty( ) )
					tmpData += "<peers />\n";
				else
				{
					tmpData += "<peers>\n";

					for( map<string, CAtom *> :: iterator j = pmapPeersDicti->begin( ); j != pmapPeersDicti->end( ); j++ )
					{
						if( (*j).second->isDicti( ) )
						{
							CAtomDicti *pPeerDicti = (CAtomDicti *)(*j).second;

							CAtom *pIP = pPeerDicti->getItem( "ip" );
							CAtom *pUpped = pPeerDicti->getItem( "uploaded" );
							CAtom *pDowned = pPeerDicti->getItem( "downloaded" );
							CAtom *pLef = pPeerDicti->getItem( "left" );
							CAtom *pConn = pPeerDicti->getItem( "connected" );

							if( ( (CAtomLong *)pLef )->getValue( ) == 0 )
								iComplete++;
							else
								iDL++;

							tmpData += "<peer id=\"";
							tmpData += UTIL_HashToString( (*j).first );
							tmpData += "\"";

							if( pIP )
								tmpData += " ip=\"" + pIP->toString( ) + "\"";

							if( pUpped )
								tmpData += " uploaded=\"" + pUpped->toString( ) + "\"";

							if( pDowned )
								tmpData += " downloaded=\"" + pDowned->toString( ) + "\"";

							if( pLef && dynamic_cast< CAtomLong *>(pLef) )
								tmpData += " left=\"" + pLef->toString( ) + "\"";

							if( pConn && dynamic_cast< CAtomLong *>(pConn) )
								tmpData += " connected=\"" + CAtomLong( GetTime( ) - (unsigned long)( (CAtomLong *)pConn )->getValue( ) ).toString( ) + "\"";

							tmpData += " />\n";
						}
					}

					tmpData += "</peers>\n";
				}
			}

			strData += " leecher=\""+ CAtomInt( iDL ).toString( ) + "\"";
			strData += " seeder=\""+ CAtomInt( iComplete ).toString( ) + "\"";
			strData += ">\n";
			if( m_bDumpXMLPeers )
				strData += tmpData;
			else
				strData += "<peers />\n";
			strData += "</torrent>\n";
		}
	}

	strData += "</torrents>\n";

	FILE *pFile = NULL;

	if( ( pFile = fopen( m_strDumpXMLFile.c_str( ), "wb" ) ) == NULL )
	{
		UTIL_LogPrint( "tracker warning - unable to open %s for writing\n", m_strDumpXMLFile.c_str( ) );

		return;
	}

	fwrite( strData.c_str( ), sizeof( char ), strData.size( ), pFile );
	fclose( pFile );
}



// addition by labarks - modified by Trevor Hogan

void CTracker :: saveRSS( string strChannelTag )
{
	string strData;

	strData += "<?xml version=\"1.0\" encoding=\"" + gstrCharSet + "\" ?>\n";
	strData += "<rss version=\"2.0\">\n";
	strData += "<channel>\n";
	strData += "<title>" + m_strDumpRSSTitle + ( !strChannelTag.empty( ) ? " - " + strChannelTag : string( ) ) + "</title>\n";
	strData += "<link>" + m_strTrackerURL + "</link>\n";
	strData += "<description>" + m_strDumpRSSDescription + "</description>\n";

	if( !strChannelTag.empty( ) )
		strData += "<category domain=\"" + m_strTrackerURL + "index.html?filter=" + UTIL_StringToEscaped( strChannelTag ) + "\">" + strChannelTag + "</category>\n";

	// image tag

	if( !m_strDumpRSSImageURL.empty( ) )
	{
		strData += "<image>\n";
		strData += "<url>" + m_strDumpRSSImageURL + "</url>\n";
		strData += "<title>" + m_strDumpRSSTitle + "</title>\n";
		strData += "<description>" + m_strDumpRSSDescription + "</description>\n";
		strData += "<link>" + m_strTrackerURL + "</link>\n";

		if( m_iDumpRSSImageWidth > 0 )
			strData += "<width>" + CAtomInt( m_iDumpRSSImageWidth ).toString( ) + "</width>\n";

		if( m_iDumpRSSImageHeight > 0 )
			strData += "<height>" + CAtomInt( m_iDumpRSSImageHeight ).toString( ) + "</height>\n";

		strData += "</image>\n";
	}

	if( !m_strDumpRSSCopyright.empty( ) )
		strData += "<copyright>" + m_strDumpRSSCopyright + "</copyright>\n";

	// format date

	time_t tNow = time( NULL );

	char pTime[256];
	memset( pTime, 0, sizeof( char ) * 256 );
	strftime( pTime, sizeof( char ) * 256, "%a, %d %b %Y %H:%M:%S", localtime( &tNow ) );

#if defined( __APPLE__ ) || defined( __FREEBSD__ )
	long timezone = -( localtime( &tNow )->tm_gmtoff );
#endif

	string strDate = pTime;

	// timezone has the wrong sign, change it

	if( timezone > 0 )
		strDate += " -";
	else
		strDate += " +";

	if( abs( (int) timezone / 3600 ) % 60 < 10 )
		strDate += "0";
		
	strDate += CAtomInt( abs( (int) timezone / 3600 ) % 60 ).toString( );

	if( abs( (int) timezone / 60 ) % 60 < 10 )
		strDate += "0";

	strDate += CAtomInt( abs( (int) timezone / 60 ) % 60 ).toString( );

	// more tags

	strData += "<pubDate>" + strDate + "</pubDate>\n";
	strData += "<language>" + m_strDumpRSSLanguage + "</language>\n";
	strData += "<ttl>" + CAtomInt( m_iDumpRSS_TTL ).toString( ) + "</ttl>\n";
	strData += "<generator>BNBT " + string( BNBT_VER ) + "</generator>\n";

	// retrieve torrent data

	if( m_pDFile )
	{
		map<string, CAtom *> *pmapDicti = m_pDFile->getValuePtr( );

		unsigned long iKeySize = (unsigned long)pmapDicti->size( );

		// Add the torrents into this structure one by one and sort it
		// afterwards.  Because the torrent_t structure includes modern
		// (circa 2018) STL strings, it can no longer be copied with a
		// memcpy, so we have a second array of pointers to elements in
		// the array and sort that.  AGMS20180929

		struct torrent_t *aTorrents = new struct torrent_t[iKeySize];
		struct torrent_t **pTorrents = new struct torrent_t * [iKeySize];
		unsigned long int iTorrent;
		for( iTorrent = 0; iTorrent < iKeySize; iTorrent++ )
			pTorrents[iTorrent] = aTorrents + iTorrent;

		unsigned long torrent_iter = 0;

		for( map<string, CAtom *> :: iterator it = pmapDicti->begin( ); it != pmapDicti->end( ); it++ )
		{
			pTorrents[torrent_iter]->strInfoHash = UTIL_HashToString( (*it).first );
			pTorrents[torrent_iter]->strName = "unknown";
			pTorrents[torrent_iter]->strLowerName = "unknown";
			pTorrents[torrent_iter]->iSeeders = 0;
			pTorrents[torrent_iter]->iLeechers = 0;
			pTorrents[torrent_iter]->iCompleted = 0;
			pTorrents[torrent_iter]->iTransferred = 0;
			pTorrents[torrent_iter]->iSize = 0;
			pTorrents[torrent_iter]->iFiles = 0;
			pTorrents[torrent_iter]->iComments = 0;
			pTorrents[torrent_iter]->iAverageLeft = 0;
			pTorrents[torrent_iter]->iAverageLeftPercent = 0;
			pTorrents[torrent_iter]->iMinLeft = 0;
			pTorrents[torrent_iter]->iMaxiLeft = 0;

			if( m_pAllowed )
			{
				CAtom *pList = m_pAllowed->getItem( (*it).first );

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
							pTorrents[torrent_iter]->strFileName = UTIL_RemoveHTML( pFileName->toString( ) );

						if( pName )
							pTorrents[torrent_iter]->strName = UTIL_RemoveHTML( pName->toString( ) );

						if( pAdded )
							pTorrents[torrent_iter]->strAdded = pAdded->toString( );

						if( pSize )
							pTorrents[torrent_iter]->iSize = dynamic_cast<CAtomLong *>( pSize )->getValue( );
						
						if( pFiles )
							pTorrents[torrent_iter]->iFiles = (unsigned int)dynamic_cast<CAtomInt *>( pFiles )->getValue( );
					}
				}
			}

			if( m_pTags )
			{
				CAtom *pDicti = m_pTags->getItem( (*it).first );

				if( pDicti && pDicti->isDicti( ) )
				{
					CAtom *pTag = ( (CAtomDicti *)pDicti )->getItem( "tag" );
					CAtom *pName = ( (CAtomDicti *)pDicti )->getItem( "name" );
					CAtom *pUploader = ( (CAtomDicti *)pDicti )->getItem( "uploader" );
					CAtom *pInfoLink = ( (CAtomDicti *)pDicti )->getItem( "infolink" );

					if( pTag )
						pTorrents[torrent_iter]->strTag = UTIL_RemoveHTML( pTag->toString( ) );

					if( pName )
						pTorrents[torrent_iter]->strName = UTIL_RemoveHTML( pName->toString( ) );

					if( pUploader )
						pTorrents[torrent_iter]->strUploader = UTIL_RemoveHTML( pUploader->toString( ) );

					if( pInfoLink )
						pTorrents[torrent_iter]->strInfoLink = pInfoLink->toString( );
				}
			}

			torrent_iter++;
		}

		qsort( pTorrents, iKeySize, sizeof( pTorrents[0] ), asortByName );

		unsigned long intLimit;

		if( m_iDumpRSSLimit == 0 || (unsigned long)m_iDumpRSSLimit > iKeySize )
			intLimit = iKeySize;
		else
			intLimit = m_iDumpRSSLimit;

		for( unsigned long i = 0; i < intLimit; i++ )
		{
			if( strChannelTag.empty( ) || strChannelTag == pTorrents[i]->strTag )
			{
				string strTorrentLink;

				if( m_strExternalTorrentDir.empty( ) )
					strTorrentLink = m_strTrackerURL + "torrents/" + pTorrents[i]->strInfoHash + ".torrent";
				else
					strTorrentLink = m_strExternalTorrentDir + UTIL_StringToEscapedStrict( pTorrents[i]->strFileName );

				int iFileSize = (int) UTIL_SizeFile( string( m_strAllowedDir + pTorrents[i]->strFileName ).c_str( ) );

				string strInfoLink;

				if( !pTorrents[i]->strInfoLink.empty( ) )
					strInfoLink = "<br/>\nInfo: <a href=\"" + pTorrents[i]->strInfoLink + "\">" + pTorrents[i]->strInfoLink + "</a>";

				strData += "<item>\n";
				strData += "<title>" + pTorrents[i]->strName + "</title>\n";
				strData += "<link>" + strTorrentLink + "</link>\n";

				// description

				strData += "<description>";
				strData += "<![CDATA[Name: " + pTorrents[i]->strName + "<br/>\n";
				strData += "Info_hash: " + pTorrents[i]->strInfoHash + "<br/>\n";
				strData += "Torrent: <a href=\"" + strTorrentLink + "\">" + strTorrentLink + "</a>";

				if( m_bShowUploader && !pTorrents[i]->strUploader.empty( ) )
					strData += "<br/>\nUploader: " + pTorrents[i]->strUploader;

				if( !pTorrents[i]->strTag.empty( ) )
					strData += "<br/>\nTag: " + pTorrents[i]->strTag;

				strData += "<br/>\nSize: " + UTIL_BytesToString( pTorrents[i]->iSize );
				strData += "<br/>\nFiles: " + CAtomInt( pTorrents[i]->iFiles ).toString( );
				strData += strInfoLink + "]]>";
				strData += "</description>\n";

				if( m_bAllowComments )
					strData += "<comments>" + m_strTrackerURL + "comments.html?info_hash=" + pTorrents[i]->strInfoHash + "</comments>\n";

				if( !pTorrents[i]->strTag.empty( ) )
					strData += "<category domain=\"" + m_strTrackerURL + "index.html?filter=" + UTIL_StringToEscaped( pTorrents[i]->strTag ) + "\">" + pTorrents[i]->strTag + "</category>\n";

				strData += "<enclosure url=\"" + strTorrentLink + "\" type=\"application/x-bittorrent\" length=\"" + CAtomInt( iFileSize ).toString( ) + "\" />\n";
				strData += "<guid isPermaLink=\"true\">" + m_strTrackerURL + "stats.html?info_hash=" + pTorrents[i]->strInfoHash + "</guid>\n";
				strData += "<pubDate>" + UTIL_AddedToDate( pTorrents[i]->strAdded ) + "</pubDate>\n";
				strData += "</item>\n";
			}
		}

		delete [] pTorrents;
		delete [] aTorrents;
	}

	strData += "</channel>\n";
	strData += "</rss>";

	FILE *pFile = NULL;

	string strRSSFile = m_strDumpRSSFile;

	if( !strChannelTag.empty( ) )
	{
		string :: size_type iExt = m_strDumpRSSFile.rfind( "." );

		string strExt;

		if( iExt != string :: npos )
			strExt = m_strDumpRSSFile.substr( iExt );

		strRSSFile = m_strDumpRSSFile.substr( 0, m_strDumpRSSFile.length( ) - strExt.length( ) ) + "-" + strChannelTag + strExt;
	}

	if( ( pFile = fopen( strRSSFile.c_str( ), "wb" ) ) == NULL )
	{
		UTIL_LogPrint( "tracker warning - unable to open %s for writing\n", strRSSFile.c_str( ) );

		return;
	}

	fwrite( strData.c_str( ), sizeof( char ), strData.size( ), pFile );
	fclose( pFile );
}

// end addition

void CTracker :: expireDownloaders( )
{
#ifdef BNBT_MYSQL
	if( m_bMySQLOverrideDState )
	{
		string strQuery;

		strQuery += "DELETE FROM dstate WHERE btime<NOW()-INTERVAL ";
		strQuery += CAtomInt( m_iDownloaderTimeOutInterval ).toString( );
		strQuery += " SECOND";

		CMySQLQuery mq01( strQuery );

		return;
	}
#endif

	if( m_pTimeDicti )
	{
		map<string, CAtom *> *pmapTimeDicti = m_pTimeDicti->getValuePtr( );

		for( map<string, CAtom *> :: iterator i = pmapTimeDicti->begin( ); i != pmapTimeDicti->end( ); i++ )
		{
			if( (*i).second->isDicti( ) )
			{
				map<string, CAtom *> *pmapPeersDicti = ( (CAtomDicti *)(*i).second )->getValuePtr( );

				for( map<string, CAtom *> :: iterator j = pmapPeersDicti->begin( ); j != pmapPeersDicti->end( ); )
				{
					if( dynamic_cast<CAtomLong *>( (*j).second ) && dynamic_cast<CAtomLong *>( (*j).second )->getValue( ) < (long) m_iPrevTime )
					{
						if( m_pDFile )
						{
							CAtom *pPeers = m_pDFile->getItem( (*i).first );

							if( pPeers && pPeers->isDicti( ) )
								( (CAtomDicti *)pPeers )->delItem( (*j).first );
						}

						delete (*j).second;

						pmapPeersDicti->erase( j++ );
					}
					else
						j++;
				}
			}
		}

		CountUniquePeers( );

		m_iPrevTime = GetTime( );

		if( m_bKeepDead )
			return;

		// delete empty hashes

		if( m_pDFile )
		{
			map<string, CAtom *> *pmapDicti = m_pDFile->getValuePtr( );

			for( map<string, CAtom *> :: iterator i = pmapDicti->begin( ); i != pmapDicti->end( ); )
			{
				if( (*i).second->isDicti( ) && ( (CAtomDicti *)(*i).second )->isEmpty( ) )
				{
					m_pTimeDicti->delItem( (*i).first );

					delete (*i).second;

					pmapDicti->erase( i++ );
				}
				else
					i++;
			}
		}
	}
}

void CTracker :: parseTorrents( const char *szDir )
{
	CAtomDicti *pAllowed = new CAtomDicti( );

#ifdef WIN32
	char szMatch[1024];
	memset( szMatch, 0, sizeof( char ) * 1024 );
	strcpy( szMatch, szDir );
	strcat( szMatch, "*.torrent" );

	WIN32_FIND_DATA fdt;

	HANDLE hFind = FindFirstFile( szMatch, &fdt );

	if( hFind != INVALID_HANDLE_VALUE )
	{
		do
#else
	DIR *pDir = opendir( szDir );

	struct dirent *dp;

	if( pDir )
	{
		while( ( dp = readdir( pDir ) ) )
#endif
		{
			// let the server accept new connections while parsing

			if( gpServer )
				gpServer->Update( false );

			char szFile[1024];
			memset( szFile, 0, sizeof( char ) * 1024 );
			strcpy( szFile, szDir );

#ifdef WIN32
			string strFileName = fdt.cFileName;
#else
			string strFileName = dp->d_name;
#endif

			strcat( szFile, strFileName.c_str( ) );

#ifndef WIN32
			if( strlen( szFile ) > strlen( ".torrent" ) )
			{
				if( strcmp( szFile + strlen( szFile ) - strlen( ".torrent" ), ".torrent" ) )
					continue;
			}
			else
				continue;
#endif

			CAtom *pFile = DecodeFile( szFile );

			if( pFile && pFile->isDicti( ) )
			{
				CAtom *pInfo = ( (CAtomDicti *)pFile )->getItem( "info" );

				if( pInfo && pInfo->isDicti( ) )
				{
					CAtomDicti *pInfoDicti = (CAtomDicti *)pInfo;

					string strHash = UTIL_InfoHash( pFile );

					CAtom *pName = pInfoDicti->getItem( "name" );
					CAtom *pLen = pInfoDicti->getItem( "length" );
					CAtom *pFiles = pInfoDicti->getItem( "files" );

					if( pName && ( ( pLen && dynamic_cast<CAtomLong *>( pLen ) ) || ( pFiles && dynamic_cast<CAtomList *>( pFiles ) ) ) )
					{
						CAtomList *pList = new CAtomList( );

						//
						// filename
						//

						pList->addItem( new CAtomString( strFileName ) );

						//
						// name
						//

						pList->addItem( new CAtomString( pName->toString( ) ) );

						//
						// added time (i.e. modification time)
						//

						char pTime[256];
						memset( pTime, 0, sizeof( char ) * 256 );

#ifdef WIN32
						FILETIME ft;
						SYSTEMTIME st;

						FileTimeToLocalFileTime( &fdt.ftLastWriteTime, &ft );
						FileTimeToSystemTime( &ft, &st );

						sprintf( pTime, "%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );
#else
						struct stat info;

						stat( szFile, &info );

						strftime( pTime, sizeof( char ) * 256, "%Y-%m-%d %H:%M:%S", localtime( &info.st_mtime ) );
#endif

						pList->addItem( new CAtomString( pTime ) );

						//
						// file size
						//

						if( pLen )
							pList->addItem( new CAtomLong( *dynamic_cast<CAtomLong *>( pLen ) ) );
						else
						{
							int64 iSize = 0;

							vector<CAtom *> *pvecFiles = dynamic_cast<CAtomList *>( pFiles )->getValuePtr( );

							for( vector<CAtom *> :: iterator j = pvecFiles->begin( ); j != pvecFiles->end( ); j++ )
							{
								if( (*j)->isDicti( ) )
								{
									CAtom *pSubLength = ( (CAtomDicti *)(*j) )->getItem( "length" );

									if( pSubLength && dynamic_cast<CAtomLong *>( pSubLength ) )
										iSize += dynamic_cast<CAtomLong *>( pSubLength )->getValue( );
								}
							}

							pList->addItem( new CAtomLong( iSize ) );
						}

						//
						// number of files
						//

						if( pLen )
							pList->addItem( new CAtomInt( 1 ) );
						else
							pList->addItem( new CAtomInt( (int)dynamic_cast<CAtomList *>( pFiles )->getValuePtr( )->size( ) ) );

						//
						// file comment
						//

						CAtom *pComment = ( (CAtomDicti *)pFile )->getItem( "comment" );

						if( pComment )
							pList->addItem( new CAtomString( pComment->toString( ) ) );
						else
							pList->addItem( new CAtomString( ) );

						pAllowed->setItem( strHash, pList );

						if( m_pDFile && m_bKeepDead )
						{
							if( !m_pDFile->getItem( strHash ) )
								m_pDFile->setItem( strHash, new CAtomDicti( ) );
						}
					}
					else
					{
						UTIL_LogPrint( "error parsing torrents - %s has an incomplete or invalid info key, skipping\n", strFileName.c_str( ) );

						if( m_bDeleteInvalid )
						{
							if( m_strArchiveDir.empty( ) )
								UTIL_DeleteFile( szFile );
							else
								UTIL_MoveFile( szFile, ( m_strArchiveDir + strFileName ).c_str( ) );
						}
					}
				}
				else
				{
					UTIL_LogPrint( "error parsing torrents - %s doesn't have an info key or info is not a valid bencoded dictionary, skipping\n", strFileName.c_str( ) );

					if( m_bDeleteInvalid )
					{
						if( m_strArchiveDir.empty( ) )
							UTIL_DeleteFile( szFile );
						else
							UTIL_MoveFile( szFile, ( m_strArchiveDir + strFileName ).c_str( ) );
					}
				}
			}
			else
			{
				UTIL_LogPrint( "error parsing torrents - %s is not a valid bencoded dictionary or unable to decode, skipping\n", strFileName.c_str( ) );

				if( m_bDeleteInvalid )
				{
					if( m_strArchiveDir.empty( ) )
						UTIL_DeleteFile( szFile );
					else
						UTIL_MoveFile( szFile, ( m_strArchiveDir + strFileName ).c_str( ) );
				}
			}

			delete pFile;

			pFile = NULL;

#ifdef WIN32
		} while( FindNextFile( hFind, &fdt ) );

		FindClose( hFind );
#else
		}

		closedir( pDir );
#endif
	}
	else
		UTIL_LogPrint( "error parsing torrents - unable to open %s or no torrents found\n", szDir );

	if( m_pAllowed )
		delete m_pAllowed;

	m_pAllowed = pAllowed;
}

void CTracker :: parseTorrent( const char *szFile )
{
#ifdef WIN32
	HANDLE hFile = CreateFile( szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if( hFile == INVALID_HANDLE_VALUE )
	{
		UTIL_LogPrint( "error parsing torrent - unable to open %s for reading\n", szFile );

		return;
	}
#endif

	CAtom *pTorrent = DecodeFile( szFile );

	if( pTorrent && pTorrent->isDicti( ) )
	{
		CAtom *pInfo = ( (CAtomDicti *)pTorrent )->getItem( "info" );

		if( pInfo && pInfo->isDicti( ) )
		{
			CAtomDicti *pInfoDicti = (CAtomDicti *)pInfo;

			string strHash = UTIL_InfoHash( pTorrent );

			CAtom *pName = pInfoDicti->getItem( "name" );
			CAtom *pLen = pInfoDicti->getItem( "length" );
			CAtom *pFiles = pInfoDicti->getItem( "files" );

			if( pName && ( ( pLen && dynamic_cast<CAtomLong *>( pLen ) ) || ( pFiles && dynamic_cast<CAtomList *>( pFiles ) ) ) )
			{
				CAtomList *pList = new CAtomList( );

				//
				// filename
				//

				pList->addItem( new CAtomString( UTIL_StripPath( szFile ).c_str( ) ) );

				//
				// name
				//

				pList->addItem( new CAtomString( pName->toString( ) ) );

				//
				// added time (i.e. modification time)
				//

				char pTime[256];
				memset( pTime, 0, sizeof( char ) * 256 );

#ifdef WIN32
				FILETIME ft;
				SYSTEMTIME st;

				GetFileTime( hFile, NULL, NULL, &ft );

				FileTimeToLocalFileTime( &ft, &ft );
				FileTimeToSystemTime( &ft, &st );

				sprintf( pTime, "%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );
#else
				struct stat info;

				stat( szFile, &info );

				strftime( pTime, sizeof( char ) * 256, "%Y-%m-%d %H:%M:%S", localtime( &info.st_mtime ) );
#endif

				pList->addItem( new CAtomString( pTime ) );

				//
				// file size
				//

				if( pLen )
					pList->addItem( new CAtomLong( *dynamic_cast<CAtomLong *>( pLen ) ) );
				else
				{
					int64 iSize = 0;

					vector<CAtom *> *pvecFiles = dynamic_cast<CAtomList *>( pFiles )->getValuePtr( );

					for( vector<CAtom *> :: iterator j = pvecFiles->begin( ); j != pvecFiles->end( ); j++ )
					{
						if( (*j)->isDicti( ) )
						{
							CAtom *pSubLength = ( (CAtomDicti *)(*j) )->getItem( "length" );

							if( pSubLength && dynamic_cast<CAtomLong *>( pSubLength ) )
								iSize += dynamic_cast<CAtomLong *>( pSubLength )->getValue( );
						}
					}

					pList->addItem( new CAtomLong( iSize ) );
				}

				//
				// number of files
				//

				if( pLen )
					pList->addItem( new CAtomInt( 1 ) );
				else
					pList->addItem( new CAtomInt( (int)dynamic_cast<CAtomList *>( pFiles )->getValuePtr( )->size( ) ) );

				//
				// file comment
				//

				CAtom *pComment = ( (CAtomDicti *)pTorrent )->getItem( "comment" );

				if( pComment )
					pList->addItem( new CAtomString( pComment->toString( ) ) );
				else
					pList->addItem( new CAtomString( ) );

				m_pAllowed->setItem( strHash, pList );

				if( m_pDFile && m_bKeepDead )
				{
					if( !m_pDFile->getItem( strHash ) )
						m_pDFile->setItem( strHash, new CAtomDicti( ) );
				}
			}
			else
			{
				UTIL_LogPrint( "error parsing torrent - %s has an incomplete or invalid info key, skipping\n", szFile );

				if( m_bDeleteInvalid )
				{
					if( m_strArchiveDir.empty( ) )
						UTIL_DeleteFile( szFile );
					else
						UTIL_MoveFile( szFile, ( m_strArchiveDir + UTIL_StripPath( szFile ) ).c_str( ) );
				}
			}
		}
		else
		{
			UTIL_LogPrint( "error parsing torrent - %s doesn't have an info key or info is not a valid bencoded dictionary, skipping\n", szFile );

			if( m_bDeleteInvalid )
			{
				if( m_strArchiveDir.empty( ) )
					UTIL_DeleteFile( szFile );
				else
					UTIL_MoveFile( szFile, ( m_strArchiveDir + UTIL_StripPath( szFile ) ).c_str( ) );
			}
		}
	}
	else
	{
		UTIL_LogPrint( "error parsing torrent - %s is not a valid bencoded dictionary or unable to decode, skipping\n", szFile );

		if( m_bDeleteInvalid )
		{
			if( m_strArchiveDir.empty( ) )
				UTIL_DeleteFile( szFile );
			else
				UTIL_MoveFile( szFile, ( m_strArchiveDir + UTIL_StripPath( szFile ) ).c_str( ) );
		}
	}

	delete pTorrent;

	pTorrent = NULL;

#ifdef WIN32
	CloseHandle( hFile );
#endif
}

bool CTracker :: checkTag( string &strTag )
{
	if( m_vecTags.empty( ) )
		return true;

	for( vector< pair<string, string> > :: iterator i = m_vecTags.begin( ); i != m_vecTags.end( ); i++ )
	{
		if( (*i).first == strTag )
			return true;
	}

	return false;
}

void CTracker :: addTag( string strInfoHash, string strTag, string strName, string strUploader, string strInfoLink )
{
	if( !m_pTags->getItem( strInfoHash ) )
		m_pTags->setItem( strInfoHash, new CAtomDicti( ) );

	CAtom *pTag = m_pTags->getItem( strInfoHash );

	if( pTag && pTag->isDicti( ) )
	{
		( (CAtomDicti *)pTag )->setItem( "tag", new CAtomString( strTag ) );

		if( !strName.empty( ) )
			( (CAtomDicti *)pTag )->setItem( "name", new CAtomString( strName ) );

		if( !strUploader.empty( ) )
			( (CAtomDicti *)pTag )->setItem( "uploader", new CAtomString( strUploader ) );

		if( !strInfoLink.empty( ) )
			( (CAtomDicti *)pTag )->setItem( "infolink", new CAtomString( strInfoLink ) );
	}

	saveTags( );
}

void CTracker :: deleteTag( string strInfoHash )
{
	m_pTags->delItem( strInfoHash );

	saveTags( );
}

user_t CTracker :: checkUser( string strLogin, string strMD5 )
{
	user_t user;

	user.iAccess = m_iGuestAccess;

	// if no users exist, grant full access

	if( m_pUsers->isEmpty( ) )
		user.iAccess = ~0;

	CAtom *pUser = m_pUsers->getItem( strLogin );

	if( pUser && pUser->isDicti( ) )
	{
		CAtom *pMD5 = ( (CAtomDicti *)pUser )->getItem( "md5" );
		CAtom *pAccess = ( (CAtomDicti *)pUser )->getItem( "access" );
		CAtom *pMail = ( (CAtomDicti *)pUser )->getItem( "email" );
		CAtom *pCreated = ( (CAtomDicti *)pUser )->getItem( "created" );

		if( pMD5 && pAccess && dynamic_cast<CAtomLong *>( pAccess ) && pMail )
		{
			// check hash

			if( strMD5 == pMD5->toString( ) )
			{
				user.strLogin = strLogin;
				user.strLowerLogin = UTIL_ToLower( user.strLogin );
				user.strMD5 = strMD5;
				user.strMail = pMail->toString( );
				user.strLowerMail = UTIL_ToLower( user.strMail );
				user.iAccess = (int)dynamic_cast<CAtomLong *>( pAccess )->getValue( );
				user.strCreated = pCreated->toString( );
			}
		}
	}

	return user;
}

void CTracker :: addUser( string strLogin, string strPass, int iAccess, string strMail )
{
	CAtomDicti *pUser = new CAtomDicti( );

	// calculate md5 hash of A1

	string strA1 = strLogin + ":" + gstrRealm + ":" + strPass;

	unsigned char szMD5[16];

	MD5_CTX md5;

	MD5Init( &md5 );
	MD5Update( &md5, (const unsigned char *)strA1.c_str( ), (unsigned int)strA1.size( ) );
	MD5Final( szMD5, &md5 );

	pUser->setItem( "md5", new CAtomString( string( (char *)szMD5, 16 ) ) );
	pUser->setItem( "access", new CAtomLong( iAccess ) );
	pUser->setItem( "email", new CAtomString( strMail ) );

	time_t tNow = time( NULL );

	char pTime[256];
	memset( pTime, 0, sizeof( char ) * 256 );
	strftime( pTime, sizeof( char ) * 256, "%Y %m/%d %H:%M:%S", localtime( &tNow ) );

	pUser->setItem( "created", new CAtomString( pTime ) );

	m_pUsers->setItem( strLogin, pUser );

	saveUsers( );
}

void CTracker :: deleteUser( string strLogin )
{
	m_pUsers->delItem( strLogin );

	saveUsers( );
}

void CTracker :: CountUniquePeers( )
{
	delete m_pIPs;

	m_pIPs = new CAtomDicti( );

	map<string, CAtom *> *pmapDicti = m_pDFile->getValuePtr( );

	for( map<string, CAtom *> :: iterator i = pmapDicti->begin( ); i != pmapDicti->end( ); i++ )
	{
		if( (*i).second->isDicti( ) )
		{
			map<string, CAtom *> *pmapPeersDicti = ( (CAtomDicti *)(*i).second )->getValuePtr( );

			for( map<string, CAtom *> :: iterator j = pmapPeersDicti->begin( ); j != pmapPeersDicti->end( ); j++ )
			{
				if( (*j).second->isDicti( ) )
				{
					CAtom *pIP = ( (CAtomDicti *)(*j).second )->getItem( "ip" );

					if( pIP )
						AddUniquePeer( pIP->toString( ) );
				}
			}
		}
	}
}

void CTracker :: AddUniquePeer( string strIP )
{
	// increment unique count for this ip

	CAtom *pNum = m_pIPs->getItem( strIP );

	int iNum = 1;

	if( pNum && dynamic_cast<CAtomInt *>( pNum ) )
		iNum = dynamic_cast<CAtomInt *>( pNum )->getValue( ) + 1;

	m_pIPs->setItem( strIP, new CAtomInt( iNum ) );
}

void CTracker :: RemoveUniquePeer( string strIP )
{
	// decrement unique count for this ip

	CAtom *pNum = m_pIPs->getItem( strIP );

	int iNum = 0;

	if( pNum && dynamic_cast<CAtomInt *>( pNum ) )
		iNum = dynamic_cast<CAtomInt *>( pNum )->getValue( ) - 1;

	if( iNum > 0 )
		m_pIPs->setItem( strIP, new CAtomInt( iNum ) );
	else
		m_pIPs->delItem( strIP );
}

void CTracker :: Announce( struct announce_t ann )
{
#ifdef BNBT_MYSQL
	if( m_bMySQLOverrideDState )
	{
		if( ann.strEvent != "stopped" )
		{
			string strQuery;

			strQuery += "REPLACE INTO dstate (bhash,bid,bkey,bip,bport,buploaded,bdownloaded,bleft,btime) VALUES(\'"; /* =X= */
			strQuery += UTIL_StringToMySQL( ann.strInfoHash );
			strQuery += "\',\'";
			strQuery += UTIL_StringToMySQL( ann.strPeerID );
			strQuery += "\',\'"; /* =X= */
			strQuery += UTIL_StringToMySQL( ann.strKey ); /* =X= */
			strQuery += "\',\'";
			strQuery += UTIL_StringToMySQL( ann.strIP.substr( 0, 15 ) );
			strQuery += "\',";
			strQuery += CAtomInt( ann.iPort ).toString( );
			strQuery += ",";
			strQuery += CAtomLong( ann.iUploaded ).toString( );
			strQuery += ",";
			strQuery += CAtomLong( ann.iDownloaded ).toString( );
			strQuery += ",";
			strQuery += CAtomLong( ann.iLeft ).toString( );
			strQuery += ",NOW())";

			CMySQLQuery mq01( strQuery );

			if( ann.strEvent == "completed" )
			{
				CMySQLQuery *pQuery = new CMySQLQuery( "SELECT bcompleted FROM completed WHERE bhash=\'" + UTIL_StringToMySQL( ann.strInfoHash ) + "\'" );

				if( pQuery->nextRow( ).size( ) == 0 )
				{
					strQuery.erase( );

					strQuery += "INSERT INTO completed (bhash,bcompleted) VALUES(\'";
					strQuery += UTIL_StringToMySQL( ann.strInfoHash );
					strQuery += "\',1)";

					CMySQLQuery mq02( strQuery );
				}
				else
				{
					strQuery.erase( );

					strQuery += "UPDATE completed SET bcompleted=bcompleted+1 WHERE bhash=\'";
					strQuery += UTIL_StringToMySQL( ann.strInfoHash );
					strQuery += "\'";

					CMySQLQuery mq03( strQuery );
				}

				delete pQuery;
			}
		}
		else
		{
			// strEvent == "stopped"

			string strQuery;

			strQuery += "DELETE FROM dstate WHERE bhash=\'";
			strQuery += UTIL_StringToMySQL( ann.strInfoHash );
			strQuery += "\' AND bid=\'";
			strQuery += UTIL_StringToMySQL( ann.strPeerID );
			strQuery += "\' AND bkey=\'"; /* =X= */
			strQuery += UTIL_StringToMySQL( ann.strKey ); /* =X= */
			//strQuery += "\' AND bip=\'"; /* =X= */
			//strQuery += UTIL_StringToMySQL( ann.strIP ); /* =X= */
			strQuery += "\'";

			CMySQLQuery mq04( strQuery );
		}

		return;
	}
#endif

	if( ann.strEvent != "stopped" )
	{
		if( m_pDFile )
		{
			if( !m_pDFile->getItem( ann.strInfoHash ) )
				m_pDFile->setItem( ann.strInfoHash, new CAtomDicti( ) );

			CAtom *pPeers = m_pDFile->getItem( ann.strInfoHash );

			if( pPeers && pPeers->isDicti( ) )
			{
				CAtom *pPeer = ( (CAtomDicti *)pPeers )->getItem( ann.strPeerID );
				
				/* =X= */
				string strTempKey = string( );

				if( pPeer && pPeer->isDicti( ) )
				{
					CAtom *pKey = ( (CAtomDicti *)pPeer )->getItem( "key" );

					if( pKey )
						strTempKey = pKey->toString( );
				}

				if( pPeer && pPeer->isDicti( ) && !strTempKey.empty( ) && ( strTempKey == ann.strKey ) )
				{
					CAtom *pPeerIP = ( (CAtomDicti *)pPeer )->getItem( "ip" );

					if( pPeerIP && ( pPeerIP->toString( ) != ann.strIP ) )
					{
						if( m_bCountUniquePeers )
							RemoveUniquePeer( pPeerIP->toString( ) );

						( (CAtomDicti *)pPeer )->setItem( "ip", new CAtomString( ann.strIP ) );

						if( m_bCountUniquePeers )
							AddUniquePeer( ann.strIP );
					} /* =X= */

					( (CAtomDicti *)pPeer )->setItem( "uploaded", new CAtomLong( ann.iUploaded ) );
					( (CAtomDicti *)pPeer )->setItem( "downloaded", new CAtomLong( ann.iDownloaded ) );
					( (CAtomDicti *)pPeer )->setItem( "left", new CAtomLong( ann.iLeft ) );
				}
				else
				{
					CAtomDicti *pPeerDicti = new CAtomDicti( );

					pPeerDicti->setItem( "ip", new CAtomString( ann.strIP ) );
					pPeerDicti->setItem( "port", new CAtomLong( ann.iPort ) );
					pPeerDicti->setItem( "uploaded", new CAtomLong( ann.iUploaded ) );
					pPeerDicti->setItem( "downloaded", new CAtomLong( ann.iDownloaded ) );
					pPeerDicti->setItem( "left", new CAtomLong( ann.iLeft ) );
					pPeerDicti->setItem( "connected", new CAtomLong( GetTime( ) ) );
					pPeerDicti->setItem( "key", new CAtomString( ann.strKey ) ); /* =X =*/

					( (CAtomDicti *)pPeers )->setItem( ann.strPeerID, pPeerDicti );

					if( m_bCountUniquePeers )
						AddUniquePeer( ann.strIP );
				}

				if( m_pTimeDicti )
				{
					if( !m_pTimeDicti->getItem( ann.strInfoHash ) )
						m_pTimeDicti->setItem( ann.strInfoHash, new CAtomDicti( ) );

					CAtom *pTS = m_pTimeDicti->getItem( ann.strInfoHash );

					if( pTS && pTS->isDicti( ) )
						( (CAtomDicti *)pTS )->setItem( ann.strPeerID, new CAtomLong( GetTime( ) ) );
				}
			}
		}

		if( ann.strEvent == "completed" )
		{
			if( m_pCompleted )
			{
				CAtom *pCompleted = m_pCompleted->getItem( ann.strInfoHash );

				int64 iCompleted = 0;

				if( pCompleted && dynamic_cast<CAtomLong *>( pCompleted ) )
					iCompleted = dynamic_cast<CAtomLong *>( pCompleted )->getValue( );

				m_pCompleted->setItem( ann.strInfoHash, new CAtomLong( iCompleted + 1 ) );
			}
		}
	}
	else
	{
		// strEvent == "stopped"

		if( m_pDFile )
		{
			if( !m_pDFile->getItem( ann.strInfoHash ) )
				m_pDFile->setItem( ann.strInfoHash, new CAtomDicti( ) );

			CAtom *pPeers = m_pDFile->getItem( ann.strInfoHash );

			if( pPeers && pPeers->isDicti( ) )
			{
				CAtom *pPeer = ( (CAtomDicti *)pPeers )->getItem( ann.strPeerID );

				/* =X= */
				string strTempKey = string( );

				if( pPeer && pPeer->isDicti( ) )
				{
					CAtom *pKey = ( (CAtomDicti *)pPeer )->getItem( "key" );

					if( pKey )
						strTempKey = pKey->toString( );
				}

				if( pPeer && pPeer->isDicti( ) && !strTempKey.empty( ) && ( strTempKey == ann.strKey ) ) /* =X= */
				{
					CAtom *pPeerIP = ( (CAtomDicti *)pPeer )->getItem( "ip" );

					if( pPeerIP )
					{
						/* =X= */
						if( pPeerIP && ( pPeerIP->toString( ) != ann.strIP ) )
						{
							if( m_bCountUniquePeers )
								RemoveUniquePeer( pPeerIP->toString( ) );

							( (CAtomDicti *)pPeer )->setItem( "ip", new CAtomString( ann.strIP ) );
							
							pPeerIP = ( (CAtomDicti *)pPeer )->getItem( "ip" );

							if( m_bCountUniquePeers )
								AddUniquePeer( ann.strIP );
						} /* =X= */

						if( pPeerIP->toString( ) == ann.strIP )
						{
							( (CAtomDicti *)pPeers )->delItem( ann.strPeerID );

							if( m_pTimeDicti )
							{
								if( !m_pTimeDicti->getItem( ann.strInfoHash ) )
									m_pTimeDicti->setItem( ann.strInfoHash, new CAtomDicti( ) );

								CAtom *pTS = m_pTimeDicti->getItem( ann.strInfoHash );

								if( pTS && pTS->isDicti( ) )
									( (CAtomDicti *)pTS )->delItem( ann.strPeerID );
							}

							if( m_bCountUniquePeers )
								RemoveUniquePeer( ann.strIP );
						}
					}
				}
			}
		}
	}
}

void CTracker :: RefreshFastCache( )
{
	delete m_pFastCache;

	m_pFastCache = new CAtomDicti( );

	if( m_pDFile )
	{
		map<string, CAtom *> *pmapDicti = m_pDFile->getValuePtr( );

		for( map<string, CAtom *> :: iterator i = pmapDicti->begin( ); i != pmapDicti->end( ); i++ )
		{
			if( (*i).second->isDicti( ) )
			{
				map<string, CAtom *> *pmapPeersDicti = ( (CAtomDicti *)(*i).second )->getValuePtr( );

				unsigned long iSeeders = 0;
				unsigned long iLeechers = 0;
				unsigned long iCompleted = 0;
				int64 iTotalLeft = 0;
				int64 iMinLeft = 0;
				int64 iMaxiLeft = 0;

				bool bFirst = true;

				for( map<string, CAtom *> :: iterator j = pmapPeersDicti->begin( ); j != pmapPeersDicti->end( ); j++ )
				{
					if( (*j).second->isDicti( ) )
					{
						CAtom *pLeft = ( (CAtomDicti *)(*j).second )->getItem( "left" );

						if( pLeft && dynamic_cast<CAtomLong *>( pLeft ) )
						{
							int64 iLeft = dynamic_cast<CAtomLong *>( pLeft )->getValue( );

							if( iLeft == 0 )
								iSeeders++;
							else
							{
								iLeechers++;

								// only calculate total / min / max on leechers

								if( m_bShowAverageLeft )
									iTotalLeft += iLeft;

								if( bFirst || iLeft < iMinLeft )
									iMinLeft = iLeft;

								if( bFirst || iLeft > iMaxiLeft )
									iMaxiLeft = iLeft;

								bFirst = false;
							}
						}
					}
				}

				if( m_pCompleted )
				{
					CAtom *pCompleted = m_pCompleted->getItem( (*i).first );

					if( pCompleted && dynamic_cast<CAtomLong *>( pCompleted ) )
						iCompleted = (unsigned long)dynamic_cast<CAtomLong *>( pCompleted )->getValue( );
				}

				CAtomList *pList = new CAtomList( );

				pList->addItem( new CAtomInt( (int) iSeeders ) );
				pList->addItem( new CAtomInt( (int) iLeechers ) );
				pList->addItem( new CAtomInt( (int) iCompleted ) );
				pList->addItem( new CAtomLong( iTotalLeft ) );
				pList->addItem( new CAtomLong( iMinLeft ) );
				pList->addItem( new CAtomLong( iMaxiLeft ) );

				m_pFastCache->setItem( (*i).first, pList );
			}
		}
	}
}

void CTracker :: serverResponseGET( struct request_t *pRequest, struct response_t *pResponse )
{
	if( pRequest->strURL == "/" || pRequest->strURL == "/index.html" )
		serverResponseIndex( pRequest, pResponse );
	else if( pRequest->strURL == "/announce" || ( !m_strCustomAnnounce.empty( ) && pRequest->strURL == m_strCustomAnnounce ) ) /* =X= */
		serverResponseAnnounce( pRequest, pResponse );
	else if( pRequest->strURL == "/scrape" || ( !m_strCustomScrape.empty( ) && pRequest->strURL == m_strCustomScrape ) ) /* =X= */
		serverResponseScrape( pRequest, pResponse );
	else if( pRequest->strURL == "/stats.html" )
		serverResponseStats( pRequest, pResponse );
	else if( pRequest->strURL.substr( 0, 10 ) == "/torrents/" )
		serverResponseTorrent( pRequest, pResponse );
	else if( pRequest->strURL.substr( 0, 7 ) == "/files/" )
		serverResponseFile( pRequest, pResponse );
	else if( pRequest->strURL == "/robots.txt" && !m_strRobots.empty( ) )
		serverResponseRobots( pRequest, pResponse );
	else if( pRequest->strURL == "/favicon.ico" && !m_strFavicon.empty( ) ) /* =X= */
		serverResponseFavicon( pRequest, pResponse );
	else if( pRequest->strURL == "/login.html" )
		serverResponseLogin( pRequest, pResponse );
	else if( pRequest->strURL == "/signup.html" )
		serverResponseSignup( pRequest, pResponse );
	else if( pRequest->strURL == "/upload.html" )
		serverResponseUploadGET( pRequest, pResponse );
	else if( pRequest->strURL == "/info.html" )
		serverResponseInfo( pRequest, pResponse );
	// added /info.bencode support - DWK
	else if( pRequest->strURL == "/info.bencode" )
		serverResponseBencodeInfo( pRequest, pResponse );
	else if( pRequest->strURL == "/admin.html" )
		serverResponseAdmin( pRequest, pResponse );
	else if( pRequest->strURL == "/users.html" )
		serverResponseUsers( pRequest, pResponse );
	else if( pRequest->strURL == "/comments.html" )
		serverResponseComments( pRequest, pResponse );
	else
		serverResponseNotFound( pRequest, pResponse ); /* =X= */
}

/* =X= */
void CTracker :: serverResponseNotFound( struct request_t *pRequest, struct response_t *pResponse )
{
	(void) pRequest; // Unused argument.

	pResponse->strCode = "404 Not Found";
		
	pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", gmapMime[".txt"] ) );

	pResponse->strContent = "404 Not Found";
}

void CTracker :: serverResponsePOST( struct request_t *pRequest, struct response_t *pResponse, CAtomList *pPost )
{
	if( pPost )
	{
		if( pRequest->strURL == "/upload.html" )
			serverResponseUploadPOST( pRequest, pResponse, pPost );
		else
			serverResponseNotFound( pRequest, pResponse );  /* =X= */
	}
	else
		pResponse->strCode = "500 Server Error";
}

void CTracker :: serverResponseRobots( struct request_t *pRequest, struct response_t *pResponse )
{
	(void) pRequest; // Unused argument.

	pResponse->strCode = "200 OK";

	pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", gmapMime[".txt"] ) );  /* =X= */

	pResponse->strContent = m_strRobots;
}

/* =X= */
void CTracker :: serverResponseFavicon( struct request_t *pRequest, struct response_t *pResponse )
{

	time_t tNow = time( NULL ) + m_iRefreshStaticInterval * 60;
	char *szTime = asctime( gmtime( &tNow ) );
	szTime[strlen( szTime ) - 1] = '\0';

	string strExt = string( );

	string :: size_type iExt = m_strFaviconFile.rfind( "." );

	if( iExt != string :: npos )
		strExt = m_strFaviconFile.substr( iExt );

	if( strExt == ".ico" )
	{
		pResponse->strCode = "200 OK";

		pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", gmapMime[".ico"] ) );
		pResponse->mapHeaders.insert( pair<string, string>( "Expires", string( szTime ) + " GMT" ) );
		
		pResponse->strContent = m_strFavicon;
	}
	else if( strExt == ".png" )
	{
		pResponse->strCode = "200 OK";

		pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", gmapMime[".png"] ) );
		pResponse->mapHeaders.insert( pair<string, string>( "Expires", string( szTime ) + " GMT" ) );

		pResponse->strContent = m_strFavicon;
	}
	else if( strExt == ".gif" )
	{
		pResponse->strCode = "200 OK";

		pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", gmapMime[".gif"] ) );
		pResponse->mapHeaders.insert( pair<string, string>( "Expires", string( szTime ) + " GMT" ) );

		pResponse->strContent = m_strFavicon;
	}
	else
		serverResponseNotFound( pRequest, pResponse );
}

void CTracker :: Update( )
{
	if( !m_strAllowedDir.empty( ) )
	{
		if( m_iParseAllowedInterval > 0 && GetTime( ) > m_iParseAllowedNext )
		{
			if( gbDebug )
				UTIL_LogPrint( "tracker - parsing torrents (%s)\n", m_strAllowedDir.c_str( ) );

			// don't parse torrents again until we're done since parseTorrents calls gpServer->Update( ) which calls m_pTracker->Update( )

			m_iParseAllowedNext = GetTime( ) + 9999;

			parseTorrents( m_strAllowedDir.c_str( ) );

			m_iParseAllowedNext = GetTime( ) + m_iParseAllowedInterval * 60;
		}
	}

#ifdef BNBT_MYSQL
	else if( m_iMySQLRefreshAllowedInterval > 0 && GetTime( ) > m_iMySQLRefreshAllowedNext )
	{
		if( gbDebug )
			UTIL_LogPrint( "mysql - refreshing allowed\n" );

		if( m_pAllowed )
			delete m_pAllowed;

		m_pAllowed = new CAtomDicti( );

		CMySQLQuery *pQuery = new CMySQLQuery( "SELECT bhash,bname FROM allowed" );

		vector<string> vecQuery;

		while( ( vecQuery = pQuery->nextRow( ) ).size( ) == 2 )
		{
			CAtomList *pList = new CAtomList( );

			pList->addItem( new CAtomString( ) );
			pList->addItem( new CAtomString( vecQuery[1] ) );
			pList->addItem( new CAtomString( ) );
			pList->addItem( new CAtomLong( ) );
			pList->addItem( new CAtomInt( ) );
			pList->addItem( new CAtomString( ) );

			m_pAllowed->setItem( vecQuery[0], pList );
		}

		delete pQuery;

		m_iMySQLRefreshAllowedNext = GetTime( ) + m_iMySQLRefreshAllowedInterval;
	}

	if( m_iMySQLRefreshStatsInterval > 0 && GetTime( ) > m_iMySQLRefreshStatsNext )
	{
		if( gbDebug )
			UTIL_LogPrint( "mysql - refreshing stats\n" );

		if( m_bMySQLOverrideDState )
		{
			CMySQLQuery mq01( "TRUNCATE TABLE hashes" );

			if( !m_strAllowedDir.empty( ) )
				CMySQLQuery mq02( "INSERT INTO hashes (bhash) SELECT DISTINCT bhash FROM dstate" );
			else
			{
				if( m_iMySQLRefreshAllowedInterval > 0 )
					CMySQLQuery mq03( "INSERT INTO hashes (bhash) SELECT bhash FROM allowed" );
				else
					CMySQLQuery mq04( "INSERT INTO hashes (bhash) SELECT DISTINCT bhash FROM dstate" );
			}

			CMySQLQuery mq11( "TRUNCATE TABLE seeders" );
			CMySQLQuery mq12( "INSERT INTO seeders (bhash) SELECT bhash FROM hashes" );
			CMySQLQuery mq13( "REPLACE INTO seeders (bhash,bseeders) SELECT bhash,COUNT(*) FROM dstate WHERE bleft=0 GROUP BY bhash" );
			CMySQLQuery mq14( "TRUNCATE TABLE leechers" );
			CMySQLQuery mq15( "INSERT INTO leechers (bhash) SELECT bhash FROM hashes" );
			CMySQLQuery mq16( "REPLACE INTO leechers (bhash,bleechers) SELECT bhash,COUNT(*) FROM dstate WHERE bleft!=0 GROUP BY bhash" );
			CMySQLQuery mq17( "TRUNCATE TABLE torrents" );
			CMySQLQuery mq18( "INSERT INTO torrents (bhash,bseeders,bleechers,bcompleted) SELECT hashes.bhash,bseeders,bleechers,bcompleted FROM hashes LEFT JOIN seeders USING(bhash) LEFT JOIN leechers USING(bhash) LEFT JOIN completed USING(bhash)" );
		}
		else
		{
			if( m_pDFile )
			{
				map<string, CAtom *> *pmapDicti = m_pDFile->getValuePtr( );

				string strQuery;

				if( !pmapDicti->empty( ) )
					strQuery += "INSERT INTO torrents (bhash,bseeders,bleechers,bcompleted) VALUES";

				for( map<string, CAtom *> :: iterator i = pmapDicti->begin( ); i != pmapDicti->end( ); )
				{
					if( (*i).second->isDicti( ) )
					{
						map<string, CAtom *> *pmapPeersDicti = ( (CAtomDicti *)(*i).second )->getValuePtr( );

						unsigned long iSeeders = 0;
						unsigned long iLeechers = 0;
						unsigned long iCompleted = 0;

						for( map<string, CAtom *> :: iterator j = pmapPeersDicti->begin( ); j != pmapPeersDicti->end( ); j++ )
						{
							if( (*j).second->isDicti( ) )
							{
								CAtom *pLeft = ( (CAtomDicti *)(*j).second )->getItem( "left" );

								if( pLeft && dynamic_cast<CAtomLong *>( pLeft ) )
								{
									if( dynamic_cast<CAtomLong *>( pLeft )->getValue( ) == 0 )
										iSeeders++;
									else
										iLeechers++;
								}
							}
						}

						if( m_pCompleted )
						{
							CAtom *pCompleted = m_pCompleted->getItem( (*i).first );

							if( pCompleted && dynamic_cast<CAtomLong *>( pCompleted ) )
								iCompleted = (unsigned long)dynamic_cast<CAtomLong *>( pCompleted )->getValue( );
						}

						strQuery += " (\'" + UTIL_StringToMySQL( (*i).first ) + "\'," + CAtomInt( iSeeders ).toString( ) + "," + CAtomInt( iLeechers ).toString( ) + "," + CAtomInt( iCompleted ).toString( ) + ")";

						if( ++i != pmapDicti->end( ) )
							strQuery += ",";
					}
					else
						i++;
				}

				if( !strQuery.empty( ) )
				{
					CMySQLQuery mq01( "TRUNCATE TABLE torrents" );
					CMySQLQuery mq02( strQuery );
				}
			}
		}

		m_iMySQLRefreshStatsNext = GetTime( ) + m_iMySQLRefreshStatsInterval;
	}
#endif

	if( GetTime( ) > m_iSaveDFileNext )
	{
		if( gbDebug )
			UTIL_LogPrint( "tracker - saving dfile (%s)\n", m_strDFile.c_str( ) );

		saveDFile( );

		m_iSaveDFileNext = GetTime( ) + m_iSaveDFileInterval;
	}

	if( ( !m_strStaticHeaderFile.empty( ) || !m_strStaticFooterFile.empty( ) || !m_strRobotsFile.empty( ) || !m_strFaviconFile.empty( ) ) && GetTime( ) > m_iRefreshStaticNext ) /* =X= */
	{
		if( gbDebug )
			UTIL_LogPrint( "tracker - refreshing static header, static footer, robots.txt and favicon.ico\n" ); /* =X= */

		if( !m_strStaticHeaderFile.empty( ) )
			m_strStaticHeader = UTIL_ReadFile( m_strStaticHeaderFile.c_str( ) );

		if( !m_strStaticFooterFile.empty( ) )
			m_strStaticFooter = UTIL_ReadFile( m_strStaticFooterFile.c_str( ) );

		if( !m_strRobotsFile.empty( ) )
			m_strRobots = UTIL_ReadFile( m_strRobotsFile.c_str( ) );
		
		/* =X= */
		if( !m_strFaviconFile.empty( ) )
		{
			if( UTIL_CheckFile( m_strFaviconFile.c_str( ) ) )
			{
				string strExt = string( );

				string :: size_type iExt = m_strFaviconFile.rfind( "." );

				if( iExt != string :: npos )
					strExt = m_strFaviconFile.substr( iExt );

				if( strExt == ".ico" || strExt == ".png" || strExt == ".gif" )
					m_strFavicon = UTIL_ReadFile( m_strFaviconFile.c_str( ) );
				else
					UTIL_LogPrint( "tracker warning - favicon file type %s not supported\n", strExt.c_str( ) );
			}
			else
				UTIL_LogPrint( "tracker warning - favicon file %s not found\n", m_strFaviconFile.c_str( ) );
		}
		else
		{
			if( gbDebug )
				UTIL_LogPrint( "tracker info - favicon is not set\n" );
		}


		m_iRefreshStaticNext = GetTime( ) + m_iRefreshStaticInterval * 60;
	}

	if( GetTime( ) > m_iDownloaderTimeOutNext )
	{
		if( gbDebug )
			UTIL_LogPrint( "tracker - expiring downloaders\n" );

		expireDownloaders( );

		m_iDownloaderTimeOutNext = GetTime( ) + m_iDownloaderTimeOutInterval;
	}

	// DWKnight - Re-adding XML Dump.

	if( !m_strDumpXMLFile.empty( ) && GetTime( ) > m_iDumpXMLNext )
	{
		if( gbDebug )
			UTIL_LogPrint( "tracker - dumping xml\n" );

		saveXML( );

		m_iDumpXMLNext = GetTime( ) + m_iDumpXMLInterval;
	}

	// addition by labarks

	if( !m_strDumpRSSFile.empty( ) && m_iDumpRSSInterval > 0 && GetTime( ) > m_iDumpRSSNext )
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

		m_iDumpRSSNext = GetTime( ) + m_iDumpRSSInterval * 60;
	}
}
