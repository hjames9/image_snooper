#ifndef COLLECTOR_H__
#define COLLECTOR_H__

#include <map>
#include <boost/thread.hpp>
#include <pcap.h>
#include <log4cxx/logger.h>

#include "connection.h"
#include "packet.h"

namespace blah
{
    using namespace std;
    using namespace boost;

    typedef map< Connection, PacketPtr > PacketData;
    typedef PacketData::iterator PacketDataIter;
    typedef PacketData::value_type PacketDataType;
    typedef pair< PacketDataIter, bool > PacketResults;

    class Processor;

    class Collector
    {
    public:
        
        Collector( Processor& processor );
        ~Collector( void );

    private:

        //packet capture thread
        static void packetCaptureThread( Collector* collector );

        //connection cleanup thread
        static void connectionCleanupThread( Collector* collector );

        //pcap packet callback
        static void packetHandler( unsigned char* collector, const pcap_pkthdr* packetInfo, const unsigned char* packet );

        //Copying
        Collector( const Collector& );
        Collector& operator=( const Collector& );

        //pcap members
        char* m_deviceName;
	    bpf_u_int32 m_network;
	    bpf_u_int32 m_netMask;
	    pcap_t* m_packetHouse;
	    bpf_program m_filter;
	    bpf_u_int32 m_netp;

        thread_group m_threadGroup;
        Processor& m_processor;
        PacketData m_packetData;
        bool m_connCleanupRunning;
        bool m_packetCaptureRunning;
        timespec m_timer;
        recursive_mutex m_mutex;
        log4cxx::LoggerPtr m_logger;
    };
}

#endif
