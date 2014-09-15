#include <cstdlib>
#include "packet.h"
#include <iomanip>

namespace blah
{
    Packet::Packet( void ) : m_id( 0 ),
                             m_buffer(),
                             m_complete( false ),
                             m_mutex()
    {
    }

    Packet::Packet( unsigned short id,
                    const unsigned char* buffer,
                    size_t size ) : m_id( id ),
                                    m_buffer( buffer, size ),
                                    m_complete( false ),
                                    m_mutex()
    {
    }

    ostream& operator<<( ostream& os, const Packet& packet )
    {
        os << hex << setiosflags( ios::uppercase )
           << "-------------------------------------------\n";

        ustring::const_iterator iter( packet.m_buffer.begin() );
        ustring::const_iterator iterEnd( packet.m_buffer.end() );
        unsigned int count;

        for( count = 0; iter != iterEnd; ++iter )
        {
            if( count % 8 == 0 )
                os << setw( 4 ) << setfill( '0' ) << right << count << "  ";

            os << setw( 2 ) << setfill( '0' ) << right << static_cast< unsigned int >( *iter ) << ' ';

            if( ( count + 1 ) % 8 == 0 )
                os << '\n';

            count++;
        }

        if( count == 0 )
            os << "No payload\n";
        
        os << "\n-------------------------------------------\n";

        return os;
    }
}
