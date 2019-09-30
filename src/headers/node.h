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

int initNode(struct NODE_INFO *node, const int argc, const char **argv);
void runNode(struct NODE_INFO *node);
void parseInStream(int fd, struct NODE_INFO* node);
bool handlePDU (struct NODE_INFO* node);
bool handleStunResponse(struct NODE_INFO* node);


#endif