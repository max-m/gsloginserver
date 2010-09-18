#include "socket.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <time.h>
using namespace std;

#ifndef WIN32
 int GetLastError( ) { return errno; }
 #include <sys/time.h>
#endif

#ifdef __APPLE__
 #include <mach/mach_time.h>
#endif

unsigned int GetTime( )
{
	return GetTicks( ) / 1000;
}

unsigned int GetTicks( )
{
#ifdef WIN32
	// don't use GetTickCount anymore because it's not accurate enough (~16ms resolution)
	// don't use QueryPerformanceCounter anymore because it isn't guaranteed to be strictly increasing on some systems and thus requires "smoothing" code
	// use timeGetTime instead, which typically has a high resolution (5ms or more) but we request a lower resolution on startup

	return timeGetTime( );
#elif __APPLE__
	uint64_t current = mach_absolute_time( );
	static mach_timebase_info_data_t info = { 0, 0 };
	// get timebase info
	if( info.denom == 0 )
		mach_timebase_info( &info );
	uint64_t elapsednano = current * ( info.numer / info.denom );
	// convert ns to ms
	return elapsednano / 1e6;
#else
	uint32_t ticks;
	struct timespec t;
	clock_gettime( CLOCK_MONOTONIC, &t );
	ticks = t.tv_sec * 1000;
	ticks += t.tv_nsec / 1000000;
	return ticks;
#endif
}

//
// CSocket
//

CSocket :: CSocket( )
{
	m_Socket = INVALID_SOCKET;
	memset( &m_SIN, 0, sizeof(m_SIN) );
	m_HasError = false;
	m_Error = 0;
}

CSocket :: CSocket( SOCKET nSocket, struct sockaddr_in nSIN )
{
	m_Socket = nSocket;
	m_SIN = nSIN;
	m_HasError = false;
	m_Error = 0;
}

CSocket :: ~CSocket( )
{
	if( m_Socket != INVALID_SOCKET )
		closesocket( m_Socket );
}

string CSocket :: GetErrorString( )
{
	if( !m_HasError )
		return "NO ERROR";

	switch( m_Error )
	{
		case EWOULDBLOCK: return "EWOULDBLOCK";
		case EINPROGRESS: return "EINPROGRESS";
		case EALREADY: return "EALREADY";
		case ENOTSOCK: return "ENOTSOCK";
		case EDESTADDRREQ: return "EDESTADDRREQ";
		case EMSGSIZE: return "EMSGSIZE";
		case EPROTOTYPE: return "EPROTOTYPE";
		case ENOPROTOOPT: return "ENOPROTOOPT";
		case EPROTONOSUPPORT: return "EPROTONOSUPPORT";
		case ESOCKTNOSUPPORT: return "ESOCKTNOSUPPORT";
		case EOPNOTSUPP: return "EOPNOTSUPP";
		case EPFNOSUPPORT: return "EPFNOSUPPORT";
		case EAFNOSUPPORT: return "EAFNOSUPPORT";
		case EADDRINUSE: return "EADDRINUSE";
		case EADDRNOTAVAIL: return "EADDRNOTAVAIL";
		case ENETDOWN: return "ENETDOWN";
		case ENETUNREACH: return "ENETUNREACH";
		case ENETRESET: return "ENETRESET";
		case ECONNABORTED: return "ECONNABORTED";
		case ECONNRESET: return "ECONNRESET";
		case ENOBUFS: return "ENOBUFS";
		case EISCONN: return "EISCONN";
		case ENOTCONN: return "ENOTCONN";
		case ESHUTDOWN: return "ESHUTDOWN";
		case ETOOMANYREFS: return "ETOOMANYREFS";
		case ETIMEDOUT: return "ETIMEDOUT";
		case ECONNREFUSED: return "ECONNREFUSED";
		case ELOOP: return "ELOOP";
		case ENAMETOOLONG: return "ENAMETOOLONG";
		case EHOSTDOWN: return "EHOSTDOWN";
		case EHOSTUNREACH: return "EHOSTUNREACH";
		case ENOTEMPTY: return "ENOTEMPTY";
		case EUSERS: return "EUSERS";
		case EDQUOT: return "EDQUOT";
		case ESTALE: return "ESTALE";
		case EREMOTE: return "EREMOTE";
	}

	std::stringstream SS;
	SS << "UNKNOWN ERROR (" << m_Error << ")";

	return SS.str();
}

void CSocket :: SetFD( fd_set *fd, fd_set *send_fd, int *nfds )
{
	if( m_Socket == INVALID_SOCKET )
		return;

	FD_SET( m_Socket, fd );
	FD_SET( m_Socket, send_fd );

#ifndef WIN32
	if( m_Socket > *nfds )
		*nfds = m_Socket;
#endif
}

void CSocket :: Allocate( int type )
{
	m_Socket = socket( AF_INET, type, 0 );

	if( m_Socket == INVALID_SOCKET )
	{
		m_HasError = true;
		m_Error = GetLastError( );
		cout << "[SOCKET] error (socket) - " << GetErrorString( ) << endl;
		return;
	}
}

void CSocket :: Reset( )
{
	if( m_Socket != INVALID_SOCKET )
		closesocket( m_Socket );

	m_Socket = INVALID_SOCKET;
	memset( &m_SIN, 0, sizeof(m_SIN) );
	m_HasError = false;
	m_Error = 0;
}

//
// CTCPSocket
//

CTCPSocket :: CTCPSocket( ) : CSocket( )
{
	Allocate( SOCK_STREAM );
	m_Connected = false;
	m_LastRecv = GetTime( );
	m_LastSend = GetTime( );

	// make socket non blocking

#ifdef WIN32
	int iMode = 1;
	ioctlsocket( m_Socket, FIONBIO, (u_long FAR*)&iMode );
#else
	fcntl( m_Socket, F_SETFL, fcntl(m_Socket, F_GETFL) | O_NONBLOCK );
#endif
}

CTCPSocket :: CTCPSocket( SOCKET nSocket, struct sockaddr_in nSIN ) : CSocket( nSocket, nSIN )
{
	m_Connected = true;
	m_LastRecv = GetTime( );
	m_LastSend = GetTime( );

	// make socket non blocking

#ifdef WIN32
	int iMode = 1;
	ioctlsocket( m_Socket, FIONBIO, (u_long FAR*)&iMode );
#else
	fcntl( m_Socket, F_SETFL, fcntl(m_Socket, F_GETFL) | O_NONBLOCK );
#endif
}

CTCPSocket :: ~CTCPSocket( )
{

}

void CTCPSocket :: Reset( )
{
	CSocket :: Reset( );

	Allocate( SOCK_STREAM );
	m_Connected = false;
	m_RecvBuffer.clear( );
	m_SendBuffer.clear( );
	m_LastRecv = GetTime( );
	m_LastSend = GetTime( );

	// make socket non blocking

#ifdef WIN32
	int iMode = 1;
	ioctlsocket( m_Socket, FIONBIO, (u_long FAR*)&iMode );
#else
	fcntl( m_Socket, F_SETFL, fcntl(m_Socket, F_GETFL) | O_NONBLOCK );
#endif
}

void CTCPSocket :: PutBytes( string bytes )
{
	m_SendBuffer += bytes;
}

void CTCPSocket :: DoRecv( fd_set *fd )
{
	if( m_Socket == INVALID_SOCKET || m_HasError || !m_Connected )
		return;

	if( FD_ISSET( m_Socket, fd ) )
	{
		// data is waiting, receive it

		char buffer[1024];
		int c = recv( m_Socket, buffer, 1024, 0 );

		if( c == SOCKET_ERROR && GetLastError( ) != EWOULDBLOCK )
		{
			// receive error

			m_HasError = true;
			m_Error = GetLastError( );
			cout << "[TCPSOCKET] error (recv) - " << GetErrorString( ) << endl;
			return;
		}
		else if( c == 0 )
		{
			// the other end closed the connection
			m_Connected = false;
		}
		else if( c > 0 )
		{
			// success! add the received data to the buffer

			m_RecvBuffer += string( buffer, c );
			m_LastRecv = GetTime( );
		}
	}
}

void CTCPSocket :: DoSend( fd_set *send_fd )
{
	if( m_Socket == INVALID_SOCKET || m_HasError || !m_Connected || m_SendBuffer.empty( ) )
		return;

	if( FD_ISSET( m_Socket, send_fd ) )
	{
		// socket is ready, send it

		int s = send( m_Socket, m_SendBuffer.c_str( ), (int)m_SendBuffer.size( ), MSG_NOSIGNAL );

		if( s == SOCKET_ERROR && GetLastError( ) != EWOULDBLOCK )
		{
			// send error

			m_HasError = true;
			m_Error = GetLastError( );
			cout << "[TCPSOCKET] error (send) - " << GetErrorString( ) << endl;
			return;
		}
		else if( s > 0 )
		{
			// success! only some of the data may have been sent, remove it from the buffer

			m_SendBuffer = m_SendBuffer.substr( s );
			m_LastSend = GetTime( );
		}
	}
}

void CTCPSocket :: Disconnect( )
{
	if( m_Socket != INVALID_SOCKET )
		shutdown( m_Socket, SHUT_RDWR );

	m_Connected = false;
}

void CTCPSocket :: SetNoDelay( bool noDelay )
{
	int OptVal = 0;

	if( noDelay )
		OptVal = 1;

	setsockopt( m_Socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&OptVal, sizeof( int ) );
}

//
// CTCPServer
//

CTCPServer :: CTCPServer( ) : CTCPSocket( )
{
	// set the socket to reuse the address in case it hasn't been released yet

	int optval = 1;

#ifdef WIN32
	setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(int) );
#else
	setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int) );
#endif
}

CTCPServer :: ~CTCPServer( )
{

}

bool CTCPServer :: Listen( string address, unsigned short port )
{
	if( m_Socket == INVALID_SOCKET || m_HasError )
		return false;

	m_SIN.sin_family = AF_INET;

	if( !address.empty( ) )
	{
		if( ( m_SIN.sin_addr.s_addr = inet_addr(address.c_str()) ) == INADDR_NONE )
			m_SIN.sin_addr.s_addr = INADDR_ANY;
	}
	else
		m_SIN.sin_addr.s_addr = INADDR_ANY;

	m_SIN.sin_port = htons( port );

	if( bind( m_Socket, (struct sockaddr*)&m_SIN, sizeof(m_SIN) ) == SOCKET_ERROR )
	{
		m_HasError = true;
		m_Error = GetLastError( );
		cout << "[TCPSERVER] error (bind) - " << GetErrorString( ) << endl;
		return false;
	}

	// listen, queue length 8

	if( listen(m_Socket, 8) == SOCKET_ERROR )
	{
		m_HasError = true;
		m_Error = GetLastError( );
		cout << "[TCPSERVER] error (listen) - " << GetErrorString( ) << endl;
		return false;
	}

	return true;
}

CTCPSocket* CTCPServer :: Accept( fd_set* fd )
{
	if( m_Socket == INVALID_SOCKET || m_HasError )
		return NULL;

	if( FD_ISSET( m_Socket, fd ) )
	{
		// a connection is waiting, accept it

		struct sockaddr_in Addr;
		int AddrLen = sizeof( Addr );
		SOCKET NewSocket;

#ifdef WIN32
		if( ( NewSocket = accept(m_Socket, (struct sockaddr*)&Addr, &AddrLen) ) == INVALID_SOCKET )
#else
		if( ( NewSocket = accept(m_Socket, (struct sockaddr*)&Addr, (socklen_t*)&AddrLen) ) == INVALID_SOCKET )
#endif
		{
			// accept error, ignore it
		}
		else
		{
			// success! return the new socket

			return new CTCPSocket( NewSocket, Addr );
		}
	}

	return NULL;
}

//
// CUDPSocket
//

CUDPSocket :: CUDPSocket( ) : CSocket( )
{
	Allocate( SOCK_DGRAM );

	// enable broadcast support

	int OptVal = 1;
	setsockopt( m_Socket, SOL_SOCKET, SO_BROADCAST, (const char*)&OptVal, sizeof(int) );
	
	// set default broadcast target
	m_BroadcastTarget.s_addr = INADDR_BROADCAST;
}

CUDPSocket :: ~CUDPSocket( )
{

}

bool CUDPSocket :: SendTo( struct sockaddr_in sin, string message )
{
	if( m_Socket == INVALID_SOCKET || m_HasError )
		return false;

	if( sendto( m_Socket, message.c_str(), message.size(), 0, (struct sockaddr*)&sin, sizeof(sin) ) == -1 )
		return false;

	return true;
}

bool CUDPSocket :: SendTo( struct sockaddr_in sin, char* message, size_t len )
{
	if( m_Socket == INVALID_SOCKET || m_HasError )
		return false;

	if( sendto( m_Socket, message, len, 0, (struct sockaddr*)&sin, sizeof(sin) ) == -1 )
		return false;

	return true;
}

bool CUDPSocket :: SendTo( string address, unsigned short port, string message )
{
	if( m_Socket == INVALID_SOCKET || m_HasError )
		return false;

	// get IP address

	struct hostent *HostInfo;
	unsigned int HostAddress;
	HostInfo = gethostbyname( address.c_str() );

	if( !HostInfo )
	{
		m_HasError = true;
		// m_Error = h_error;
		cout << "[UDPSOCKET] error (gethostbyname)" << endl;
		return false;
	}

	memcpy( &HostAddress, HostInfo->h_addr, HostInfo->h_length );
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = HostAddress;
	sin.sin_port = htons(port);

	return SendTo( sin, message );
}

bool CUDPSocket :: Broadcast( unsigned short port, string message )
{
	if( m_Socket == INVALID_SOCKET || m_HasError )
		return false;

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = m_BroadcastTarget.s_addr;
	sin.sin_port = htons( port );

	if( sendto( m_Socket, message.c_str(), message.size(), 0, (struct sockaddr*)&sin, sizeof(sin) ) == -1 )
	{
		cout << "[UDPSOCKET] failed to broadcast packet (port " << port << ", size " << message.size() << " bytes)" << endl;
		return false;
	}

	return true;
}

void CUDPSocket :: SetBroadcastTarget( string subnet )
{
	if( subnet.empty( ) )
	{
		cout << "[UDPSOCKET] using default broadcast target" << endl;
		m_BroadcastTarget.s_addr = INADDR_BROADCAST;
	}
	else
	{
		// this function does not check whether the given subnet is a valid subnet the user is on
		// convert string representation of ip/subnet to in_addr

		cout << "[UDPSOCKET] using broadcast target [" << subnet << "]" << endl;
		m_BroadcastTarget.s_addr = inet_addr( subnet.c_str( ) );

		// if conversion fails, inet_addr( ) returns INADDR_NONE

		if( m_BroadcastTarget.s_addr == INADDR_NONE )
		{
			cout << "[UDPSOCKET] invalid broadcast target, using default broadcast target" << endl;
			m_BroadcastTarget.s_addr = INADDR_BROADCAST;
		}
	}
}

void CUDPSocket :: SetDontRoute( bool dontRoute )
{
	int OptVal = 0;

	if( dontRoute )
		OptVal = 1;

	// don't route packets; make them ignore routes set by routing table and send them to the interface
	// belonging to the target address directly

	setsockopt( m_Socket, SOL_SOCKET, SO_DONTROUTE, (const char*)&OptVal, sizeof(int) );
}

//
// CUDPServer
//

CUDPServer :: CUDPServer( ) : CUDPSocket( )
{
	// make socket non blocking

#ifdef WIN32
	int iMode = 1;
	ioctlsocket( m_Socket, FIONBIO, (u_long FAR *)&iMode );
#else
	fcntl( m_Socket, F_SETFL, fcntl( m_Socket, F_GETFL ) | O_NONBLOCK );
#endif

	// set the socket to reuse the address
	// with UDP sockets this allows more than one program to listen on the same port

	int optval = 1;

#ifdef WIN32
	setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof( int ) );
#else
	setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof( int ) );
#endif
}

CUDPServer :: ~CUDPServer( )
{

}

bool CUDPServer :: Bind( struct sockaddr_in sin )
{
	if( m_Socket == INVALID_SOCKET || m_HasError )
		return false;

	m_SIN = sin;

	if( bind( m_Socket, (struct sockaddr*)&m_SIN, sizeof(m_SIN) ) == SOCKET_ERROR )
	{
		m_HasError = true;
		m_Error = GetLastError( );
		cout << "[UDPSERVER] error (bind) - " << GetErrorString() << endl;
		return false;
	}

	return true;
}

bool CUDPServer :: Bind( string address, unsigned short port )
{
	if( m_Socket == INVALID_SOCKET || m_HasError )
		return false;

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;

	if( !address.empty( ) )
	{
		if( ( sin.sin_addr.s_addr = inet_addr( address.c_str( ) ) ) == INADDR_NONE )
			sin.sin_addr.s_addr = INADDR_ANY;
	}
	else
		sin.sin_addr.s_addr = INADDR_ANY;

	sin.sin_port = htons( port );

	return Bind( sin );
}

void CUDPServer :: RecvFrom( fd_set* fd, struct sockaddr_in* sin, string* message )
{
	if( m_Socket == INVALID_SOCKET || m_HasError || !sin || !message )
		return;

	int AddrLen = sizeof( *sin );

	if( FD_ISSET(m_Socket, fd) )
	{
		// data is waiting, receive it

		char buffer[1024];

#ifdef WIN32
		int c = recvfrom( m_Socket, buffer, 1024, 0, (struct sockaddr*)sin, &AddrLen );
#else
		int c = recvfrom( m_Socket, buffer, 1024, 0, (struct sockaddr*)sin, (socklen_t*)&AddrLen );
#endif

		if( c == SOCKET_ERROR && GetLastError() != EWOULDBLOCK )
		{
			// receive error

			m_HasError = true;
			m_Error = GetLastError( );
			cout << "[UDPSERVER] error (recvfrom) - " << GetErrorString( ) << endl;
		}
		else if( c > 0 )
		{
			// success!

			*message = string( buffer, c );
		}
	}
}

