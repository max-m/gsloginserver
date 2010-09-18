#ifndef _LOG_H_
#define _LOG_H_

#include <iostream> 
#include <streambuf>

class CLog : public std::streambuf 
{ 
public: 
    typedef std::streambuf base_type; 
    typedef base_type::traits_type traits_type; 

    CLog( std::ostream& out, std::ostream& out2 ) 
		: m_sb1( out.rdbuf() ), m_sb2( out2.rdbuf() ), m_out1( out ) 
    { 
        out.rdbuf( this );
    } 

    ~CLog() 
    { 
        m_out1.rdbuf( m_sb1 );
    } 

protected: 
    virtual int_type overflow( int_type c = traits_type::eof() ) 
    {
		if( m_sb1 && m_sb2 
            && !traits_type::eq_int_type( m_sb1->sputc( c ), traits_type::eof() ) 
            && !traits_type::eq_int_type( m_sb2->sputc( c ), traits_type::eof() ) 
          ) 
			return traits_type::not_eof( c ); 

        return traits_type::eof();
    } 

private: 
    CLog( const CLog& ); 
    CLog& operator=( const CLog& );

    std::streambuf* m_sb1; 
    std::streambuf* m_sb2; 
    std::ostream& m_out1; 
};

#endif