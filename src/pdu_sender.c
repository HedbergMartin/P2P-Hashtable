#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


#include "headers/pdu_sender.h"
#include "headers/pdu.h"


void sendUDP(int socket, char* toAddress, uint16_t toPort, uint8_t* msg, uint32_t msg_len) {
    
    struct sockaddr_in to;
    to.sin_family = AF_INET;
    to.sin_port = htons(toPort);
    inet_aton(toAddress, &(to.sin_addr));

    uint32_t sent = 0;
    do {
        sent += sendto(socket, (char *) msg + sent, msg_len - sent, 0, (struct sockaddr*)&to, sizeof(to));
        //fprintf(stderr, "SendUDP: Type: %d, To %s:%d\n", msg[0], toAddress, toPort);
    } while(sent != msg_len);
    if (sent < 0) {
        perror("sendUDP:");
        fprintf(stderr, "\n");
        exit(1);
    }
}

void sendStunLookup(uint16_t port, int fd, char* toAddress, uint16_t toPort) {
    struct STUN_LOOKUP_PDU pdu;
    pdu.type = STUN_LOOKUP;
    pdu.port = htons(port);

    sendUDP(fd, toAddress, toPort, (uint8_t*)&pdu, sizeof(pdu));
}

void sendNetGetNode(uint16_t port, int fd, char* toAddress, uint16_t toPort) {
    struct NET_GET_NODE_PDU pdu;
    pdu.type = NET_GET_NODE;
    pdu.port = htons(port);

    sendUDP(fd, toAddress, toPort, (uint8_t*)&pdu, sizeof(pdu));
}

void sendNetAlive(uint16_t port, int fd, char* toAddress, uint16_t toPort) {
    struct NET_ALIVE_PDU pdu;
    pdu.type = NET_ALIVE;
    pdu.port = htons(port);

    sendUDP(fd, toAddress, toPort, (uint8_t*)&pdu, sizeof(pdu));
}

void sendNetJoin(char* srcAddr, uint16_t srcPort, int fd, char* toAddress, uint16_t toPort) {
    struct NET_JOIN_PDU pdu;
    pdu.type = NET_JOIN;
    memcpy(pdu.src_address, srcAddr, ADDRESS_LENGTH);
    pdu.src_port = htons(srcPort);
    pdu.max_span = 0;
    memset(pdu.max_address, 0, ADDRESS_LENGTH);
    pdu.max_port = 0;
    
    sendUDP(fd, toAddress, toPort, (uint8_t*)&pdu, sizeof(pdu));    
}

void forwardNetJoin(int fd, struct NET_JOIN_PDU pdu, uint8_t span, char* addr, uint16_t port) {
    pdu.src_port = htons(pdu.src_port);
    pdu.max_port = htons(pdu.max_port);
    if (pdu.max_span < span) {
        memcpy(pdu.max_address, addr, ADDRESS_LENGTH);
        pdu.max_port = htons(port);
        pdu.max_span = span;
    }

    write(fd, (uint8_t*)&pdu, sizeof(pdu));
}

void sendNetJoinResp(int fd, char* nextAddr, uint16_t nextPort) {
    struct NET_JOIN_RESPONSE_PDU pdu;
    pdu.type = NET_JOIN_RESPONSE;
    memcpy(pdu.next_address, nextAddr, ADDRESS_LENGTH);
    pdu.next_port = htons(nextPort);
    pdu.range_start = 0;
    pdu.range_end = 0;
    
    // send(fd, (uint8_t*)&pdu, sizeof(struct NET_JOIN_RESPONSE_PDU), 0);
    write(fd, (uint8_t*)&pdu, sizeof(pdu));
    //sendUDP(fd, toAddress, toPort, (uint8_t*)&pdu, sizeof(struct NET_JOIN_PDU));    
}
