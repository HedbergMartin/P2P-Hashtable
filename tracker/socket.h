#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <math.h>
#include <fcntl.h>
#include <poll.h>

struct message {
    uint8_t type;
    uint8_t* data;
    ssize_t read_bytes;
    struct sockaddr_in remote;
    socklen_t remote_length;
};

int create_socket(int local);
void send_all(int socket, struct sockaddr_in* to, uint8_t* msg, uint32_t msg_len);
struct message* read_message(int socket);
