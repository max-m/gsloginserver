#pragma once
#ifndef _BF2_AVAILABLE_H_
#define _BF2_AVAILABLE_H_

class CGSServer;
class CUDPServer;

class CBF2Available
{
private:
	CGSServer*	m_GSServer;
	CUDPServer* m_Socket;

public:
	CBF2Available	( CGSServer* nGSServer );
	~CBF2Available	( );

	unsigned int	SetFD	( void* fd, void* send_fd, int* fds );
	bool			Update	( void* fd, void* send_fd );
};

#endif