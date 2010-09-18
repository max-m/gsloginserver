#ifndef _DATABASE_H_
#define _DATABASE_H_

#ifdef WIN32
 #include <winsock.h>
#endif

#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <map>

#include "sqlite3.h"

class CDB
{
protected:
	bool m_Valid;
public:
	CDB( );
	virtual ~CDB( );

	bool HasError( )	{ return !m_Valid; }

	virtual bool UserExists( std::string Username ) = 0;
	virtual void CreateUser( std::string Username, std::string Password, std::string Mail, std::string Country ) = 0; 

	virtual std::map< std::string, std::string > GetData( std::string Username ) = 0;
	virtual std::map< std::string, std::string > GetData( std::string EMail, std::string Password ) = 0;
	virtual void SetData( std::string Username, std::map< std::string, std::string >& Data ) = 0;
};

class CMySQL : public CDB
{
private:
	std::vector< std::string > MySQLFetchRow( MYSQL_RES* res );
	MYSQL* m_Connection;

	std::string m_Server;
	std::string m_DataBase;
	std::string m_UserName;
	std::string m_Password;
	unsigned short m_Port;

public:
	CMySQL( std::string nServer, std::string nDataBase, std::string nUserName, std::string nPassword, unsigned short nPort );
	~CMySQL( );

	bool UserExists( std::string Username );
	void CreateUser( std::string Username, std::string Password, std::string Mail, std::string Country );
	
	std::map< std::string, std::string > GetData( std::string Username );
	std::map< std::string, std::string > GetData( std::string EMail, std::string Password );
	void SetData( std::string Username, std::map< std::string, std::string >& Data );
};

class CSQLite3 : public CDB
{
private:
	std::string m_FileName;
	std::vector<std::string> m_Row;
	sqlite3* m_DB;

	int Step( sqlite3_stmt* Statement );

public:
	CSQLite3( std::string nFileName );
	~CSQLite3( );

	bool UserExists( std::string Username );
	void CreateUser( std::string Username, std::string Password, std::string Mail, std::string Country );
	
	std::map< std::string, std::string > GetData( std::string Username );
	std::map< std::string, std::string > GetData( std::string EMail, std::string Password );
	void SetData( std::string Username, std::map< std::string, std::string >& Data );
};

#endif