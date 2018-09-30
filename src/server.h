//
// Copyright (C) 2003-2005 Trevor Hogan
//

#ifndef SERVER_H
 #define SERVER_H

class CServer
{
public:
	CServer( );
	virtual ~CServer( );

	void Kill( );
	bool isDying( );

	// returns true if the server should be killed

	bool Update( bool bBlock );

	vector<CClient *> m_vecClients;

private:
	bool m_bKill;

	int m_iSocketTimeOut;
	string m_strBind;
	int m_iCompression;

	// tphogan - the server is listening on each of these sockets
	// the vector container will handle memory allocation for the SOCKET object

	vector<SOCKET> m_vecListeners;

	// tphogan - helper function to add a new socket listener
	// returns true on success, false on error

	bool AddListener( struct sockaddr_in sin );
};

#endif
