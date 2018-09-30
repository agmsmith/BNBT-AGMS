<html>
<head>
<title>BNBTPHP Tracker Info</title>
<link rel=stylesheet type="text/css" href="style.css">
</head>
<body>
<h3>BNBTPHP Tracker Info</h3>
<?

define( 'BNBTPHP', 1 );

include './include/config.inc.php';
include './include/functions.inc.php';

if( !defined( 'BNBTPHP_INSTALLED' ) ) die( '<h1>Please run install.php before using BNBTPHP.</h1>' );

$gen = getmicrotime( );

mysql_pconnect( $dbhost, $dbuser, $dbpassword ) or die( mysql_error( ) );
mysql_select_db( $dbdatabase ) or die( mysql_error( ) );

$result = mysql_query( 'SELECT COUNT(*), SUM(bseeders), SUM(bleechers) FROM torrents' ) or die( mysql_error( ) );

if( $row = mysql_fetch_row( $result ) )
{
	$peers = $row[1] + $row[2];

	echo "<ul>\n";
	echo "<li>Tracking $row[0] Files, $peers Peers, $row[1] Seeders, $row[2] Leechers</li>\n";
	echo "</ul>\n";
}

echo "<p>Stats on this page may be delayed.</p>\n";

/* generation time */

echo "<p>Generated in " . ( number_format( getmicrotime( ) - $gen, 4 ) ) . " seconds.</p>\n";

?>
</body>
</html>