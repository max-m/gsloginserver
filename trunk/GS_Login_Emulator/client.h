#ifndef _CLIENT_H_
#define _CLIENT_H_

/* 
	HOW THE SYSTEM WORKS:
	1. The server sends m_ServerChallenge to the client. 
	2. a) The client will now generate a random string itself and create a new value (response value)
	      which will be sent to the server following this format:
	      md5( md5(password) + m_ClientChallenge + m_ServerChallenge + md5(password) )
		  [password = the password the user has entered]
       b) The client will send this value together with the random string it has generated
		  to the server (so that the server can save both of them)
    3. The server now has all neccessary strings to create this value itself.
	   Cause we own the right password un-encrypted we will generate the same value
	   the client generated if the password was correct. If the string, the client gave us
	   is something different, he has also entered the wrong password. Thus we will send
	   an error message and close the connection.
    4. If the password was correct we will send him his profile data (email, profileid, etc.).
*/

class CTCPSocket;
class CGSServer;
class CGPCM;
class CGPSP;

#include <string>
#include <vector>

class CClient
{
protected:
	unsigned int m_State;
	CTCPSocket* m_Socket;
	CGSServer* m_GSServer;

public:
	CClient( CTCPSocket* nSocket, CGSServer* nGSServer );
	virtual ~CClient( );

	CTCPSocket* GetSocket( )				{ return m_Socket; }
	int GetState( )							{ return m_State; }

	virtual bool Update( void* fd ) = 0;
};

class CClientCM : public CClient
{
private:
	unsigned int m_State;
	
	std::string m_Name;
	std::string m_Password;
	unsigned short m_Session;

	CGPCM* m_GPCM;

	std::string m_Response;		   // the response value created by the client
	std::string m_ServerChallenge; // a random string the response value is created with
	std::string m_ClientChallenge; // a random string generated by the client

public:
	CClientCM( CGPCM* nGPCM, CTCPSocket* nSocket );
	~CClientCM( );

	bool Update( void* fd );

private:
	void SendServerChallenge( );
	bool SendProof( );
	void SendProfile( bool retrieve = false );
	bool HandleNewUser( std::vector< std::string >& RecvVals );

	std::string GenerateRandomString( std::string table, unsigned int len );
	std::string GenerateResponseValue( );
	std::string GenerateProofValue( );
	unsigned short GenerateSession( );
};

class CClientSP : public CClient
{
private:
	std::string m_Name;	
	CGPSP* m_GPSP;

public:
	CClientSP( CGPSP* nGPSP, CTCPSocket* nSocket );
	~CClientSP( );

	bool Update( void* fd );

private:
	void SendGPSP( int NR );
	void SendCheck( );
};

#endif