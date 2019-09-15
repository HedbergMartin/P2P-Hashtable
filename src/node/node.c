#include "node.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../pdu/headers/pdu_parser.h"

// struct NET_ALIVE_PDU {
//     uint8_t type;
//     uint8_t pad;
//     uint16_t port;
// };

int main(int argc, char** argv) {

    
    uint8_t a[10000] = {0, 0, 144, 1, 0, 0, 143, 0};
    size_t len = 4;
    parseInStream(0, a, &len);

    // struct NET_ALIVE_PDU* pdu = malloc(sizeof(struct NET_ALIVE_PDU));
    // // uint8_t* type = malloc(sizeof(uint8_t));
    // // uint16_t* port = malloc(sizeof(uint16_t));


    // memcpy(pdu, a, 4);
    // // memcpy(port, a+2, 2);


    // printf("%d", sizeof(struct NET_ALIVE_PDU));
    //printf("HATHSENU\n");
    return 0;
}