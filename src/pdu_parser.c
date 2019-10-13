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
    uint16_t pdusize = 0;
    uint8_t namePos = 1 + SSN_LENGTH + 1 + 1;
    pdusize += namePos; //First bytes til lenght dependent field
    if (*buffLen < pdusize) {
        return false;
    }

    uint8_t namelen = buffer[1+SSN_LENGTH];
    pdusize += namelen + 1;
    if (*buffLen < pdusize) {
        return false;
    }
    uint8_t email_len = buffer[pdusize - 1];
    uint16_t emailPos = namePos + namelen + 1 + 7;
    pdusize += 7 + email_len;

    if (*buffLen < pdusize) {
        return false;
    }
    fprintf(stderr, "PDU Len: %d\n", pdusize);

    pdu->type = buffer[0];
    memcpy(pdu->ssn, buffer + 1, SSN_LENGTH);

    pdu->name_length = namelen;
    fprintf(stderr, "name len %d - '", pdu->name_length);
    // for (int i = 0; i < pdu->name_length; i++)
    //     fprintf(stderr, "%c", *(buffer + namePos + i));
    // fprintf(stderr, "'\n");
    pdu->name = malloc(pdu->name_length);
    memcpy(pdu->name, buffer + namePos, pdu->name_length);

    pdu->email_length = email_len;//buffer[namePos + pdu->name_length];
    // int emailPos = namePos + pdu->email_length + sizeof(uint8_t) * 7;
    fprintf(stderr, "Email len %d - ", pdu->email_length);

    // for (int i = 0; i < pdu->email_length; i++)
    //     fprintf(stderr, "%c", *(buffer + pdu->email_length + i));
    pdu->email = malloc(pdu->email_length);
    printf("Email pos: %d\n", emailPos);
    memcpy(pdu->email, buffer + emailPos, pdu->email_length);

    fprintf(stderr, "\n");
    // fprintf(stderr, "ALLOCATED SPACE: %ld\n", sizeof(pdu->email));

    *buffLen -= pdusize;
    memcpy(buffer, &buffer[pdusize], *buffLen);

    return true;
}