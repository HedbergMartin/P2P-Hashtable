#ifndef __NODE__
#define __NODE__

#define STDIN_FD 0
#define TRACKER_FD 1
#define AGENT_FD 2
#define TCP_ACCEPT_FD 3
#define TCP_SEND_FD 4
#define TCP_RECEIVE_FD 5

typedef struct NODE_INFO NODE_INFO;

int createSocket(int port, int type);

void sendStunLookup(uint16_t port, int fd, struct NODE_INFO* node);
void sendUDP(int socket, struct sockaddr_in* to, uint8_t* msg, uint32_t msg_len);

int initNode(struct NODE_INFO *node, const int argc, const char **argv);

#endif