#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "headers/pdu.h"
#include "headers/pdu_handler.h"
#include "headers/pdu_parser.h"
#include "headers/pdu_sender.h"
#include "headers/hash_table.h"
#include "headers/hash.h"
#include "headers/node.h"

bool handlePDU(struct NODE_INFO* node) {
    bool read = 0;
    switch(node->buffer[0]) {
        case NET_ALIVE: ;
            struct NET_ALIVE_PDU pdu;
            read = PDUparseNetAlive(node->buffer, &(node->buffLen), &pdu);
            break;
        case STUN_RESPONSE:
            read = handleStunResponse(node); 
            break;
        case NET_CLOSE_CONNECTION:
            fprintf(stderr, "NET CLOSE\n");
            read = handleNetCloseConnection(node);
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
        case VAL_INSERT:
            read = handleValInsert(node);
            break;
        case VAL_LOOKUP:
            read = handleValLookup(node);
            break;
        case VAL_REMOVE:
            read = handleValRemove(node);
            break;
        case NET_NEW_RANGE:
            read = handleNetNewRange(node);
            break;
        case NET_LEAVING:
            read = handleNetLeaving(node);
            break;
        case NET_FINGER_TABLE:
            read = handleFingerTable(node);
            break;
        default:
            break;
    }

    return read;
}

bool handleNetGetNodeResponse(struct NODE_INFO* node) {
    struct NET_GET_NODE_RESPONSE_PDU pdu;
    bool read = PDUparseNetGetNodeResp(node->buffer, &(node->buffLen), &pdu);
#ifdef SHOW_PDU
    fprintf(stderr, "Got NET_GET_NODE_RESPONSE:\n");
    fprintf(stderr, "%s:%d\n", pdu.address, pdu.port);
#endif
    if (read) {
        if (pdu.port != 0) {
            struct CONNECTION to;
            memcpy(to.address, pdu.address, ADDRESS_LENGTH);
            to.port = pdu.port;
            sendNetJoin(node->fds[UDP_FD].fd, to, node->nodeConnection);
        } else {
            node->range_start = 0;
            node->range_end = 255;
            node->table = table_create(hash_ssn, getRange(node));
        }
        node->connected = true;
    }
    return read;
}

bool handleNetJoin(struct NODE_INFO* node) {
    struct NET_JOIN_PDU pdu;
    bool read = PDUparseNetJoin(node->buffer, &(node->buffLen), &pdu);
#ifdef SHOW_PDU
    fprintf(stderr, "Got NET_JOIN_PDU:\n");
    fprintf(stderr, "Src - %s:%d. Max span %d -> %s:%d\n", pdu.src_address, pdu.src_port, pdu.max_span, pdu.max_address, pdu.max_port);
    fprintf(stderr, "My range is: %d.\n", getRange(node));
#endif
    if (read) {
        if (getRange(node) == 255) {
            connectToNode(node, pdu.src_address, pdu.src_port);
            
            uint8_t successor_range_start;
            uint8_t successor_range_end;
            setNewNodeRanges(&node->range_start, &node->range_end, &successor_range_start, &successor_range_end);

            sendNetJoinResp(node->fds[TCP_SEND_FD].fd, node->nodeConnection, successor_range_start, successor_range_end);
            divideHashTable(node);
        } else if (strcmp(pdu.max_address, node->nodeConnection.address) == 0 && pdu.max_port == node->nodeConnection.port) {
            struct CONNECTION oldNext;
            memcpy(&oldNext, &(node->nextNodeConnection), sizeof(struct CONNECTION));
            connectToNode(node, pdu.src_address, pdu.src_port);
            
            uint8_t successor_range_start;
            uint8_t successor_range_end;
            setNewNodeRanges(&node->range_start, &node->range_end, &successor_range_start, &successor_range_end);
            
            sendNetJoinResp(node->fds[TCP_SEND_FD].fd, oldNext, successor_range_start, successor_range_end);
            divideHashTable(node);
        } else {
            forwardNetJoin(node->fds[TCP_SEND_FD].fd, pdu, getRange(node), node->nodeConnection);
        }
    }
    return read;
}

bool handleNetJoinResponse(struct NODE_INFO* node) {
    struct NET_JOIN_RESPONSE_PDU pdu;
    bool read = PDUparseNetJoinResp(node->buffer, &(node->buffLen), &pdu);
#ifdef SHOW_PDU
    fprintf(stderr, "Got NET_JOIN_RESPONSE_PDU:\n");
    fprintf(stderr, "Next - %s:%d. Range: %d-%d\n", pdu.next_address, pdu.next_port, pdu.range_start, pdu.range_end);
#endif
    if (read) {
        connectToNode(node, pdu.next_address, pdu.next_port);
        node->range_start = pdu.range_start;
        node->range_end = pdu.range_end;
        node->table = table_create(hash_ssn, getRange(node));
    }
    return read;
}

bool handleNetCloseConnection(struct NODE_INFO* node) {
    struct NET_CLOSE_CONNECTION_PDU pdu;
    bool read = PDUparseNetCloseConnection(node->buffer, &(node->buffLen), &pdu);
    if (read) {
        fprintf(stderr, "handleNetCloseConnection - Closing TCP_RECEIVE_FD\n");
        if (close(node->fds[TCP_RECEIVE_FD].fd) < 0) {
            perror("Closing TCP_RECEIVE_FD");
        }
        node->fds[TCP_RECEIVE_FD].fd = -1;
    }
    return read;
}

bool handleNetNewRange(struct NODE_INFO* node) {
    struct NET_NEW_RANGE_PDU pdu;
    fprintf(stderr, "Got NET_NEW_RANGE_PDU - ");
    fprintf(stderr, "new max: %d\n\n", pdu.new_range_end);
    bool read = PDUparseNetNewRange(node->buffer, &(node->buffLen), &pdu);
    if (read) {
        if (node->range_end < pdu.new_range_end) {
            node->range_end = pdu.new_range_end;
        } else {
            fprintf(stderr, "Nah, range was bad: YEET\n");
            sendNetNewRange(node->fds[TCP_RECEIVE_FD].fd, pdu.new_range_end);
        }
    }

    return read;
}

bool handleNetLeaving(struct NODE_INFO* node) {
    struct NET_LEAVING_PDU pdu;
    bool read = PDUparseNetLeaving(node->buffer, &(node->buffLen), &pdu);
    if (read) {
        fprintf(stderr, "Got NET_LEAVING_PDU\n");
        fprintf(stderr, "My node %s:%d\n", node->nodeConnection.address, node->nodeConnection.port);
        fprintf(stderr, "New node %s:%d\n", pdu.next_address, pdu.next_port);
        fprintf(stderr, "Nodes equals: %d\n", strncmp(node->nodeConnection.address, pdu.next_address, ADDRESS_LENGTH));
        if (close(node->fds[TCP_SEND_FD].fd) < 0) {
            perror("Closing TCP_SEND_FD");
        }
        node->fds[TCP_SEND_FD].fd = -1;
        if (pdu.next_port != node->nodeConnection.port || strncmp(node->nodeConnection.address, pdu.next_address, ADDRESS_LENGTH) != 0) {
            connectToNode(node, pdu.next_address, pdu.next_port);
        }
    }

    return read;
}

bool handleValInsert(struct NODE_INFO* node) {
    struct VAL_INSERT_PDU pdu;
    bool read = PDUparseValInsert(node->buffer, &(node->buffLen), &pdu);
    if (read) {
#ifdef SHOW_PDU
        fprintf(stderr, "Got VAL_INSERT_PDU:\n");
        fprintf(stderr, "%s : %s : %s\n", pdu.ssn, pdu.name, pdu.email);
#endif
        int index = hash_ssn((char *) pdu.ssn) % 255;
        fprintf(stderr, "index: %d. Range %d-%d. ", index, node->range_start, node->range_end);
        if (inRange(node, index)) {
            fprintf(stderr, "Inserting: %s", (char *)pdu.ssn);
            table_insert(node->table, (char *)pdu.ssn, (char *)pdu.name, (char *)pdu.email);
        } else {
            fprintf(stderr, "Forwarding");
            sendValInsert(node->fds[TCP_SEND_FD].fd, (char *)pdu.ssn, (char *)pdu.name, (char *)pdu.email);
        }


        free(pdu.name);
        free(pdu.email);
        fprintf(stderr, "\n\n");
    }
    return read;
}

bool handleValLookup(struct NODE_INFO* node) {
    struct VAL_LOOKUP_PDU pdu;
    bool read = PDUparseValLookup(node->buffer, &(node->buffLen), &pdu);
    if (read) {
        int index = hash_ssn(pdu.ssn) % 255;
        if (inRange(node, index)) {
            
            struct table_entry* entry = table_lookup(node->table, pdu.ssn);
            struct CONNECTION to;
            memcpy(to.address, pdu.sender_address, ADDRESS_LENGTH);
            to.port = pdu.sender_port;
            if (entry) {
                sendValLookupResp(node->fds[AGENT_FD].fd, to, pdu.ssn, entry->name, entry->email);
            } else {
                sendValLookupResp(node->fds[AGENT_FD].fd, to, pdu.ssn, "", "");
            }
            
        } else {
            
            forwardValLookup(node->fds[TCP_SEND_FD].fd, pdu);
        }
    }
    return read;
}

bool handleValRemove(struct NODE_INFO* node) {
    struct VAL_REMOVE_PDU pdu;
    bool read = PDUparseValRemove(node->buffer, &(node->buffLen), &pdu);
    if (read) {
        int index = hash_ssn((char*)pdu.ssn) % 255;
        if (inRange(node, index)) {
            table_remove(node->table, (char*)pdu.ssn);
        } else {
            forwardValRemove(node->fds[TCP_SEND_FD].fd, pdu);
        }
    }
    return read;
}

bool handleStunResponse(struct NODE_INFO* node) {
    struct STUN_RESPONSE_PDU pdu;
    bool read = readToPDUStruct(node->buffer, &(node->buffLen), &pdu, sizeof(pdu));
#ifdef SHOW_PDU
    fprintf(stderr, "Got STUN_RESPNSE:\n");
    fprintf(stderr, "%s\n", pdu.address);
#endif
    if (read) {
        memcpy(node->nodeConnection.address, pdu.address, ADDRESS_LENGTH);
        sendNetGetNode(node->fds[UDP_FD].fd, node->trackerConnection, node->responsePort);
    }
    return read;
}

bool handleFingerTable(struct NODE_INFO* node) {
    struct NET_FINGER_TABLE_PDU pdu;
    bool read = readToPDUStruct(node->buffer, &(node->buffLen), &pdu, sizeof(pdu));
    if (read) {

    }
    return read;
}

