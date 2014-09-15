#ifndef PROCESSOR_H__
#define PROCESSOR_H__

#include <boost/thread.hpp>
#include <log4cxx/logger.h>
#include <queue>

#include "packet.h"

namespace blah
{
    using namespace std;
    using namespace boost;

    typedef queue< PacketPtr > PacketQueue;
    typedef shared_ptr< thread > ThreadPtr;

    class Processor
    {
    public:
        
        Processor( void );
        ~Processor( void );

        void newPacket( PacketPtr packet );

    private:
        
        static void packetProcessorThread( Processor* processor );
        
        //No copying
        Processor( const Processor& );
        Processor& operator=( const Processor& );

        PacketQueue m_packetQueue;
        recursive_mutex m_mutex;
        bool m_running;
        ThreadPtr m_thread;
        log4cxx::LoggerPtr m_logger;
    };
}

#endif
