#pragma once
#ifndef _GPSP_H_
#define _GPSP_H_

#include <vector>

class CTCPSocket;
class CTCPServer;
class CClientSP;
class CGSServer;

class CGPSP
{
public:
	CGSServer* m_GSServer;

private:
	CTCPServer* m_Socket;
	std::vector< CClientSP* > m_Clients;
	std::vector< CTCPSocket* > m_TCPSockets;

public:
	CGPSP( CGSServer* nGSServer );
	~CGPSP( );

	unsigned int SetFD( void* fd, void* send_fd, int* nfds );
	bool Update( void* fd, void* send_fd );
};

#endif