#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>

#include "headers/pdu_parser.h"
#include "headers/pdu.h"

bool readToPDUStruct(uint8_t* buffer, size_t* buffLen, void* pdu, size_t size) {
    if (*buffLen >= size) {
        memcpy(pdu, buffer, size);
        *buffLen -= size;
        memcpy(buffer, &buffer[size], *buffLen);
        return true;
    }

    return false;
}

bool PDUparseNetAlive(uint8_t* buffer, size_t* buffLen, struct NET_ALIVE_PDU* pdu) {
    bool read = readToPDUStruct(buffer, buffLen, pdu, sizeof(*pdu));
    pdu->port = ntohs(pdu->port);
    return read;
}

bool PDUparseNetGetNodeResp(uint8_t* buffer, size_t* buffLen, struct NET_GET_NODE_RESPONSE_PDU* pdu) {
    bool read = readToPDUStruct(buffer, buffLen, pdu, sizeof(*pdu));
    if (read) {
        pdu->port = ntohs(pdu->port);
    }
    return read;
}

bool PDUparseNetJoinResp(uint8_t* buffer, size_t* buffLen, struct NET_JOIN_RESPONSE_PDU* pdu) {
    bool read = readToPDUStruct(buffer, buffLen, pdu, sizeof(*pdu));
    if (read) {
        pdu->next_port = ntohs(pdu->next_port);
    }
    return read;
}

bool PDUparseNetJoin(uint8_t* buffer, size_t* buffLen, struct NET_JOIN_PDU* pdu) {
    bool read = readToPDUStruct(buffer, buffLen, pdu, sizeof(*pdu));
    if (read) {
        pdu->src_port = ntohs(pdu->src_port);
        pdu->max_port = ntohs(pdu->max_port);
    }
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