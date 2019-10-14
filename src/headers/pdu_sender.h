#include <inttypes.h>
#include "pdu.h"

#ifndef __PDU_SENDER__
#define __PDU_SENDER__

struct CONNECTION {
    char address[ADDRESS_LENGTH];
    uint16_t port;
};

typedef struct NET_JOIN_PDU NET_JOIN_PDU;
typedef struct table_entry table_entry;

void sendUDP(int fd, struct CONNECTION to, uint8_t* msg, uint32_t msg_len);
void sendNetCloseConnection(int fd);
void sendStunLookup(int fd, struct CONNECTION to, uint16_t port);
void sendNetGetNode(int fd, struct CONNECTION to, uint16_t port);
void sendNetAlive(int fd, struct CONNECTION to, uint16_t port);
void sendNetJoin(int fd, struct CONNECTION to, struct CONNECTION src);
void sendNetJoinResp(int fd, struct CONNECTION next, uint8_t range_start, uint8_t range_end);
void sendValInsert(int fd, char* ssn, char* name, char* email);
void sendNetNewRange(int fd, uint8_t new_range_end);
void sendNetLeaving(int fd, struct CONNECTION next);
void forwardNetJoin(int fd, struct NET_JOIN_PDU pdu, uint8_t span, struct CONNECTION tcp);

#endif