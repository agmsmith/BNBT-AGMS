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
#include "atom.h"
#include "md5.h"
#include "sort.h"
#include "tracker.h"
#include "util.h"

void CTracker :: serverResponseUsers( struct request_t *pRequest, struct response_t *pResponse )
{
	struct bnbttv btv = UTIL_CurrentTime( );
	
	time_t tNow = time( NULL ) + m_iRefreshFastCacheInterval;
	char *szTime = asctime( gmtime( &tNow ) );
	szTime[strlen( szTime ) - 1] = '\0';
	
	pResponse->strCode = "200 OK";

	pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", string( gmapMime[".html"] + "; charset=" ) + gstrCharSet ) ); /* =X= */
	pResponse->mapHeaders.insert( pair<string, string>( "Expires", string( szTime ) + " GMT" ) );

	pResponse->strContent += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n"; /* =X= */
	pResponse->strContent += "<html lang=\"en\">\n";
	pResponse->strContent += "<head>\n";
	pResponse->strContent += "<title>BNBT Users Info</title>\n";

	if( !gstrStyle.empty( ) )
		pResponse->strContent += "<link rel=\"stylesheet\" title=\"external\" type=\"" + gmapMime[".css"] + "\" href=\"" + gstrStyle + "\">\n"; /* =X= */

	/* =X= */
	// Embeded Styles for browser bugfixes
	// The style tags are visible on Pre-HTML 3.2 browsers
	if( pRequest->user.iAccess & ACCESS_VIEW )
	{
		pResponse->strContent += "<style type=\"" + gmapMime[".css"] + "\" title=\"internal\">\n";
		pResponse->strContent += "<!--\n";
		pResponse->strContent += "html{overflow-x:auto}\n"; /* IE6 scroll bar bugfix */
		pResponse->strContent += "td{empty-cells:show}\n"; /* Empty cells bugfix */
		pResponse->strContent += "-->\n";
		pResponse->strContent += "</style>\n";
	}

	/* =X= */
	if( !m_strFavicon.empty( ) )
	{
		string strExt = string( );

		string :: size_type iExt = m_strFaviconFile.rfind( "." );

		if( iExt != string :: npos )
			strExt = m_strFaviconFile.substr( iExt );

		if( strExt == ".ico" )
			pResponse->strContent += "<link rel=\"Shortcut Icon\" type=\"" + gmapMime[".ico"] + "\" href=\"favicon.ico\">\n";
		else if( strExt == ".png" )
			pResponse->strContent += "<link rel=\"Shortcut Icon\" type=\"" + gmapMime[".png"] + "\" href=\"favicon.ico\">\n";
		else if( strExt == ".gif" )
			pResponse->strContent += "<link rel=\"Shortcut Icon\" type=\"" + gmapMime[".gif"] + "\" href=\"favicon.ico\">\n";
	}

	/* =X= */
	// META information
	pResponse->strContent += "<META http-equiv=\"Content-Type\" content=\"text/html; charset=" + gstrCharSet + "\">\n";
	pResponse->strContent += "<META http-equiv=\"Content-Script-Type\" content=\"text/javascript\">\n";
	pResponse->strContent += "<META http-equiv=\"Expires\" content=\"" + string( szTime ) + " GMT\">\n";
	pResponse->strContent += "<META name=\"Generator\" content=\"BNBT " + string( BNBT_VER ) + "\">\n";
	pResponse->strContent += "<META name=\"MSSmartTagsPreventParsing\" content=\"true\">\n";

	pResponse->strContent += "</head>\n";
	pResponse->strContent += "<body>\n\n";

	/* =X= */
	// Display a message if javascript not supported by browser
	pResponse->strContent += "<noscript>\n";    
	pResponse->strContent += "<p class=\"js_warning\">Please enable JavaScript support or upgrade your browser.</p>\n";   
	pResponse->strContent += "</noscript>\n\n"; 

	if( pRequest->user.strLogin.empty( ) )
		pResponse->strContent += "<p class=\"login1_upload\">You are not logged in. Click <a href=\"/login.html\">here</a> to login.</p>\n";
	else
		pResponse->strContent += "<p class=\"login2_upload\">You are logged in as <span class=\"username\">" + UTIL_RemoveHTML( pRequest->user.strLogin ) + "</span>. Click <a href=\"/login.html?logout=1\">here</a> to logout.</p>\n";

	pResponse->strContent += m_strStaticHeader; /* =X= */

	pResponse->strContent += "<h3>BNBT Users Info</h3>\n";

	if( pRequest->user.iAccess & ACCESS_ADMIN )
	{
		//
		// create user
		//

		if( pRequest->mapParams.find( "us_login" ) != pRequest->mapParams.end( ) &&
			pRequest->mapParams.find( "us_password" ) != pRequest->mapParams.end( ) &&
			pRequest->mapParams.find( "us_password_verify" ) != pRequest->mapParams.end( ) &&
			pRequest->mapParams.find( "us_email" ) != pRequest->mapParams.end( ) )
		{
			string strLogin = pRequest->mapParams["us_login"];
			string strPass = pRequest->mapParams["us_password"];
			string strPass2 = pRequest->mapParams["us_password_verify"];
			string strMail = pRequest->mapParams["us_email"];
			string strAccView = pRequest->mapParams["us_access_view"];
			string strAccDL = pRequest->mapParams["us_access_dl"];
			string strAccComments = pRequest->mapParams["us_access_comments"];
			string strAccUpload = pRequest->mapParams["us_access_upload"];
			string strAccEdit = pRequest->mapParams["us_access_edit"];
			string strAccAdmin = pRequest->mapParams["us_access_admin"];
			string strAccSignup = pRequest->mapParams["us_access_signup"];

			if( strLogin.empty( ) || strPass.empty( ) || strPass2.empty( ) || strMail.empty( ) )
			{
				pResponse->strContent += "<p>Unable to create user. You must fill in all the fields. Click <a href=\"/users.html\">here</a> to return to the users page.</p>\n";
				pResponse->strContent += m_strStaticFooter; /* =X= */
				
				if( m_bGen )
					pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

				pResponse->strContent += "</body>\n";
				pResponse->strContent += "</html>\n";

				return;
			}
			else
			{
				if( strPass == strPass2 )
				{
					if( m_pUsers->getItem( strLogin ) )
					{
						pResponse->strContent += "<p>Unable to create user. The user \"" + UTIL_RemoveHTML( strLogin ) + "\" already exists. Click <a href=\"/users.html\">here</a> to return to the users page.</p>\n";
						pResponse->strContent += m_strStaticFooter; /* =X= */

						if( m_bGen )
							pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
						
						pResponse->strContent += "</body>\n";
						pResponse->strContent += "</html>\n";

						return;
					}
					else
					{
						int iAccess = 0;

						if( strAccView == "on" )
							iAccess += ACCESS_VIEW;
						if( strAccDL == "on" )
							iAccess += ACCESS_DL;
						if( strAccComments == "on" )
							iAccess += ACCESS_COMMENTS;
						if( strAccUpload == "on" )
							iAccess += ACCESS_UPLOAD;
						if( strAccEdit == "on" )
							iAccess += ACCESS_EDIT;
						if( strAccAdmin == "on" )
							iAccess += ACCESS_ADMIN;
						if( strAccSignup == "on" )
							iAccess += ACCESS_SIGNUP;

						addUser( strLogin, strPass, iAccess, strMail );

						pResponse->strContent += "<p>Created user \"" + UTIL_RemoveHTML( strLogin ) + "\". Click <a href=\"/users.html\">here</a> to return to the users page.</p>\n";
						pResponse->strContent += m_strStaticFooter; /* =X= */

						if( m_bGen )
							pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
						
						pResponse->strContent += "</body>\n";
						pResponse->strContent += "</html>\n";

						return;
					}
				}
				else
				{
					pResponse->strContent += "<p>Unable to create user. The passwords did not match. Click <a href=\"/users.html\">here</a> to return to the users page.</p>\n";
					pResponse->strContent += m_strStaticFooter; /* =X= */
					
					if( m_bGen )
						pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
					
					pResponse->strContent += "</body>\n";
					pResponse->strContent += "</html>\n";

					return;
				}
			}
		}

		//
		// edit user
		//

		string strPass = pRequest->mapParams["us_password"];
		string strPass2 = pRequest->mapParams["us_password_verify"];
		string strMail = pRequest->mapParams["us_email"];
		string strAccView = pRequest->mapParams["us_access_view"];
		string strAccDL = pRequest->mapParams["us_access_dl"];
		string strAccComments = pRequest->mapParams["us_access_comments"];
		string strAccUpload = pRequest->mapParams["us_access_upload"];
		string strAccEdit = pRequest->mapParams["us_access_edit"];
		string strAccAdmin = pRequest->mapParams["us_access_admin"];
		string strAccSignup = pRequest->mapParams["us_access_signup"];

		string strUser = pRequest->mapParams["user"];
		string strAction = pRequest->mapParams["action"];
		string strOK = pRequest->mapParams["ok"];

		if( strAction == "edit" )
		{
			CAtom *pUserToEdit = m_pUsers->getItem( strUser );

			if( pUserToEdit && pUserToEdit->isDicti( ) )
			{
				if( strOK == "1" )
				{
					// edit password

					if( !strPass.empty( ) && !strPass2.empty( ) )
					{
						if( strPass == strPass2 )
						{
							string strA1 = strUser + ":" + gstrRealm + ":" + strPass;

							unsigned char szMD5[16];

							MD5_CTX md5;

							MD5Init( &md5 );
							MD5Update( &md5, (const unsigned char *)strA1.c_str( ), (unsigned int)strA1.size( ) );
							MD5Final( szMD5, &md5 );

							( (CAtomDicti *)pUserToEdit )->setItem( "md5", new CAtomString( string( (char *)szMD5, 16 ) ) );
						}
						else
						{
							pResponse->strContent += "<p>Unable to edit user. The passwords did not match. Click <a href=\"/users.html\">here</a> to return to the users page.</p>\n";
							pResponse->strContent += m_strStaticFooter; /* =X= */

							if( m_bGen )
								pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
							
							pResponse->strContent += "</body>\n";
							pResponse->strContent += "</html>\n";

							return;
						}
					}

					// edit mail

					( (CAtomDicti *)pUserToEdit )->setItem( "email", new CAtomString( strMail ) );

					// edit access

					int iAccess = 0;

					if( strAccView == "on" )
						iAccess += ACCESS_VIEW;
					if( strAccDL == "on" )
						iAccess += ACCESS_DL;
					if( strAccComments == "on" )
						iAccess += ACCESS_COMMENTS;
					if( strAccUpload == "on" )
						iAccess += ACCESS_UPLOAD;
					if( strAccEdit == "on" )
						iAccess += ACCESS_EDIT;
					if( strAccAdmin == "on" )
						iAccess += ACCESS_ADMIN;
					if( strAccSignup == "on" )
						iAccess += ACCESS_SIGNUP;

					( (CAtomDicti *)pUserToEdit )->setItem( "access", new CAtomLong( iAccess ) );

					saveUsers( );

					pResponse->strContent += "<p>Edited user \"" + UTIL_RemoveHTML( strUser ) + "\". Click <a href=\"/users.html\">here</a> to return to the users page.</p>\n";
					pResponse->strContent += m_strStaticFooter; /* =X= */

					if( m_bGen )
						pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
					
					pResponse->strContent += "</body>\n";
					pResponse->strContent += "</html>\n";

					return;
				}
				else
				{
					int iAccess = 0;

					CAtom *pAccessToEdit = ( (CAtomDicti *)pUserToEdit )->getItem( "access" );

					if( pAccessToEdit && dynamic_cast<CAtomLong *>( pAccessToEdit ) )
						iAccess = (int)dynamic_cast<CAtomLong *>( pAccessToEdit )->getValue( );

					pResponse->strContent += "<form method=\"get\" action=\"/users.html\">\n";
					pResponse->strContent += "<p><strong>Edit User \"" + UTIL_RemoveHTML( strUser ) + "\"</strong></p>\n";
					pResponse->strContent += "<input name=\"user\" type=hidden value=\"" + UTIL_StringToEscaped( strUser ) + "\">\n";
					pResponse->strContent += "<input name=\"action\" type=hidden value=\"edit\">\n";
					pResponse->strContent += "<input name=\"ok\" type=hidden value=1>\n";
					pResponse->strContent += "<input name=\"us_password\" type=password size=20> Password (optional)<br>\n";
					pResponse->strContent += "<input name=\"us_password_verify\" type=password size=20> Verify Password (optional)<br><br>\n";
					pResponse->strContent += "<input name=\"us_email\" type=text size=40";

					CAtom *pMailToEdit = ( (CAtomDicti *)pUserToEdit )->getItem( "email" );

					if( pMailToEdit )
						pResponse->strContent += " value=\"" + pMailToEdit->toString( ) + "\"";

					pResponse->strContent += "> E-Mail<br><br>\n";
					pResponse->strContent += "<input name=\"us_access_view\" type=checkbox";

					if( iAccess & ACCESS_VIEW )
						pResponse->strContent += " checked";

					pResponse->strContent += "> View Access (Basic)<br>\n";
					pResponse->strContent += "<input name=\"us_access_dl\" type=checkbox";

					if( iAccess & ACCESS_DL )
						pResponse->strContent += " checked";

					pResponse->strContent += "> DL Access (Downloader)<br>\n";
					pResponse->strContent += "<input name=\"us_access_comments\" type=checkbox";

					if( iAccess & ACCESS_COMMENTS )
						pResponse->strContent += " checked";

					pResponse->strContent += "> Comments Access (Poster)<br>\n";
					pResponse->strContent += "<input name=\"us_access_upload\" type=checkbox";

					if( iAccess & ACCESS_UPLOAD )
						pResponse->strContent += " checked";

					pResponse->strContent += "> Upload Access (Uploader)<br>\n";
					pResponse->strContent += "<input name=\"us_access_edit\" type=checkbox";

					if( iAccess & ACCESS_EDIT )
						pResponse->strContent += " checked";

					pResponse->strContent += "> Edit Access (Moderator)<br>\n";
					pResponse->strContent += "<input name=\"us_access_admin\" type=checkbox";

					if( iAccess & ACCESS_ADMIN )
						pResponse->strContent += " checked";

					pResponse->strContent += "> Admin Access (Admin)<br>\n";
					pResponse->strContent += "<input name=\"us_access_signup\" type=checkbox";

					if( iAccess & ACCESS_SIGNUP )
						pResponse->strContent += " checked";

					pResponse->strContent += "> Signup Access<br><br>\n";
					pResponse->strContent += "<input type=submit value=\"Edit User\">\n";
					pResponse->strContent += "</form>\n";
					pResponse->strContent += m_strStaticFooter; /* =X= */
					
					if( m_bGen )
						pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
					
					pResponse->strContent += "</body>\n";
					pResponse->strContent += "</html>\n";

					return;
				}
			}
			else
			{
				pResponse->strContent += "<p>Unable to edit user. The user \"" + UTIL_RemoveHTML( strUser ) + "\" does not exist. Click <a href=\"/users.html\">here</a> to return to the users page.</p>\n";
				pResponse->strContent += m_strStaticFooter; /* =X= */

				if( m_bGen )
					pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
				
				pResponse->strContent += "</body>\n";
				pResponse->strContent += "</html>\n";

				return;
			}
		}

		//
		// delete user
		//

		else if( strAction == "delete" )
		{
			if( strOK == "1" )
			{
				deleteUser( strUser );

				pResponse->strContent += "<p>Deleted user \"" + UTIL_RemoveHTML( strUser ) + "\". Click <a href=\"/users.html\">here</a> to return to the users page.</p>\n";
				pResponse->strContent += m_strStaticFooter; /* =X= */

				if( m_bGen )
					pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
				
				pResponse->strContent += "</body>\n";
				pResponse->strContent += "</html>\n";

				return;
			}
			else
			{
				pResponse->strContent += "<p>Are you sure you want to delete the user \"" + UTIL_RemoveHTML( strUser ) + "\"? WARNING! If there are no admin users, you won't be able to administrate your tracker!</p>\n";
				pResponse->strContent += "<p><a href=\"/users.html?user=" + UTIL_StringToEscaped( strUser ) + "&action=delete&ok=1\">OK</a></p>\n";
				pResponse->strContent += m_strStaticFooter; /* =X= */

				if( m_bGen )
					pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
				
				pResponse->strContent += "</body>\n";
				pResponse->strContent += "</html>\n";

				return;
			}
		}

		//
		// create user
		//

		pResponse->strContent += "<form method=\"get\" action=\"/users.html\">\n";
		pResponse->strContent += "<p><strong>Create User</strong></p>\n";
		pResponse->strContent += "<input name=\"us_login\" type=text size=24> Login<br><br>\n";
		pResponse->strContent += "<input name=\"us_password\" type=password size=20> Password<br>\n";
		pResponse->strContent += "<input name=\"us_password_verify\" type=password size=20> Verify Password<br><br>\n";
		pResponse->strContent += "<input name=\"us_email\" type=text size=40> E-Mail<br><br>\n";
		pResponse->strContent += "<input name=\"us_access_view\" type=checkbox> View Access (Basic)<br>\n";
		pResponse->strContent += "<input name=\"us_access_dl\" type=checkbox> DL Access (Downloader)<br>\n";
		pResponse->strContent += "<input name=\"us_access_comments\" type=checkbox> Comments Access (Poster)<br>\n";
		pResponse->strContent += "<input name=\"us_access_upload\" type=checkbox> Upload Access (Uploader)<br>\n";
		pResponse->strContent += "<input name=\"us_access_edit\" type=checkbox> Edit Access (Moderator)<br>\n";
		pResponse->strContent += "<input name=\"us_access_admin\" type=checkbox> Admin Access (Admin)<br>\n";
		pResponse->strContent += "<input name=\"us_access_signup\" type=checkbox> Signup Access<br><br>\n";
		pResponse->strContent += "<input type=submit value=\"Create User\">\n";
		pResponse->strContent += "</form>\n";

		//
		// user table
		//

		if( m_pUsers )
		{
			if( m_pUsers->isEmpty( ) )
			{
				pResponse->strContent += "<p><strong>WARNING! Your tracker does not have any users. Guests will have full access until someone creates the first user.</strong></p>\n";
				pResponse->strContent += m_strStaticFooter; /* =X= */

				if( m_bGen )
					pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
				
				pResponse->strContent += "</body>\n";
				pResponse->strContent += "</html>\n";

				return;
			}

			map<string, CAtom *> *pmapDicti = m_pUsers->getValuePtr( );

			unsigned long iKeySize = (unsigned long)pmapDicti->size( );

			// add the users into this structure one by one and sort it afterwards

			struct user_t *pUsersT = new struct user_t[iKeySize];

			unsigned long user_iter = 0;

			for( map<string, CAtom *> :: iterator it = pmapDicti->begin( ); it != pmapDicti->end( ); it++ )
			{
				pUsersT[user_iter].strLogin = (*it).first;
				pUsersT[user_iter].strLowerLogin = UTIL_ToLower( pUsersT[user_iter].strLogin );
				pUsersT[user_iter].iAccess = m_iGuestAccess;

				if( (*it).second->isDicti( ) )
				{
					CAtom *pMD5 = ( (CAtomDicti *)(*it).second )->getItem( "md5" );
					CAtom *pAccess = ( (CAtomDicti *)(*it).second )->getItem( "access" );
					CAtom *pMail = ( (CAtomDicti *)(*it).second )->getItem( "email" );
					CAtom *pCreated = ( (CAtomDicti *)(*it).second )->getItem( "created" );

					if( pMD5 )
						pUsersT[user_iter].strMD5 = pMD5->toString( );

					if( pMail )
					{
						pUsersT[user_iter].strMail = pMail->toString( );
						pUsersT[user_iter].strLowerMail = UTIL_ToLower( pUsersT[user_iter].strMail );
					}

					if( pAccess && dynamic_cast<CAtomLong *>( pAccess ) )
						pUsersT[user_iter].iAccess = (int)dynamic_cast<CAtomLong *>( pAccess )->getValue( );

					if( pCreated )
						pUsersT[user_iter].strCreated = pCreated->toString( );
				}

				user_iter++;
			}

			string strSort = pRequest->mapParams["sort"];

			if( !strSort.empty( ) )
			{
				int iSort = atoi( strSort.c_str( ) );

				if( iSort == SORTU_ALOGIN )
					qsort( pUsersT, iKeySize, sizeof( struct user_t ), asortuByLogin );
				else if( iSort == SORTU_AACCESS )
					qsort( pUsersT, iKeySize, sizeof( struct user_t ), asortuByAccess );
				else if( iSort == SORTU_AEMAIL )
					qsort( pUsersT, iKeySize, sizeof( struct user_t ), asortuByMail );
				else if( iSort == SORTU_ACREATED )
					qsort( pUsersT, iKeySize, sizeof( struct user_t ), asortuByCreated );
				else if( iSort == SORTU_DLOGIN )
					qsort( pUsersT, iKeySize, sizeof( struct user_t ), dsortuByLogin );
				else if( iSort == SORTU_DACCESS )
					qsort( pUsersT, iKeySize, sizeof( struct user_t ), dsortuByAccess );
				else if( iSort == SORTU_DEMAIL )
					qsort( pUsersT, iKeySize, sizeof( struct user_t ), dsortuByMail );
				else if( iSort == SORTU_DCREATED )
					qsort( pUsersT, iKeySize, sizeof( struct user_t ), dsortuByCreated );
				else
				{
					// default action is to sort by created

					qsort( pUsersT, iKeySize, sizeof( struct user_t ), dsortuByCreated );
				}
			}
			else
			{
				// default action is to sort by created

				qsort( pUsersT, iKeySize, sizeof( struct user_t ), dsortuByCreated );
			}

			// some preliminary search crap

			string strSearch = pRequest->mapParams["search"];
			string strLowerSearch = UTIL_ToLower( strSearch );
			string strSearchResp = UTIL_StringToEscaped( strSearch );

			if( !strSearch.empty( ) )
				pResponse->strContent += "<p class=\"search_results\">Search results for \"" + UTIL_RemoveHTML( strSearch ) + "\".</p>\n";

			// which page are we viewing

			unsigned int iStart = 0;

			if( m_iUsersPerPage > 0 )
			{
				string strPage = pRequest->mapParams["page"];

				if( !strPage.empty( ) )
					iStart = atoi( strPage.c_str( ) ) * m_iUsersPerPage;

				pResponse->strContent += "<p class=\"pagenum_top\">Page " + CAtomInt( ( iStart / m_iUsersPerPage ) + 1 ).toString( ) + "</p>\n";
			}

			bool bFound = false;

			int iAdded = 0;
			int iSkipped = 0;

			// for correct page numbers after searching

			int iFound = 0;

			for( unsigned long i = 0; i < iKeySize; i++ )
			{
				if( !strSearch.empty( ) )
				{
					// only display entries that match the search

					if( pUsersT[i].strLowerLogin.find( strLowerSearch ) == string :: npos )
						continue;
				}

				iFound++;

				if( m_iUsersPerPage == 0 || iAdded < m_iUsersPerPage )
				{
					if( !bFound )
					{
						// output table headers

						pResponse->strContent += "<p>Users</p>\n";
						pResponse->strContent += "<table summary=\"users\">\n";
						pResponse->strContent += "<tr><th class=\"name\">Login";
						pResponse->strContent += "<br><a class=\"sort\" href=\"/users.html?sort=" + SORTUSTR_ALOGIN;

						if( !strSearch.empty( ) )
							pResponse->strContent += "&search=" + strSearchResp;

						pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/users.html?sort=" + SORTUSTR_DLOGIN;

						if( !strSearch.empty( ) )
							pResponse->strContent += "&search=" + strSearchResp;

						pResponse->strContent += "\">Z</a>";
						pResponse->strContent += "</th><th>Access";
						pResponse->strContent += "<br><a class=\"sort\" href=\"/users.html?sort=" + SORTUSTR_AACCESS;

						if( !strSearch.empty( ) )
							pResponse->strContent += "&search=" + strSearchResp;

						pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/users.html?sort=" + SORTUSTR_DACCESS;

						if( !strSearch.empty( ) )
							pResponse->strContent += "&search=" + strSearchResp;

						pResponse->strContent += "\">Z</a>";
						pResponse->strContent += "</th><th>E-Mail";
						pResponse->strContent += "<br><a class=\"sort\" href=\"/users.html?sort=" + SORTUSTR_AEMAIL;

						if( !strSearch.empty( ) )
							pResponse->strContent += "&search=" + strSearchResp;

						pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/users.html?sort=" + SORTUSTR_DEMAIL;

						if( !strSearch.empty( ) )
							pResponse->strContent += "&search=" + strSearchResp;

						pResponse->strContent += "\">Z</a>";
						pResponse->strContent += "</th><th>Created";
						pResponse->strContent += "<br><a class=\"sort\" href=\"/users.html?sort=" + SORTUSTR_ACREATED;

						if( !strSearch.empty( ) )
							pResponse->strContent += "&search=" + strSearchResp;

						pResponse->strContent += "\">A</a> <a class=\"sort\" href=\"/users.html?sort=" + SORTUSTR_DCREATED;

						if( !strSearch.empty( ) )
							pResponse->strContent += "&search=" + strSearchResp;

						pResponse->strContent += "\">Z</a>";
						pResponse->strContent += "</th><th>Admin</th></tr>\n";

						bFound = true;
					}

					if( iSkipped == (int)iStart )
					{
						// output table rows

						if( iAdded % 2 )
							pResponse->strContent += "<tr class=\"even\">";
						else
							pResponse->strContent += "<tr class=\"odd\">";

						pResponse->strContent += "<td class=\"name\">";
						pResponse->strContent += UTIL_RemoveHTML( pUsersT[i].strLogin );
						pResponse->strContent += "</td><td>";
						pResponse->strContent += UTIL_AccessToString( pUsersT[i].iAccess );
						pResponse->strContent += "</td><td>";
						pResponse->strContent += UTIL_RemoveHTML( pUsersT[i].strMail );
						pResponse->strContent += "</td><td>";

						if( !pUsersT[i].strCreated.empty( ) )
						{
							// strip year and seconds from time

							pResponse->strContent += pUsersT[i].strCreated.substr( 5, pUsersT[i].strCreated.size( ) - 8 );
						}

						pResponse->strContent += "</td><td>[<a href=\"/users.html?user=" + UTIL_StringToEscaped( pUsersT[i].strLogin ) + "&action=edit\">Edit</a>] [<a href=\"/users.html?user=" + UTIL_StringToEscaped( pUsersT[i].strLogin ) + "&action=delete\">Delete</a>]</td></tr>\n";

						iAdded++;
					}
					else
						iSkipped++;
				}
			}

			delete [] pUsersT;

			// some finishing touches

			if( bFound )
				pResponse->strContent += "</table>\n";

			pResponse->strContent += "<span class=\"search_users\"><form method=\"get\" action=\"/users.html\">\n";

			if( !strSort.empty( ) )
				pResponse->strContent += "<input name=\"sort\" type=hidden value=\"" + strSort + "\">\n";

			pResponse->strContent += "Search <input name=\"search\" type=text size=40> <a href=\"/users.html\">Clear Search</a>\n";
			pResponse->strContent += "</form></span>\n";

			// page numbers

			if( m_iUsersPerPage > 0 )
			{
				pResponse->strContent += "<p class=\"pagenum_bottom\">";

				for( unsigned long i = 0; i < (unsigned int)iFound; i += m_iUsersPerPage )
				{
					pResponse->strContent += " ";

					// don't link to current page

					if( i != iStart )
					{
						pResponse->strContent += "<a href=\"/users.html?page=" + CAtomInt( (int) i / m_iUsersPerPage ).toString( );

						if( !strSort.empty( ) )
							pResponse->strContent += "&sort=" + strSort;

						if( !strSearch.empty( ) )
							pResponse->strContent += "&search=" + strSearchResp;

						pResponse->strContent += "\">";
					}

					pResponse->strContent += CAtomInt( ( (int) i / m_iUsersPerPage ) + 1 ).toString( );

					if( i != iStart )
						pResponse->strContent += "</a>";

					pResponse->strContent += " ";

					// don't display a bar after the last page

					if( i + (unsigned int)m_iUsersPerPage < (unsigned int)iFound )
						pResponse->strContent += "|";
				}

				pResponse->strContent += "</p>\n";
			}
		}
	}
	else
		pResponse->strContent += "<p class=\"denied\">You are not authorized to view this page.</p>\n";

	pResponse->strContent += m_strStaticFooter; /* =X= */

	if( m_bGen )
		pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

	pResponse->strContent += "</body>\n";
	pResponse->strContent += "</html>\n";
}
