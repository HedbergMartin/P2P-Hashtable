#include <inttypes.h>

#ifndef __PDU_SENDER__
#define __PDU_SENDER__

typedef struct NET_JOIN_PDU NET_JOIN_PDU;

void sendUDP(int socket, char* toAddress, uint16_t toPort, uint8_t* msg, uint32_t msg_len);
void sendStunLookup(uint16_t port, int fd, char* toAddress, uint16_t toPort);
void sendNetGetNode(uint16_t port, int fd, char* toAddress, uint16_t toPort);
void sendNetAlive(uint16_t port, int fd, char* toAddress, uint16_t toPort);
void sendNetJoin(char* srcAddr, uint16_t srcPort, int fd, char* toAddress, uint16_t toPort);
void sendNetJoinResp(int fd, char* nextAddr, uint16_t nextPort);
void forwardNetJoin(int fd, struct NET_JOIN_PDU pdu, uint8_t span, char* addr, uint16_t port);

#endif