<html>
<head>
<title>BNBTPHP Tracker Info</title>
<link rel=stylesheet type="text/css" href="style.css">
</head>
<body>
<center>
<p><a href="info.php">Info</a> | <a href="upload.html">Upload</a> | <a href="admin.php">Admin</a></p>
<h3>My BNBTPHP Tracker</h3>
<?

define( 'BNBTPHP', 1 );

include './include/config.inc.php';
include './include/functions.inc.php';

if( !defined( 'BNBTPHP_INSTALLED' ) ) die( '<h1>Please run install.php before using BNBTPHP.</h1>' );

$gen = getmicrotime( );

mysql_pconnect( $dbhost, $dbuser, $dbpassword ) or die( mysql_error( ) );
mysql_select_db( $dbdatabase ) or die( mysql_error( ) );

echo "<p><strong>Developer's Notes.</strong> This page will only show name, torrent, added, size, and files columns if the allowed and allowed_ex tables are populated. In other words, they will only display if the torrent was uploaded through the BNBTPHP File Uploader.</p>\n";

if( $allowed_dir != '' )
	$query = 'SELECT allowed.bhash,bseeders,bleechers,bcompleted,bname,badded,bsize,bfiles FROM allowed LEFT JOIN allowed_ex USING(bhash) LEFT JOIN torrents USING(bhash)';
else
	$query = 'SELECT bhash,bseeders,bleechers,bcompleted FROM torrents';

/* sort */

$sort = isset( $_GET['sort'] ) ? $_GET['sort'] : '';

if( $sort == 'aseeders' )
	$query .= ' ORDER BY bseeders';
else if( $sort == 'dseeders' )
	$query .= ' ORDER BY bseeders DESC';
else if( $sort == 'aleechers' )
	$query .= ' ORDER BY bleechers';
else if( $sort == 'dleechers' )
	$query .= ' ORDER BY bleechers DESC';
else if( $sort == 'acompleted' )
	$query .= ' ORDER BY bcompleted';
else if( $sort == 'dcompleted' )
	$query .= ' ORDER BY bcompleted DESC';
else if( $allowed_dir != '' )
{
	if( $sort == 'aname' )
		$query .= ' ORDER BY bname';
	else if( $sort == 'dname' )
		$query .= ' ORDER BY bname DESC';
	else if( $sort == 'aadded' )
		$query .= ' ORDER BY badded';
	else if( $sort == 'dadded' )
		$query .= ' ORDER BY badded DESC';
	else if( $sort == 'asize' )
		$query .= ' ORDER BY bsize';
	else if( $sort == 'dsize' )
		$query .= ' ORDER BY bsize DESC';
	else if( $sort == 'afiles' )
		$query .= ' ORDER BY bfiles';
	else if( $sort == 'dfiles' )
		$query .= ' ORDER BY bfiles DESC';

	/* default sort */

	else
		$query .= ' ORDER BY badded DESC';
}

/* pages */

$per_page = 20;

$page = isset( $_GET['page'] ) ? $_GET['page'] : '';

if( !is_numeric( $page ) || $page < 1 )
	$page = 1;

echo "<p class=\"pagenum_top\">Page $page</p>\n";

$offset = ( $page - 1 ) * $per_page;

if( $offset == 0 )
	$query .= " LIMIT $per_page";
else
	$query .= " LIMIT $offset,$per_page";

/* queries */

$result = mysql_query( $query ) or die( mysql_error( ) );

if( $allowed_dir != '' )
	$torrents = mysql_query( 'SELECT COUNT(*) FROM allowed' ) or die( mysql_error( ) );
else
	$torrents = mysql_query( 'SELECT COUNT(*) FROM torrents' ) or die( mysql_error( ) );

$torrents = mysql_result( $torrents, 0 );

/* table */

if( $torrents == 0 )
	echo "<p>Not tracking any files yet!</p>\n";
else
{
	echo "<table>\n";
	echo "<tr><th class=\"hash\">Info Hash</th>";

	if( $allowed_dir != '' )
	{
		echo "<th class=\"name\">Name<br><a class=\"sort\" href=\"index.php?sort=aname\">A</a> <a class=\"sort\" href=\"index.php?sort=dname\">Z</a></th>";
		echo "<th class=\"download\">Torrent</th>";
		echo "<th class=\"date\">Added<br><a class=\"sort\" href=\"index.php?sort=aadded\">A</a> <a class=\"sort\" href=\"index.php?sort=dadded\">Z</a></th>";
		echo "<th class=\"bytes\">Size<br><a class=\"sort\" href=\"index.php?sort=asize\">A</a> <a class=\"sort\" href=\"index.php?sort=dsize\">Z</a></th>";
		echo "<th class=\"number\">Files<br><a class=\"sort\" href=\"index.php?sort=afiles\">A</a> <a class=\"sort\" href=\"index.php?sort=dfiles\">Z</a></th>";
	}

	echo "<th class=\"number\">Seeders<br><a class=\"sort\" href=\"index.php?sort=aseeders\">A</a> <a class=\"sort\" href=\"index.php?sort=dseeders\">Z</a></th>";
	echo "<th class=\"number\">Leechers<br><a class=\"sort\" href=\"index.php?sort=aleechers\">A</a> <a class=\"sort\" href=\"index.php?sort=dleechers\">Z</a></th>";
	echo "<th class=\"number\">Completed<br><a class=\"sort\" href=\"index.php?sort=acompleted\">A</a> <a class=\"sort\" href=\"index.php?sort=dcompleted\">Z</a></th></tr>\n";

	$added = 0;

	while( $row = mysql_fetch_row( $result ) )
	{
		/*

		$row[0] is bhash
		$row[1] is bseeders
		$row[2] is bleechers
		$row[3] is bcompleted

		if( $allowed_dir != '' )
		{
			$row[4] is bname
			$row[5] is badded
			$row[6] is bsize
			$row[7] is bfiles
		}

		*/

		if( $added % 2 )
			echo "<tr class=\"even\">";
		else
			echo "<tr class=\"odd\">";

		/* info hash */

		$hash_string = hash_to_string( $row[0] );

		echo "<td class=\"hash\"><a class=\"stats\" href=\"stats.php?hash=$hash_string\">$hash_string</a></td>";

		/* name, torrent, added, size, files */

		if( $allowed_dir != '' )
		{
			$name_temp = htmlspecialchars( stripslashes( $row[4] ) );

			$torrent_url = 'http://' . $_SERVER['HTTP_HOST'] . dirname( $_SERVER['PHP_SELF'] ) . '/' . $allowed_dir . $hash_string . '.torrent';

			$size_temp = bytes_to_string( $row[6] );

			echo "<td class=\"name\"><a class=\"stats\" href=\"stats.php?hash=$hash_string\">$name_temp</a></td>";
			echo "<td class=\"download\"><a class=\"download\" href=\"$torrent_url\">DL</a></td>";
			echo "<td class=\"date\">$row[5]</td>\n";
			echo "<td class=\"bytes\">$size_temp</td>\n";
			echo "<td class=\"number\">$row[7]</td>\n";
		}

		/* seeders */

		if( $row[1] == 0 )
			echo "<td class=\"number_red\">$row[1]</td>";
		else if( $row[1] < 5 )
			echo "<td class=\"number_yellow\">$row[1]</td>";
		else
			echo "<td class=\"number_green\">$row[1]</td>";

		/* leechers */

		if( $row[2] == 0 )
			echo "<td class=\"number_red\">$row[2]</td>";
		else if( $row[2] < 5 )
			echo "<td class=\"number_yellow\">$row[2]</td>";
		else
			echo "<td class=\"number_green\">$row[2]</td>";

		/* completed */

		echo "<td class=\"number\">$row[3]</td></tr>\n";

		$added++;
	}

	echo "</table>\n";
}

/* pages */

$count = ceil( $torrents / $per_page );

echo "<p class=\"pagenum_bottom\">";

for( $i = 1; $i <= $count; $i++ )
{
	if( $i == $page )
		echo " $i ";
	else
		echo " <a href=\"index.php?page=$i&sort=$sort\">$i</a> ";

	if( $i < $count )
		echo "|";
}

echo "</p>\n";
echo "<p>Stats on this page may be delayed.</p>\n";

/* generation time */

echo "<p>Generated in " . ( number_format( getmicrotime( ) - $gen, 4 ) ) . " seconds.</p>\n";

?>
<p>POWERED BY BNBTPHP Beta 1.0</p>
</center>
</body>
</html>