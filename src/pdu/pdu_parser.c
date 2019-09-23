#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>

#include "headers/pdu_parser.h"
#include "headers/pdu.h"


void parseInStream(int fd, uint8_t* buffer, size_t* buffLen) {

    size_t len = read(fd, buffer, BUFFER_SIZE);
    if (len < 0) {
        perror("Failed to read");
        return;
    } else if (len == 0) {
        // TODO: decide what to do
        return;
    }
    (*buffLen) += len;

    bool readAgain = true;
    while (readAgain) {
        printf("Len = %lu\n", *buffLen);
        if ((*buffLen) > 0) {
            readAgain = PDUParse(buffer, buffLen);
        } else {
            readAgain = false;
        }
    }

}

bool PDUParse(uint8_t* buffer, size_t* buffLen) {
    bool read;
    switch(buffer[0]) {
        case NET_ALIVE:
            read = PDUparseNetAlive(buffer, buffLen);
            break;
        case STUN_RESPONSE:
            read = PDUparseStunResponse(buffer, buffLen);
            break;
        default:
            break;
    }

    return read;
}

bool readToPDUStruct(uint8_t* buffer, size_t* buffLen, void* pdu, size_t size) {
    if (*buffLen >= size) {
        memcpy(pdu, buffer, size);
        *buffLen -= size;
        memcpy(buffer, &buffer[size], *buffLen);
        return true;
    }

    return false;
}

bool PDUparseNetAlive(uint8_t* buffer, size_t* buffLen) {
    struct NET_ALIVE_PDU pdu;
    bool read = readToPDUStruct(buffer, buffLen, &pdu, sizeof(pdu));
    printf("Type: %d, Port: %d\n", pdu.type, pdu.port);
    pdu.port = ntohs(pdu.port);
    printf("Type: %d, Port: %d\n", pdu.type, pdu.port);
    return read;
}

bool PDUparseStunResponse(uint8_t* buffer, size_t* buffLen) {
    struct STUN_RESPONSE_PDU pdu;
    bool read = readToPDUStruct(buffer, buffLen, &pdu, sizeof(pdu));
    printf("%s\n\n", pdu.address);
    return read;
}

// char* readBuffer(uint8_t* buffer, int* pos, int bytesToRead) {
//     char* text = malloc(bytesToRead*sizeof(char));
//     memcpy(text, &buffer[*pos], bytesToRead);
//     *pos += bytesToRead;
//     return text;
// }

// struct NET_GET_NODE_RESPONSE_PDU* PDUparseNetGetNodeResponse() {
//     return NULL;
// }

// struct NET_CLOSE_CONNECTION_PDU *
// 
// struct NET_GET_NODE_PDU *
// 
// struct NET_JOIN_PDU *
// 
// struct NET_JOIN_RESPONSE_PDU *
// 
// struct STUN_LOOKUP_PDU *
// 
// struct NET_NEW_RANGE_PDU *
// 
// struct NET_LEAVING_PDU *
// 
// struct STUN_RESPONSE_PDU *
// 
// struct VAL_INSERT_PDU *
// 
// struct VAL_LOOKUP_PDU *
// 
// struct VAL_LOOKUP_RESPONSE_PDU *
// 
// struct VAL_REMOVE_PDU *