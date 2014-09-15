#ifndef PACKET_H__
#define PACKET_H__

#include <string>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/shared_ptr.hpp>

namespace blah
{
    using namespace std;
    using namespace boost;
    
    class Packet;
    typedef shared_ptr< Packet > PacketPtr;
    typedef basic_string< unsigned char > ustring;

    class Packet
    {
    public:

        Packet( void );
        Packet( unsigned short id, const unsigned char* payload, size_t size );

        //Create packet buffer
        void assign( const unsigned char* payload, size_t size );
        void append( const unsigned char* payload, size_t size );
        
        //Fetch packet data
        const unsigned char* data( void ) const;
        size_t size( void ) const;

        void setId( unsigned short );
        unsigned short id( void ) const;

        void completed( void );
        bool isComplete( void ) const;

        void clear( void );
        bool empty( void ) const;

        operator recursive_mutex &( void );
        friend ostream& operator<<( ostream& os, const Packet& packet );

    private:

        unsigned short m_id;
        ustring m_buffer;
        bool m_complete;
        recursive_mutex m_mutex;
    };

    inline void Packet::assign( const unsigned char* buffer, size_t size ) { m_buffer.assign( buffer, size ); }
    inline void Packet::append( const unsigned char* buffer, size_t size ) { m_buffer.append( buffer, size ); }

    inline const unsigned char* Packet::data( void ) const { return m_buffer.data(); }
    inline size_t Packet::size( void ) const { return m_buffer.size(); }
    
    inline void Packet::setId( unsigned short id ) { m_id = id; }
    inline unsigned short Packet::id( void ) const { return m_id; }

    inline void Packet::completed( void ) { m_complete = true; }
    inline bool Packet::isComplete( void ) const { return m_complete; }

    inline void Packet::clear( void ) { m_buffer.clear(); }
    inline bool Packet::empty( void ) const { return m_buffer.empty(); }

    inline Packet::operator recursive_mutex &( void ) { return m_mutex; }
}

#endif
