/* 
 * Authours:    Buster Hultgren Wärn <busterw@cs.umu.se>
 *              Martin Hedberg <c17mhg@cs.umu.se>
 * Published:   2019-10-16
 * 
 * Sends PDU struct via either TCP or UDP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "headers/pdu.h"
#include "headers/pdu_sender.h"
#include "headers/hash_table.h"


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

void sendNetAlive(int fd, struct CONNECTION to, uint16_t port) {
    struct NET_ALIVE_PDU pdu;
    pdu.type = NET_ALIVE;
    pdu.port = htons(port);

    sendUDP(fd, to, (uint8_t*)&pdu, sizeof(pdu));
}

void sendNetGetNode(int fd, struct CONNECTION to, uint16_t port) {
    struct NET_GET_NODE_PDU pdu;
    pdu.type = NET_GET_NODE;
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

void forwardNetJoin(int fd, struct NET_JOIN_PDU pdu, uint8_t span, struct CONNECTION tcp) {
    pdu.src_port = htons(pdu.src_port);
    pdu.max_port = htons(pdu.max_port);
    if (pdu.max_span < span) {
        memcpy(pdu.max_address, tcp.address, ADDRESS_LENGTH);
        pdu.max_port = htons(tcp.port);
        pdu.max_span = span;
    }

    write(fd, (uint8_t*)&pdu, sizeof(pdu));
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

void sendNetCloseConnection(int fd) {
    struct NET_CLOSE_CONNECTION_PDU pdu;
    pdu.type = NET_CLOSE_CONNECTION;
    write(fd, (uint8_t*)&pdu, sizeof(pdu));
}

void sendNetNewRange(int fd, uint8_t new_range_end) {
    struct NET_NEW_RANGE_PDU pdu;
    pdu.new_range_end = new_range_end;
    pdu.type = NET_NEW_RANGE;
    fprintf(stderr, "Sending new range :%d\n", pdu.new_range_end);
    write(fd, (uint8_t*)&pdu, sizeof(pdu));
}

void sendNetLeaving(int fd, struct CONNECTION next) {
    struct NET_LEAVING_PDU pdu;
    memcpy(pdu.next_address, next.address, ADDRESS_LENGTH);
    pdu.next_port = ntohs(next.port);
    pdu.type = NET_LEAVING;
    write(fd, (uint8_t*)&pdu, sizeof(pdu));
}

void sendValInsert(int fd, char* ssn, char* name, char* email) {
    struct VAL_INSERT_PDU pdu;
    pdu.type = VAL_INSERT;
    pdu.email = (uint8_t *) email;
    pdu.email_length = strlen(email) + 1;
    pdu.name = (uint8_t *) name;
    pdu.name_length = strlen(name) + 1;
    memcpy(pdu.ssn, ssn, SSN_LENGTH);

    size_t pdusize = sizeof(pdu) + pdu.email_length - 8 + pdu.name_length - 8; //-8 for the pointer sizes

    uint8_t buffer[pdusize];
    size_t buffSize = 0;
    
    addToBuffer(buffer, &buffSize, (uint8_t*)&pdu, 16);
    addToBuffer(buffer, &buffSize, pdu.name, pdu.name_length);
    addToBuffer(buffer, &buffSize, &pdu.email_length, 1);
    addToBuffer(buffer, &buffSize, pdu.PAD2, 7);
    addToBuffer(buffer, &buffSize, pdu.email, pdu.email_length);

    if (buffSize != pdusize) {
        fprintf(stderr, "ERROR SIZE NOT MATCH!!!!\n");
    }

    write(fd, buffer, pdusize);
}

void sendValLookupResp(int fd, struct CONNECTION to, char* ssn, char* name, char* email) {
    struct VAL_LOOKUP_RESPONSE_PDU pdu;
    pdu.type = VAL_LOOKUP_RESPONSE;
    pdu.email = (uint8_t *) email;
    pdu.email_length = strlen(email) + 1;
    pdu.name = (uint8_t *) name;
    pdu.name_length = strlen(name) + 1;
    memcpy(pdu.ssn, ssn, SSN_LENGTH);

    size_t pdusize = sizeof(pdu) + pdu.email_length - 8 + pdu.name_length - 8; //-8 for the pointer sizes

    uint8_t buffer[pdusize];
    size_t buffSize = 0;
    
    addToBuffer(buffer, &buffSize, (uint8_t*)&pdu, 16);
    addToBuffer(buffer, &buffSize, pdu.name, pdu.name_length);
    addToBuffer(buffer, &buffSize, &pdu.email_length, 1);
    addToBuffer(buffer, &buffSize, pdu.PAD2, 7);
    addToBuffer(buffer, &buffSize, pdu.email, pdu.email_length);

    if (buffSize != pdusize) {
        fprintf(stderr, "ERROR SIZE NOT MATCH!!!!\n");
    }
    sendUDP(fd, to, buffer, pdusize);
}

void forwardValLookup(int fd, struct VAL_LOOKUP_PDU pdu) {
    pdu.sender_port = htons(pdu.sender_port);

    write(fd, (uint8_t*)&pdu, sizeof(pdu));
}

void forwardValRemove(int fd, struct VAL_REMOVE_PDU pdu) {
    write(fd, (uint8_t*)&pdu, sizeof(pdu));
}

void sendStunLookup(int fd, struct CONNECTION to, uint16_t port) {
    struct STUN_LOOKUP_PDU pdu;
    pdu.type = STUN_LOOKUP;
    pdu.port = htons(port);

    sendUDP(fd, to, (uint8_t*)&pdu, sizeof(pdu));
}

void addToBuffer(uint8_t* buffer, size_t* buffSize, uint8_t* input, size_t len) {
    memcpy(&buffer[*buffSize], input, len);
    *buffSize += len;
}
