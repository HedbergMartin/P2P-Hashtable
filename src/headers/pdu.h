#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#define ADDRESS_LENGTH 16
#define SSN_LENGTH 13

#define NET_ALIVE 0
#define NET_GET_NODE 1
#define NET_GET_NODE_RESPONSE 2
#define NET_JOIN 3
#define NET_JOIN_RESPONSE 4
#define NET_CLOSE_CONNECTION 5

#define NET_NEW_RANGE 6
#define NET_LEAVING 7

#define VAL_INSERT 100
#define VAL_REMOVE 101
#define VAL_LOOKUP 102
#define VAL_LOOKUP_RESPONSE 103

#define STUN_LOOKUP 200
#define STUN_RESPONSE 201

#ifndef PDU_DEF
#define PDU_DEF

struct NET_GET_NODE_RESPONSE_PDU {
    uint8_t type;
    char address[ADDRESS_LENGTH];
    uint8_t PAD;
    uint16_t port;
};

struct NET_ALIVE_PDU {
    uint8_t type;
    uint8_t pad;
    uint16_t port;
};

struct NET_CLOSE_CONNECTION_PDU {
    uint8_t type;
};

struct NET_GET_NODE_PDU {
    uint8_t type;
    uint8_t pad;
    uint16_t port;
};

struct NET_JOIN_PDU {
    uint8_t type;
    char src_address[ADDRESS_LENGTH];
    uint8_t PAD;
    uint16_t src_port;
    uint8_t max_span;
    char max_address[ADDRESS_LENGTH];
    uint8_t PAD2;
    uint16_t max_port;
};

struct NET_JOIN_RESPONSE_PDU {
    uint8_t type;
    char next_address[ADDRESS_LENGTH];
    uint8_t PAD;
    uint16_t next_port;
    uint8_t range_start;
    uint8_t range_end;
};

struct STUN_LOOKUP_PDU {
    uint8_t type;
    uint8_t PAD;
    uint16_t port;
};

struct NET_NEW_RANGE_PDU {
    uint8_t type;
    uint8_t new_range_end;
};

struct NET_LEAVING_PDU {
    uint8_t type;
    char next_address[ADDRESS_LENGTH]; 
    uint8_t pad;
    uint16_t next_port;
};

struct STUN_RESPONSE_PDU {
    uint8_t type;
    uint8_t address[ADDRESS_LENGTH];
};

struct VAL_INSERT_PDU {
    uint8_t type;
    uint8_t ssn[SSN_LENGTH];
    uint8_t name_length;
    uint8_t PAD;
    uint8_t* name;
    uint8_t email_length;
    uint8_t PAD2[7];
    uint8_t* email;
};

struct VAL_LOOKUP_PDU {
    uint8_t type;
    char ssn[SSN_LENGTH];
    char sender_address[ADDRESS_LENGTH];
    uint16_t sender_port;
};

struct VAL_LOOKUP_RESPONSE_PDU {
    uint8_t type;
    uint8_t ssn[SSN_LENGTH];
    uint8_t name_length;
    uint8_t PAD;
    uint8_t* name;
    uint8_t email_length;
    uint8_t PAD2[7];
    uint8_t* email;
};

struct VAL_REMOVE_PDU {
    uint8_t type;
    uint8_t ssn[SSN_LENGTH];
};

#endif
