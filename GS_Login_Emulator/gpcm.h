#ifndef _GPCM_H_
#define _GPCM_H_

#include <vector>

class CTCPSocket;
class CTCPServer;
class CClientCM;
class CGSServer;

class CGPCM
{
public:
	CGSServer* m_GSServer;

private:
	CTCPServer* m_Socket;
	std::vector< CClientCM* > m_Clients;
	std::vector< CTCPSocket* > m_TCPSockets;

public:
	CGPCM( CGSServer* nGSServer );
	~CGPCM( );

	unsigned int SetFD( void* fd, void* send_fd, int* nfds );
	bool Update( void* fd, void* send_fd );
};

#endif