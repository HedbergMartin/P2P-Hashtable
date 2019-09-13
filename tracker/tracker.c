#include "tracker.h"

static struct client clients[MAX_CLIENTS]; 
static int client_count;
static volatile uint8_t running = 1;
static int random_selection = 1;
static int timeout = 10;

int main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "Expected a single port argument.");
        return 1;
    }

    int socket = create_socket(strtol(argv[1], NULL ,10));    
    
    if(socket < 0) {
        return -1;
    }

    printf("Tracker listening on port %s!\nCurrent node timeout is %d seconds.\n", argv[1], timeout);
    printf("Valid commands are: [status, forget, random, first, timeout, quit]\n");

    unsigned long last_update = 0;
    struct timespec time;

    install_handler();

    clock_gettime(CLOCK_REALTIME, &time);
    srandom(time.tv_sec);

    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = socket;
    fds[1].events = POLLIN;
    
    while(running) {
        int ret = poll(fds, 2, UPDATE_FREQUENCY); 
        if(ret > 0) {

            if(fds[0].revents & POLLIN) {
                handle_stdin();
            }

            if(fds[1].revents & POLLIN) {
                struct message* message = read_message(socket);
                handle_incoming(socket, message);
            }
        }

        clock_gettime(CLOCK_REALTIME, &time);
        if(last_update - time.tv_sec >= UPDATE_FREQUENCY) {
            handle_clients();
            last_update = time.tv_sec;
        }
    } 

    printf("Bye!\n");
    shutdown(socket, SHUT_RDWR);
    close(socket);
}

void handle_incoming(int server, struct message* msg) {
    switch(msg->type) {
        case NET_ALIVE:
            handle_alive(msg); 
        break;

        case NET_GET_NODE:
            handle_get_node(server, msg); 
        break;
        case STUN_LOOKUP:
            handle_stun(server, msg);
        break;
        default:
            printf("Got an invalid PDU!\n");
        break;
    }
}

void handle_get_node(int server, struct message* msg) {
    struct NET_GET_NODE_RESPONSE_PDU pdu;
    struct NET_GET_NODE_PDU req;
    memcpy(&req, msg->data, sizeof(struct NET_GET_NODE_PDU));
    memset(&pdu, 0, sizeof(struct NET_GET_NODE_RESPONSE_PDU));
    
    pdu.type = NET_GET_NODE_RESPONSE;
    if(client_count != 0) {
        int node = random_selection ? random() % client_count : 0;
        pdu.type = NET_GET_NODE_RESPONSE;
        strncpy((char*)pdu.address, clients[node].addr, ADDRESS_LENGTH);
        pdu.port = htons(clients[node].port);
    } 

    uint16_t rport = req.port;
    printf("Received a GET_NODE, responding to %s:%d\n", inet_ntoa(msg->remote.sin_addr), ntohs(rport));

    msg->remote.sin_port = rport;

    send_all(server, &msg->remote, (uint8_t*)&pdu, sizeof(struct NET_GET_NODE_RESPONSE_PDU));
}

void handle_alive(struct message* msg) {
    uint16_t port;

    struct timespec time;
    struct NET_ALIVE_PDU pdu;
    memcpy(&pdu, msg->data, sizeof(struct NET_ALIVE_PDU));
    char remote[16];
    strcpy(remote, inet_ntoa(msg->remote.sin_addr));
    port = htons(pdu.port);

    clock_gettime(CLOCK_REALTIME, &time);

    for(int i = 0; i < client_count; i++) {
        if(!strcmp(clients[i].addr, remote) && clients[i].port == port) {
            clients[i].last_ping = time.tv_sec;
            return;
        } 
    } 

    printf("Client %s:%d connected!\n", remote, port);
    struct client client;
    strcpy(client.addr, remote);
    client.last_ping = time.tv_sec;
    client.port = port;

    clients[client_count] = client;

    client_count++;
}

void handle_clients() {
    struct timespec time;

    clock_gettime(CLOCK_REALTIME, &time);

    for(int i = 0; i < client_count; i++) {
        if(time.tv_sec - clients[i].last_ping > timeout) {
            printf("Dropping client %s:%d because of inactivity\n", clients[i].addr, clients[i].port);
            clients[i] = clients[client_count - 1];
            client_count--;
            i--;
        }
    } 
}

void handle_stun(int server, struct message* msg) {
    char* addr = inet_ntoa(msg->remote.sin_addr);
    struct STUN_RESPONSE_PDU stun;
    struct STUN_LOOKUP_PDU lookup;
    memcpy(&lookup, msg->data, sizeof(struct STUN_LOOKUP_PDU));
    stun.type = STUN_RESPONSE;
    memcpy(&stun.address, addr, ADDRESS_LENGTH);


    printf("Got STUN request from %s:%d, responding to port: %d\n", addr, ntohs(msg->remote.sin_port),
            ntohs(lookup.port));

    msg->remote.sin_port = lookup.port; 

    send_all(server, &msg->remote, (uint8_t*)&stun, sizeof(struct STUN_RESPONSE_PDU));
}

static void sighant(int signal) {
    if(signal == SIGINT) {
        running = 0;
    }
}

void handle_stdin() {
    char buff[256];
    fgets(buff, 255, stdin);
    *strchr(buff, '\n') = 0;
    
    if(!strcmp(buff, "status")) {
        printf("Number of connected nodes: %d\n\n", client_count);
        for(int i = 0; i < client_count; i++) {
            printf("%d: %s:%d }\n", i + 1, clients[i].addr, clients[i].port);
        }
        printf("\n");
    } else if(!strcmp(buff, "quit")) {
        running = 0;
    } else if(!strcmp(buff, "forget")) {
        printf("Removed all nodes\n");
        client_count = 0;
    } else if(!strcmp(buff, "random")) {
        printf("Using random response mode, tracker responds with random alive node\n"); 
        random_selection = 1;
    } else if(!strcmp(buff, "first")) {
        printf("Using first response mode, tracker responds with first alive node\n"); 
        random_selection = 1;
    } else if(!strncmp(buff, "timeout", 7)) {
        char* s = strchr(buff, ' ');
        if(!s) {
            printf("Expected an integer argument, the new timeout in seconds\n");
        } else {
            timeout = strtol(s + 1, NULL, 10);
            printf("New timeout is %d seconds\n", timeout);
        }
    
    }else {
        printf("Unknown command, valid commands are [status, forget, random, first, timeout, quit]\n");
    }
}

void install_handler() {
    struct sigaction sig = {0}; 
    sig.sa_handler = sighant;
    sig.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sig, NULL);
}
