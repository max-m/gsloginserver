#include "gs_server.h"
#include "log.h"
#include "md5.h"
#include "database.h"
#include "client.h"
#include "config.h"
#include "gpcm.h"
#include "gpsp.h"
#include "bf2.available.h"

#include <iostream> 
#include <fstream> 
#include <csignal>
#include <sstream>
using namespace std;

#ifdef WIN32
 #define MILLISLEEP( x ) Sleep( x )
#else
 #define MILLISLEEP( x ) usleep( ( x ) * 1000 )
#endif

CGSServer* gGSServer = NULL;
string gCFGFile;

void SignalCatcher( int s )
{
	cout << "[!!!] caught signal " << s << ", exiting!" << endl;

	exit( EXIT_SUCCESS );
}

int main( int argc, char** argv )
{
	// cout -> gs_login.log
	ofstream logfile( "gs_log.log" ); 
	CLog log( cout, logfile );

	cout << "[GS_SERVER] starting up" << endl;

	gCFGFile = "gs_server.cfg";

	if( argc > 1 && argv[1] )
		gCFGFile = argv[1];

	CConfig CFG( gCFGFile );

#ifdef WIN32
	WSADATA wsadata;

    if( WSAStartup( MAKEWORD(2, 2), &wsadata ) != 0 )
    {
		cout << "[WINSOCK] error starting winsock" << endl;
		//logfile.close():
		return EXIT_FAILURE;
	}
#endif

	signal( SIGINT, SignalCatcher );

#ifndef WIN32
	// disable SIGPIPE since some systems like OS X don't define MSG_NOSIGNAL

	signal( SIGPIPE, SIG_IGN );
#endif
	
	gGSServer = new CGSServer( &CFG );

	while( true )
	{
		if( gGSServer->Update( 50000 ) )
			break;
	}

#ifdef WIN32
	WSACleanup( );
#endif

	delete gGSServer;

	cout << "[GS_SERVER] shutting down" << endl;
	
	logfile.close();
	return EXIT_SUCCESS;
}

CGSServer :: CGSServer( CConfig* nCFG )
{
	m_CFG = nCFG;
	m_Exiting = false;

	m_DB = NULL;
	m_GPCM = NULL;
	m_GPSP = NULL;
	m_BF2Available = NULL;

	if( m_CFG->GetString("db_type", "sqlite3") == "sqlite3" )
		m_DB = new CSQLite3( m_CFG->GetString("db_sqlite3_file", "gs_login_server.db3") );
	else
		m_DB = new CMySQL( m_CFG->GetString("db_mysql_server", string()), m_CFG->GetString("db_mysql_database", "gs_login_server"), m_CFG->GetString("db_mysql_user", string()), m_CFG->GetString("db_mysql_password", string()), m_CFG->GetInt("db_mysql_port", 0) );

	if( m_DB->HasError() )
		m_Exiting = true;
	else
	{
		m_GPCM = new CGPCM( this );
		m_GPSP = new CGPSP( this );
		m_BF2Available = new CBF2Available( this );
	}
}

CGSServer :: ~CGSServer( )
{
	if( m_DB )
	{
		delete m_DB;
		m_DB = NULL;
	}

	if( m_GPCM )
	{
		delete m_GPCM;
		m_GPCM = NULL;
	}

	if( m_GPSP )
	{
		delete m_GPSP;
		m_GPSP = NULL;
	}

	if( m_BF2Available )
	{
		delete m_BF2Available;
		m_BF2Available = NULL;
	}
}

bool CGSServer :: Update( long usecBlock )
{
	if( m_Exiting )
		return true;

	unsigned int NumFDs = 0;

	// take every socket we own and throw it in one select statement so we can block on all sockets

	int nfds = 0;
	fd_set fd;
	fd_set send_fd;
	FD_ZERO( &fd );
	FD_ZERO( &send_fd );

	NumFDs += m_GPCM->SetFD( &fd, &send_fd, &nfds );
	NumFDs += m_GPSP->SetFD( &fd, &send_fd, &nfds );
	NumFDs += m_BF2Available->SetFD( &fd, &send_fd, &nfds );

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = usecBlock;

	struct timeval send_tv;
	send_tv.tv_sec = 0;
	send_tv.tv_usec = 0;

#ifdef WIN32
	select( 1, &fd, NULL, NULL, &tv );
	select( 1, NULL, &send_fd, NULL, &send_tv );
#else
	select( nfds + 1, &fd, NULL, NULL, &tv );
	select( nfds + 1, NULL, &send_fd, NULL, &send_tv );
#endif

	if( NumFDs == 0 )
		MILLISLEEP( 50 );

	m_GPCM->Update(&fd, &send_fd);
	m_GPSP->Update(&fd, &send_fd);
	m_BF2Available->Update(&fd, &send_fd);
	
	return false;
}
