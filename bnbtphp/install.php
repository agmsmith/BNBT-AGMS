<html>
<head>
<title>BNBTPHP Install</title>
<link rel=stylesheet type="text/css" href="style.css">
</head>
<body>
<h3>BNBTPHP Install</h3>
<?

define( 'BNBTPHP', 1 );

include './include/config.inc.php';
include './include/functions.inc.php';

if( defined( 'BNBTPHP_INSTALLED' ) ) die( 'already installed' );

$install = isset( $_GET['install'] ) ? $_GET['install'] : '';

if( $install == 'ok' )
{
	echo "<p>Writing config to ./include/config.inc.php...</p>\n";

	$new_dbhost = isset( $_GET['dbhost'] ) ? $_GET['dbhost'] : '';
	$new_dbdatabase = isset( $_GET['dbdatabase'] ) ? $_GET['dbdatabase'] : '';
	$new_dbuser = isset( $_GET['dbuser'] ) ? $_GET['dbuser'] : '';
	$new_dbpassword = isset( $_GET['dbpassword'] ) ? $_GET['dbpassword'] : '';
	$new_admin_password = isset( $_GET['admin_password'] ) ? $_GET['admin_password'] : '';
	$new_allowed_dir = isset( $_GET['allowed_dir'] ) ? $_GET['allowed_dir'] : '';
	$new_force_announce_url = isset( $_GET['force_announce_url'] ) ? $_GET['force_announce_url'] : '';

	$config = '<?' . "\n";
	$config .= "\n";
	$config .= 'if( !defined( \'BNBTPHP\' ) ) die( "You\'re not allowed in here!" );' . "\n";
	$config .= "\n";
	$config .= 'define( \'BNBTPHP_INSTALLED\', 1 );' . "\n";
	$config .= "\n";
	$config .= '$dbhost = \'' . $new_dbhost . '\';' . "\n";
	$config .= '$dbdatabase = \'' . $new_dbdatabase . '\';' . "\n";
	$config .= '$dbuser = \'' . $new_dbuser . '\';' . "\n";
	$config .= '$dbpassword = \'' . $new_dbpassword . '\';' . "\n";
	$config .= "\n";
	$config .= '$admin_password = \'' . $new_admin_password . '\';' . "\n";
	$config .= "\n";
	$config .= '$allowed_dir = \'' . $new_allowed_dir . '\';' . "\n";
	$config .= "\n";
	$config .= '$force_announce_url = \'' . $new_force_announce_url . '\';' . "\n";
	$config .= "\n";
	$config .= '?>';

	if( !( $fp = fopen( './include/config.inc.php', 'w' ) ) )
		die( 'error opening ./include/config.inc.php for writing' );

	fputs( $fp, $config, strlen( $config ) );
	fclose( $fp );

	echo "<p>Done!</p>\n";
	echo "<p>Configuring MySQL...</p>\n";

	mysql_pconnect( $new_dbhost, $new_dbuser, $new_dbpassword ) or die( mysql_error( ) );

	echo "<p>Dropping database if it exists...</p>\n";

	mysql_query( "DROP DATABASE IF EXISTS $new_dbdatabase" ) or die( mysql_error( ) );

	echo "<p>Creating database...</p>\n";

	mysql_query( "CREATE DATABASE $new_dbdatabase" ) or die( mysql_error ( ) );

	mysql_select_db( $new_dbdatabase ) or die( mysql_error( ) );

	echo "<p>Creating tables...</p>\n";

	mysql_query( 'CREATE TABLE allowed ( bhash BLOB, bname VARCHAR(255) NOT NULL, PRIMARY KEY( bhash(20) ) )' ) or die( mysql_error ( ) );
	mysql_query( 'CREATE TABLE allowed_ex ( bhash BLOB, badded DATETIME NOT NULL, bsize BIGINT UNSIGNED NOT NULL, bfiles INT UNSIGNED NOT NULL, PRIMARY KEY( bhash(20) ) )' ) or die( mysql_error ( ) );
	mysql_query( 'CREATE TABLE torrents ( bhash BLOB, bseeders INT UNSIGNED NOT NULL, bleechers INT UNSIGNED NOT NULL, bcompleted INT UNSIGNED NOT NULL, PRIMARY KEY( bhash(20) ) )' ) or die( mysql_error ( ) );
	mysql_query( 'CREATE TABLE hashes ( bhash BLOB, PRIMARY KEY( bhash(20) ) )' ) or die( mysql_error ( ) );
	mysql_query( 'CREATE TABLE seeders ( bhash BLOB, bseeders INT UNSIGNED NOT NULL, PRIMARY KEY( bhash(20) ) )' ) or die( mysql_error ( ) );
	mysql_query( 'CREATE TABLE leechers ( bhash BLOB, bleechers INT UNSIGNED NOT NULL, PRIMARY KEY( bhash(20) ) )' ) or die( mysql_error ( ) );
	mysql_query( 'CREATE TABLE dstate ( bhash BLOB, bid BLOB, bip VARCHAR(15) NOT NULL, bport SMALLINT UNSIGNED NOT NULL, buploaded BIGINT UNSIGNED NOT NULL, bdownloaded BIGINT UNSIGNED NOT NULL, bleft BIGINT UNSIGNED NOT NULL, btime DATETIME NOT NULL, PRIMARY KEY( bhash(20), bid(20) ) )' ) or die( mysql_error ( ) );
	mysql_query( 'CREATE TABLE completed ( bhash BLOB, bcompleted INT UNSIGNED NOT NULL, PRIMARY KEY( bhash(20) ) )' ) or die( mysql_error ( ) );

	echo "<p>Done!</p>\n";
	echo "<p>You should remove global write access from ./include/config.inc.php as soon as possible.</p>\n";
}
else
{
?>
<p><strong>Requirements</strong></p>
<p>Please ensure ./include/config.inc.php has global write access before continuing.</p>
<p>If you are reinstalling BNBTPHP, you should delete your allowed dir and recreate it before continuing.</p>
<p><strong>Step 1 - Setup MySQL</strong></p>
<p>Warning! This install script will attempt to drop and recreate the specified database.</p>
<form method="get" action="install.php">
<input name="dbhost" type="text" size="24" value="localhost"> MySQL Host<br><br>
<input name="dbdatabase" type="text" size="24" value="bnbt"> MySQL Database (required)<br><br>
<input name="dbuser" type="text" size="24"> MySQL User<br><br>
<input name="dbpassword" type="text" size="24"> MySQL Password<br><br>
<p><strong>Step 2 - Setup Admin Password</strong></p>
<input name="admin_password" type="text" size="24"> Admin Password<br><br>
<p><strong>Step 3 - Setup Allowed Dir</strong></p>
<p>Please read the BNBT documentation for more information on the allowed dir.</p>
<input name="allowed_dir" type="text" size="24" value="allowed/"> Allowed Dir (if blank, some features will be disabled)<br><br>
<p><strong>Step 4 - Setup Force Announce URL</strong></p>
<p>Please read the BNBT documentation for more information on the force announce URL.</p>
<input name="force_announce_url" type="text" size="24"> Force Announce URL (if blank, force announce URL will be disabled)<br><br>
<input name="install" type="hidden" value="ok">
<input type="submit" value="Submit">
</form>
<?
}
?>
</body>
</html>