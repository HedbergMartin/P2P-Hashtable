#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
// #include <netinet/in.h>

#include "headers/node.h"
#include "headers/pdu_parser.h"
#include "headers/pdu_sender.h"
#include "headers/pdu.h"

// struct NET_ALIVE_PDU {
//     uint8_t type;
//     uint8_t pad;
//     uint16_t port;
// };

struct NODE_INFO {
    uint8_t buffer[BUFFER_SIZE];
    size_t buffLen;
    uint16_t trackerPort;
    uint16_t nodePort;
    char trackerAddress[ADDRESS_LENGTH];
    char nodeAddress[ADDRESS_LENGTH];
    struct pollfd fds[6];
};

int main(const int argc, const char** argv) {

    // printf("%lu", ntohs(400));
    // uint8_t a[10000] = {0, 0, 1, 145, 0, 0, 143, 0};
    // size_t len = 4;
    // parseInStream(0, a, &len);
    struct NODE_INFO node;

    if (!initNode(&node, argc, argv)) {
        return -1;
    }

    runNode(&node);

    return 0;
}

void runNode(struct NODE_INFO *node) {

    sendStunLookup(node->nodePort, node->fds[TRACKER_FD].fd, node->trackerAddress, node->trackerPort);
    
    while (1) {
        int pollret = poll(node->fds, 4, 10000);
        if (pollret > 0) {

            for (int i = 0; i < 4; i++) {
                if (node->fds[i].revents & POLLIN) {
                    switch (i) {
                        case STDIN_FD:
                            break;
                        // case TRACKER_FD:
                        //     break;
                        // case AGENT_FD:
                        //     break;
                        // case TCP_ACCEPT_FD:
                        //     break;
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
        // printf("Len = %lu\n", *buffLen);
        if (node->buffLen > 0) {
            readAgain = handlePDU(node);
        } else {
            readAgain = false;
        }
    }
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
        default:
            break;
    }

    return read;
}

bool handleStunResponse(struct NODE_INFO* node) {
    struct STUN_RESPONSE_PDU pdu;
    bool read = PDUparseStunResponse(node->buffer, &(node->buffLen), &pdu);
    if (read) {
        memcpy(node->nodeAddress, pdu.address, ADDRESS_LENGTH);
        sendNetGetNode(node->nodePort, node->fds[TRACKER_FD].fd, node->trackerAddress, node->trackerPort);
    }
    return read;
}

int initNode(struct NODE_INFO *node, const int argc, const char **argv) {
    
    if (argc != 3) {
        fprintf(stderr, "Invalid arguments.");
        return -1;
    }
    
    node->buffLen = 0;

    char *rest;
    memcpy(node->trackerAddress, argv[1], ADDRESS_LENGTH);
    node->trackerPort = strtol(argv[2], &rest, 10);
    node->nodePort = 7000;

    if (strlen(rest) != 0) {
        printf("Port argument must be a number");
        return -1;
    }
    node->fds[STDIN_FD].fd = STDIN_FILENO;
    node->fds[STDIN_FD].events = POLLIN;

    node->fds[TRACKER_FD].fd = createSocket(node->nodePort, SOCK_DGRAM);
    node->fds[TRACKER_FD].events = POLLIN;

    node->fds[AGENT_FD].fd = createSocket(8000, SOCK_DGRAM); // TODO: determine port
    node->fds[AGENT_FD].events = POLLIN;

    node->fds[TCP_ACCEPT_FD].fd = createSocket(node->nodePort, SOCK_STREAM);
    node->fds[TCP_ACCEPT_FD].events = POLLIN;

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

int createSocket(int port, int type) {
    struct sockaddr_in in_addr;
    int insocket = socket(AF_INET, type, 0);

    if(insocket < 0) {
        perror("failed to create incoming socket");
        return -3;
    } else {
        in_addr.sin_family = AF_INET;
        in_addr.sin_port = htons(port);
        in_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    if(bind(insocket, (struct sockaddr*)&in_addr, sizeof(in_addr))) {
        perror("Failed to bind socket");
        return -4;
    }

    return insocket;
}