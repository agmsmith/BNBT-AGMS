<html>
<head>
<title>BNBTPHP Admin Panel</title>
<link rel=stylesheet type="text/css" href="style.css">
</head>
<body>
<h3>BNBTPHP Admin Panel</h3>
<?

define( 'BNBTPHP', 1 );

include './include/config.inc.php';
include './include/functions.inc.php';

if( !defined( 'BNBTPHP_INSTALLED' ) ) die( '<h1>Please run install.php before using BNBTPHP.</h1>' );

$gen = getmicrotime( );

mysql_pconnect( $dbhost, $dbuser, $dbpassword ) or die( mysql_error( ) );
mysql_select_db( $dbdatabase ) or die( mysql_error( ) );

echo "<p><strong>Developer's Notes.</strong> This page will only work if the allowed and allowed_ex tables are populated since deleting torrents from an open tracker has no effect. In other words, it will only work if the torrent was uploaded through the BNBTPHP File Uploader.</p>\n";

$password = isset( $_GET['password'] ) ? $_GET['password'] : '';

if( $password == $admin_password )
{
	if( $allowed_dir != '' )
	{
		$delete = isset( $_GET['delete'] ) ? $_GET['delete'] : '';

		if( $delete != '' )
		{
			echo "<p>Deleting torrent from disk...</p>\n";

			unlink( $allowed_dir . $delete . '.torrent' );

			echo "<p>Deleting torrent from database...</p>\n";

			$delete_mysql = mysql_escape_string( string_to_hash( $delete ) );

			mysql_query( "DELETE FROM allowed WHERE bhash='$delete_mysql'" ) or die( mysql_error( ) );
			mysql_query( "DELETE FROM allowed_ex WHERE bhash='$delete_mysql'" ) or die( mysql_error( ) );
			mysql_query( "DELETE FROM dstate WHERE bhash='$delete_mysql'" ) or die( mysql_error( ) );

			echo "<p>Done!</p>\n";
		}

		$result = mysql_query( 'SELECT allowed.bhash,bname FROM allowed LEFT JOIN torrents USING(bhash)' ) or die( mysql_error( ) );

		echo "<table>\n";
		echo "<tr><th>Info Hash</th><th>Name</th><th>Delete</th></tr>\n";

		while( $row = mysql_fetch_row( $result ) )
			printf( "<tr><td>%s</td><td>%s</td><td><a href=\"admin.php?password=%s&delete=%s\">Delete</a></td></tr>\n", hash_to_string( $row[0] ), htmlspecialchars( stripslashes( $row[1] ) ), $password, hash_to_string( $row[0] ) );

		echo "</table>\n";
	}
	else
		echo "<p>Nothing here!</p>\n";
}
else
{
?>
<form method="get" action="admin.php">
<input name="password" type="password" size="24"> Password<br><br>
<input type="submit" value="Submit">
</form>
<?
}

/* generation time */

echo "<p>Generated in " . ( number_format( getmicrotime( ) - $gen, 4 ) ) . " seconds.</p>\n";

?>
</body>
</html>