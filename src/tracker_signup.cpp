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
#include "tracker.h"
#include "util.h"

void CTracker :: serverResponseSignup( struct request_t *pRequest, struct response_t *pResponse )
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
	pResponse->strContent += "<title>BNBT Signup</title>\n";

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
		pResponse->strContent += "<p class=\"login1_signup\">You are not logged in. Click <a href=\"/login.html\">here</a> to login.</p>\n";
	else
		pResponse->strContent += "<p class=\"login2_signup\">You are logged in as <span class=\"username\">" + UTIL_RemoveHTML( pRequest->user.strLogin ) + "</span>. Click <a href=\"/login.html?logout=1\">here</a> to logout.</p>\n";

	pResponse->strContent += m_strStaticHeader; /* =X= */

	pResponse->strContent += "<h3>BNBT Signup</h3>\n";

	if( pRequest->user.iAccess & ACCESS_SIGNUP )
	{
		if( pRequest->mapParams.find( "us_login" ) != pRequest->mapParams.end( ) &&
			pRequest->mapParams.find( "us_password" ) != pRequest->mapParams.end( ) &&
			pRequest->mapParams.find( "us_password_verify" ) != pRequest->mapParams.end( ) &&
			pRequest->mapParams.find( "us_email" ) != pRequest->mapParams.end( ) )
		{
			string strLogin = pRequest->mapParams["us_login"];
			string strPass = pRequest->mapParams["us_password"];
			string strPass2 = pRequest->mapParams["us_password_verify"];
			string strMail = pRequest->mapParams["us_email"];

			if( strLogin.empty( ) || strPass.empty( ) || strPass2.empty( ) || strMail.empty( ) )
			{
				pResponse->strContent += "<p>Unable to signup. You must fill in all the fields. Click <a href=\"/signup.html\">here</a> to return to the signup page.</p>\n";
				pResponse->strContent += m_strStaticFooter; /* =X= */

				if( m_bGen )
					pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
				
				pResponse->strContent += "</body>\n";
				pResponse->strContent += "</html>\n";

				return;
			}
			else
			{
				if( strLogin[0] == ' ' || strLogin[strLogin.size( ) - 1] == ' ' || strLogin.size( ) > (unsigned int)m_iNameLength )
				{
					pResponse->strContent += "<p>Unable to signup. Your name must be less than " + CAtomInt( m_iNameLength ).toString( ) + " characters long and it must not start or end with spaces. Click <a href=\"/signup.html\">here</a> to return to the signup page.</p>\n";
					pResponse->strContent += m_strStaticFooter; /* =X= */

					if( m_bGen )
						pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
					
					pResponse->strContent += "</body>\n";
					pResponse->strContent += "</html>\n";

					return;
				}

				if( strMail.find( "@" ) == string :: npos || strMail.find( "." ) == string :: npos )
				{
					pResponse->strContent += "<p>Unable to signup. Your e-mail address is invalid. Click <a href=\"/signup.html\">here</a> to return to the signup page.</p>\n";
					pResponse->strContent += m_strStaticFooter; /* =X= */

					if( m_bGen )
						pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
					
					pResponse->strContent += "</body>\n";
					pResponse->strContent += "</html>\n";

					return;
				}

				if( strPass == strPass2 )
				{
					if( m_pUsers->getItem( strLogin ) )
					{
						pResponse->strContent += "<p>Unable to signup. The user \"" + UTIL_RemoveHTML( strLogin ) + "\" already exists. Click <a href=\"/signup.html\">here</a> to return to the signup page.</p>\n";
						pResponse->strContent += m_strStaticFooter; /* =X= */

						if( m_bGen )
							pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
						
						pResponse->strContent += "</body>\n";
						pResponse->strContent += "</html>\n";

						return;
					}
					else
					{
						addUser( strLogin, strPass, m_iMemberAccess, strMail );

						pResponse->strContent += "<p>Thanks! You've successfully signed up! Click <a href=\"/login.html\">here</a> to login.</p>\n";
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
					pResponse->strContent += "<p>Unable to signup. The passwords did not match. Click <a href=\"/signup.html\">here</a> to return to the signup page.</p>\n";
					pResponse->strContent += m_strStaticFooter; /* =X= */
					
					if( m_bGen )
						pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";
				
					pResponse->strContent += "</body>\n";
					pResponse->strContent += "</html>\n";

					return;
				}
			}
		}

		pResponse->strContent += "<form method=\"get\" action=\"/signup.html\">\n";
		pResponse->strContent += "<p><strong>Signup</strong></p>\n";
		pResponse->strContent += "<ul>\n";
		pResponse->strContent += "<li>Names must be less than " + CAtomInt( m_iNameLength ).toString( ) + " characters long</li>\n";
		pResponse->strContent += "<li>Names are case sensitive</li>\n";
		pResponse->strContent += "<li>No HTML</li>\n";
		pResponse->strContent += "</ul>\n";
		pResponse->strContent += "<input name=\"us_login\" type=text size=24 maxlength=" + CAtomInt( m_iNameLength ).toString( ) + "> Login<br><br>\n";
		pResponse->strContent += "<input name=\"us_password\" type=password size=20> Password<br>\n";
		pResponse->strContent += "<input name=\"us_password_verify\" type=password size=20> Verify Password<br><br>\n";
		pResponse->strContent += "<input name=\"us_email\" type=text size=40> E-Mail<br><br>\n";
		pResponse->strContent += "<input type=submit value=\"Signup\">\n";
		pResponse->strContent += "</form>\n";
	}
	else
		pResponse->strContent += "<p class=\"denied\">You are not authorized to view this page.</p>\n";

	pResponse->strContent += m_strStaticFooter; /* =X= */

	if( m_bGen )
		pResponse->strContent += "<p class=\"gen\">Generated in " + UTIL_ElapsedTimeStr( btv, UTIL_CurrentTime( ) ) + " seconds.</p>\n";

	pResponse->strContent += "</body>\n";
	pResponse->strContent += "</html>\n";
}
