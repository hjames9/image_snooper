#include "connection.h"

#include <string>
#include <iomanip>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace blah
{
    Connection::Connection( in_addr ipSrc,
                            in_addr ipDst,
                            unsigned short portSrc,
                            unsigned short portDst,
                            time_t lastUpdate ) : m_ipSrc( ipSrc ),
                                                  m_ipDst( ipDst ),
                                                  m_portSrc( portSrc ),
                                                  m_portDst( portDst ),
                                                  m_lastUpdate( lastUpdate )
    {
    }

    bool Connection::operator<( const Connection& conn ) const
    {
        if( m_ipSrc.s_addr != conn.m_ipSrc.s_addr )
            return m_ipSrc.s_addr < conn.m_ipSrc.s_addr;
        else if( m_ipDst.s_addr != conn.m_ipDst.s_addr )
            return m_ipDst.s_addr < conn.m_ipDst.s_addr;
        else if( m_portSrc != conn.m_portSrc )
            return m_portSrc < conn.m_portSrc;
        else if( m_portDst != conn.m_portDst )
            return m_portDst < conn.m_portDst;
        else
            return false;
    }

    bool Connection::operator==( const Connection& conn ) const
    {
        return m_ipSrc.s_addr == conn.m_ipSrc.s_addr
            && m_ipDst.s_addr == conn.m_ipDst.s_addr
            && m_portSrc == conn.m_portSrc
            && m_portDst == conn.m_portDst;
    }

    bool Connection::operator!=( const Connection& conn ) const
    {
        return !operator==( conn );
    }

    ostream& operator<<( ostream& os, const Connection& conn )
    {
        os << "Source ip: " << setw( 16 ) << left << string( inet_ntoa( conn.m_ipSrc ) ) << '\t'
           << "Destination ip: " << setw( 16 ) << string( inet_ntoa( conn.m_ipDst ) ) << '\t'
           << "Source port: " << setw( 6 ) << ntohs( conn.m_portSrc ) << '\t'
           << "Destination port: " << setw( 6 ) << ntohs( conn.m_portDst ) << '\t';

        return os;
    }
}
