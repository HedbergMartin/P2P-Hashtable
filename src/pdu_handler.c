#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "headers/pdu_handler.h"

struct NODE_INFO {
    uint8_t buffer[BUFFER_SIZE];
    size_t buffLen;
    uint16_t responsePort;
    struct CONNECTION trackerConnection;
    struct CONNECTION nodeConnection;
    struct CONNECTION nextNodeConnection;
    struct pollfd fds[6];
    bool connected;
    uint8_t range;
    short min_val;
    short max_val;
};

bool handlePDU (struct NODE_INFO* node) {
    bool read = 0;
    switch(node->buffer[0]) {
        case NET_ALIVE: ;
            struct NET_ALIVE_PDU pdu;
            read = PDUparseNetAlive(node->buffer, &(node->buffLen), &pdu);
            break;
        case STUN_RESPONSE:
            read = handleStunResponse(node); 
            break;
        case NET_GET_NODE_RESPONSE:
            read = handleNetGetNodeResponse(node);
            break;
        case NET_JOIN:
            read = handleNetJoin(node);
            break;
        case NET_JOIN_RESPONSE:
            read = handleNetJoinResponse(node);
            break;
        default:
            break;
    }

    return read;
}

bool handleStunResponse(struct NODE_INFO* node) {
    struct STUN_RESPONSE_PDU pdu;
    bool read = readToPDUStruct(node->buffer, &(node->buffLen), &pdu, sizeof(pdu));
    if (read) {
        memcpy(node->nodeConnection.address, pdu.address, ADDRESS_LENGTH);
        sendNetGetNode(node->fds[UDP_FD].fd, node->trackerConnection, node->responsePort);
    }
    return read;
}

bool handleNetGetNodeResponse(struct NODE_INFO* node) {
    struct NET_GET_NODE_RESPONSE_PDU pdu;
    bool read = PDUparseNetGetNodeResp(node->buffer, &(node->buffLen), &pdu);
    if (read) {
        if (pdu.port != 0) { //TODO eller address len?
            // Connect to predecesor
            struct CONNECTION to;
            memcpy(to.address, pdu.address, ADDRESS_LENGTH);
            to.port = pdu.port;
            sendNetJoin(node->fds[UDP_FD].fd, to, node->nodeConnection);
        } else {
            node->range = 255;
        }
        // Init table
        node->connected = true;
    }
    return read;
}

bool handleNetJoinResponse(struct NODE_INFO* node) {
    struct NET_JOIN_RESPONSE_PDU pdu;
    bool read = PDUparseNetJoinResp(node->buffer, &(node->buffLen), &pdu);
    if (read) {
        connectToNode(node, pdu.next_address, pdu.next_port);
        node->range = 100;
    }
    return read;
}

bool handleNetJoin(struct NODE_INFO* node) {
    struct NET_JOIN_PDU pdu;
    bool read = PDUparseNetJoin(node->buffer, &(node->buffLen), &pdu);
    if (read) {
        //printf("Node: %d Max: %d\n", node->nodeAcceptPort, pdu.max_port);
        if (node->range == 255) {
            connectToNode(node, pdu.src_address, pdu.src_port);
            
            sendNetJoinResp(node->fds[TCP_SEND_FD].fd, node->nodeConnection);
            node->range /= 2;
        } else if (strcmp(pdu.max_address, node->nodeConnection.address) == 0 && pdu.max_port == node->nodeConnection.port) {
            struct CONNECTION oldNext;
            memcpy(&oldNext, &(node->nextNodeConnection), sizeof(struct CONNECTION));
            
            //char oldNextAddr[ADDRESS_LENGTH];
            //memcpy(oldNextAddr, node->nextNodeConnection.address, ADDRESS_LENGTH);
            //uint16_t oldNextPort = node->nextNodePort;

            connectToNode(node, pdu.src_address, pdu.src_port);
            
            sendNetJoinResp(node->fds[TCP_SEND_FD].fd, oldNext);
            node->range /= 2;
        } else {
            forwardNetJoin(node->fds[TCP_SEND_FD].fd, pdu, node->range, node->nodeConnection);
        }
    }
    return read;
}