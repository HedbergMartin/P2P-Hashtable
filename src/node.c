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

    sendStunLookup(node->fds[TRACKER_FD].fd, node->trackerConnection, node->responsePort);
    
    while (1) {
        handleInstreams(node);

        if (node->connected) {
            sendNetAlive(node->fds[TRACKER_FD].fd, node->trackerConnection, node->responsePort);
        }
    }
}

void handleInstreams(struct NODE_INFO* node) {
    uint8_t nrCheck = 4;
    if (node->fds[TCP_RECEIVE_FD].fd != -1) {
        nrCheck = 5;
    }
    int pollret = poll(node->fds, nrCheck, 5000);

    if (pollret > 0) {

        for (int i = 0; i < nrCheck; i++) {
            if (node->fds[i].revents & POLLIN) {
                switch (i) {
                    case STDIN_FD:
                        handle_stdin();
                        break;
                    // case TRACKER_FD:
                    //     break;
                    // case AGENT_FD:
                    //     break;
                    case TCP_ACCEPT_FD:
                        printf("Got connecion!\n");
                        node->fds[TCP_RECEIVE_FD].fd = accept(node->fds[TCP_ACCEPT_FD].fd, NULL, NULL);
                        break;
                    default:
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

    size_t len = read(fd, node->buffer, BUFFER_SIZE);
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
#ifdef DEBUG
        printf("Len = %lu\n", node->buffLen);
#endif
        if (node->buffLen > 0) {
            readAgain = handlePDU(node);
        } else {
            readAgain = false;
        }
    }
#ifdef DEBUG
    printf("Done parse, len above should be 0\n");
#endif
}

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
        sendNetGetNode(node->fds[TRACKER_FD].fd, node->trackerConnection, node->responsePort);
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
            sendNetJoin(node->fds[TRACKER_FD].fd, to, node->nodeConnection);
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

int initNode(struct NODE_INFO *node, const int argc, const char **argv) {
    
    if (argc != 3) {
        fprintf(stderr, "Invalid arguments.");
        return -1;
    }
    
    node->buffLen = 0;
    node->connected = false;

    char *rest;
    memcpy(node->trackerConnection.address, argv[1], ADDRESS_LENGTH);
    node->trackerConnection.port = strtol(argv[2], &rest, 10);

    if (strlen(rest) != 0) {
        printf("Port argument must be a number");
        return -1;
    }
    node->fds[STDIN_FD].fd = STDIN_FILENO;
    node->fds[STDIN_FD].events = POLLIN;

    node->fds[TRACKER_FD].fd = createServerSocket(0, SOCK_DGRAM);
    node->fds[TRACKER_FD].events = POLLIN;

    node->fds[AGENT_FD].fd = createServerSocket(0, SOCK_DGRAM); // TODO: determine port
    node->fds[AGENT_FD].events = POLLIN;

    node->fds[TCP_ACCEPT_FD].fd = createServerSocket(0, SOCK_STREAM);
    node->fds[TCP_ACCEPT_FD].events = POLLIN;

    node->fds[TCP_RECEIVE_FD].fd = -1;
    node->fds[TCP_RECEIVE_FD].events = POLLIN;

    node->fds[TCP_SEND_FD].events = POLLOUT;

    node->responsePort = getSocketPort(node->fds[TRACKER_FD].fd);
    node->nodeConnection.port = getSocketPort(node->fds[TCP_ACCEPT_FD].fd);
    
    printf("UDP port: %d\nTCP port: %d\n", node->responsePort, node->nodeConnection.port);

    for (int i = TRACKER_FD; i < TCP_ACCEPT_FD; i++) {
        if (node->fds[i].fd < 0) {
            return -1;
        }
    }

    if (listen(node->fds[TCP_ACCEPT_FD].fd, 100) < 0) { // TODO: Listen queue len?
        perror("Listen for connection");
        return -1;
    }

    return 1;
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
// #ifdef DEBUG
    printf("Connecting to: %s : %d\n", address, port);
// #endif
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

void handle_stdin() {
    char buff[256];
    fgets(buff, 255, stdin);
    *strchr(buff, '\n') = 0;
    
    if(strcmp(buff, "exit") == 0) {
        exit(0); // TODO Fix frees
    } else {
        printf("Unknown command, valid commands are [exit]\n");
    }
}
