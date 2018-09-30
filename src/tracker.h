//
// Copyright (C) 2003-2005 Trevor Hogan
//

#ifndef TRACKER_H
 #define TRACKER_H

struct announce_t
{
	string strInfoHash;
	string strIP;
	string strEvent;
	unsigned int iPort;
	int64 iUploaded;
	int64 iDownloaded;
	int64 iLeft;
	string strPeerID;
	string strKey; /* =X= */
};

struct torrent_t
{
	string strInfoHash;
	string strName;
	string strLowerName;
	string strFileName;
	string strAdded;
	unsigned int iSeeders;
	unsigned int iLeechers;
	unsigned int iCompleted;
	int64 iTransferred;
	int64 iSize;
	unsigned int iFiles;
	unsigned int iComments;
	int64 iAverageLeft;
	unsigned int iAverageLeftPercent;
	int64 iMinLeft;
	int64 iMaxiLeft;
	string strTag;
	string strUploader;
	string strInfoLink;
	string strIP;
};

struct peer_t
{
	string strIP;
	int64 iUpped;
	int64 iDowned;
	int64 iLeft;
	unsigned long iConnected;
	float flShareRatio;
	string strClientType;
};

/*

#define ALW_FILENAME		0		// string
#define ALW_NAME			1		// string
#define ALW_ADDED			2		// string
#define ALW_SIZE			3		// long
#define ALW_FILES			4		// int
#define ALW_FILECOMMENT		5		// string

*/

#define MAX_FILENAME_LEN	128		// user specified filename on upload
#define MAX_INFO_LINK_LEN	128		// user specified info link on upload

extern map<string, string> gmapMime;

class CTracker
{
public:
	CTracker( );
	virtual ~CTracker( );

	void saveDFile( );
	void saveComments( );
	void saveTags( );
	void saveUsers( );
	// DWK - XML
	void saveXML( );

	void saveRSS( string strChannelTag = string( ) );

	void expireDownloaders( );
	void parseTorrents( const char *szDir );
	void parseTorrent( const char *szFile );
	bool checkTag( string &strTag );
	void addTag( string strInfoHash, string strTag, string strName, string strUploader, string strInfoLink );
	void deleteTag( string strInfoHash );
	user_t checkUser( string strLogin, string strMD5 );
	void addUser( string strLogin, string strPass, int iAccess, string strMail );
	void deleteUser( string strLogin );
	void CountUniquePeers( );
	void AddUniquePeer( string strIP );
	void RemoveUniquePeer( string strIP );
	void Announce( struct announce_t ann );
	void RefreshFastCache( );

	void serverResponseGET( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponsePOST( struct request_t *pRequest, struct response_t *pResponse, CAtomList *pPost );
	void serverResponseIndex( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseAnnounce( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseScrape( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseStats( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseTorrent( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseFile( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseRobots( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseFavicon( struct request_t *pRequest, struct response_t *pResponse ); /* =X= */
	void serverResponseLogin( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseSignup( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseUploadGET( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseUploadPOST( struct request_t *pRequest, struct response_t *pResponse, CAtomList *pPost );
	void serverResponseInfo( struct request_t *pRequest, struct response_t *pResponse );
	// Added /info.bencode support - DWK
	void serverResponseBencodeInfo( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseAdmin( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseUsers( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseComments( struct request_t *pRequest, struct response_t *pResponse );
	void serverResponseNotFound( struct request_t *pRequest, struct response_t *pResponse );


	void Update( );

private:
	string m_strAllowedDir;
	string m_strUploadDir;
	string m_strExternalTorrentDir;
	string m_strArchiveDir;
	string m_strFileDir;
	string m_strDFile;
	string m_strCommentsFile;
	string m_strTagFile;
	string m_strUsersFile;
	string m_strStaticHeaderFile;
	string m_strStaticHeader;
	string m_strStaticFooterFile;
	string m_strStaticFooter;
	string m_strRobotsFile;
	string m_strRobots;

	// DWK - XML

	string m_strDumpXMLFile;
	bool m_bDumpXMLPeers;
	int m_iDumpXMLInterval;
	unsigned long m_iDumpXMLNext;

	// addition by labarks

	string m_strDumpRSSFile;
	string m_strDumpRSSFileDir;
	int m_iDumpRSSFileMode;
	string m_strDumpRSSTitle;
	string m_strDumpRSSDescription;
	int m_iDumpRSS_TTL;
	string m_strDumpRSSLanguage;
	string m_strDumpRSSImageURL;
	int m_iDumpRSSImageWidth;
	int m_iDumpRSSImageHeight;
	string m_strDumpRSSCopyright;
	int m_iDumpRSSLimit;

	// end addition

	string m_strImageBarFill;
	string m_strImageBarTrans;
	string m_strTrackerURL;
	string m_strForceAnnounceURL;
	bool m_bForceAnnounceOnDL;
	int m_iParseAllowedInterval;
	int m_iSaveDFileInterval;
	int m_iDownloaderTimeOutInterval;
	int m_iRefreshStaticInterval;
	int m_iDumpRSSInterval;
	int m_iMySQLRefreshAllowedInterval;
	int m_iMySQLRefreshStatsInterval;
	int m_iRefreshFastCacheInterval;
	unsigned long m_iParseAllowedNext;
	unsigned long m_iSaveDFileNext;
	unsigned long m_iPrevTime;
	unsigned long m_iDownloaderTimeOutNext;
	unsigned long m_iRefreshStaticNext;
	unsigned long m_iDumpRSSNext;
	unsigned long m_iMySQLRefreshAllowedNext;
	unsigned long m_iMySQLRefreshStatsNext;
	unsigned long m_iRefreshFastCacheNext;
	int m_iAnnounceInterval;
	int m_iMinRequestInterval;
	int m_iResponseSize;
	int m_iMaxGive;
	bool m_bKeepDead;
	bool m_bAllowScrape;
	bool m_bCountUniquePeers;
	bool m_bDeleteInvalid;
	bool m_bParseOnUpload;
	int m_iMaxTorrents;
	bool m_bShowInfoHash;
	bool m_bShowNames;
	bool m_bShowStats;
	bool m_bAllowTorrentDownloads;
	bool m_bAllowComments;
	bool m_bShowAdded;
	bool m_bShowSize;
	bool m_bShowNumFiles;
	bool m_bShowCompleted;
	bool m_bShowTransferred;
	bool m_bShowMinLeft;
	bool m_bShowAverageLeft;
	bool m_bShowMaxiLeft;
	bool m_bShowLeftAsProgress;
	bool m_bShowUploader;
	bool m_bAllowInfoLink;
	bool m_bSearch;
	bool m_bSort;
	bool m_bShowFileComment;
	bool m_bShowFileContents;
	bool m_bShowShareRatios;
	bool m_bShowAvgDLRate;
	bool m_bShowAvgULRate;
	bool m_bDeleteOwnTorrents;
	bool m_bGen;
	bool m_bMySQLOverrideDState;
	int m_iPerPage;
	int m_iUsersPerPage;
	int m_iMaxPeersDisplay;
	int m_iGuestAccess;
	int m_iMemberAccess;
	int m_iFileExpires;
	int m_iNameLength;
	int m_iCommentLength;

	// Harold - NAT IP Handling
	int m_iBlockNATedIP;
	int m_iLocalOnly;

	// DWK - Private Tracker Flag
	int m_iPrivateTracker;

	/* =X= */
	// Favicon support
	string m_strFaviconFile; 
	string m_strFavicon;

	/* =X= */
	// Internalised mouseover
	bool m_bUseMouseovers;
	vector< pair<string, string> > m_vecTagsMouse;

	vector< pair<string, string> > m_vecTags;

	/* =X= */
	// Custom scrape and announce strings
	string m_strCustomAnnounce; 
	string m_strCustomScrape; 

	/* =X= */
	// Announce and scrape authorisation
	int m_iAuthAnnounce;
	int m_iAuthScrape;

	CAtomDicti *m_pAllowed;		// self.allowed
	CAtomDicti *m_pState;		// self.state
	CAtomDicti *m_pDFile;		// self.downloads
	CAtomDicti *m_pCompleted;	// self.completed
	CAtomDicti *m_pTimeDicti;	// self.times
	CAtomDicti *m_pCached;		// self.cached
	CAtomDicti *m_pComments;
	CAtomDicti *m_pTags;
	CAtomDicti *m_pUsers;
	CAtomDicti *m_pIPs;
	CAtomDicti *m_pFastCache;
};

#endif
