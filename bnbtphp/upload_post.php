<html>
<head>
<title>BNBTPHP File Uploader</title>
<link rel=stylesheet type="text/css" href="style.css">
</head>
<body>
<h3>BNBTPHP File Uploader</h3>
<?

define( 'BNBTPHP', 1 );

include './include/config.inc.php';
include './include/functions.inc.php';

if( !defined( 'BNBTPHP_INSTALLED' ) ) die( '<h1>Please run install.php before using BNBTPHP.</h1>' );

include './include/benc.inc.php';

mysql_pconnect( $dbhost, $dbuser, $dbpassword ) or die( mysql_error( ) );
mysql_select_db( $dbdatabase ) or die( mysql_error( ) );

if( $allowed_dir == '' )
	die( 'This tracker does not allow file uploads.' );

if( isset( $_FILES['torrent'] ) && strlen( $_POST['name'] ) < 100 )
{
	/* initial stuff */

	$torrent = $_FILES['torrent'];

	if( !is_uploaded_file( $torrent['tmp_name'] ) )
		die( 'not is_uploaded_file' );

	if( !filesize( $torrent['tmp_name'] ) )
		die( 'null file size' );

	$max_size = 524288;

	$dicti = bdec_file( $torrent['tmp_name'], $max_size );

	if( !isset( $dicti ) || $dicti['type'] != 'dictionary' )
		die( 'not a torrent or torrent too large' );

	/* force announce url */

	if( $force_announce_url != '' )
	{
		$announce = $dicti['value']['announce'];

		if( !isset( $announce ) || $announce['type'] != 'string' )
			die( 'not a torrent (missing announce key or announce key not a string)' );

		if( $announce['value'] != $force_announce_url )
			die( "announce url must be '$force_announce_url'" );
	}

	$info = $dicti['value']['info'];

	if( !isset( $info ) || $info['type'] != 'dictionary' )
		die( 'not a torrent (missing info key or info key not a dictionary)' );

	/* info hash */

	$hash_string = sha1( $info['string'] );
	$hash = string_to_hash( $hash_string );
	$hash_mysql = mysql_escape_string( $hash );

	/* name */

	$name = $info['value']['name'];

	if( isset( $name ) && $name['type'] == 'string' )
		$name = $name['value'];
	else
		die( 'not a torrent (missing name key or name key not a string)' );

	if( $_POST['name'] != '' )
		$name = $_POST['name'];

	$name_mysql = mysql_escape_string( $name );

	/* size and files */

	$size = 0;
	$files = 1;

	$len = $info['value']['length'];

	if( isset( $len ) && $len['type'] == 'integer' )
		$size = $len['value'];
	else
	{
		/* no length key, count sub lengths */

		$files_list = $info['value']['files'];

		if( isset( $files_list ) && $files_list['type'] == 'list' )
		{
			$files_list = $files_list['value'];

			foreach( $files_list as $f )
			{
				if( $f['type'] == 'dictionary' )
				{
					$f = $f['value']['length'];

					if( isset( $f ) && $f['type'] == 'integer' )
						$size += $f['value'];
					else
						die( 'not a torrent (missing files.x.length key or files.x.length key not an integer)' );
				}
				else
					die( 'not a torrent (files.x not a dictionary)' );
			}

			$files = count( $files_list );
		}
		else
			die( 'not a torrent (missing length key and files key)' );
	}

	/* done */

	$result = mysql_query( "SELECT COUNT(*) FROM allowed WHERE bhash='$hash_mysql'" ) or die( mysql_error( ) );

	if( mysql_result( $result, 0 ) > 0 )
		die( 'uploaded torrent already exists' );

	move_uploaded_file( $torrent['tmp_name'], $allowed_dir . $hash_string . '.torrent' ) or die( );

	mysql_query( "INSERT INTO allowed VALUES('$hash_mysql','$name_mysql')" ) or die( mysql_error( ) );
	mysql_query( "INSERT INTO allowed_ex VALUES('$hash_mysql',NOW(),$size,$files)" ) or die( mysql_error( ) );

	$name_temp = htmlspecialchars( stripslashes( $name ) );

	$size_temp = bytes_to_string( $size );

	echo "<h3>Upload Successful</h3>\n";
	echo "<ul>\n";
	echo "<li><strong>Name:</strong> $name_temp</li>\n";
	echo "<li><strong>Info Hash:</strong> $hash_string</li>\n";
	echo "<li><strong>Size:</strong> $size_temp</li>\n";
	echo "<li><strong>Files:</strong> $files</li>\n";
	echo "</ul>\n";
}
else
	echo "<p>I think you're looking for <a href=\"upload.html\">this</a>.</p>\n";

?>
</body>
</html>