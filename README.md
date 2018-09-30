# BNBT-AGMS
This is a fixed up version of the BNBT BitTorrent Tracker to get it to run on current day computers, which have larger numbers (32 or 64 bits) and newer code libraries.

A Tracker is a dual purpose server-side program that lists available torrents for download on a web page, and lets client programs find other client programs for file sharing.

I've started with the code from BNBT 8.5 as of July 2006, as found on https://sourceforge.net/projects/bnbtusermods/files/BNBT85/July%202006%20Critical%20Update/ in 20060727-bnbt85-src.zip  The documentation and sample files (without the precompiled executables) are from bnbt-85b-untested.zip, a slightly earlier version.

I'm trying to get the basic BNBT to compile under Fedora 28 Linux, a circa 2018 version of the GNU/Linux operating system.  There's an option for a MySQL database version of BNBT, which I'm ignoring, so that likely won't build.

- AGMS20180930


# BNBT Beta 8.1 (a C++ BitTorrent tracker)

http://bnbt.go-dedicated.com/

BNBT was written by Trevor Hogan. BNBT is a complete port of the original Python BitTorrent tracker to C++ using the STL for data storage and basic network sockets for network communication. BNBT is fast, efficient, customizable, easy to use, powerful, and portable. BNBT is covered under the GNU Lesser General Public License (LGPL).

## DOCUMENTATION

THIS IS IMPORTANT! PLEASE READ THE DOCUMENTATION!

The documentation for this release is available in the bnbt/doc directory. Updated documentation is available at http://bnbt.go-dedicated.com/doc/.

## CHANGELOG

Beta 8.1
 - major optimizations (no more threads)
  * tracker links were removed since they were threaded
 - added 'fast cache' to speed up index/scrape generation at a cost of delayed stats
  * added the "bnbt_refresh_fast_cache_interval" config value
  * the tracker will cache index/scrape data for at most this many seconds
 - slightly better error messages (thanks Olaf)
 - fixed a bug where BNBT would report bad headers when none were sent
 - minor optimizations (e.g. global recv buffer)
 - minor MySQL query changes
 - modified MySQL tables to use BLOB instead of BINARY CHAR
 - removed XML dumping
 - added RSS dumping (thanks labarks)
  * added a ton of config values for RSS dumping
 - changed /torrent.html linking to /torrents/<hash>.torrent
 - no longer send compressed responses if size_of_compressed > size_of_raw
 - fixed some compiler warnings and removed -w flag from Makefile

Beta 8.0 Release 2
 - fixed a serious crash bug with tracker links on secondary trackers
 - fixed a bug where BNBT would constantly refresh the static header and footer (credit Elandal)
 - fixed a query error with stats dumping when mysql_override_dstate was off
 - mysql.h is now include as mysql/mysql.h on some systems
 - and stdio.h is now included by default on all systems
 - fixed a major bug where some internal databases would be needlessly copied and deleted
 - added the "bnbt_max_recv_size" config value

Beta 8.0
 - supressed connection reset errors
 - supressed deflate errors (on Z_OK)
 - added MySQL support
  * added the "mysql_host" config value
  * added the "mysql_database" config value
  * added the "mysql_user" config value
  * added the "mysql_password" config value
  * added the "mysql_port" config value
  * added the "mysql_refresh_allowed_interval" config value
  * added the "mysql_refresh_stats_interval" config value
  * added the "mysql_override_dstate" config value
  * added BNBTPHP (example PHP frontend)
 - added the "bnbt_robots_txt" config value
 - removed the zlib source code folder
 - fixed a bug where deleted torrents would sometimes recover their tag data on restart
 - externally linked torrents are now linked even more correctly
 - pruned the changelog
