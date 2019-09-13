#ifndef PDU_PARSER
#define PDU_PARSER

#include <inttypes.h>


#define BUFFER_SIZE 1024

struct NET_ALIVE_PDU* PDUparseNetAlive(uint8_t* buffer, int* pos);

#endif