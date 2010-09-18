#include "bf2.available.h"
#include "gs_server.h"
#include "socket.h"

#include <iostream>
#include <string>
using namespace std;

CBF2Available :: CBF2Available( CGSServer* nGSServer )
{
	m_GSServer = nGSServer;
	m_Socket = new CUDPServer( );

	if( m_Socket->Bind( string(), 27900 ) )
		cout << "[bf2.available.gamespy.com] binding on port " << 27900 << endl;
	else
	{
		cout << "[bf2.available.gamespy.com] error binding on port " << 27900 << endl;
		m_GSServer->m_Exiting = true;
	}
	
}

CBF2Available :: ~CBF2Available( )
{
	delete m_Socket;
	m_Socket = NULL;
}

unsigned int CBF2Available :: SetFD( void* fd, void* send_fd, int* nfds )
{
	unsigned int NumFDs = 0;

	if( m_Socket )
	{
		m_Socket->SetFD( (fd_set*)fd, (fd_set*)send_fd, nfds );
		NumFDs++;
	}

	return NumFDs;
}

bool CBF2Available :: Update( void* fd, void* send_fd )
{
	return !m_Socket->HasError();

	if( m_Socket )
	{
		//m_Socket->Broadcast( 27900, "......." );
		
		sockaddr_in Client;

		string message;
		m_Socket->RecvFrom( (fd_set*)fd, &Client, &message );

		if( !message.empty() )
		{
			if( message == ".....battlefield2." )
			{
				cout << "sending" << endl;
				m_Socket->SendTo( Client, "......." );
			}
			else
				cout << "MSG RECIEVED: " << message << endl;
		}
		

		if( m_Socket->HasError() )
			m_Socket->Reset();
	}

	return !m_Socket->HasError();
}