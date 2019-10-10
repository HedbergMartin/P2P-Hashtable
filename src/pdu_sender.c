#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


#include "headers/pdu_sender.h"


void sendUDP(int fd, struct CONNECTION to, uint8_t* msg, uint32_t msg_len) {
    
    struct sockaddr_in toSock;
    toSock.sin_family = AF_INET;
    toSock.sin_port = htons(to.port);
    inet_aton(to.address, &(toSock.sin_addr));

    uint32_t sent = 0;
    do {
        sent += sendto(fd, (char *) msg + sent, msg_len - sent, 0, (struct sockaddr*)&toSock, sizeof(toSock));
        if (sent < 0) {
            perror("sendUDP:");
            fprintf(stderr, "\n");
            exit(1);
        }
    } while(sent != msg_len);
}

void sendStunLookup(int fd, struct CONNECTION to, uint16_t port) {
    struct STUN_LOOKUP_PDU pdu;
    pdu.type = STUN_LOOKUP;
    pdu.port = htons(port);

    sendUDP(fd, to, (uint8_t*)&pdu, sizeof(pdu));
}

void sendNetGetNode(int fd, struct CONNECTION to, uint16_t port) {
    struct NET_GET_NODE_PDU pdu;
    pdu.type = NET_GET_NODE;
    pdu.port = htons(port);

    sendUDP(fd, to, (uint8_t*)&pdu, sizeof(pdu));
}

void sendNetAlive(int fd, struct CONNECTION to, uint16_t port) {
    struct NET_ALIVE_PDU pdu;
    pdu.type = NET_ALIVE;
    pdu.port = htons(port);

    sendUDP(fd, to, (uint8_t*)&pdu, sizeof(pdu));
}

void sendNetJoin(int fd, struct CONNECTION to, struct CONNECTION src) {
    struct NET_JOIN_PDU pdu;
    pdu.type = NET_JOIN;
    memcpy(pdu.src_address, src.address, ADDRESS_LENGTH);
    pdu.src_port = htons(src.port);
    pdu.max_span = 0;
    memset(pdu.max_address, 0, ADDRESS_LENGTH);
    pdu.max_port = 0;
    
    sendUDP(fd, to, (uint8_t*)&pdu, sizeof(pdu));    
}

void sendNetJoinResp(int fd, struct CONNECTION next, uint8_t range_start, uint8_t range_end) {
    struct NET_JOIN_RESPONSE_PDU pdu;
    pdu.type = NET_JOIN_RESPONSE;
    memcpy(pdu.next_address, next.address, ADDRESS_LENGTH);
    pdu.next_port = htons(next.port);
    pdu.range_start = range_start;
    pdu.range_end = range_end;
    write(fd, (uint8_t*)&pdu, sizeof(pdu));
}

void forwardNetJoin(int fd, struct NET_JOIN_PDU pdu, uint8_t span, struct CONNECTION tcp) {
    pdu.src_port = htons(pdu.src_port);
    pdu.max_port = htons(pdu.max_port);
    if (pdu.max_span < span) {
        memcpy(pdu.max_address, tcp.address, ADDRESS_LENGTH);
        pdu.max_port = htons(tcp.port);
        pdu.max_span = span;
        
        printf("Span: %d, New max: %s : %d\n", pdu.max_span, pdu.max_address, ntohs(pdu.max_port));
    }

    write(fd, (uint8_t*)&pdu, sizeof(pdu));
}
