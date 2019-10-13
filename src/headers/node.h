#ifndef __NODE__
#define __NODE__

#define STDIN_FD 0
#define UDP_FD 1
#define AGENT_FD 2
#define TCP_ACCEPT_FD 3
#define TCP_RECEIVE_FD 4
#define TCP_SEND_FD 5

#define SERVER_SOCK 0
#define CLIENT_SOCK 1

#define HASH_RANGE 255

#define DEBUG
#define SHOW_PDU

typedef struct NODE_INFO NODE_INFO;
typedef struct CONNECTION CONNECTION;

int createSocket(char* address, int port, int commType, int sockType);
int createServerSocket(int port, int commType);
void connectToNode(struct NODE_INFO* node, char* address, uint16_t port);
uint16_t getSocketPort(int fd);

bool handlePDU(struct NODE_INFO* node);
bool handleStunResponse(struct NODE_INFO* node);
bool handleNetGetNodeResponse(struct NODE_INFO* node);
bool handleNetJoinResponse(struct NODE_INFO* node);
bool handleNetJoin(struct NODE_INFO* node);
bool handleValInsert(struct NODE_INFO *node);

uint8_t getRange(struct NODE_INFO* node);
void setNewNodeRanges(uint8_t *pre_min, uint8_t* pre_max, uint8_t* succ_min, uint8_t* succ_max);
int initNode(struct NODE_INFO *node, const int argc, const char **argv);
void runNode(struct NODE_INFO *node);
void parseInStream(int fd, struct NODE_INFO* node);
void handleInstreams(struct NODE_INFO* node);
void handle_stdin(struct NODE_INFO* node);


#endif