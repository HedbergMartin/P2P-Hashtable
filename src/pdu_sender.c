#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
        fprintf(stderr, "SendUDP: %d. %s : %d\n", sent, toAddress, toPort);
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

    sendUDP(fd, toAddress, toPort, (uint8_t*)&pdu, sizeof(struct STUN_LOOKUP_PDU));
}

void sendNetGetNode(uint16_t port, int fd, char* toAddress, uint16_t toPort) {
    struct NET_GET_NODE_PDU pdu;
    pdu.type = NET_GET_NODE;
    pdu.port = htons(port);

    sendUDP(fd, toAddress, toPort, (uint8_t*)&pdu, sizeof(struct STUN_LOOKUP_PDU));
}