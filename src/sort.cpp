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

#include "bnbt.h"
#include "sort.h"
#include "tracker.h"
#include "util.h"

int asortByName( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem1 )->strLowerName.compare( ( (const struct torrent_t *)elem2 )->strLowerName );
}

int asortByComplete( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem1 )->iSeeders - ( (const struct torrent_t *)elem2 )->iSeeders;
}

int asortByDL( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem1 )->iLeechers - ( (const struct torrent_t *)elem2 )->iLeechers;
}

int asortByAdded( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem1 )->strAdded.compare( ( (const struct torrent_t *)elem2 )->strAdded );
}

int asortBySize( const void *elem1, const void *elem2 )
{
	// int64's will overflow, force a compare

	const struct torrent_t *tor1 = (const struct torrent_t *)elem1;
	const struct torrent_t *tor2 = (const struct torrent_t *)elem2;

	if( tor1->iSize < tor2->iSize )
		return -1;
	else if( tor1->iSize > tor2->iSize )
		return 1;
	else
		return 0;
}

int asortByFiles( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem1 )->iFiles - ( (const struct torrent_t *)elem2 )->iFiles;
}

int asortByComments( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem1 )->iComments - ( (const struct torrent_t *)elem2 )->iComments;
}

int asortByAvgLeft( const void *elem1, const void *elem2 )
{
	// int64's will overflow, force a compare

	const struct torrent_t *tor1 = (const struct torrent_t *)elem1;
	const struct torrent_t *tor2 = (const struct torrent_t *)elem2;

	if( tor1->iAverageLeft < tor2->iAverageLeft )
		return -1;
	else if( tor1->iAverageLeft > tor2->iAverageLeft )
		return 1;
	else
		return 0;
}

int asortByAvgLeftPercent( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem1 )->iAverageLeftPercent - ( (const struct torrent_t *)elem2 )->iAverageLeftPercent;
}

int asortByCompleted( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem1 )->iCompleted - ( (const struct torrent_t *)elem2 )->iCompleted;
}

int asortByTransferred( const void *elem1, const void *elem2 )
{
	// int64's will overflow, force a compare

	const struct torrent_t *tor1 = (const struct torrent_t *)elem1;
	const struct torrent_t *tor2 = (const struct torrent_t *)elem2;

	if( tor1->iTransferred < tor2->iTransferred )
		return -1;
	else if( tor1->iTransferred > tor2->iTransferred )
		return 1;
	else
		return 0;
}

int asortByTag( const void *elem1, const void *elem2 ) /* =X= */
{
	return ( (const struct torrent_t *)elem1 )->strTag.compare( ( (const struct torrent_t *)elem2 )->strTag );
}

int asortByUploader( const void *elem1, const void *elem2 ) /* =X= */
{
	return ( (const struct torrent_t *)elem1 )->strUploader.compare( ( (const struct torrent_t *)elem2 )->strUploader );
}

int dsortByName( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem2 )->strLowerName.compare( ( (const struct torrent_t *)elem1 )->strLowerName );
}

int dsortByComplete( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem2 )->iSeeders - ( (const struct torrent_t *)elem1 )->iSeeders;
}

int dsortByDL( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem2 )->iLeechers - ( (const struct torrent_t *)elem1 )->iLeechers;
}

int dsortByAdded( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem2 )->strAdded.compare( ( (const struct torrent_t *)elem1 )->strAdded );
}

int dsortBySize( const void *elem1, const void *elem2 )
{
	// int64's will overflow, force a compare

	const struct torrent_t *tor1 = (const struct torrent_t *)elem1;
	const struct torrent_t *tor2 = (const struct torrent_t *)elem2;

	if( tor1->iSize < tor2->iSize )
		return 1;
	else if( tor1->iSize > tor2->iSize )
		return -1;
	else
		return 0;
}

int dsortByFiles( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem2 )->iFiles - ( (const struct torrent_t *)elem1 )->iFiles;
}

int dsortByComments( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem2 )->iComments - ( (const struct torrent_t *)elem1 )->iComments;
}

int dsortByAvgLeft( const void *elem1, const void *elem2 )
{
	// int64's will overflow, force a compare

	const struct torrent_t *tor1 = (const struct torrent_t *)elem1;
	const struct torrent_t *tor2 = (const struct torrent_t *)elem2;

	if( tor1->iAverageLeft < tor2->iAverageLeft )
		return 1;
	else if( tor1->iAverageLeft > tor2->iAverageLeft )
		return -1;
	else
		return 0;
}

int dsortByAvgLeftPercent( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem2 )->iAverageLeftPercent - ( (const struct torrent_t *)elem1 )->iAverageLeftPercent;
}

int dsortByCompleted( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem2 )->iCompleted - ( (const struct torrent_t *)elem1 )->iCompleted;
}

int dsortByTransferred( const void *elem1, const void *elem2 )
{
	// int64's will overflow, force a compare

	const struct torrent_t *tor1 = (const struct torrent_t *)elem1;
	const struct torrent_t *tor2 = (const struct torrent_t *)elem2;

	if( tor1->iTransferred < tor2->iTransferred )
		return 1;
	else if( tor1->iTransferred > tor2->iTransferred )
		return -1;
	else
		return 0;
}

int dsortByTag( const void *elem1, const void *elem2 ) /* =X= */
{
	return ( (const struct torrent_t *)elem2 )->strTag.compare( ( (const struct torrent_t *)elem1 )->strTag );
}

int dsortByUploader( const void *elem1, const void *elem2 ) /* =X= */
{
	return ( (const struct torrent_t *)elem2 )->strUploader.compare( ( (const struct torrent_t *)elem1 )->strUploader );
}

int asortpByUpped( const void *elem1, const void *elem2 )
{
	// int64's will overflow, force a compare

	const struct peer_t *peer1 = (const struct peer_t *)elem1;
	const struct peer_t *peer2 = (const struct peer_t *)elem2;

	if( peer1->iUpped < peer2->iUpped )
		return -1;
	else if( peer1->iUpped > peer2->iUpped )
		return 1;
	else
		return 0;
}

int asortpByDowned( const void *elem1, const void *elem2 )
{
	// int64's will overflow, force a compare

	const struct peer_t *peer1 = (const struct peer_t *)elem1;
	const struct peer_t *peer2 = (const struct peer_t *)elem2;

	if( peer1->iDowned < peer2->iDowned )
		return -1;
	else if( peer1->iDowned > peer2->iDowned )
		return 1;
	else
		return 0;
}

int asortpByLeft( const void *elem1, const void *elem2 )
{
	// int64's will overflow, force a compare

	const struct peer_t *peer1 = (const struct peer_t *)elem1;
	const struct peer_t *peer2 = (const struct peer_t *)elem2;

	if( peer1->iLeft < peer2->iLeft )
		return -1;
	else if( peer1->iLeft > peer2->iLeft )
		return 1;
	else
		return 0;
}

int asortpByConnected( const void *elem1, const void *elem2 )
{
	return ( (const struct peer_t *)elem1 )->iConnected - ( (const struct peer_t *)elem2 )->iConnected;
}

int asortpByShareRatio( const void *elem1, const void *elem2 )
{
	const float fl1 = ( (const struct peer_t *)elem1 )->flShareRatio;
	const float fl2 = ( (const struct peer_t *)elem2 )->flShareRatio;

	// this is complicated because -1 means infinite and casting to ints won't work

	// Elandal: else is unnecessary because of return...
	// Elandal: floating point comparison using == or != is not recommended.
	//          It just might not work as expected. Don't have time to fix now.
	//          (fix I did against 80b-2 might not be any better)

	if( ( ( -0.001 < ( fl1 - fl2 ) ) && ( ( fl1 - fl2 ) < 0.001 ) ) ) return 0;
	if( ( -1.001 < fl1 ) && ( fl1 < -0.999 ) ) return 1;
	if( ( -1.001 < fl2 ) && ( fl2 < -0.999 ) ) return -1;
	if( ( ( fl1 - fl2 ) < -0.001 ) )return -1;

	return 1;
}

int dsortpByUpped( const void *elem1, const void *elem2 )
{
	// int64's will overflow, force a compare

	const struct peer_t *peer1 = (const struct peer_t *)elem1;
	const struct peer_t *peer2 = (const struct peer_t *)elem2;

	if( peer1->iUpped < peer2->iUpped )
		return 1;
	else if( peer1->iUpped > peer2->iUpped )
		return -1;
	else
		return 0;
}

int dsortpByDowned( const void *elem1, const void *elem2 )
{
	// int64's will overflow, force a compare

	const struct peer_t *peer1 = (const struct peer_t *)elem1;
	const struct peer_t *peer2 = (const struct peer_t *)elem2;

	if( peer1->iDowned < peer2->iDowned )
		return 1;
	else if( peer1->iDowned > peer2->iDowned )
		return -1;
	else
		return 0;
}

int dsortpByLeft( const void *elem1, const void *elem2 )
{
	// int64's will overflow, force a compare

	const struct peer_t *peer1 = (const struct peer_t *)elem1;
	const struct peer_t *peer2 = (const struct peer_t *)elem2;

	if( peer1->iLeft < peer2->iLeft )
		return 1;
	else if( peer1->iLeft > peer2->iLeft )
		return -1;
	else
		return 0;
}

int dsortpByConnected( const void *elem1, const void *elem2 )
{
	return ( (const struct peer_t *)elem2 )->iConnected - ( (const struct peer_t *)elem1 )->iConnected;
}

int dsortpByShareRatio( const void *elem1, const void *elem2 )
{
	const float fl1 = ( (const struct peer_t *)elem1 )->flShareRatio;
	const float fl2 = ( (const struct peer_t *)elem2 )->flShareRatio;

	// this is complicated because -1 means infinite and casting to ints won't work

	if( ( ( -0.001 < ( fl1 - fl2 ) ) && ( ( fl1 - fl2 ) < 0.001 ) ) ) return 0;
	if( ( -1.001 < fl1 ) && ( fl1 < -0.999 ) ) return -1;
	if( ( -1.001 < fl2 ) && ( fl2 < -0.999 ) ) return 1;
	if( ( ( fl1 - fl2 ) < -0.001 ) )return 1;

	return -1;
}

int asortuByLogin( const void *elem1, const void *elem2 )
{
	return ( (const struct user_t *)elem1 )->strLowerLogin.compare( ( (const struct user_t *)elem2 )->strLowerLogin );
}

int asortuByAccess( const void *elem1, const void *elem2 )
{
	return ( (const struct user_t *)elem2 )->iAccess - ( (const struct user_t *)elem1 )->iAccess;
}

int asortuByMail( const void *elem1, const void *elem2 )
{
	return ( (const struct user_t *)elem1 )->strLowerMail.compare( ( (const struct user_t *)elem2 )->strLowerMail );
}

int asortuByCreated( const void *elem1, const void *elem2 )
{
	return ( (const struct user_t *)elem1 )->strCreated.compare( ( (const struct user_t *)elem2 )->strCreated );
}

int dsortuByLogin( const void *elem1, const void *elem2 )
{
	return ( (const struct user_t *)elem2 )->strLowerLogin.compare( ( (const struct user_t *)elem1 )->strLowerLogin );
}

int dsortuByAccess( const void *elem1, const void *elem2 )
{
	return ( (const struct user_t *)elem1 )->iAccess - ( (const struct user_t *)elem2 )->iAccess;
}

int dsortuByMail( const void *elem1, const void *elem2 )
{
	return ( (const struct user_t *)elem2 )->strLowerMail.compare( ( (const struct user_t *)elem1 )->strLowerMail );
}

int dsortuByCreated( const void *elem1, const void *elem2 )
{
	return ( (const struct user_t *)elem2 )->strCreated.compare( ( (const struct user_t *)elem1 )->strCreated );
}

int asortByIP( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem1 )->strIP.compare( ( (const struct torrent_t *)elem2 )->strIP );
}

int dsortByIP( const void *elem1, const void *elem2 )
{
	return ( (const struct torrent_t *)elem2 )->strIP.compare( ( (const struct torrent_t *)elem1 )->strIP );
}

int asortpByClient( const void *elem1, const void *elem2 )
{
	return ( (const struct peer_t *)elem1 )->strClientType.compare( ( (const struct peer_t *)elem2 )->strClientType );
}

int dsortpByClient( const void *elem1, const void *elem2 )
{
	return ( (const struct peer_t *)elem2 )->strClientType.compare( ( (const struct peer_t *)elem1 )->strClientType );
}

int asortpByIP( const void *elem1, const void *elem2 )
{
	return ( (const struct peer_t *)elem1 )->strIP.compare( ( (const struct peer_t *)elem2 )->strIP );
}

int dsortpByIP( const void *elem1, const void *elem2 )
{
	return ( (const struct peer_t *)elem2 )->strIP.compare( ( (const struct peer_t *)elem1 )->strIP );
}

int asortpByAUR( const void *elem1, const void *elem2 )
{
	const struct peer_t *peer1 = (const struct peer_t *)elem1;
	const struct peer_t *peer2 = (const struct peer_t *)elem2;

	if( peer1->iConnected !=0 && peer2->iConnected !=0 )
	{
		if( peer1->iConnected && ( peer1->iUpped / peer1->iConnected ) < ( peer2->iUpped / peer2->iConnected ) )
			return 1;
		else if( ( peer1->iUpped / peer1->iConnected ) > ( peer2->iUpped / peer2->iConnected ) )
			return -1;
		else
			return 0;
	}
	else
		return 0;
}

int dsortpByAUR( const void *elem1, const void *elem2 )
{
	const struct peer_t *peer1 = (const struct peer_t *)elem1;
	const struct peer_t *peer2 = (const struct peer_t *)elem2;

	if( peer1->iConnected != 0 && peer2->iConnected != 0 )
	{
		if( ( peer1->iConnected && peer2->iConnected ) && ( peer2->iUpped / peer2->iConnected ) < ( peer1->iUpped / peer1->iConnected ) )
			return 1;
		else if( ( peer2->iUpped / peer2->iConnected ) > ( peer1->iUpped / peer1->iConnected ) )
			return -1;
		else
			return 0;
	}
	else
		return 0;
}

int asortpByADR( const void *elem1, const void *elem2 )
{
	const struct peer_t *peer1 = (const struct peer_t *)elem1;
	const struct peer_t *peer2 = (const struct peer_t *)elem2;

	if( peer1->iConnected !=0 && peer2->iConnected !=0 )
	{
		if( ( peer1->iDowned / peer1->iConnected ) < ( peer2->iDowned / peer2->iConnected ) )
			return 1;
		else if( ( peer1->iDowned / peer1->iConnected ) > ( peer2->iDowned / peer2->iConnected ) )
			return -1;
		else
			return 0;
	}
	else
		return 0;
}

int dsortpByADR( const void *elem1, const void *elem2 )
{
	const struct peer_t *peer1 = (const struct peer_t *)elem1;
	const struct peer_t *peer2 = (const struct peer_t *)elem2;

	if( peer1->iConnected !=0 && peer2->iConnected !=0 )
	{
		if( ( peer2->iDowned / peer2->iConnected ) < ( peer1->iDowned / peer1->iConnected ) )
			return 1;
		else if( ( peer2->iDowned / peer2->iConnected ) > ( peer1->iDowned / peer1->iConnected ) )
			return -1;
		else
			return 0;
	}
	else
		return 0;
}

