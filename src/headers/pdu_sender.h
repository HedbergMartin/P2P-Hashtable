/* 
 * Authours:    Buster Hultgren Wärn <busterw@cs.umu.se>
 *              Martin Hedberg <c17mhg@cs.umu.se>
 * Published:   2019-10-16
 * 
 * Sends PDU struct via either TCP or UDP.
 */

#ifndef __PDU_SENDER__
#define __PDU_SENDER__

#include "pdu.h"

/* 
 * Sends a message to the given address over UDP.
 * @param fd - The filedescritor to send through.
 * @param to - CONNECTION struct with the address to the receiver.
 * @param msg - Message to send
 * @param msg_len - Length of the message
 */
void sendUDP(int fd, struct CONNECTION to, uint8_t* msg, uint32_t msg_len);

/* 
 * Sends a net alive pdu, aka pings the tracker to show that the node is alive
 * @param fd - The filedescritor to send through.
 * @param to - CONNECTION struct with the address to the receiver.
 * @param port - port to accept traffic to.
 */
void sendNetAlive(int fd, struct CONNECTION to, uint16_t port);

/* 
 * Sends a request to get a node to connect to in the networks to the tracker
 * @param fd - The filedescritor to send through.
 * @param to - CONNECTION struct with the address to the receiver.
 * @param port - port for tracker to answer to.
 */
void sendNetGetNode(int fd, struct CONNECTION to, uint16_t port);

/* 
 * Sends request to join a network.
 * @param fd - The filedescritor to send through.
 * @param to - CONNECTION struct with the address to a node in the network.
 * @param src - CONNECTION struct with TCP accept data to the node.
 */
void sendNetJoin(int fd, struct CONNECTION to, struct CONNECTION src);

/* 
 * Forwards a net join to the next pdu and compares the span.
 * @param fd - The filedescritor to send through.
 * @param pdu - PDU to be forwarded
 * @param span - Span of the current node.
 * @param tcp - tcp accept address to the current node.
 */
void forwardNetJoin(int fd, struct NET_JOIN_PDU pdu, uint8_t span, struct CONNECTION tcp);


/* 
 * Answers the lonly node that it can join the network and connect to a given node.
 * @param fd - The filedescritor to send through.
 * @param next - CONNECTION struct with the address to the next node.
 * @param range_start - Start of hash table for the node to setup.
 * @param range_end - End of hash table for the node to setup.
 */
void sendNetJoinResp(int fd, struct CONNECTION next, uint8_t range_start, uint8_t range_end);

/* 
 * Sends a net close connection pdu through TCP.
 * @param fd - The filedescritor to send through.
 */
void sendNetCloseConnection(int fd);

/* 
 * Sends a update of the range to a node
 * @param fd - The filedescritor to send through.
 * @param new_range_end - place for the range to end.
 */
void sendNetNewRange(int fd, uint8_t new_range_end);

/* 
 * Sends a leaving pdu. Should be sent to predecessor.
 * @param fd - The filedescritor to send through.
 * @param next - The leavings node successor.
 */
void sendNetLeaving(int fd, struct CONNECTION next);

/* 
 * Sends a val insert to the next node over TCP.
 * @param fd - The filedescritor to send through.
 * @param ssn - Social security number to add to table.
 * @param name - Name to add to table.
 * @param email - Email to add to table.
 */
void sendValInsert(int fd, char* ssn, char* name, char* email);

/* 
 * Sends a response to the agents request of val lookup.
 * @param fd - The filedescritor to send through.
 * @param to - CONNECTION struct with the address to the next agent.
 * @param ssn - Social security number to add to table.
 * @param name - Name to add to table.
 * @param email - Email to add to table.
 */
void sendValLookupResp(int fd, struct CONNECTION to, char* ssn, char* name, char* email);

/* 
 * Forwards a val lookup request to the next node.
 * @param fd - The filedescritor to send through.
 * @param pdu - PDU to be forwarded.
 */
void forwardValLookup(int fd, struct VAL_LOOKUP_PDU pdu);

/* 
 * Forward val remove.
 * @param fd - The filedescritor to send through.
 * @param pdu - PDU to be forwarded.
 */
void forwardValRemove(int fd, struct VAL_REMOVE_PDU pdu);

/* 
 * Sends a stun lookup request to get public ip to the tracker through UDP.
 * @param fd - The filedescritor to send through.
 * @param to - CONNECTION struct with the address to the receiver.
 * @param port - port for tracker to answer to.
 */
void sendStunLookup(int fd, struct CONNECTION to, uint16_t port);

/* 
 * Helper function to easily build a buffer.
 * @param buffer - Buffer to store everything in.
 * @param buffSize - Size of the buffer, a pointer as this value will be updated.
 * @param input - Data to be added.
 * @param len - Len of the input.
 */
void addToBuffer(uint8_t* buffer, size_t* buffSize, uint8_t* input, size_t len);

#endif