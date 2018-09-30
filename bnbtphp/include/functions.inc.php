<?

if( !defined( 'BNBTPHP' ) ) die( "You're not allowed in here!" );

function getmicrotime( )
{
	list( $usec, $sec ) = explode( " ", microtime( ) );

	return ( (float)$usec + (float)$sec );
}

function bytes_to_string( $bytes )
{
	$sizes = array( 'B', 'KB', 'MB', 'GB', 'TB', 'PB' );
	$extension = $sizes[0];

	for( $i = 1; ( ( $i < count( $sizes ) ) && ( $bytes >= 1024 ) ); $i++ )
	{
		$bytes /= 1024;
		$extension = $sizes[$i];
	}

	return round( $bytes, 2 ) . ' ' . $extension;
}

function hash_to_string( $hash )
{
	return bin2hex( $hash );
}

function string_to_hash( $string )
{
	return pack( "H*", $string );
}

?>