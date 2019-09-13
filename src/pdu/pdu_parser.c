#include <unistd.h>
#include <stdio.h>

#include "headers/pdu_parser.h"
#include "headers/pdu.h"


void parseInStream(int fd) {
    uint8_t buffer[BUFFER_SIZE] = {0};
    int pos = 0;
    size_t len = read(fd, buffer, BUFFER_SIZE);
    if (len < 0) {
        perror("Failed to read");
        return;
    } else if (len == 0) {
        // TODO: decide what to do
        return;
    }

    uint8_t type = buffer[0];

}

void PDUParse(uint8_t type, uint8_t* buffer, int* pos) {
    switch(type) {
        case NET_ALIVE:
            struct NET_ALIVE_PDU pdu; 
            int readBytes = PDUparseNetAlive(buffer, &pdu);
            // useNetAlivePdu(p);
            break;
        case NET_GET_NODE:
            break;
        default:
            break;
    }
}

struct NET_GET_NODE_RESPONSE_PDU* PDUparseNetGetNodeResponse() {
    return NULL;
}

int PDUparseNetAlive(uint8_t* buffer, NET_ALIVE_PDU* pdu) {
    int readBytes = 0;
    short pad = 1;
    pdu->type = buffer[readBytes += 1];
    readBytes += pad;
    pdu->port = buffer[readBytes += 2];
    
    return readBytes;
}

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