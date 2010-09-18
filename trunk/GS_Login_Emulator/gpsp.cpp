#include "gpsp.h"
#include "gpcm.h"
#include "socket.h"
#include "client.h"
#include "gs_server.h"

#include <iostream>
#include <string>
using namespace std;

CGPSP :: CGPSP( CGSServer* nGSServer )
	: m_GSServer(nGSServer)
{
	m_Socket = new CTCPServer( );

	if( m_Socket->Listen( string(), 29901 ) )
		cout << "[gpsp.gamespy.com] listening on port " << 29901 << endl;
	else
	{
		cout << "[gpsp.gamespy.com] error listening on port " << 29901 << endl;
		m_GSServer->m_Exiting = true;
	}
}

CGPSP :: ~CGPSP( )
{
	delete m_Socket;
	m_Socket = NULL;

	for( vector<CClientSP*>::iterator i = m_Clients.begin(); i != m_Clients.end(); i++ )
		delete *i;
}

unsigned int CGPSP :: SetFD( void* fd, void* send_fd, int* nfds )
{
	unsigned int NumFDs = 0;

	if( m_Socket )
	{
		m_Socket->SetFD( (fd_set*)fd, (fd_set*)send_fd, nfds );
		NumFDs++;
	}

	for( vector< CClientSP* >::iterator i = m_Clients.begin(); i != m_Clients.end(); i++ )
	{
		if( (*i)->GetSocket() )
		{
			(*i)->GetSocket( )->SetFD( (fd_set*)fd, (fd_set*)send_fd, nfds );
			NumFDs++;
		}
	}

	return NumFDs;
}

bool CGPSP:: Update( void* fd, void* send_fd )
{
	for( vector< CClientSP* >::iterator i = m_Clients.begin(); i != m_Clients.end(); )
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
			m_Clients.push_back( new CClientSP(this, NewSocket) );

		if( m_Socket->HasError() )
			m_Socket->Reset();
	}

	return !m_Socket->HasError();
}