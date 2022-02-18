#ifndef TCPPACKET_H_
#define TCPPACKET_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "Macro.h"

#define RX_BUFFER_SIZE 1024
#define PACKET_BUFFER_SIZE 1024


class TCP_Packet
{
public:
    TCP_Packet();
    ~TCP_Packet();

    int readPacket();

    template<class T>
    void decode(T &val)
    {
        memcpy(&val, &packetBuffer[decodeIndex], sizeof(val));
        decodeIndex = decodeIndex + sizeof(val);
    }

    short getHeader()
    {
        return header;
    }

private:
    struct sockaddr_in server_addr, client_addr;
    int client_addr_size = sizeof(client_addr);
    int server_fd, client_fd;

    unsigned char rxBuffer[RX_BUFFER_SIZE];
    unsigned char packetBuffer[PACKET_BUFFER_SIZE];
    int retval;
    int decodeIndex;

    short header;
};

#endif /* TCPPACKET_H_ */