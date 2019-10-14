#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "headers/node.h"
#include "headers/pdu_parser.h"
#include "headers/pdu_sender.h"
#include "headers/pdu.h"
#include "headers/hash.h"
#include "headers/hash_table.h"


struct NODE_INFO {
    uint8_t buffer[BUFFER_SIZE];
    size_t buffLen;
    uint16_t responsePort;
    uint16_t agentPort;
    struct CONNECTION trackerConnection;
    struct CONNECTION nodeConnection;
    struct CONNECTION nextNodeConnection;
    struct pollfd fds[6];
    bool connected;
    uint8_t range_start;
    uint8_t range_end;
    struct hash_table* table;
};

int main(const int argc, const char** argv) {
    
    struct NODE_INFO node;
    if (!initNode(&node, argc, argv)) {
        return -1;
    }
    runNode(&node);
    return 0;
}

void runNode(struct NODE_INFO *node) {

    sendStunLookup(node->fds[UDP_FD].fd, node->trackerConnection, node->responsePort);
    
    while (1) {
        handleInstreams(node);

        if (node->connected) {
            sendNetAlive(node->fds[UDP_FD].fd, node->trackerConnection, node->responsePort);
        }
    }
}

void handleInstreams(struct NODE_INFO* node) {
    uint8_t nrCheck = 4;
    if (node->fds[TCP_RECEIVE_FD].fd != -1) {
        nrCheck = 6;
    }
    int pollret = poll(node->fds, nrCheck, 5000);

    if (pollret > 0) {

        for (int i = 0; i < nrCheck; i++) {
            if (node->fds[i].revents & POLLIN) {
                switch (i) {
                    case STDIN_FD:
                        handle_stdin(node);
                        break;
                    // case UDP_FD:
                    //     break;
                     case AGENT_FD:
                        parseInStream(node->fds[i].fd, node);
                        break;
                    case TCP_ACCEPT_FD:
                        printf("Got TCP connecion!\n");
                        node->fds[TCP_RECEIVE_FD].fd = accept(node->fds[TCP_ACCEPT_FD].fd, NULL, NULL);
                        break;
                    default:
                        printf("RACAEINVG\n");
                        parseInStream(node->fds[i].fd, node);
                        break;
                }
            }
        }

    } else if (pollret < 0) {
        perror("Poll error");
        //TODO figure out poll error handling.
    }
}

void parseInStream(int fd, struct NODE_INFO* node) {

    ssize_t len = read(fd, node->buffer + node->buffLen, BUFFER_SIZE);
    if (len < 0) {
        perror("Failed to read");
        return;
    } else if (len == 0) {
        // TODO: decide what to do
        return;
    }
    node->buffLen += len;

    bool readAgain = true;
    while (readAgain) {
// #ifdef DEBUG
        fprintf(stderr, "Instream message len = %lu. Instream message: ", node->buffLen);
        for (int i = 0; i < node->buffLen; i++) {
            fprintf(stderr, "%c", (char) (node->buffer[i]));
        }
        fprintf(stderr, "\n");
// #endif
        if (node->buffLen > 0) {
            readAgain = handlePDU(node);
        } else {
            readAgain = false;
        }
    }
#ifdef DEBUG
    fprintf(stderr, "Done parsing instream message, len above should be 0. Actual len: %ld\n", node->buffLen);
#endif
}

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
            printf("Woohoo, new range!\n"); // TODO still no receive =(
            read = handleNetNewRange(node);
            break;
        case NET_LEAVING:
            fprintf(stderr, "NET LEAVING\n"); // TODO still no receive :(
            read = handleNetLeaving(node);
            break;
        default:
            break;
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

bool handleNetCloseConnection(struct NODE_INFO* node) {
    struct NET_CLOSE_CONNECTION_PDU pdu;
    bool read = PDUparseNetCloseConnection(node->buffer, &(node->buffLen), &pdu);
    if (read) {
        if (node->fds[TCP_RECEIVE_FD].fd == node->fds[TCP_SEND_FD].fd) { // TODO get this working :/
            fprintf(stderr, "What the shit!?\n");
            if (close(node->fds[TCP_SEND_FD].fd) < 0) {
                perror("Closing TCP_SEND_FD\n");
            }
            node->fds[TCP_SEND_FD].fd = -1;
        }
        if (close(node->fds[TCP_RECEIVE_FD].fd) < 0) {
            perror("Closing TCP_RECEIVE_FD");
        }
        node->fds[TCP_RECEIVE_FD].fd = -1;
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

void divideHashTable(struct NODE_INFO* node) {
    struct hash_table* t = table_create(hash_ssn, getRange(node));
    struct table_entry* e = NULL;
    while ((e = get_entry_iterator(node->table)) != NULL) {
        int index = hash_ssn(e->ssn) % 255;
        if (index >= node->range_start && index <= node->range_end) {
            fprintf(stderr, "Sneding\n");
            table_insert(t, e->ssn, e->name, e->email);
        } else {
            fprintf(stderr, "Insnerting\n");
            sendValInsert(node->fds[TCP_SEND_FD].fd, e->ssn, e->name, e->email);
        }
    }
    table_free(node->table);
    node->table = t;
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
        if (index >= node->range_start && index <= node->range_end) {
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

    }
    return read;
}

bool handleValRemove(struct NODE_INFO* node) {
    struct VAL_REMOVE_PDU pdu;
    bool read = PDUparseValRemove(node->buffer, &(node->buffLen), &pdu);
    if (read) {

    }
    return read;
}

bool handleNetNewRange(struct NODE_INFO* node) {
    struct NET_NEW_RANGE_PDU pdu;
    fprintf(stderr, "Got NET_NEW_RANGE_PDU:");
    fprintf(stderr, "New max: %d\n\n", pdu.new_range_end);
    bool read = PDUparseNetNewRange(node->buffer, &(node->buffLen), &pdu);
    if (read) {
        node->range_end = pdu.new_range_end;
    }

    return read;
}

bool handleNetLeaving(struct NODE_INFO* node) {
    struct NET_LEAVING_PDU pdu;
    bool read = PDUparseNetLeaving(node->buffer, &(node->buffLen), &pdu);
    if (read) {
        fprintf(stderr, "Got NET_LEAVING_PDU");
        fprintf(stderr, "New node %s:%d", pdu.next_address, pdu.next_port);
        if (close(node->fds[TCP_SEND_FD].fd) < 0) {
            perror("Closing TCP_SEND_FD");
        }
        node->fds[TCP_SEND_FD].fd = -1;
        if (pdu.next_port == node->nodeConnection.port && strncmp(node->nodeConnection.address, pdu.next_address, ADDRESS_LENGTH)) {
            node->fds[TCP_RECEIVE_FD].fd = -1;
        } else {
            connectToNode(node, pdu.next_address, pdu.next_port);
            divideHashTable(node);
        }
    }

    return read;
}

int initNode(struct NODE_INFO *node, const int argc, const char **argv) {
    
    if (argc != 3) {
        fprintf(stderr, "Invalid arguments.\n");
        return 0;
    }
    
    node->buffLen = 0;
    node->connected = false;

    char *rest;
    memcpy(node->trackerConnection.address, argv[1], ADDRESS_LENGTH);
    node->trackerConnection.port = strtol(argv[2], &rest, 10);

    if (strlen(rest) != 0) {
        fprintf(stderr, "Port argument must be a number\n");
        return 0;
    }
    node->fds[STDIN_FD].fd = STDIN_FILENO;
    node->fds[STDIN_FD].events = POLLIN;

    node->fds[UDP_FD].fd = createServerSocket(0, SOCK_DGRAM);
    node->fds[UDP_FD].events = POLLIN;

    node->fds[AGENT_FD].fd = createServerSocket(0, SOCK_DGRAM); // TODO: determine port
    node->fds[AGENT_FD].events = POLLIN;

    node->fds[TCP_ACCEPT_FD].fd = createServerSocket(0, SOCK_STREAM);
    node->fds[TCP_ACCEPT_FD].events = POLLIN;

    node->fds[TCP_RECEIVE_FD].fd = -1;
    node->fds[TCP_RECEIVE_FD].events = POLLIN;

    node->fds[TCP_SEND_FD].fd = -1;
    node->fds[TCP_SEND_FD].events = POLLIN;

    node->responsePort = getSocketPort(node->fds[UDP_FD].fd);
    node->agentPort = getSocketPort(node->fds[AGENT_FD].fd);
    node->nodeConnection.port = getSocketPort(node->fds[TCP_ACCEPT_FD].fd);

    node->table = NULL;
    
    fprintf(stderr, "UDP port: %d\nTCP port: %d\nAgent port: %d\n", node->responsePort, node->nodeConnection.port, node->agentPort);

    for (int i = UDP_FD; i < TCP_ACCEPT_FD; i++) {
        if (node->fds[i].fd < 0) {
            return 0;
        }
    }
    if (listen(node->fds[TCP_ACCEPT_FD].fd, 100) < 0) { // TODO: Listen queue len?
        perror("Listen for connection");
        return 0;
    }

    return 1;
}

uint8_t getRange(struct NODE_INFO* node) {
    return node->range_end - node->range_start;
}

void setNewNodeRanges(uint8_t *pre_min, uint8_t* pre_max, uint8_t* succ_min, uint8_t* succ_max) {
    *succ_max = *pre_max;
    *pre_max = ((*pre_max - *pre_min) / 2) + *pre_min;
    *succ_min = *pre_max + 1;
}

int createSocket(char* address, int port, int commType, int sockType) {
    struct sockaddr_in in_addr;
    int insocket = socket(AF_INET, commType, 0);

    if(insocket < 0) {
        perror("failed to create incoming socket");
        return -3;
    }
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(port);

    if (sockType == SERVER_SOCK) {
        in_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if(bind(insocket, (struct sockaddr*)&in_addr, sizeof(in_addr))) {
            perror("Failed to bind socket");
            return -4;
        }
    } else if (sockType == CLIENT_SOCK) {
        if(inet_pton(AF_INET, address, &in_addr.sin_addr) <= 0) {
            fprintf(stderr, "Invalid adress\n");
            return -1;
        }

        if (connect(insocket, (struct sockaddr*)&in_addr, sizeof(in_addr))) {
            perror("Failed to connect to socket");
            return -1;
        }
    }

    return insocket;
}

int createServerSocket(int port, int commType) {
    return createSocket(NULL, port, commType, SERVER_SOCK);
}

void connectToNode(struct NODE_INFO* node, char* address, uint16_t port) {
    printf("Connecting to: %s : %d\n", address, port);
    if (node->fds[TCP_SEND_FD].fd != -1) {
        sendNetCloseConnection(node->fds[TCP_SEND_FD].fd);
    }
    node->fds[TCP_SEND_FD].fd = createSocket(address, port, SOCK_STREAM, CLIENT_SOCK);
    memcpy(node->nextNodeConnection.address, address, ADDRESS_LENGTH);
    node->nextNodeConnection.port = port;
}

uint16_t getSocketPort(int fd) {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(fd, (struct sockaddr *)&sin, &len) == -1) {
        perror("getsockname");
        return -1;
    }
    return ntohs(sin.sin_port);
}

void handle_stdin(struct NODE_INFO* node) {
    char buff[256];
    fgets(buff, 255, stdin);
    *strchr(buff, '\n') = 0;
    
    if(strcmp(buff, "exit") == 0) {

        nodeTerminate(node);
    } else if (strcmp(buff, "range") == 0) {
        printf("Range: %d - %d\n", node->range_start, node->range_end);
    } else {
        printf("Unknown command, valid commands are [range, exit]\n");
    }
}

void nodeTerminate(struct NODE_INFO *node) {
    if (node->nextNodeConnection.port != node->nodeConnection.port
    || strncmp(node->nextNodeConnection.address, node->nextNodeConnection.address, ADDRESS_LENGTH) != 0) {
        printf("SNEDINGSNEDINGSNEDING\n");
        sendNetCloseConnection(node->fds[TCP_SEND_FD].fd);
        sendNetNewRange(node->fds[TCP_RECEIVE_FD].fd, node->range_end);
        sendNetLeaving(node->fds[TCP_RECEIVE_FD].fd, node->nextNodeConnection);
    }
    for (int i = TCP_RECEIVE_FD; i < TCP_SEND_FD; i++) {
        close(node->fds[i].fd);
    }
    if (node->table != NULL) {
        table_free(node->table);
    }
    printf("Goodbye!\n");
    exit(0);
}