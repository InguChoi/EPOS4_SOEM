#include "udpPacket.h"


UDP_Packet::UDP_Packet()
{
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        errorHandling("Server: socket() error");

    int option = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(UDP_PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        errorHandling("Server: bind() error");

    // int flag = fcntl(client_fd, F_GETFL, 0);
    // fcntl(client_fd, F_SETFL, flag | O_NONBLOCK);
}

UDP_Packet::~UDP_Packet()
{
    close(server_fd);
}

void UDP_Packet::setCommandHeader(short header)
{
   // Initialize Variable
    encodeIndex = 0;

    for (int i = 0; i < TX_BUFFER_SIZE; i++)
    {
        txBuffer[i] = 0;
    }

    // Packet Header 1
    txBuffer[encodeIndex] = 13;
    encodeIndex++;

    // Packet Header 2
    txBuffer[encodeIndex] = 10;
    encodeIndex++;

    // Command Header
    memcpy(&txBuffer[encodeIndex], &header, sizeof(header));
    encodeIndex = encodeIndex + sizeof(header);
}


void UDP_Packet::sendPacket()
{
   ///////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////
   // Packet Header 1,
   // Packet Header 2,
   // Command Header,
   // Data
   ///////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////

    sendto(server_fd, txBuffer, encodeIndex, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
}