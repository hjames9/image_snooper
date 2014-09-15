#ifndef CONNECTION_H__
#define CONNECTION_H__

#include <netinet/ip.h>	//IP
#include <netinet/tcp.h> //TCP

#include <iostream>
#include <ctime>

namespace blah
{
    using namespace std;

    class Connection
    {
    public:

        Connection( const unsigned char* );
        Connection( in_addr ip_src, in_addr ip_dst, unsigned short port_src, unsigned short port_dst, time_t lastUpdate = time( NULL ) );

        bool operator<( const Connection& ) const;
        bool operator==( const Connection& ) const;
        bool operator!=( const Connection& ) const;

        time_t getLastUpdateTime( void ) const;
        void setLatestUpdateTime( void );
    
        friend ostream& operator<<( ostream&, const Connection& );

    private:

        in_addr m_ipSrc;
        in_addr m_ipDst;
        unsigned short m_portSrc;
        unsigned short m_portDst;
        time_t m_lastUpdate;
    };

    inline time_t Connection::getLastUpdateTime( void ) const { return m_lastUpdate; }
    inline void Connection::setLatestUpdateTime( void ) { time( &m_lastUpdate ); } 
}

#endif
