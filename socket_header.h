#ifndef SOCKET_HEADER_H__
#define SOCKET_HEADER_H__

//Network headers
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h> //Ethernet
#include <netinet/ip.h>	//IP
#include <netinet/tcp.h> //TCP

#include <iostream>

namespace blah
{
    #pragma pack(1)
    class SocketHeader : protected ether_header, protected ip, protected tcphdr
    {
    public:
        
        SocketHeader( void ) : ether_header(), ip(), tcphdr() { }
        
        friend ostream& operator<<( ostream& os, const SocketHeader& socketHeader );
    };
    #pragma pack()
}

#endif
