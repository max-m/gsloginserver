#include "database.h"

#include <iostream>
#include <sstream>
using namespace std;

CDB :: CDB( )
{
	m_Valid = false;
}

CDB :: ~CDB( )
{

}


CMySQL :: CMySQL( string nServer, string nDataBase, string nUserName, string nPassword, unsigned short nPort ) : CDB( )
{
	m_Server = nServer;
	m_DataBase = nDataBase;
	m_UserName = nUserName;
	m_Password = nPassword;
	m_Port = nPort;

	mysql_library_init( 0, NULL, NULL );
	cout << "[MySQL] connecting to database server" << endl;

	if( !( m_Connection = mysql_init( NULL ) ) )
	{
		cout << "[MYSQL] error -" << mysql_error(m_Connection) << endl;
		return;
	}

	my_bool Reconnect = true;
	mysql_options( m_Connection, MYSQL_OPT_RECONNECT, &Reconnect );
	if( !mysql_real_connect(m_Connection, nServer.c_str(), nUserName.c_str(), nPassword.c_str(), nDataBase.c_str(), nPort, NULL, 0) )
	{
		cout << "[MYSQL] error - " << mysql_error(m_Connection) << endl;
		return;
	}

	m_Valid = true;
}

CMySQL :: ~CMySQL( )
{
	mysql_close( m_Connection );
	mysql_library_end( );
}

vector<string> CMySQL :: MySQLFetchRow( MYSQL_RES* res )
{
	vector<string> Result;

	MYSQL_ROW Row = mysql_fetch_row( res );

	if( Row )
	{
		unsigned long *Lengths;
		Lengths = mysql_fetch_lengths( res );

		for( unsigned int i = 0; i < mysql_num_fields( res ); i++ )
		{
			if( Row[i] )
				Result.push_back( string( Row[i], Lengths[i] ) );
			else
				Result.push_back( string( ) );
		}
	}

	return Result;
}

map<string, string> CMySQL :: GetData( string username )
{
	map<string, string> Values;

	if( m_Valid && UserExists(username) )
	{
		string Query = "SELECT id, name, password, email, country, session FROM users WHERE name='" + username + "'";

		if( mysql_real_query( m_Connection, Query.c_str(), Query.size() ) == 0 )
		{
			MYSQL_RES* result = mysql_store_result( m_Connection );

			if( result )
			{
				vector<string> row = MySQLFetchRow( result );

				if( !row.empty() )
				{
					Values["id"] = row[0];
					Values["name"] = row[1];
					Values["password"] = row[2];
					Values["email"] = row[3];
					Values["country"] = row[4];

					unsigned int id = 0, temp;
					stringstream ss;
					ss << row[0];
					ss >> id;

					ss.str("");
					ss.clear();
					temp = 40000000 - id;
					ss << temp;
					Values["userid"] = ss.str();
	
					ss.str("");
					ss.clear();
					temp = 50000000 - id;
					ss << temp;
					Values["profileid"] = ss.str();

					Values["session"] = row[5];
				}					
				
				mysql_free_result( result );
			}
			else
				cout << "[MySQL] error getting data of user [" << username << "] - " << mysql_error( m_Connection ) << endl;
		}
		else
			cout << "[MySQL] error getting data of user [" << username << "] - " << mysql_error( m_Connection ) << endl;
	}

	return Values;
}

map<string, string> CMySQL :: GetData( string email, string password )
{
	map<string, string> Values;

	if( m_Valid )
	{
		string Query = "SELECT id, name, country, session FROM users WHERE email='" + email + "' AND password='" + password + "'";

		if( mysql_real_query( m_Connection, Query.c_str(), Query.size() ) == 0 )
		{
			MYSQL_RES* result = mysql_store_result( m_Connection );

			if( result )
			{
				vector<string> row = MySQLFetchRow( result );

				if( !row.empty() )
				{
					Values["id"] = row[0];
					Values["name"] = row[1];
					Values["password"] = password;
					Values["email"] = email;
					Values["country"] = row[2];
					Values["session"] = row[3];

					unsigned int id = 0, temp;
					stringstream ss;
					ss << row[0];
					ss >> id;

					ss.str("");
					ss.clear();
					temp = 40000000 - id;
					ss << temp;
					Values["userid"] = ss.str();
	
					ss.str("");
					ss.clear();
					temp = 50000000 - id;
					ss << temp;
					Values["profileid"] = ss.str();
				}					
				
				mysql_free_result( result );
			}
			else
				cout << "[MySQL] error getting data of user with E-Mail [" << email << "] - " << mysql_error( m_Connection ) << endl;
		}
		else
			cout << "[MySQL] error getting data of user with E-Mail [" << email << "] - " << mysql_error( m_Connection ) << endl;
	}

	return Values;
}

void CMySQL :: SetData( string username, map<string, string>& data )
{
	if( m_Valid && UserExists(username) )
	{
		map<string, string> OldValues = GetData( username );

		string Password = data["password"];
		string EMail = data["email"];
		string Country = data["country"];
		string Session = data["session"];

		stringstream Query;
		Query << "UPDATE users SET "
			<< "password='" << (Password.empty() ? OldValues["password"] : Password) << "', "
			<< "email='" << (EMail.empty() ? OldValues["email"] : EMail) << "', "
			<< "country='" << (Country.empty() ? OldValues["country"] : Country) << "', "
			<< "session='" << (Session.empty() ? OldValues["session"] : Session) << "' "
			<< "WHERE name='" << username << "'";

		if( mysql_real_query( m_Connection, Query.str().c_str(), Query.str().size() ) != 0 )
			cout << "[MySQL] error setting data of user [" << username << "] - " << mysql_error( m_Connection ) << endl;	
	}
}

bool CMySQL :: UserExists( string Username )
{
	bool Existing = false;
	string Query = "SELECT COUNT(*) FROM users WHERE name='" + Username + "'";

	if( m_Valid )
	{
		if( mysql_real_query( m_Connection, Query.c_str(), Query.size() ) == 0 )
		{
			MYSQL_RES* result = mysql_store_result( m_Connection );

			if( result )
			{
				vector<string> row = MySQLFetchRow( result );

				if( !row.empty() && (row[0] != "0") )
					Existing = true;
				
				mysql_free_result( result );
			}
			else
				cout << "[MySQL] error checking user [" << Username << "] - " << mysql_error( m_Connection ) << endl;
		}
		else
			cout << "[MySQL error checking user [" << Username << "] - " << mysql_error( m_Connection ) << endl;
	}

	return Existing;
}

void CMySQL :: CreateUser( string Username, string Password, string Mail, string Country )
{
	if( m_Valid && !UserExists(Username) )
	{
		string Query = "INSERT INTO users (name, password, email, country) VALUES ('";
		Query += Username + "', '";
		Query += Password + "', '";
		Query += Mail + "', '";
		Query += Country + "')";

		if( mysql_real_query( m_Connection, Query.c_str(), Query.size() ) != 0 )
			cout << "[MySQL] error creating user [" << Username << "] - " << mysql_error( m_Connection ) << endl;	
	}
}



CSQLite3 :: CSQLite3( string nFileName ) : CDB( )
{
	m_FileName = nFileName;

	if( sqlite3_open_v2( m_FileName.c_str(), &m_DB, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL ) == SQLITE_OK )
	{
		m_Valid = true;

		sqlite3_stmt* Statement;
		sqlite3_prepare_v2( m_DB, "SELECT * FROM sqlite_master WHERE type='table' AND name='users'", -1, &Statement, NULL );

		if( Statement )
		{
			int RC = Step( Statement );

			if( RC != SQLITE_ROW )
			{
				cout << "[SQLite3] No database found - creating now!" << endl;
				sqlite3_exec( m_DB, "CREATE TABLE users ( id INTEGER PRIMARY KEY, name TEXT NOT NULL, password TEXT NOT NULL, email TEXT NOT NULL, country TEXT NOT NULL, session INTEGER NULL DEFAULT '0' )", NULL, NULL, NULL );
			}

			sqlite3_finalize( Statement );
		}
	}
	else
		cout << "[SQLite3] initializing error - " << sqlite3_errmsg(m_DB) << endl;
}

CSQLite3 :: ~CSQLite3( )
{
	sqlite3_close( m_DB );
}

int CSQLite3 :: Step( sqlite3_stmt* Statement )
{
	int RC = sqlite3_step( Statement );

	if( RC == SQLITE_ROW )
	{
		m_Row.clear( );

		for( int i = 0; i < sqlite3_column_count( Statement ); i++ )
		{
			char* ColumnText = (char*)sqlite3_column_text( Statement, i );

			if( ColumnText )
			{
				m_Row.push_back( ColumnText );
			}
			else
				m_Row.push_back( string( ) );
		}
	}

	return RC;
}

map<string, string> CSQLite3 :: GetData( string username )
{
	map< string, string > Values;

	if( m_Valid && UserExists(username) )
	{
		sqlite3_stmt* Statement;
		sqlite3_prepare_v2( m_DB, "SELECT id, password, email, country, session FROM users WHERE name=?", -1, &Statement, NULL );

		if( Statement )
		{
			sqlite3_bind_text( Statement, 1, username.c_str( ), -1, SQLITE_TRANSIENT );
			int RC = Step( Statement );

			if( RC == SQLITE_ROW )
			{
				Values["id"] = m_Row[0];
				Values["name"] = username;
				Values["password"] = m_Row[1];
				Values["email"] = m_Row[2];
				Values["country"] = m_Row[3];

				unsigned int id = 0, temp;
				stringstream ss;
				ss << m_Row[0];
				ss >> id;

				ss.str("");
				ss.clear();
				temp = 40000000 - id;
				ss << temp;
				Values["userid"] = ss.str();

				ss.str("");
				ss.clear();
				temp = 50000000 - id;
				ss << temp;
				Values["profileid"] = ss.str();

				Values["session"] = m_Row[4];
			}

			sqlite3_finalize( Statement );
		}
		else
			cout << "[SQLite3] prepare error requesting data for user [" << username << "] - " << sqlite3_errmsg( m_DB ) << endl;
	}

	return Values;
}

map<string, string> CSQLite3 :: GetData( string email, string password )
{
	map< string, string > Values;

	if( m_Valid )
	{
		sqlite3_stmt* Statement;
		sqlite3_prepare_v2( m_DB, "SELECT id, name, country, session FROM users WHERE email=? AND password=?", -1, &Statement, NULL );

		if( Statement )
		{
			sqlite3_bind_text( Statement, 1, email.c_str( ), -1, SQLITE_TRANSIENT );
			sqlite3_bind_text( Statement, 2, password.c_str( ), -1, SQLITE_TRANSIENT );
			int RC = Step( Statement );

			if( RC == SQLITE_ROW )
			{
				Values["id"] = m_Row[0];
				Values["name"] = m_Row[1];
				Values["password"] = password;
				Values["email"] = email;
				Values["country"] = m_Row[2];

				unsigned int id = 0, temp;
				stringstream ss;
				ss << m_Row[0];
				ss >> id;

				ss.str("");
				ss.clear();
				temp = 40000000 - id;
				ss << temp;
				Values["userid"] = ss.str();

				ss.str("");
				ss.clear();
				temp = 50000000 - id;
				ss << temp;
				Values["profileid"] = ss.str();

				Values["session"] = m_Row[3];
			}

			sqlite3_finalize( Statement );
		}
		else
			cout << "[SQLite3] prepare error requesting data for user with E-Mail [" << email << "] - " << sqlite3_errmsg( m_DB ) << endl;
	}

	return Values;
}

void CSQLite3 :: SetData( string username, map<string, string>& values )
{
	if( m_Valid && UserExists(username) )
	{
		map<string, string> OldValues = GetData( username );

		sqlite3_stmt* Statement;
		sqlite3_prepare_v2( m_DB, "UPDATE users SET password=?, email=?, country=?, session=? WHERE name=?", -1, &Statement, NULL );

		if( Statement )
		{
			string Password = values["password"];
			string EMail = values["email"];
			string Country = values["country"];

			unsigned int session = 0;
			stringstream ss;
			ss << values["session"];
			ss >> session;

			if( !session )
			{
				ss.str("");
				ss.clear();
				ss << OldValues["session"];
				ss >> session;
			}

			sqlite3_bind_text( Statement, 1, (Password.empty() ? OldValues["password"] : Password).c_str(), -1, SQLITE_TRANSIENT );
			sqlite3_bind_text( Statement, 2, (EMail.empty() ? OldValues["email"] : Password).c_str(), -1, SQLITE_TRANSIENT );
			sqlite3_bind_text( Statement, 3, (Country.empty() ? OldValues["country"] : Password).c_str(), -1, SQLITE_TRANSIENT );
			sqlite3_bind_int( Statement, 4, session );
			sqlite3_bind_text( Statement, 5, username.c_str(), -1, SQLITE_TRANSIENT );

			int RC = Step( Statement );

			if( RC == SQLITE_ERROR )
				cout << "[SQLite3] prepare error setting data of user [" << username << "] - " << sqlite3_errmsg( m_DB ) << endl;
		
			sqlite3_finalize( Statement );
		}
		else
			cout << "[SQLite3] prepare error setting data of user [" << username << "] - " << sqlite3_errmsg( m_DB ) << endl;
	}
}

bool CSQLite3 :: UserExists( string Username )
{
	bool Existing = false;
	
	if( m_Valid )
	{
		sqlite3_stmt* Statement;
		sqlite3_prepare_v2( m_DB, "SELECT COUNT(*) FROM users WHERE name=?", -1, &Statement, NULL );

		if( Statement )
		{
			sqlite3_bind_text( Statement, 1, Username.c_str(), -1, SQLITE_TRANSIENT );

			int RC = Step( Statement );

			if( RC == SQLITE_ROW )
			{
				if( !m_Row.empty() && m_Row[0] != "0" )
					Existing = true;
			}
			else if( RC == SQLITE_ERROR )
				cout << "[SQLite3 ERROR] error checking user [" <<  Username << "] - " << sqlite3_errmsg( m_DB ) << endl;

			sqlite3_finalize( Statement );
		}
		else
			cout << "[SQLite3] prepare error checking user [" << Username << "] - " << sqlite3_errmsg( m_DB ) << endl;
	}

	return Existing;
}

void CSQLite3 :: CreateUser( string Username, string Password, string Mail, string Country )
{
	if( m_Valid && !UserExists(Username) )
	{
		sqlite3_stmt* Statement;
		sqlite3_prepare_v2( m_DB, "INSERT INTO users (name, password, email, country) VALUES ( ?, ?, ?, ? )", -1, &Statement, NULL );

		if( Statement )
		{
			sqlite3_bind_text( Statement, 1, Username.c_str(), -1, SQLITE_TRANSIENT );
			sqlite3_bind_text( Statement, 2, Password.c_str( ), -1, SQLITE_TRANSIENT );
			sqlite3_bind_text( Statement, 3, Mail.c_str( ), -1, SQLITE_TRANSIENT );
			sqlite3_bind_text( Statement, 4, Country.c_str( ), -1, SQLITE_TRANSIENT );

			int RC = Step( Statement );

			if( RC == SQLITE_ERROR )
				cout << "[SQLite3] error creating user [" << Username << "] - " << sqlite3_errmsg( m_DB ) << endl;
		
			sqlite3_finalize( Statement );
		}
		else
			cout << "[SQLite3] prepare error creating user [" << Username << "] - " << sqlite3_errmsg( m_DB ) << endl;
	}
}