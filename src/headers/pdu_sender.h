#include <inttypes.h>

#ifndef __PDU_SENDER__
#define __PDU_SENDER__

void sendUDP(int socket, char* toAddress, uint16_t toPort, uint8_t* msg, uint32_t msg_len);
void sendStunLookup(uint16_t port, int fd, char* toAddress, uint16_t toPort);
void sendNetGetNode(uint16_t port, int fd, char* toAddress, uint16_t toPort);
void sendNetAlive(uint16_t port, int fd, char* toAddress, uint16_t toPort);
void sendNetJoin(char* srcAddr, uint16_t srcPort, int fd, char* toAddress, uint16_t toPort);
void sendNetJoinResp(char* nextAddr, uint16_t nextPort, int fd);

#endif