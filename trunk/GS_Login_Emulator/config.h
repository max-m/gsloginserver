#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>
using namespace std;

class CConfig
{
private:
	map<string, string> m_CFG;

public:
	CConfig( string file );
	~CConfig( );

	bool Exists( string key );

	int GetInt( string key, int x );			// x = value which GetInt returns if key doesn't exist
	string GetString( string key, string x );	// x = value which GetString returns if key doesn't exist

	void Set( string key, string x );
};

#endif