#include <stdio.h>

#include "udpPacket.h"

int main()
{
    UDP_Packet* pUdpPacket = new UDP_Packet;

    while (1)
    {
        short header = 0001;
        int32_t iData = 1234;
        int16_t siData = 101;
        pUdpPacket->setCommandHeader(header);
        pUdpPacket->encode(iData);
        pUdpPacket->encode(siData);
        pUdpPacket->sendPacket();

        usleep(10000);
    }

    delete pUdpPacket;
}