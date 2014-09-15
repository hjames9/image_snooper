#include "collector.h"
#include "processor.h"

//Network headers
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h> //Ethernet
#include <netinet/ip.h>	//IP
#include <netinet/tcp.h> //TCP

#include <boost/bind.hpp>
#include <iomanip>

using namespace std;

namespace blah
{
    Collector::Collector( Processor& processor ) : m_deviceName( NULL ),
                                                   m_network( 0 ),
                                                   m_netMask( 0 ),
                                                   m_packetHouse( NULL ),
                                                   m_filter(),
                                                   m_netp(),
                                                   m_threadGroup(),
                                                   m_processor( processor ),
                                                   m_packetData(),
                                                   m_connCleanupRunning( true ),
                                                   m_packetCaptureRunning( true ),
                                                   m_timer(),
                                                   m_mutex(),
                                                   m_logger( log4cxx::Logger::getLogger( "Collector" ) )
    {
        char errorBuffer[ PCAP_ERRBUF_SIZE ] = { 0 };

        //Look up available ethernet device
        if( NULL == ( m_deviceName = pcap_lookupdev( errorBuffer ) ) )
        {
            throw runtime_error( errorBuffer );
        }

        //Get network information about device
        if( -1 == pcap_lookupnet( m_deviceName, &m_network, &m_netMask, errorBuffer ) )
        {
            throw runtime_error( errorBuffer );
        }

        //Open device for sniffing
        if( NULL == ( m_packetHouse = pcap_open_live( m_deviceName, BUFSIZ, 0, -1, errorBuffer ) ) )
        {
            throw runtime_error( errorBuffer );
        }

        //Compile pcap filter
        if( -1 == pcap_compile( m_packetHouse, &m_filter, "tcp", 0, m_netp ) )
        {
            throw runtime_error( "pcap_compile failed" );
        }

        //Set pcap filter
        if( -1 == pcap_setfilter( m_packetHouse, &m_filter ) )
        {
            throw runtime_error( "pcap_setfilter failed" );
        }

        m_timer.tv_sec = 300;
        m_timer.tv_nsec = 0;

        m_threadGroup.create_thread( bind( &packetCaptureThread, this ) );
        m_threadGroup.create_thread( bind( &connectionCleanupThread, this ) );
        assert( m_threadGroup.size() == 2 );

        LOG4CXX_INFO( m_logger, "Created network collector thread" );
    }

    Collector::~Collector( void )
    {
        m_packetCaptureRunning = false;
        pcap_breakloop( m_packetHouse );
        m_connCleanupRunning = false;

        m_threadGroup.join_all();

        pcap_close( m_packetHouse );

        LOG4CXX_INFO( m_logger, "Destroyed network collector thread" );
    }

    void Collector::packetCaptureThread( Collector* collector )
    {
        try
        {
            while( collector->m_packetCaptureRunning )
                pcap_dispatch( collector->m_packetHouse, -1, &packetHandler, reinterpret_cast< unsigned char* >( collector ) );
        }
        catch( const std::exception& exp )
        {
            LOG4CXX_ERROR( collector->m_logger, exp.what() );
        }
        catch( ... )
        {
            LOG4CXX_FATAL( collector->m_logger, "Caught unknown exception" );
        }
    }

    void Collector::connectionCleanupThread( Collector* collector )
    {
        try
        {
            while( collector->m_connCleanupRunning )
            {
                time_t currentTime( time( NULL ) );

                {
                    recursive_mutex::scoped_lock lock( collector->m_mutex );

                    PacketDataIter iter( collector->m_packetData.begin() );
                    PacketDataIter iterEnd( collector->m_packetData.end() );

                    for( ; iter != iterEnd; ++iter )
                    {
                        Connection conn( iter->first );

                        if( currentTime > conn.getLastUpdateTime() + 300 )
                        {
                            //Connection has been idle for 5 minutes, nuke it!
                            LOG4CXX_INFO( collector->m_logger,  "Erasing connection: " << conn );
                            collector->m_packetData.erase( iter );
                        } 
                    }
                }

                nanosleep( &collector->m_timer, NULL );
            }
        }
        catch( const std::exception& exp )
        {
            LOG4CXX_ERROR( collector->m_logger, exp.what() );
        }
        catch( ... )
        {
            LOG4CXX_FATAL( collector->m_logger, "Caught unknown exception" );
        }
    }

    void Collector::packetHandler( unsigned char* collect, const pcap_pkthdr* packetInfo, const unsigned char* packet )
    {
        Collector & collector = *reinterpret_cast< Collector* >( collect );
        const ether_header& ethernetHeader = *reinterpret_cast< const ether_header* >( packet );
        const ip& IPHeader = *reinterpret_cast< const ip* >( packet + sizeof( ether_header ) );
        const tcphdr& TCPHeader = *reinterpret_cast< const tcphdr* >( packet + sizeof( ether_header ) + sizeof( ip ) );

        assert( ETHERTYPE_IP == htons( ethernetHeader.ether_type ) );
        assert( IPPROTO_TCP == IPHeader.ip_p );
    
        Connection conn( IPHeader.ip_src, IPHeader.ip_dst, TCPHeader.source, TCPHeader.dest );
        PacketDataIter currentPacketData;

        //LOG4CXX_INFO( collector.m_logger, conn );

        if( TCPHeader.fin || TCPHeader.rst )
        {
            //Connection is about to close
            recursive_mutex::scoped_lock lock( collector.m_mutex );
            collector.m_packetData.erase( conn );
            return;
        }

        {
            recursive_mutex::scoped_lock lock( collector.m_mutex );
            if( collector.m_packetData.end() == ( currentPacketData = collector.m_packetData.find( conn ) ) )
            {
                //Brand new connection seen
                PacketResults packetResults = collector.m_packetData.insert( PacketDataType( conn, PacketPtr( new Packet() ) ) );
                assert( packetResults.second );
                currentPacketData = packetResults.first;
            }
            else
            {
                conn.setLatestUpdateTime();
            }
        }

        //LOG4CXX_DEBUG( collector.m_logger, << hex << setiosflags( ios::showbase ) << "LE: " << setw( 4 ) << setfill( '0' ) 
        //     << right << IPHeader.ip_off << '\t' << "BE: " << ntohs( IPHeader.ip_off ) );

        if( ntohs( IPHeader.ip_off ) & IP_DF )
        {
            //Packet not fragmented

            Packet & payload = *currentPacketData->second;
            recursive_mutex::scoped_lock( static_cast< recursive_mutex& >( payload ) );

            payload.setId( IPHeader.ip_id );
            payload.assign( packet + sizeof( ether_header ) + sizeof( ip ) + sizeof( tcphdr ),
                            packetInfo->len - ( sizeof( ether_header ) + sizeof( ip ) + sizeof( tcphdr ) ) );

            //New complete packet, send notification to processor thread
            payload.completed();
            collector.m_processor.newPacket( currentPacketData->second );
        }
        else
        {
            //Packet is fragmented
            
            Packet & payload = *currentPacketData->second;
            recursive_mutex::scoped_lock( static_cast< recursive_mutex& >( payload ) );

            //cout << "Packet is fragmented " << "id is " << IPHeader.ip_id << " size is " << packetInfo->len;
            //if( ntohs( IPHeader.ip_off ) & IP_MF )
                //cout << " and has more fragments!";
            
            //cout << endl;

            //if( offset != 0 && payload.empty() )
            {
                //We missed first beginning of fragmentation, disregard
            }
        }
    }
}
