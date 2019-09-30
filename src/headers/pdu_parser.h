#ifndef PDU_PARSER
#define PDU_PARSER

#include <inttypes.h>
#include <stdbool.h>
#include "pdu.h"

#define BUFFER_SIZE 1100

bool readToPDUStruct(uint8_t* buffer, size_t* buffLen, void* pdu, size_t size);

bool PDUparseNetAlive(uint8_t* buffer, size_t* buffLen, struct NET_ALIVE_PDU* pdu);

bool PDUparseStunResponse(uint8_t* buffer, size_t* buffLen, struct STUN_RESPONSE_PDU* pdu);

#endif