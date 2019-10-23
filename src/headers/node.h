#ifndef __NODE__
#define __NODE__

#define TCP_RECEIVE_FD 0
#define TCP_SEND_FD 1
#define STDIN_FD 2
#define UDP_FD 3
#define AGENT_FD 4
#define TCP_ACCEPT_FD 5

#define SERVER_SOCK 0
#define CLIENT_SOCK 1

#define HASH_RANGE 255

#include "pdu.h"
#include "pdu_handler.h"

// #define DEBUG
// #define SHOW_PDU

int createSocket(char* address, int port, int commType, int sockType);
int createServerSocket(int port, int commType);
uint16_t getSocketPort(int fd);
void connectToNode(struct NODE_INFO* node, char* address, uint16_t port);


void divideHashTable(struct NODE_INFO* node);

uint8_t getRange(struct NODE_INFO* node);
bool inRange(struct NODE_INFO* node, uint8_t index);
void setNewNodeRanges(uint8_t *pre_min, uint8_t* pre_max, uint8_t* succ_min, uint8_t* succ_max);
int initNode(struct NODE_INFO *node, const int argc, const char **argv);
void runNode(struct NODE_INFO *node);
void parseInStream(int fd, struct NODE_INFO* node);
void handleInstreams(struct NODE_INFO* node);
void handle_stdin(struct NODE_INFO* node);
void terminate(struct NODE_INFO* node);


/* SUPPORT FUNCTION FOR BITSTRING
* description: Computes the a exponentiation of integers.
* param[in]: base - Base of the Exponentiation.
* param[in]: expontent - Expontent of the Exponentiation.
* return: The Exponentiation as an integer.
*/
int powerOf (int base, int exponent);

#endif