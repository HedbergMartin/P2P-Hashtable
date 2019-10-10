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
        pdu->range_start = pdu->range_start;
        pdu->range_end = pdu->range_end;
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

bool PDUparseValInsert(uint8_t* buffer, size_t* buffLen, struct VAL_INSERT_PDU* pdu) {
    //bool read = readToPDUStruct(buffer, buffLen, pdu, sizeof(*pdu));
    //if (read) {
    //    
    //}
    // uint8_t type;
    // uint8_t ssn[SSN_LENGTH];
    // uint8_t name_length;
    // uint8_t PAD;
    // uint8_t* name;
    // uint8_t email_length;
    // uint8_t PAD2[7];
    // uint8_t* email;
    
    int len = sizeof(uint8_t) * (1 + SSN_LENGTH + 1 + 1);
    fprintf(stderr, "name?\n");
    for (int i = 0; i < 10; i++)
        fprintf(stderr, "%c\n", *(buffer + len + i));
    fprintf(stderr, "\n");


    bool read = true;
    return read;
}