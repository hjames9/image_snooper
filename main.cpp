#include <csignal>
#include <exception>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>

#include "collector.h"
#include "processor.h"

using namespace std;
using namespace blah;

int main( int argc, char* argv[] )
{
    log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("main"));
    int result = -1;

    try
    {
        log4cxx::BasicConfigurator::configure();
        LOG4CXX_INFO( logger, "Starting file snooper" );

        sigset_t signal_set;

        // block all signals
        sigfillset( &signal_set );
        sigdelset( &signal_set, SIGINT );
        pthread_sigmask( SIG_BLOCK, &signal_set, NULL );

        //Start collector and processor threads
        Processor processor;
        Collector collector( processor );
        
        LOG4CXX_INFO( logger, "Completed startup of file snooper" );

        int sig;
        sigemptyset( &signal_set );
        sigaddset( &signal_set, SIGTERM );
        //sigaddset( &signal_set, SIGINT );
        sigwait( &signal_set, &sig );

        switch( sig )
        {
        case SIGINT:
        case SIGTERM:
            LOG4CXX_INFO( logger, "Clean shutdown" );
            break;
        default:
            break;
        }

        result = 0;
        
        LOG4CXX_INFO( logger, "Shutting down file snooper" );
    }
    catch( const std::exception& exp )
    {
        LOG4CXX_ERROR(logger, exp.what());
    }
    catch( int i )
    {
        LOG4CXX_ERROR( logger, "Caught integer exception: " << i );
    }
    catch( ... )
    {
        LOG4CXX_ERROR( logger, "Caught unknown exception" );
    }

    return result;
}
