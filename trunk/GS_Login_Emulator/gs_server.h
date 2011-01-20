#pragma once
#ifndef _GS_SERVER_H_
#define _GS_SERVER_H_

#include <vector>
#include <string>

class CDB;
class CGPCM;
class CGPSP;
class CBF2Available;
class CConfig;

class CGSServer
{
public:
	CGSServer( CConfig* nCFG );
	~CGSServer( );

	bool Update( long usecBlock );
	
	bool			m_Exiting;
	CDB*			m_DB;
	CGPCM*			m_GPCM;			// gpcm.gamespy.com
	CGPSP*			m_GPSP;			// gpsp.gamespy.com
	CBF2Available*	m_BF2Available; // bf2.available.gamespy.com
};

#endif