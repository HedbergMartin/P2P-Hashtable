#include "socket.h"
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <signal.h>

#define MAX_CLIENTS 256
#define UPDATE_FREQUENCY 3

#define ADDRESS_LENGTH 16

#define NET_ALIVE 0
#define NET_GET_NODE 1
#define NET_GET_NODE_RESPONSE 2
#define STUN_LOOKUP 200
#define STUN_RESPONSE 201


struct client {
    char addr[16];
    uint16_t port;
    unsigned long last_ping;
};

struct NET_GET_NODE_RESPONSE_PDU {
    uint8_t type;
    uint8_t address[ADDRESS_LENGTH];
    uint8_t PAD;
    uint16_t port;
};

struct NET_GET_NODE_PDU {
    uint8_t type;
    uint8_t PAD;
    uint16_t port;
};

struct NET_ALIVE_PDU {
    uint8_t type;
    uint8_t PAD;
    uint16_t port;
};

struct STUN_LOOKUP_PDU {
    uint8_t type;
    uint8_t PAD;
    uint16_t port;
};

struct STUN_RESPONSE_PDU {
    uint8_t type;
    uint8_t address[ADDRESS_LENGTH];
};

void handle_incoming(int server, struct message* msg);
void handle_stdin();
void handle_clients();
void handle_stun(int server, struct message* msg);
void handle_alive(struct message* msg);
void handle_get_node(int server, struct message* msg);
int resolve_fqdn(char *fqdn, size_t n);
void install_handler();
