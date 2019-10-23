/* 
 * Authours:    Buster Hultgren Wärn <busterw@cs.umu.se>
 *              Martin Hedberg <c17mhg@cs.umu.se>
 * Published:   2019-10-16
 * 
 * Reads a buffer containing some PDU and copies it to a PDU pointer.
 */

#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "headers/pdu.h"
#include "headers/pdu_parser.h"

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

bool PDUparseNetJoin(uint8_t* buffer, size_t* buffLen, struct NET_JOIN_PDU* pdu) {
    bool read = readToPDUStruct(buffer, buffLen, pdu, sizeof(*pdu));
    if (read) {
        pdu->src_port = ntohs(pdu->src_port);
        pdu->max_port = ntohs(pdu->max_port);
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

bool PDUparseNetCloseConnection(uint8_t* buffer, size_t* buffLen, struct NET_CLOSE_CONNECTION_PDU* pdu) {
    return readToPDUStruct(buffer, buffLen, pdu, sizeof(*pdu));
}

bool PDUparseNetNewRange(uint8_t* buffer, size_t* buffLen, struct NET_NEW_RANGE_PDU* pdu) { 
    return readToPDUStruct(buffer, buffLen, pdu, sizeof(*pdu));
}

bool PDUparseNetLeaving(uint8_t* buffer, size_t* buffLen, struct NET_LEAVING_PDU* pdu) {
    if (readToPDUStruct(buffer, buffLen, pdu, sizeof(*pdu))) {
        pdu->next_port = ntohs(pdu->next_port);
        return true;
    }
    return false;
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
    pdu->type = buffer[0];
    memcpy(pdu->ssn, buffer + 1, SSN_LENGTH);

    pdu->name_length = namelen;
    pdu->name = malloc(pdu->name_length);
    memcpy(pdu->name, buffer + namePos, pdu->name_length);

    pdu->email_length = email_len;
    pdu->email = malloc(pdu->email_length);
    memcpy(pdu->email, buffer + emailPos, pdu->email_length);
    
    *buffLen -= pdusize;
    memcpy(buffer, &buffer[pdusize], *buffLen);

    return true;
}

bool PDUparseValLookup(uint8_t* buffer, size_t* buffLen, struct VAL_LOOKUP_PDU* pdu) {
    bool read = readToPDUStruct(buffer, buffLen, pdu, sizeof(*pdu));
    if (read) {
        pdu->sender_port = ntohs(pdu->sender_port);
    }
    return read;
}

bool PDUparseValRemove(uint8_t* buffer, size_t* buffLen, struct VAL_REMOVE_PDU* pdu) {
    bool read = readToPDUStruct(buffer, buffLen, pdu, sizeof(*pdu));
    return read;
}

bool PDUparseNetFingerTable(uint8_t* buffer, size_t* buffLen, struct NET_FINGER_TABLE_PDU* pdu) {
    bool read = readToPDUStruct(buffer, buffLen, pdu, sizeof(*pdu));
    if (read) {
        pdu->origin.port = ntohs(pdu->origin.port);
        for (int i = 0; i < 8; i++) {
            pdu->ranges[i].port = ntohs(pdu->ranges[i].port);
        }
    }
    return read;
}