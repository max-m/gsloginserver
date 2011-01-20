#pragma once
#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <string>
#include <cstring>

#ifdef WIN32
 #include <winsock2.h>
 #include <errno.h>
 #include <ws2tcpip.h>		// for WSAIoctl

 #define EADDRINUSE WSAEADDRINUSE
 #define EADDRNOTAVAIL WSAEADDRNOTAVAIL
 #define EAFNOSUPPORT WSAEAFNOSUPPORT
 #define EALREADY WSAEALREADY
 #define ECONNABORTED WSAECONNABORTED
 #define ECONNREFUSED WSAECONNREFUSED
 #define ECONNRESET WSAECONNRESET
 #define EDESTADDRREQ WSAEDESTADDRREQ
 #define EDQUOT WSAEDQUOT
 #define EHOSTDOWN WSAEHOSTDOWN
 #define EHOSTUNREACH WSAEHOSTUNREACH
 #define EINPROGRESS WSAEINPROGRESS
 #define EISCONN WSAEISCONN
 #define ELOOP WSAELOOP
 #define EMSGSIZE WSAEMSGSIZE
 // #define ENAMETOOLONG WSAENAMETOOLONG
 #define ENETDOWN WSAENETDOWN
 #define ENETRESET WSAENETRESET
 #define ENETUNREACH WSAENETUNREACH
 #define ENOBUFS WSAENOBUFS
 #define ENOPROTOOPT WSAENOPROTOOPT
 #define ENOTCONN WSAENOTCONN
 // #define ENOTEMPTY WSAENOTEMPTY
 #define ENOTSOCK WSAENOTSOCK
 #define EOPNOTSUPP WSAEOPNOTSUPP
 #define EPFNOSUPPORT WSAEPFNOSUPPORT
 #define EPROTONOSUPPORT WSAEPROTONOSUPPORT
 #define EPROTOTYPE WSAEPROTOTYPE
 #define EREMOTE WSAEREMOTE
 #define ESHUTDOWN WSAESHUTDOWN
 #define ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
 #define ESTALE WSAESTALE
 #define ETIMEDOUT WSAETIMEDOUT
 #define ETOOMANYREFS WSAETOOMANYREFS
 #define EUSERS WSAEUSERS
 #define EWOULDBLOCK WSAEWOULDBLOCK
#else
 #include <arpa/inet.h>
 #include <errno.h>
 #include <fcntl.h>
 #include <netdb.h>
 #include <netinet/in.h>
 #include <netinet/tcp.h>
 #include <sys/ioctl.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <unistd.h>

 typedef int SOCKET;

 #define INVALID_SOCKET -1
 #define SOCKET_ERROR -1

 #define closesocket close

 extern int GetLastError( );
#endif

#ifndef INADDR_NONE
 #define INADDR_NONE -1
#endif

#ifndef MSG_NOSIGNAL
 #define MSG_NOSIGNAL 0
#endif

#ifdef WIN32
 #define SHUT_RDWR 2
#endif


unsigned int GetTime( );
unsigned int GetTicks( );

//
// CSocket
//

class CSocket
{
protected:
	SOCKET m_Socket;
	struct sockaddr_in m_SIN;

	bool m_HasError;
	int m_Error;

public:
	CSocket( );
	CSocket( SOCKET nSocket, struct sockaddr_in nSIN );
	~CSocket( );

	unsigned short GetPort( )						{ return m_SIN.sin_port; }
	virtual std::string GetIPString( )				{ return inet_ntoa(m_SIN.sin_addr); }


	virtual bool HasError( )						{ return m_HasError; }
	virtual int GetError( )							{ return m_Error; }
	virtual std::string GetErrorString( );

	virtual void SetFD( fd_set* fd, fd_set* send_fd, int* nfds );
	virtual void Allocate( int type );
	virtual void Reset( );
};

//
// CTCPSocket
//

class CTCPSocket : public CSocket
{
protected:
	bool m_Connected;

private:
	std::string m_RecvBuffer;
	std::string m_SendBuffer;
	unsigned int m_LastRecv;
	unsigned int m_LastSend;

public:
	CTCPSocket( );
	CTCPSocket( SOCKET nSocket, struct sockaddr_in nSIN );
	virtual ~CTCPSocket( );

	virtual void Reset( );
	virtual bool GetConnected( )					{ return m_Connected; }

	virtual std::string *GetBytes( )				{ return &m_RecvBuffer; }
	virtual void PutBytes( std::string bytes );
	virtual void ClearRecvBuffer( )					{ m_RecvBuffer.clear( ); }
	virtual void ClearSendBuffer( )					{ m_SendBuffer.clear( ); }

	virtual unsigned int GetLastRecv( )				{ return m_LastRecv; }
	virtual unsigned int GetLastSend( )				{ return m_LastSend; }

	virtual void DoRecv( fd_set* fd );
	virtual void DoSend( fd_set* send_fd );
	virtual void Disconnect( );

	virtual void SetNoDelay( bool noDelay );
};

//
// CTCPServer
//

class CTCPServer : public CTCPSocket
{
public:
	CTCPServer( );
	virtual ~CTCPServer( );

	virtual bool Listen( std::string address, unsigned short port );
	virtual CTCPSocket* Accept( fd_set* fd );
};

//
// CUDPSocket
//

class CUDPSocket : public CSocket
{
protected:
	struct in_addr m_BroadcastTarget;

public:
	CUDPSocket( );
	virtual ~CUDPSocket( );

	virtual bool SendTo( struct sockaddr_in sin, std::string message );
	virtual bool SendTo( struct sockaddr_in sin, char* message, size_t len );
	virtual bool SendTo( std::string address, unsigned short port, std::string message );
	virtual bool Broadcast( unsigned short port, std::string message );
	virtual void SetBroadcastTarget( std::string subnet );
	virtual void SetDontRoute( bool dontRoute );
};

//
// CUDPServer
//

class CUDPServer : public CUDPSocket
{
public:
	CUDPServer( );
	virtual ~CUDPServer( );

	virtual bool Bind( struct sockaddr_in sin );
	virtual bool Bind( std::string address, unsigned short port );
	virtual void RecvFrom( fd_set* fd, struct sockaddr_in* sin, std::string* message );
};

#endif