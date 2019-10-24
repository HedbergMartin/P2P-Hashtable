#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#include "headers/pdu.h"
#include "headers/node.h"
#include "headers/pdu_parser.h"
#include "headers/pdu_sender.h"
#include "headers/hash.h"
#include "headers/hash_table.h"
#include "headers/sighant.h"

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
    uint8_t start = 2;
    if (node->fds[TCP_RECEIVE_FD].fd != -1 || node->fds[TCP_SEND_FD].fd != -1) {
        start = 0;
    }
    int pollret = poll(node->fds + start, 6-start, 5000);
    
    if (pollret > 0) {
        for (int i = start; i < 6; i++) {
            if (node->fds[i].revents & POLLIN) {
                switch (i) {
                    case STDIN_FD:
                        handle_stdin(node);
                        break;
                     case AGENT_FD:
                        parseInStream(node->fds[i].fd, node);
                        break;
                    case TCP_ACCEPT_FD:
                        printf("Got TCP connecion!\n");
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
        exit(0);
    }
}

void parseInStream(int fd, struct NODE_INFO* node) {

    ssize_t len = read(fd, node->buffer + node->buffLen, 1024);
    if (len < 0) {
        perror("Failed to read");
        return;
    } else if (len == 0) {
        return;
    }
    node->buffLen += len;

    bool readAgain = true;
    while (readAgain) {
#ifdef DEBUG
        fprintf(stderr, "Instream message len = %lu. Instream message: ", node->buffLen);
        for (uint i = 0; i < node->buffLen; i++) {
            fprintf(stderr, "%c", (char) (node->buffer[i]));
        }
        fprintf(stderr, "\n");
#endif
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

void divideHashTable(struct NODE_INFO* node) {
    struct hash_table* t = table_create(hash_ssn, getRange(node));
    struct table_entry* e = NULL;
    while ((e = get_entry_iterator(node->table)) != NULL) {
        int index = hash_ssn(e->ssn) % 255;
        if (inRange(node, index)) {
            fprintf(stderr, "Inserting\n");
            table_insert(t, e->ssn, e->name, e->email);
        } else {
            fprintf(stderr, "Sneding\n");
            sendValInsert(node->fds[TCP_SEND_FD].fd, e->ssn, e->name, e->email);
        }
    }
    table_free(node->table);
    node->table = t;
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

    node->fds[AGENT_FD].fd = createServerSocket(0, SOCK_DGRAM);
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
    if (listen(node->fds[TCP_ACCEPT_FD].fd, 100) < 0) {
        perror("Listen for connection");
        return 0;
    }

    return 1;
}

uint8_t getRange(struct NODE_INFO* node) {
    return node->range_end - node->range_start;
}

bool inRange(struct NODE_INFO* node, uint8_t index) {
    fprintf(stderr, "In range? Node range: %d-%d. Index %d\n", node->range_start, node->range_end, index);
    return index >= node->range_start && index <= node->range_end && getRange(node) > 0;
}

void setNewNodeRanges(uint8_t *pre_min, uint8_t* pre_max, uint8_t* succ_min, uint8_t* succ_max) {
    *succ_max = *pre_max;
    *pre_max = ((*pre_max - *pre_min) / 2) + *pre_min;
    *succ_min = *pre_max + 1;
}

struct CONNECTION nodeGetConnectionFromFingerTable(struct NODE_INFO* node, int index) {
    fprintf(stderr, "\nGetting connection with index %d\n", index);
    struct CONNECTION to = node->fingerTable[7];
    for (int i = 0; i < 8; i++) {
        int bluh = (node->range_end + powerOf(2, i)) % 256;
        fprintf(stderr, "i: %d. %d -> %d\n", i, node->range_end, bluh);
        if (index >= bluh) {
            fprintf(stderr, "This finger looks nice\n");
            to = node->fingerTable[i];
        }
    }
    return to;
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
    
    if(strncmp(buff, "exit", 4) == 0) {
        terminate(node);
    } else if (strncmp(buff, "ports", 5) == 0) {
        printf("\t--------------------\n");
        printf("UDP port: %d\nTCP port: %d\nAgent port: %d\n"
            , node->responsePort, node->nodeConnection.port, node->agentPort);
        printf("\t--------------------\n");
    } else if (strncmp(buff, "status", 6) == 0) {
        printf("\t--------------------\n");
        printf("TCP Receive port: %s\n", node->fds[TCP_RECEIVE_FD].fd != -1 ? "Connected" : "Disconnected");
        printf("TCP Send port: %s\n", node->fds[TCP_SEND_FD].fd != -1 ? "Connected" : "Disconnected");
        printf("Range: %d - %d\n", node->range_start, node->range_end);
        printf("Hash table number of entries: %d\n", node->table == NULL ? 0 : table_get_nr_entries(node->table));
        printf("\t--------------------\n");
    } else if (strncmp(buff, "finger", 6) == 0) {
        printf("\t--------------------\n");
        for (int i = 0; i < 8; i++) {
            int entry = (node->range_end + powerOf(2, i)) % 256;
            printf("Value: %d: Address: %s : %d\n", entry, node->fingerTable[i].address, node->fingerTable[i].port);
        }
        printf("\t--------------------\n");
    } else {
        printf("Unknown command, valid commands are [ports, status, exit]\n");
    }
}

void terminate(struct NODE_INFO* node) {

    if (getRange(node) < 255) {
        sendNetNewRange(node->fds[TCP_RECEIVE_FD].fd, node->range_end);
        node->range_end = node->range_start;
        if (node->table != NULL) {
            divideHashTable(node);
            table_free(node->table);
        }
        sendNetCloseConnection(node->fds[TCP_SEND_FD].fd);
        sendNetLeaving(node->fds[TCP_RECEIVE_FD].fd, node->nextNodeConnection);
    }
    for (int i = AGENT_FD; i < TCP_SEND_FD; i++) {
        if (node->fds[i].fd > 0) {
            if (close(node->fds[i].fd) < 0) {
                perror("Error closing port\n");
            } else {
                printf("Succesfully closed TCP port\n");
            }
        } else {
            printf("Port already closed\n");
        }
    }
    printf("Goodbye!\n");
    exit(0);
}

int powerOf(int base, int exponent) {
	int Exponentiation = 1;
	for (int i = 0; i < exponent; i++) {
		Exponentiation *= base;
	}

	return Exponentiation;
}