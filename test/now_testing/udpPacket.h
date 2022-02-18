#ifndef UDPPACKET_H_
#define UDPPACKET_H_

#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include "errorHandling.h"

#define TCP_PORT 2000
#define UDP_PORT 3000
#define GUI_PC_IP "192.168.0.9"
#define RASPBERRY_PI_IP "192.168.0.10"

#define RX_BUFFER_SIZE 1024
#define TX_BUFFER_SIZE 1024
#define PACKET_BUFFER_SIZE 1024

class UDP_Packet
{
public:
    UDP_Packet();
    ~UDP_Packet();

    void setCommandHeader(short header);

    template <class T>
    void encode(T &val)
    {
        memcpy(&txBuffer[encodeIndex], &val, sizeof(val));
        encodeIndex = encodeIndex + sizeof(val);
    }

    void sendPacket();

private:
    struct sockaddr_in server_addr, client_addr;
    int client_addr_size = sizeof(client_addr);
    int server_fd, client_fd;

    short header;
    int encodeIndex;
    unsigned char txBuffer[TX_BUFFER_SIZE];
};

#endif /* UDPPACKET_H_ */