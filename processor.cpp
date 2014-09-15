#include "processor.h"

#include <stdexcept>
#include <boost/bind.hpp>

namespace blah
{
    Processor::Processor( void ) : m_packetQueue(),
                                   m_mutex(),
                                   m_running( true ),
                                   m_thread( ThreadPtr( new thread( bind( &packetProcessorThread, this ) ) ) ),
                                   m_logger( log4cxx::Logger::getLogger( "Processor" ) )
    {
        LOG4CXX_INFO( m_logger, "Created packet processor" );
    }

    Processor::~Processor( void )
    {
        m_running = false;
        m_thread->join();

        LOG4CXX_INFO( m_logger, "Destroyed packet processor" );
    }

    void Processor::packetProcessorThread( Processor* processor )
    {
        try
        {
            timespec timer;
            timer.tv_sec = 0;
            timer.tv_nsec = 500000;

            while( processor->m_running )
            {
                {
                    recursive_mutex::scoped_lock queueLock( processor->m_mutex );
                    if( !processor->m_packetQueue.empty() )
                    {
                        PacketPtr packet( processor->m_packetQueue.front() );
                        recursive_mutex::scoped_lock packetLock( static_cast< recursive_mutex& >( *packet ) );

                        assert( packet->isComplete() );

                        //LOGCXX_INFO( processor->m_logger, *packet );
                        static unsigned int count = 0;

                        if( memmem( packet->data(), packet->size(), "GIF89a", 6 ) )
                        {
                            LOG4CXX_INFO( processor->m_logger, "Found gif " << ++count << "!" );
                        }
                        else if( memmem( packet->data(), packet->size(), "JFIF", 4 ) )
                        {
                            LOG4CXX_INFO( processor->m_logger, "Found jpeg" );
                        }

                        //We can clear packet now, we're done with it.
                        packet->clear();
                        processor->m_packetQueue.pop();
                    }
                }

                nanosleep( &timer, NULL );
            }
        }
        catch( const std::exception& exp )
        {
            LOG4CXX_ERROR( processor->m_logger, exp.what() );
        }
        catch( ... )
        {
            LOG4CXX_FATAL( processor->m_logger, "Caught unknown exception" );
        }
    }

    void Processor::newPacket( PacketPtr packet )
    {
        recursive_mutex::scoped_lock lock( m_mutex );
        m_packetQueue.push( packet );
    }
}
