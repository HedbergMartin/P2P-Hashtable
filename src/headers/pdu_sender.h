#include <inttypes.h>

#ifndef __PDU_SENDER__
#define __PDU_SENDER__

void sendUDP(int socket, char* toAddress, uint16_t toPort, uint8_t* msg, uint32_t msg_len);
void sendStunLookup(uint16_t port, int fd, char* toAddress, uint16_t toPort);
void sendNetGetNode(uint16_t port, int fd, char* toAddress, uint16_t toPort);

#endif