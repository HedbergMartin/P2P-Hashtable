#ifndef PDU_PARSER
#define PDU_PARSER

#include <inttypes.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024

void parseInStream(int fd, uint8_t* buffer, size_t* buffLen);
bool PDUParse(uint8_t* buffer, size_t* buffLen);
bool readToPDUStruct(uint8_t* buffer, size_t* buffLen, void* pdu, size_t size);

bool PDUparseNetAlive(uint8_t* buffer, size_t* buffLen);

#endif