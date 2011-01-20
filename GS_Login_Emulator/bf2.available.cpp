#include "bf2.available.h"
#include "gs_server.h"
#include "socket.h"

#include <iostream>
#include <string>
#include <cstring>
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
	if( m_Socket )
	{
		sockaddr_in Client;

		string message;
		m_Socket->RecvFrom( (fd_set*)fd, &Client, &message );

		if( !message.empty() )
		{
			static const char expected[] = {
				0x09, 0x00, 0x00, 0x00, 0x00, 0x62, 0x61, 0x74, 0x74, 
				0x6c, 0x65, 0x66, 0x69, 0x65, 0x6c, 0x64, 0x32, 0x00 
			};

			static const unsigned char send[] = {
				0xfe, 0xfd, 0x09, 0x00, 0x00, 0x00, 0x00 
			};

			if( strncmp(message.c_str(), expected, 18) == 0 )
				m_Socket->SendTo( Client, (char*)send, 7 );
		}
		
		if( m_Socket->HasError() )
			m_Socket->Reset();
	}

	return !m_Socket->HasError();
}