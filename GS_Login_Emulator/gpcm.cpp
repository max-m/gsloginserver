#include "gpcm.h"
#include "socket.h"
#include "client.h"
#include "gs_server.h"

#include <iostream>
#include <string>
using namespace std;

CGPCM :: CGPCM( CGSServer* nGSServer )
	: m_GSServer(nGSServer)
{
	m_Socket = new CTCPServer( );

	if( m_Socket->Listen( string(), 29900 ) )
		cout << "[gpcm.gamespy.com] listening on port " << 29900 << endl;
	else
	{
		cout << "[gpcm.gamespy.com] error listening on port " << 29900 << endl;
		m_GSServer->m_Exiting = true;
	}
}

CGPCM :: ~CGPCM( )
{
	delete m_Socket;
	m_Socket = NULL;

	for( vector<CClientCM*>::iterator i = m_Clients.begin(); i != m_Clients.end(); i++ )
		delete *i;
}

unsigned int CGPCM :: SetFD( void* fd, void* send_fd, int* nfds )
{
	unsigned int NumFDs = 0;

	if( m_Socket )
	{
		m_Socket->SetFD( (fd_set*)fd, (fd_set*)send_fd, nfds );
		NumFDs++;
	}

	for( vector< CClientCM* >::iterator i = m_Clients.begin(); i != m_Clients.end(); i++ )
	{
		if( (*i)->GetSocket() )
		{
			(*i)->GetSocket( )->SetFD( (fd_set*)fd, (fd_set*)send_fd, nfds );
			NumFDs++;
		}
	}

	return NumFDs;
}

bool CGPCM :: Update( void* fd, void* send_fd )
{
	for( vector< CClientCM* >::iterator i = m_Clients.begin(); i != m_Clients.end(); )
	{
		if( (*i)->Update(fd) )
		{
			delete *i;
			i = m_Clients.erase(i);
		}
		else
		{
			(*i)->GetSocket()->DoSend( (fd_set*)send_fd );
			i++;
		}
	}

	if( m_Socket )
	{
		CTCPSocket* NewSocket = m_Socket->Accept( (fd_set*)fd );

		if( NewSocket )
			m_Clients.push_back( new CClientCM(this, NewSocket) );

		if( m_Socket->HasError() )
			m_Socket->Reset();
	}

	return !m_Socket->HasError();
}