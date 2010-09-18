#include "client.h"
#include "gpcm.h"
#include "gpsp.h"
#include "gs_server.h"
#include "socket.h"

#include "database.h"
#include "md5.h"

#include <cstdlib>
#include <cstring>
#include <ctime>

#include <map>
#include <sstream>
#include <iostream>
using namespace std;

namespace
{
	vector<string> SplitString( string& string_, char token_ )
	{
		vector< std::string > tempValues;

		stringstream ss( string_ );
		string currentItem;

		while( getline(ss, currentItem, token_) )
		{
			if( !currentItem.empty() )
				tempValues.push_back( currentItem );
		}

		return tempValues;
	}

	int gslame( int num ) 
	{
		int c = (num >> 16) & 0xffff;
		int a = num & 0xffff;

		c *= 0x41a7;
		a *= 0x41a7;
		a += ( (c & 0x7fff) << 16 );

		if( a < 0 ) 
		{
			a &= 0x7fffffff;
			a++;
		}

		a += (c >> 15);

		if( a < 0 ) 
		{
			a &= 0x7fffffff;
			a++;
		}

		return a;
	}

	int gspassenc( char* pass ) 
	{
		int passlen = strlen(pass);
		int num = 0x79707367;   // "gspy"

		if( !num )
			num = 1;
		else
			num &= 0x7fffffff;

		int a, c, d;
		for( int i = 0; i < passlen; i++ ) 
		{
			d = 0xff;
			c = 0;
			d -= c;
			if( d ) 
			{
				num = gslame( num );
				a = num % d;
				a += c;
			}
			else
				a = c;

			pass[i] ^= a;
		}
			
		return passlen;
	}

	char* base64_encode( char* data, int* size ) // Gamespy specific!!!
	{
		static const unsigned char base[64] = {
			'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
			'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
			'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
			'w','x','y','z','0','1','2','3','4','5','6','7','8','9','[',']'
		};

		int len;

		if( !size || (*size < 0) )      // use size -1 for auto text size!
			len = strlen(data);
		else
			len = *size;
			
		char* buff = (char*)malloc( ( (len / 3) << 2 ) + 6 );
			
		if( !buff )
			return NULL;

		char* p = buff;


		int a, b, c;

		do 
		{
			a = data[0];
			b = data[1];
			c = data[2];

			*p++ = base[ (a >> 2) & 63 ];
			*p++ = base[ ( ((a &  3) << 4) | ((b >> 4) & 15) ) & 63 ];
			*p++ = base[ ( ((b & 15) << 2) | ((c >> 6) &  3) ) & 63 ];
			*p++ = base[ c & 63 ];

			data += 3;
			len  -= 3;
		} while( len > 0 );

		for( *p = 0; len < 0; len++ ) 
			*(p + len) = '_';

		if( size ) 
			*size = p - buff;

		return buff;
	}

	char* base64_decode( char* data, size_t* size ) 
	{
		static const unsigned char base[128] = {   // supports also the Gamespy base64
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x00,0x00,0x00,0x3f,
			0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
			0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x3e,0x00,0x3f,0x00,0x00,
			0x00,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
			0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x00,0x00,0x00,0x00,0x00
		};

		int len;

		if( !size || (*size < 0) )      // use size -1 for auto size!
			len = strlen(data);
		else
			len = *size;

		int xlen = ( (len >> 2) * 3 ) + 1;    // NULL included in output for text
		char* buff = (char*)malloc( xlen );

		if( !buff ) 
			return NULL;

		char* p = buff;
		char* limit = data + len;


		int a = 0, b = 0, c;

		for( int step = 0; /* data < limit */; step++ ) 
		{
			do 
			{
				if( data >= limit ) 
				{
					c = 0;
					break;
				}

				c = *data;
				data++;

				if( (c == '=') || (c == '_') )  // supports also the Gamespy base64
				{ 
					c = 0;
					break;
				}
			} while( c && ( (c <= ' ') || (c > 0x7f) ) );
		        
			if( !c ) 
				break;

			switch( step & 3 ) 
			{
				case 0:
					a = base[c];
					break;

				case 1:
					b = base[c];
					*p++ = (a << 2) | (b >> 4);
					break;

				case 2:
					a = base[c];
					*p++ = ( (b & 15) << 4 ) | (a >> 2);
					break;

				case 3:
					*p++ = ( (a & 3) << 6 )  | base[c];
					break;
			}
		}

		*p = 0;

		len = p - buff;

		if( size ) 
			*size = len;

		if( (len + 1) != xlen ) // save memory 
		{
			buff = (char*)realloc( buff, len + 1 );  // NULL included!
	        
			if( !buff ) 
				return NULL;
		}

		return buff;
	}
}

CClient :: CClient( CTCPSocket* nSocket, CGSServer* nGSServer )
{
	m_Socket = nSocket;
	m_GSServer = nGSServer;
	m_State = 0;

	m_Socket->ClearRecvBuffer();
	m_Socket->ClearSendBuffer();
}

CClient :: ~CClient( )
{
	delete m_Socket;
	m_Socket = NULL;
}

bool CClient :: Update( void* fd )
{
	if( !m_Socket || !m_Socket->GetConnected() )
		return true;

	return false;
}


CClientCM :: CClientCM( CGPCM* nGPCM, CTCPSocket* nSocket ) : CClient( nSocket, nGPCM->m_GSServer )
{
	m_GPCM = nGPCM;
	m_State = 0;	// dunno why but sometimes m_State isn't getting zerod by CClient
	m_BF2_15 = false;

	cout << " - client connected    : " << m_Socket->GetIPString() << ":" << m_Socket->GetPort() << endl;
}

CClientCM :: ~CClientCM( )
{
	cout << " - client disconnected : " << m_Socket->GetIPString() << ":" << m_Socket->GetPort() << endl;
}

bool CClientCM :: Update( void* fd )
{
	if( CClient::Update(fd) )
		return true;

	vector< string > RecvVals;

	switch( m_State )
	{
		case 0:
			SendServerChallenge();

			m_State++;
			break;

		case 1:
			m_Socket->ClearRecvBuffer();
			m_Socket->DoRecv( (fd_set*)fd );
			RecvVals = SplitString( *m_Socket->GetBytes(), '\\' );

			if( RecvVals.size() == 18 && RecvVals[0] == "login" )
			{
				m_Name = RecvVals[4];
				m_ClientChallenge = RecvVals[2];
				m_Response = RecvVals[6];
				
				if( SendProof() )
					m_State++;
			}
			else if( RecvVals.size() == 22 && RecvVals[0] == "login" ) // recursive from newuser
			{
				m_Name = RecvVals[4];
				m_ClientChallenge = RecvVals[2];
				m_Response = RecvVals[10];
				
				if( SendProof() )
					m_State++;

			}
			else if( RecvVals.size() == 24 && RecvVals[0] == "login" ) // BF2 1.5
			{
				m_BF2_15 = true;
				m_Name = RecvVals[4];
				m_ClientChallenge = RecvVals[2];
				m_Response = RecvVals[10];

				if( SendProof() )
					m_State++;
			}
			else if( RecvVals.size() == 20 && RecvVals[0] == "login" ) // BF2 1.5
			{
				m_BF2_15 = true;
				m_Name = RecvVals[4];
				m_ClientChallenge = RecvVals[2];
				m_Response = RecvVals[6];

				if( SendProof() )
					m_State++;
			}
			else if( RecvVals.size() == 18 && RecvVals[0] == "newuser" )
				HandleNewUser( RecvVals );

			break;

		case 2:
			m_Socket->ClearRecvBuffer();
			m_Socket->DoRecv( (fd_set*)fd );
			RecvVals = SplitString( *m_Socket->GetBytes(), '\\' );

			if( RecvVals.size() == 8 && RecvVals[0] == "getprofile" )
			{
				SendProfile( );

				m_State++;
			}
			else if( RecvVals.size() == 10 && RecvVals[0] == "updatepro" )
			{
				map<string, string> ClientData;
				ClientData["country"] = RecvVals[6];

				m_GSServer->m_DB->SetData( m_Name, ClientData );
				m_State++;
			}

			break;

		case 3:
			m_Socket->ClearRecvBuffer();
			m_Socket->DoRecv( (fd_set*)fd );
			RecvVals = SplitString( *m_Socket->GetBytes(), '\\' );

			if( RecvVals.size() == 4 && RecvVals[0] == "logout" )
			{
				map< string, string > ClientData;
				ClientData["session"] = "0";

				m_GSServer->m_DB->SetData( m_Name, ClientData );
				m_State++;

			}
			else if( RecvVals.size() == 8 && RecvVals[0] == "getprofile" ) // retrieve account
				SendProfile( true );
			else if( m_BF2_15 )
			{
				static unsigned int State = 0;

				switch( State )
				{
					case 0:
					case 1:
					case 3:
						m_Socket->ClearSendBuffer( );
						m_Socket->PutBytes( "\\ka\\\\final\\" );
						State++;
						break;

					case 2:
						m_Socket->ClearSendBuffer( );
						stringstream SendMessage;
						SendMessage << "\\lt\\"
									<< GenerateRandomString( "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ][", 22 ) << "__" 
									<< "\\final\\";
						m_Socket->PutBytes( SendMessage.str() );
						State++;
						break;
				}
			}

			break;

		case 4:
			return true;
	}

	return false;
}

void CClientCM :: SendServerChallenge( )
{
	m_Socket->ClearSendBuffer( );

	m_ServerChallenge = GenerateRandomString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 10 );

	stringstream ServerMessage;
	ServerMessage << "\\lc\\" << 1
				  << "\\challenge\\" << m_ServerChallenge
				  << "\\id\\" << 1
				  << "\\final\\";

	m_Socket->PutBytes( ServerMessage.str() );
}

bool CClientCM :: SendProof( )
{
	m_Socket->ClearSendBuffer( );

	bool Password = false;
	stringstream ServerMessage;

	map< string, string > ClientData = m_GSServer->m_DB->GetData( m_Name );
	if( !ClientData.empty() )
	{
		m_Password = ClientData["password"];

		if( m_Response == GenerateResponseValue() )
		{
			m_Session = GenerateSession( );

			stringstream ss;
			ss << m_Session;
			ClientData["session"] = ss.str();

			ServerMessage << "\\lc\\" << 2
						  << "\\sesskey\\" << ClientData["session"]
						  << "\\proof\\" << GenerateProofValue( )
						  << "\\userid\\" << ClientData["userid"] 
						  << "\\profileid\\" << ClientData["profileid"]
						  << "\\uniquenick\\" << m_Name
						  << "\\lt\\" << GenerateRandomString( "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ][", 22 ) << "__"
						  << "\\id\\" << 1
						  << "\\final\\";
			Password = true;
		}
		else
			ServerMessage << "\\error\\\\err\\260\\fatal\\\\errmsg\\The password provided is incorrect.\\id\\1\\final\\";
	}
	else // retrieve account!
	{
		ServerMessage << "\\error\\\\err\\260\\fatal\\\\errmsg\\Username ["
			          << m_Name
					  << "] doesn't exist!\\id\\1\\final\\";
	}

	m_Socket->PutBytes( ServerMessage.str() );
	return Password;
}

void CClientCM :: SendProfile( bool retrieve )
{
	m_Socket->ClearSendBuffer( );

	map< string, string > ClientData = m_GSServer->m_DB->GetData( m_Name );

	stringstream SendMessage;
	SendMessage << "\\pi\\"
				<< "\\profileid\\" << ClientData["profileid"]
				<< "\\nick\\" << m_Name
				<< "\\userid\\" << ClientData["userid"]
				<< "\\email\\" << ClientData["email"]
				<< "\\sig\\" << GenerateRandomString( "0123456789abcdef", 32 )
				<< "\\uniquenick\\" << m_Name
				<< "\\pid\\" << 0
				<< "\\firstname\\"
				<< "\\lastname\\"
				//<< "\\homepage\\"
				//<< "\\zipcode\\"
				<< "\\countrycode\\" << ClientData["country"]
				//<< "\\st\\" << "  "
				<< "\\birthday\\" << 16844722
				//<< "\\pmask\\" << 64
				//<< "\\conn\\" << 0
				//<< "\\i1\\" << 0
				//<< "\\o1\\" << 0
				//<< "\\mp\\" << 0
				<< "\\lon\\" << "0.000000"
				<< "\\lat\\" << "0.000000"
				<< "\\loc\\"
				<< "\\id\\" << ( retrieve ? 5 : 2 )
				<< "\\final\\";	
	
	m_Socket->PutBytes( SendMessage.str() );
}

bool CClientCM :: HandleNewUser( vector<string>& RecvVals )
{
	bool returnValue = false;

	m_Socket->ClearSendBuffer( );
	m_Name = RecvVals[4];

	if( m_GSServer->m_DB->UserExists(m_Name) )
		m_Socket->PutBytes( "\\error\\\\err\\516\\fatal\\\\errmsg\\This account name is already in use!\\id\\1\\final\\" );
	else
	{
		string password = RecvVals[6];
		size_t len = password.size();

		char* pw_decrypt = base64_decode( const_cast<char*>( password.c_str() ), &len );
		len = gspassenc( pw_decrypt );

		m_Password = pw_decrypt;

		m_GSServer->m_DB->CreateUser( m_Name, m_Password, RecvVals[2], "00" );

		map<string, string> UserData = m_GSServer->m_DB->GetData( m_Name );

		stringstream SendMessage;
		SendMessage << "\\nur\\" 
					<< "\\userid\\" << UserData["userid"]
					<< "\\profileid\\" << UserData["profileid"]
					<< "\\id\\" << 1
					<< "\\final\\";

		m_Socket->PutBytes( SendMessage.str() );
		returnValue = true;
	}

	return returnValue;
}

string CClientCM :: GenerateRandomString( string table, unsigned int len )
{
	srand( (unsigned int)time(NULL) );
	int randomInt = rand( );
	int tableLen = table.size( );

	string randomString( " ", len );

    for( unsigned int i = 0; i < len; i++ )
	{
        randomInt = ( randomInt * 0x343FD ) + 0x269EC3;
		randomInt >>= 1;
        randomString[i] = table[ (unsigned int)randomInt % tableLen ];
    }

    return randomString;
}

string CClientCM :: GenerateResponseValue( )
{
	string md5pw = md5( m_Password );

	string value( md5pw );

	for( unsigned int i = 0; i < 48; i++ )
		value += " ";

	value += m_Name;
	value += m_ClientChallenge;
	value += m_ServerChallenge;
	value += md5pw;

	return md5( value );
}

string CClientCM :: GenerateProofValue( )
{
	string md5pw = md5( m_Password );

	string value( md5pw );

	for( unsigned int i = 0; i < 48; i++ )
		value += " ";

	value += m_Name;
	value += m_ServerChallenge;
	value += m_ClientChallenge;
	value += md5pw;

	return md5( value );
}

unsigned short CClientCM :: GenerateSession( ) 
{
	static const unsigned short crc_table[256] = {
        0x0000,0xC0C1,0xC181,0x0140,0xC301,0x03C0,0x0280,0xC241,
        0xC601,0x06C0,0x0780,0xC741,0x0500,0xC5C1,0xC481,0x0440,
        0xCC01,0x0CC0,0x0D80,0xCD41,0x0F00,0xCFC1,0xCE81,0x0E40,
        0x0A00,0xCAC1,0xCB81,0x0B40,0xC901,0x09C0,0x0880,0xC841,
        0xD801,0x18C0,0x1980,0xD941,0x1B00,0xDBC1,0xDA81,0x1A40,
        0x1E00,0xDEC1,0xDF81,0x1F40,0xDD01,0x1DC0,0x1C80,0xDC41,
        0x1400,0xD4C1,0xD581,0x1540,0xD701,0x17C0,0x1680,0xD641,
        0xD201,0x12C0,0x1380,0xD341,0x1100,0xD1C1,0xD081,0x1040,
        0xF001,0x30C0,0x3180,0xF141,0x3300,0xF3C1,0xF281,0x3240,
        0x3600,0xF6C1,0xF781,0x3740,0xF501,0x35C0,0x3480,0xF441,
        0x3C00,0xFCC1,0xFD81,0x3D40,0xFF01,0x3FC0,0x3E80,0xFE41,
        0xFA01,0x3AC0,0x3B80,0xFB41,0x3900,0xF9C1,0xF881,0x3840,
        0x2800,0xE8C1,0xE981,0x2940,0xEB01,0x2BC0,0x2A80,0xEA41,
        0xEE01,0x2EC0,0x2F80,0xEF41,0x2D00,0xEDC1,0xEC81,0x2C40,
        0xE401,0x24C0,0x2580,0xE541,0x2700,0xE7C1,0xE681,0x2640,
        0x2200,0xE2C1,0xE381,0x2340,0xE101,0x21C0,0x2080,0xE041,
        0xA001,0x60C0,0x6180,0xA141,0x6300,0xA3C1,0xA281,0x6240,
        0x6600,0xA6C1,0xA781,0x6740,0xA501,0x65C0,0x6480,0xA441,
        0x6C00,0xACC1,0xAD81,0x6D40,0xAF01,0x6FC0,0x6E80,0xAE41,
        0xAA01,0x6AC0,0x6B80,0xAB41,0x6900,0xA9C1,0xA881,0x6840,
        0x7800,0xB8C1,0xB981,0x7940,0xBB01,0x7BC0,0x7A80,0xBA41,
        0xBE01,0x7EC0,0x7F80,0xBF41,0x7D00,0xBDC1,0xBC81,0x7C40,
        0xB401,0x74C0,0x7580,0xB541,0x7700,0xB7C1,0xB681,0x7640,
        0x7200,0xB2C1,0xB381,0x7340,0xB101,0x71C0,0x7080,0xB041,
        0x5000,0x90C1,0x9181,0x5140,0x9301,0x53C0,0x5280,0x9241,
        0x9601,0x56C0,0x5780,0x9741,0x5500,0x95C1,0x9481,0x5440,
        0x9C01,0x5CC0,0x5D80,0x9D41,0x5F00,0x9FC1,0x9E81,0x5E40,
        0x5A00,0x9AC1,0x9B81,0x5B40,0x9901,0x59C0,0x5880,0x9841,
        0x8801,0x48C0,0x4980,0x8941,0x4B00,0x8BC1,0x8A81,0x4A40,
        0x4E00,0x8EC1,0x8F81,0x4F40,0x8D01,0x4DC0,0x4C80,0x8C41,
        0x4400,0x84C1,0x8581,0x4540,0x8701,0x47C0,0x4680,0x8641,
        0x8201,0x42C0,0x4380,0x8341,0x4100,0x81C1,0x8081,0x4040
    };

	char* string = const_cast< char* >( m_Name.c_str() );
	int len = m_Name.size();

	unsigned short session = 0;
    while( len-- ) 
	{
        session = crc_table[ (*string ^ session) & 0xff ] ^ (session >> 8);
        string++;
    }

    return session;
}


CClientSP :: CClientSP( CGPSP* nGPSP, CTCPSocket* nSocket ) : CClient( nSocket, nGPSP->m_GSServer )
{
	m_GPSP = nGPSP;
	m_State = 0;

	//cout << " - client connected    : " << m_Socket->GetIPString() << ":" << m_Socket->GetPort() << endl;
}

CClientSP :: ~CClientSP( )
{
	//cout << " - client disconnected : " << m_Socket->GetIPString() << ":" << m_Socket->GetPort() << endl;
}


bool CClientSP :: Update( void* fd )
{
	if( CClient :: Update(fd) )
		return true;

	vector< string > RecvVals;

	switch( m_State )
	{
		case 0:
			m_Socket->ClearRecvBuffer();
			m_Socket->DoRecv( (fd_set*)fd );
			RecvVals = SplitString( *m_Socket->GetBytes(), '\\' );
			
			if( RecvVals.size() >= 10 && RecvVals[0] == "nicks" )
			{
				map< string, string > ClientData = m_GSServer->m_DB->GetData( RecvVals[2], RecvVals[4] );
				
				if( ClientData.empty() )
				{
					// client tries to create an account but the account name is already taken
					// or he tries to recieve an account but it is not existing
					m_Socket->ClearSendBuffer( );
					m_Socket->PutBytes( "\\nr\\\\ndone\\\\final\\" );
					break;
				}

				m_Name = ClientData["name"];

				SendGPSP( 1 );
				m_State++;
			}
			else if( RecvVals.size() == 10 && RecvVals[0] == "check" )
			{
				m_Name = RecvVals[2];

				SendCheck( );
				m_State++;
			}
			else if( RecvVals.size() == 12 && RecvVals[0] == "nicks" ) // BF2 1.5
			{
				map< string, string > ClientData = m_GSServer->m_DB->GetData( RecvVals[2], RecvVals[4] );
				m_Socket->ClearSendBuffer( );
				
				if( ClientData.empty() )
					m_Socket->PutBytes( "\\nr\\0\\ndone\\\\final\\" );
				else
				{
					SendGPSP( 1 );
					m_State++;
				}

				break;
			}


			break;

		case 1:
			m_State++;
			break;
			
		case 2:
			return true;
	}

	return false;
}

void CClientSP :: SendGPSP( int NR )
{
	stringstream SendMessage;
	SendMessage << "\\nr\\" << NR
				<< "\\nick\\" << m_Name
				<< "\\uniquenick\\" << m_Name
				//<< "\\nick\\" << m_Name
				//<< "\\uniquenick\\"
				<< "\\ndone\\"
				<< "\\final\\";

	m_Socket->PutBytes( SendMessage.str() );
}

void CClientSP :: SendCheck( )
{
	map< string, string > ClientData = m_GSServer->m_DB->GetData( m_Name );

	stringstream SendMessage;
	SendMessage << "\\cur\\" << 0
				<< "\\pid\\" << ClientData["profileid"]
				<< "\\final\\";

	m_Socket->PutBytes( SendMessage.str() );
}