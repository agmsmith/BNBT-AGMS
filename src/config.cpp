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

#include <fstream>

#include "bnbt.h"
#include "atom.h"
#include "config.h"
#include "tracker.h"
#include "util.h"

map<string, string> gmapCFG;

void CFG_Open( const char *szFile )
{
	gmapCFG.clear( );

	ifstream in;

	in.open( szFile );

	if( in.fail( ) )
	{
		UTIL_LogPrint( "config warning - unable to open %s for reading\n", szFile );

		return;
	}

	while( !in.eof( ) )
	{
		char pBuf[1024];

		in.getline( pBuf, 1023 );

		string strTemp = pBuf;

		// ignore blank lines and comments

		if( strTemp.empty( ) || strTemp[0] == '#' )
			continue;

		string :: size_type iSplit = strTemp.find( "=" );

		if( iSplit == string :: npos )
			continue;

		string :: size_type iKeyStart = strTemp.find_first_not_of( " " );
		string :: size_type iKeyEnd = strTemp.find( " ", iKeyStart );
		string :: size_type iValueStart = strTemp.find_first_not_of( " ", iSplit + 1 );
		string :: size_type iValueEnd = strTemp.size( );

		if( iValueStart != string :: npos )
			gmapCFG[strTemp.substr( iKeyStart, iKeyEnd - iKeyStart )] = strTemp.substr( iValueStart, iValueEnd - iValueStart );
	}

	in.close( );
}

void CFG_SetInt( string strKey, int x )
{
	gmapCFG[strKey] = CAtomInt( x ).toString( );
}

void CFG_SetString( string strKey, string x )
{
	gmapCFG[strKey] = x;
}

int CFG_GetInt( string strKey, int x )
{
	if( gmapCFG.find( strKey ) == gmapCFG.end( ) )
		return x;
	else
		return atoi( gmapCFG[strKey].c_str( ) );
}

string CFG_GetString( string strKey, string x )
{
	if( gmapCFG.find( strKey ) == gmapCFG.end( ) )
		return x;
	else
		return gmapCFG[strKey];
}

void CFG_Delete( string strKey )
{
	gmapCFG.erase( strKey );
}

void CFG_Close( const char *szFile )
{
	ofstream out;

	out.open( szFile );

	if( out.fail( ) )
	{
		UTIL_LogPrint( "config warning - unable to open %s for writing\n", szFile );

		return;
	}

	for( map<string, string> :: iterator i = gmapCFG.begin( ); i != gmapCFG.end( ); i++ )
		out << (*i).first.c_str( ) << " = " << (*i).second.c_str( ) << endl;

	out.close( );
}

void CFG_SetDefaults( )
{
	if( gbDebug )
		UTIL_LogPrint( "config - setting defaults\n" );

	// bnbt.cpp

	if( CFG_GetInt( "bnbt_debug", -1 ) < 0 )
		CFG_SetInt( "bnbt_debug", 0 );

	if( CFG_GetInt( "bnbt_max_conns", 0 ) < 1 )
		CFG_SetInt( "bnbt_max_conns", 64 );

	if( CFG_GetString( "bnbt_style_sheet", string( ) ).empty( ) )
		CFG_SetString( "bnbt_style_sheet", string( ) );

	if( CFG_GetString( "bnbt_charset", string( ) ).empty( ) )
		CFG_SetString( "bnbt_charset", "iso-8859-1" );

	if( CFG_GetString( "bnbt_realm", string( ) ).empty( ) )
		CFG_SetString( "bnbt_realm", "BNBT" );

	if( CFG_GetString( "bnbt_error_log_dir", string( ) ).empty( ) )
		CFG_SetString( "bnbt_error_log_dir", string( ) );

	if( CFG_GetString( "bnbt_access_log_dir", string( ) ).empty( ) )
		CFG_SetString( "bnbt_access_log_dir", string( ) );

	if( CFG_GetInt( "bnbt_flush_interval", 0 ) < 1 )
		CFG_SetInt( "bnbt_flush_interval", 100 );

	// mysql

	if( CFG_GetString( "mysql_host", string( ) ).empty( ) )
		CFG_SetString( "mysql_host", string( ) );

	if( CFG_GetString( "mysql_database", string( ) ).empty( ) )
		CFG_SetString( "mysql_database", "bnbt" );

	if( CFG_GetString( "mysql_user", string( ) ).empty( ) )
		CFG_SetString( "mysql_user", string( ) );

	if( CFG_GetString( "mysql_password", string( ) ).empty( ) )
		CFG_SetString( "mysql_password", string( ) );

	if( CFG_GetInt( "mysql_port", -1 ) < 0 )
		CFG_SetInt( "mysql_port", 0 );

	if( CFG_GetInt( "mysql_refresh_allowed_interval", -1 ) < 0 )
		CFG_SetInt( "mysql_refresh_allowed_interval", 0 );

	if( CFG_GetInt( "mysql_refresh_stats_interval", 0 ) < 1 )
		CFG_SetInt( "mysql_refresh_stats_interval", 600 );

	if( CFG_GetInt( "mysql_override_dstate", -1 ) < 0 )
		CFG_SetInt( "mysql_override_dstate", 0 );

	// link.cpp

	if( CFG_GetInt( "bnbt_tlink_server", -1 ) < 0 )
		CFG_SetInt( "bnbt_tlink_server", 0 );

	if( CFG_GetString( "bnbt_tlink_connect", string( ) ).empty( ) )
		CFG_SetString( "bnbt_tlink_connect", string( ) );

	if( CFG_GetString( "bnbt_tlink_password", string( ) ).empty( ) )
		CFG_SetString( "bnbt_tlink_password", string( ) );

	if( CFG_GetString( "bnbt_tlink_bind", string( ) ).empty( ) )
		CFG_SetString( "bnbt_tlink_bind", string( ) );

	if( CFG_GetInt( "bnbt_tlink_port", 0 ) < 1 )
		CFG_SetInt( "bnbt_tlink_port", 5204 );

	// server.cpp

	if( CFG_GetInt( "socket_timeout", 0 ) < 1 )
		CFG_SetInt( "socket_timeout", 15 );

	if( CFG_GetInt( "bnbt_compression_level", -1 ) < 0 )
		CFG_SetInt( "bnbt_compression_level", 6 );

	if( CFG_GetString( "bind", string( ) ).empty( ) )
		CFG_SetString( "bind", string( ) );

	if( CFG_GetInt( "port", 0 ) < 1 )
		CFG_SetInt( "port", 6969 );

	// tracker.cpp

	if( CFG_GetString( "allowed_dir", string( ) ).empty( ) )
		CFG_SetString( "allowed_dir", string( ) );

	if( CFG_GetString( "bnbt_upload_dir", string( ) ).empty( ) )
		CFG_SetString( "bnbt_upload_dir", string( ) );

	if( CFG_GetString( "bnbt_external_torrent_dir", string( ) ).empty( ) )
		CFG_SetString( "bnbt_external_torrent_dir", string( ) );

	if( CFG_GetString( "bnbt_archive_dir", string( ) ).empty( ) )
		CFG_SetString( "bnbt_archive_dir", string( ) );

	if( CFG_GetString( "bnbt_file_dir", string( ) ).empty( ) )
		CFG_SetString( "bnbt_file_dir", string( ) );

	if( CFG_GetString( "dfile", string( ) ).empty( ) )
		CFG_SetString( "dfile", "dstate.bnbt" );

	if( CFG_GetString( "bnbt_comments_file", string( ) ).empty( ) )
		CFG_SetString( "bnbt_comments_file", string( ) );

	if( CFG_GetString( "bnbt_tag_file", string( ) ).empty( ) )
		CFG_SetString( "bnbt_tag_file", "tags.bnbt" );

	if( CFG_GetString( "bnbt_users_file", string( ) ).empty( ) )
		CFG_SetString( "bnbt_users_file", "users.bnbt" );

	if( CFG_GetString( "bnbt_static_header", string( ) ).empty( ) )
		CFG_SetString( "bnbt_static_header", string( ) );

	if( CFG_GetString( "bnbt_static_footer", string( ) ).empty( ) )
		CFG_SetString( "bnbt_static_footer", string( ) );

	if( CFG_GetString( "bnbt_robots_txt", string( ) ).empty( ) )
		CFG_SetString( "bnbt_robots_txt", string( ) );

	// addition by labarks

	if( CFG_GetString( "bnbt_rss_file", string( ) ).empty( ) )
		CFG_SetString( "bnbt_rss_file", string( ) );

	if( CFG_GetString( "bnbt_rss_online_dir", string( ) ).empty( ) )
		CFG_SetString( "bnbt_rss_online_dir", string( ) );

	if( CFG_GetInt( "bnbt_rss_file_mode", -1 ) < 0 )
		CFG_SetInt( "bnbt_rss_file_mode", 0 );

	if( CFG_GetString( "bnbt_rss_channel_title", string( ) ).empty( ) )
		CFG_SetString( "bnbt_rss_channel_title", "My BNBT RSS Feed" );

	if( CFG_GetString( "bnbt_rss_channel_description", string( ) ).empty( ) )
		CFG_SetString( "bnbt_rss_channel_description", "BitTorrent RSS Feed for BNBT" );

	if( CFG_GetInt( "bnbt_rss_channel_ttl", -1 ) < 0 )
		CFG_SetInt( "bnbt_rss_channel_ttl", 60 );

	if( CFG_GetString( "bnbt_rss_channel_language", string( ) ).empty( ) )
		CFG_SetString( "bnbt_rss_channel_language", "en-us" );

	if( CFG_GetString( "bnbt_rss_channel_image_url", string( ) ).empty( ) )
		CFG_SetString( "bnbt_rss_channel_image_url", string( ) );

	if( CFG_GetInt( "bnbt_rss_channel_image_width", -1 ) < 0 )
		CFG_SetInt( "bnbt_rss_channel_image_width", 0 );

	if( CFG_GetInt( "bnbt_rss_channel_image_height", -1 ) < 0 )
		CFG_SetInt( "bnbt_rss_channel_image_height", 0 );

	if( CFG_GetString( "bnbt_rss_channel_copyright", string( ) ).empty( ) )
		CFG_SetString( "bnbt_rss_channel_copyright", string( ) );

	if( CFG_GetInt( "bnbt_rss_limit", -1 ) < 0 )
		CFG_SetInt( "bnbt_rss_limit", 25 );

	// end addition

	if( CFG_GetString( "image_bar_fill", string( ) ).empty( ) )
		CFG_SetString( "image_bar_fill", string( ) );

	if( CFG_GetString( "image_bar_trans", string( ) ).empty( ) )
		CFG_SetString( "image_bar_trans", string( ) );

	if( CFG_GetString( "bnbt_rss_tracker_url", string( ) ).empty( ) )
	{
		string strTemp = CFG_GetString( "bnbt_rss_channel_link", string( ) );

		if( strTemp.empty( ) )
			CFG_SetString( "bnbt_rss_tracker_url", "http://localhost:6969/" );
		else
		{
			UTIL_LogPrint( "config - migrating bnbt_rss_channel_link to bnbt_rss_tracker_url\n" );

			CFG_SetString( "bnbt_rss_tracker_url", strTemp );
		}
	}

	if( CFG_GetString( "bnbt_force_announce_url", string( ) ).empty( ) )
		CFG_SetString( "bnbt_force_announce_url", string( ) );

	if( CFG_GetInt( "bnbt_force_announce_on_download", -1 ) < 0 )
		CFG_SetInt( "bnbt_force_announce_on_download", 0 );

	if( CFG_GetInt( "parse_allowed_interval", -1 ) < 0 )
		CFG_SetInt( "parse_allowed_interval", 0 );

	if( CFG_GetInt( "save_dfile_interval", 0 ) < 1 )
		CFG_SetInt( "save_dfile_interval", 300 );

	if( CFG_GetInt( "downloader_timeout_interval", 0 ) < 1 )
		CFG_SetInt( "downloader_timeout_interval", 2700 );

	if( CFG_GetInt( "bnbt_refresh_static_interval", 0 ) < 1 )
		CFG_SetInt( "bnbt_refresh_static_interval", 10 );

	if( CFG_GetInt( "bnbt_refresh_fast_cache_interval", -1 ) < 0 )
		CFG_SetInt( "bnbt_refresh_fast_cache_interval", 30 );

	// added by labarks

	if( CFG_GetInt( "bnbt_rss_interval", -1 ) < 0 )
		CFG_SetInt( "bnbt_rss_interval", 30 );

	if( CFG_GetInt( "announce_interval", -1 ) < 0 )
		CFG_SetInt( "announce_interval", 1800 );

	if( CFG_GetInt( "min_request_interval", -1 ) < 0 )
		CFG_SetInt( "min_request_interval", 18000 );

	if( CFG_GetInt( "response_size", -1 ) < 0 )
		CFG_SetInt( "response_size", 50 );

	if( CFG_GetInt( "max_give", -1 ) < 0 )
		CFG_SetInt( "max_give", 200 );

	if( CFG_GetInt( "keep_dead", -1 ) < 0 )
		CFG_SetInt( "keep_dead", 0 );

	if( CFG_GetInt( "bnbt_allow_scrape", -1 ) < 0 )
		CFG_SetInt( "bnbt_allow_scrape", 1 );

	if( CFG_GetInt( "bnbt_count_unique_peers", -1 ) < 0 )
		CFG_SetInt( "bnbt_count_unique_peers", 1 );

	if( CFG_GetInt( "bnbt_delete_invalid", -1 ) < 0 )
		CFG_SetInt( "bnbt_delete_invalid", 0 );

	if( CFG_GetInt( "bnbt_parse_on_upload", -1 ) < 0 )
		CFG_SetInt( "bnbt_parse_on_upload", 1 );

	if( CFG_GetInt( "bnbt_max_torrents", -1 ) < 0 )
		CFG_SetInt( "bnbt_max_torrents", 0 );

	if( CFG_GetInt( "bnbt_show_info_hash", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_info_hash", 0 );

	if( CFG_GetInt( "show_names", -1 ) < 0 )
		CFG_SetInt( "show_names", 1 );

	if( CFG_GetInt( "bnbt_show_stats", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_stats", 1 );

	if( CFG_GetInt( "bnbt_allow_torrent_downloads", -1 ) < 0 )
		CFG_SetInt( "bnbt_allow_torrent_downloads", 1 );

	if( CFG_GetInt( "bnbt_allow_comments", -1 ) < 0 )
		CFG_SetInt( "bnbt_allow_comments", 0 );

	if( CFG_GetInt( "bnbt_show_added", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_added", 1 );

	if( CFG_GetInt( "bnbt_show_size", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_size", 1 );

	if( CFG_GetInt( "bnbt_show_num_files", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_num_files", 1 );

	if( CFG_GetInt( "bnbt_show_completed", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_completed", 0 );

	if( CFG_GetInt( "bnbt_show_transferred", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_transferred", 0 );

	if( CFG_GetInt( "bnbt_show_min_left", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_min_left", 0 );

	if( CFG_GetInt( "bnbt_show_average_left", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_average_left", 0 );

	if( CFG_GetInt( "bnbt_show_max_left", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_max_left", 0 );

	if( CFG_GetInt( "bnbt_show_left_as_progress", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_left_as_progress", 1 );

	if( CFG_GetInt( "bnbt_show_uploader", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_uploader", 0 );

	if( CFG_GetInt( "bnbt_allow_info_link", -1 ) < 0 )
		CFG_SetInt( "bnbt_allow_info_link", 0 );

	if( CFG_GetInt( "bnbt_allow_search", -1 ) < 0 )
		CFG_SetInt( "bnbt_allow_search", 1 );

	if( CFG_GetInt( "bnbt_allow_sort", -1 ) < 0 )
		CFG_SetInt( "bnbt_allow_sort", 1 );

	if( CFG_GetInt( "bnbt_show_file_comment", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_file_comment", 1 );

	if( CFG_GetInt( "bnbt_show_file_contents", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_file_contents", 0 );

	if( CFG_GetInt( "bnbt_show_share_ratios", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_share_ratios", 1 );

	if( CFG_GetInt( "bnbt_show_average_dl_rate", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_average_dl_rate", 0 );

	if( CFG_GetInt( "bnbt_show_average_ul_rate", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_average_ul_rate", 0 );

	if( CFG_GetInt( "bnbt_delete_own_torrents", -1 ) < 0 )
		CFG_SetInt( "bnbt_delete_own_torrents", 1 );

	if( CFG_GetInt( "bnbt_show_gen_time", -1 ) < 0 )
		CFG_SetInt( "bnbt_show_gen_time", 1 );

	if( CFG_GetInt( "bnbt_per_page", -1 ) < 0 )
		CFG_SetInt( "bnbt_per_page", 20 );

	if( CFG_GetInt( "bnbt_users_per_page", -1 ) < 0 )
		CFG_SetInt( "bnbt_users_per_page", 50 );

	if( CFG_GetInt( "bnbt_max_peers_display", -1 ) < 0 )
		CFG_SetInt( "bnbt_max_peers_display", 500 );

	if( CFG_GetInt( "bnbt_guest_access", -1 ) < 0 )
		CFG_SetInt( "bnbt_guest_access", ACCESS_VIEW | ACCESS_DL | ACCESS_SIGNUP );

	if( CFG_GetInt( "bnbt_member_access", -1 ) < 0 )
		CFG_SetInt( "bnbt_member_access", ACCESS_VIEW | ACCESS_DL | ACCESS_COMMENTS | ACCESS_UPLOAD | ACCESS_SIGNUP );

	if( CFG_GetInt( "bnbt_file_expires", -1 ) < 0 )
		CFG_SetInt( "bnbt_file_expires", 180 );

	if( CFG_GetInt( "bnbt_name_length", -1 ) < 0 )
		CFG_SetInt( "bnbt_name_length", 32 );

	if( CFG_GetInt( "bnbt_comment_length", -1 ) < 0 )
		CFG_SetInt( "bnbt_comment_length", 800 );

	if( CFG_GetInt( "bnbt_max_recv_size", -1 ) < 0 )
		CFG_SetInt( "bnbt_max_recv_size", 524288 );

	if( CFG_GetInt( "only_local_override_ip", -1 ) < 0 )
		CFG_SetInt( "only_local_override_ip", 0 );

	if( CFG_GetInt( "bnbt_block_private_ip", -1 ) < 0 )
		if( CFG_GetInt( "cbtt_block_private_ip", -1 ) >= 0 ) {
            CFG_SetInt( "bnbt_block_private_ip", CFG_GetInt( "cbtt_block_private_ip", -1 ));
			CFG_Delete( "cbtt_block_private_ip");
		}
		else
			CFG_SetInt( "bnbt_block_private_ip", 0 );

	/* =X= */
	// Favicon support
	if( CFG_GetString( "favicon", string( ) ).empty() )
		CFG_SetString( "favicon", string( ) );

	/* =X= */
	// Internalised mouseover
	if( CFG_GetInt( "bnbt_use_mouseovers", -1 ) < 0 )
	{
		if( CFG_GetInt( "trinity_use_mouseovers", -1 ) < 1 )
		{
			CFG_SetInt( "bnbt_use_mouseovers", 0 );
			CFG_Delete( "trinity_use_mouseovers" );
		}
		else
		{
            CFG_SetInt( "bnbt_use_mouseovers", 1 );
			CFG_Delete( "trinity_use_mouseovers" );
		}
	}

	/* =X= */
	// Custom scrape and announce strings
	if( CFG_GetString( "bnbt_custom_announce", string( ) ).empty( ) )
	{
		string strTemp = CFG_GetString( "xbnbt_custom_announce", string( ) );

		if( strTemp.empty( ) )
		{
			CFG_SetString( "bnbt_custom_announce", string( ) );
			CFG_Delete( "xbnbt_custom_announce" );
		}
		else
		{
			CFG_SetString( "bnbt_custom_announce", strTemp );
			CFG_Delete( "xbnbt_custom_announce" );
		}
	}

	if( CFG_GetString( "bnbt_custom_scrape", string( ) ).empty( ) )
	{
		string strTemp = CFG_GetString( "xbnbt_custom_scrape", string( ) );

		if( strTemp.empty( ) )
		{
			CFG_SetString( "bnbt_custom_scrape", string( ) );
			CFG_Delete( "xbnbt_custom_scrape" );
		}
		else
		{
			CFG_SetString( "bnbt_custom_scrape", strTemp );
			CFG_Delete( "xbnbt_custom_scrape" );
		}
	}

	/* =X= */
	// Announce and scrape authorisation
	if( CFG_GetInt( "bnbt_announce_access_required", -1 ) < 0 )
		CFG_SetInt( "bnbt_announce_access_required", 0 );

	if( CFG_GetInt( "bnbt_scrape_access_required", -1 ) < 0 )
		CFG_SetInt( "bnbt_scrape_access_required", 0 );

	// DWKnight - XML Dump
	if( CFG_GetString( "bnbt_dump_xml_file", string( ) ).empty( ) )
		CFG_SetString( "bnbt_dump_xml_file", string( ) );

	if( CFG_GetInt( "bnbt_dump_xml_interval", 0 ) < 1 )
		CFG_SetInt( "bnbt_dump_xml_interval", 600 );

	if( CFG_GetInt( "bnbt_dump_xml_peers", -1 ) < 0 )
		CFG_SetInt( "bnbt_dump_xml_peers", 1 );

	// DWKnight - Private Tracker Flag
	if( CFG_GetInt( "bnbt_private_tracker_flag", -1 ) < 0 )
		CFG_SetInt( "bnbt_private_tracker_flag", 0 );

	if( CFG_GetInt( "bnbt_private_tracker_flag", -1 ) > 1)
		CFG_SetInt( "bnbt_private_tracker_flag", 1 );
}
