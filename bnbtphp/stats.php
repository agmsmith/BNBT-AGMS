<html>
<head>
<title>BNBTPHP File Info</title>
<link rel=stylesheet type="text/css" href="style.css">
</head>
<body>
<h3>BNBTPHP File Info</h3>
<?

define( 'BNBTPHP', 1 );

include './include/config.inc.php';
include './include/functions.inc.php';

if( !defined( 'BNBTPHP_INSTALLED' ) ) die( '<h1>Please run install.php before using BNBTPHP.</h1>' );

$gen = getmicrotime( );

mysql_pconnect( $dbhost, $dbuser, $dbpassword ) or die( mysql_error( ) );
mysql_select_db( $dbdatabase ) or die( mysql_error( ) );

echo "<p><strong>Developer's Notes.</strong> This page will only show seeders and leechers if mysql_override_dstate = 1 in bnbt.cfg. The File Information list will only display if the allowed and allowed_ex tables are populated. In other words, it will only display if the torrent was uploaded through the BNBTPHP File Uploader.</p>\n";

$hash_string = isset( $_GET['hash'] ) ? $_GET['hash'] : '';
$hash = string_to_hash( $hash_string );
$hash_mysql = mysql_escape_string( $hash );

if( $allowed_dir != '' )
{
	/* query allowed_ex data */

	$result = mysql_query( "SELECT bname,badded,bsize,bfiles FROM allowed LEFT JOIN allowed_ex USING(bhash) WHERE allowed.bhash='$hash_mysql'" ) or die( mysql_error( ) );

	if( $row = mysql_fetch_row( $result ) )
	{
		$name_temp = htmlspecialchars( stripslashes( $row[0] ) );

		$torrent_url = 'http://' . $_SERVER['HTTP_HOST'] . dirname( $_SERVER['PHP_SELF'] ) . '/' . $allowed_dir . $hash_string . '.torrent';

		$size_temp = bytes_to_string( $row[2] );

		echo "<p>File Information</p>\n";
		echo "<ul>\n";
		echo "<li><strong>Name:</strong> $name_temp</li>\n";
		echo "<li><strong>Info Hash:</strong> $hash_string</li>\n";
		echo "<li><strong>Added:</strong> $row[1]</li>\n";
		echo "<li><strong>Size:</strong> $size_temp</li>\n";
		echo "<li><strong>Files:</strong> $row[3]</li>\n";
		echo "</ul>\n";
		echo "<p><a class=\"download\" href=\"$torrent_url\">DOWNLOAD TORRENT</a></p>";
	}
}

$query_seeders = "SELECT bip,buploaded,bdownloaded FROM dstate WHERE bhash='$hash_mysql' AND bleft=0";
$query_leechers = "SELECT bip,buploaded,bdownloaded,bleft FROM dstate WHERE bhash='$hash_mysql' AND bleft!=0";

/* sort */

$sort = isset( $_GET['sort'] ) ? $_GET['sort'] : '';

if( $sort == 'aip' )
{
	$query_seeders .= ' ORDER BY bip';
	$query_leechers .= ' ORDER BY bip';
}
else if( $sort == 'dip' )
{
	$query_seeders .= ' ORDER BY bip DESC';
	$query_leechers .= ' ORDER BY bip DESC';
}
else if( $sort == 'auploaded' )
{
	$query_seeders .= ' ORDER BY buploaded';
	$query_leechers .= ' ORDER BY buploaded';
}
else if( $sort == 'duploaded' )
{
	$query_seeders .= ' ORDER BY buploaded DESC';
	$query_leechers .= ' ORDER BY buploaded DESC';
}
else if( $sort == 'adownloaded' )
{
	$query_seeders .= ' ORDER BY bdownloaded';
	$query_leechers .= ' ORDER BY bdownloaded';
}
else if( $sort == 'ddownloaded' )
{
	$query_seeders .= ' ORDER BY bdownloaded DESC';
	$query_leechers .= ' ORDER BY bdownloaded DESC';
}
else if( $sort == 'aleft' )
	$query_leechers .= ' ORDER BY bleft';
else if( $sort == 'dleft' )
	$query_leechers .= ' ORDER BY bleft DESC';

/* default sort */

else
{
	$query_seeders .= ' ORDER BY buploaded DESC';
	$query_leechers .= ' ORDER BY bleft';
}

/* note - no need to htmlspecialchars or stripslashes on bip since BNBT won't insert bad IP's */

/* seeders */

$result = mysql_query( $query_seeders ) or die( mysql_error( ) );

$seeders = mysql_num_rows( $result );

if( $seeders == 0 )
	echo "<p>No Seeders</p>\n";
else
{
	echo "<p>Seeders ($seeders)</p>\n";
	echo "<table>\n";
	echo "<tr><th class=\"ip\">Peer IP<br><a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=aip\">A</a> <a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=dip\">Z</a></th>";
	echo "<th class=\"bytes\">Uploaded<br><a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=auploaded\">A</a> <a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=duploaded\">Z</a></th>";
	echo "<th class=\"bytes\">Downloaded<br><a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=adownloaded\">A</a> <a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=ddownloaded\">Z</a></th></tr>\n";

	while( $row = mysql_fetch_row( $result ) )
		printf( "<tr><td class=\"ip\">%s</td><td class=\"bytes\">%s</td><td class=\"bytes\">%s</td></tr>\n", $row[0], bytes_to_string( $row[1] ), bytes_to_string( $row[2] ) );

	echo "</table>\n";
}

/* leechers */

$result = mysql_query( $query_leechers ) or die( mysql_error( ) );

$leechers = mysql_num_rows( $result );

if( $leechers == 0 )
	echo "<p>No Leechers</p>\n";
else
{
	echo "<p>Leechers ($leechers)</p>\n";
	echo "<table>\n";
	echo "<tr><th class=\"ip\">Peer IP<br><a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=aip\">A</a> <a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=dip\">Z</a></th>";
	echo "<th class=\"bytes\">Uploaded<br><a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=auploaded\">A</a> <a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=duploaded\">Z</a></th>";
	echo "<th class=\"bytes\">Downloaded<br><a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=adownloaded\">A</a> <a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=ddownloaded\">Z</a></th>";
	echo "<th class=\"bytes\">Left<br><a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=aleft\">A</a> <a class=\"sort\" href=\"stats.php?hash=$hash_string&sort=dleft\">Z</a></th></tr>\n";

	while( $row = mysql_fetch_row( $result ) )
		printf( "<tr><td class=\"ip\">%s</td><td class=\"bytes\">%s</td><td class=\"bytes\">%s</td><td class=\"bytes\">%s</td></tr>\n", $row[0], bytes_to_string( $row[1] ), bytes_to_string( $row[2] ), bytes_to_string( $row[3] ) );

	echo "</table>\n";
}

echo "<p>Stats on this page are real time.</p>\n";

/* generation time */

echo "<p>Generated in " . ( number_format( getmicrotime( ) - $gen, 4 ) ) . " seconds.</p>\n";

?>
</body>
</html>