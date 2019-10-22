/* 
 * Authours:    Buster Hultgren Wärn <busterw@cs.umu.se>
 *              Martin Hedberg <c17mhg@cs.umu.se>
 * Published:   2019-10-16
 * 
 * Reads a buffer containing some PDU and copies it to a PDU pointer.
 */

#ifndef PDU_PARSER
#define PDU_PARSER

#include "pdu.h"

/* 
 * Reads from a buffer and copies that with memcpy to a PDU struct.
 * @param buffer - The buffer.
 * @param buffLen - How much actual data is in the buffer. Could be smaller than PDU size if sent via TCP.
 * @param pdu - The pdu to read to.
 * @param size - Size of the pdu
 * @returns - True if size <= buffLen, else false (the PDU has has not been read).
 */
bool readToPDUStruct(uint8_t* buffer, size_t* buffLen, void* pdu, size_t size);

/* 
 * Reads from a buffer containing a NET_ALIVE_PDU and copies that to a PDU struct.
 * @param buffer - The buffer.
 * @param buffLen - How much actual data is in the buffer. Could be smaller than PDU size if sent via TCP.
 * @param pdu - The PDU to read to.
 * @returns - True if PDU could be read; false if size of PDU is greater than buffLen and PDU could not be read.
 */
bool PDUparseNetAlive(uint8_t* buffer, size_t* buffLen, struct NET_ALIVE_PDU* pdu);

/* 
 * Reads from a buffer containing a NET_GET_NODE_RESPONSE_PDU and copies that to a PDU struct.
 * @param buffer - The buffer.
 * @param buffLen - How much actual data is in the buffer. Could be smaller than PDU size if sent via TCP.
 * @param pdu - The PDU to read to.
 * @returns - True if PDU could be read; false if size of PDU is greater than buffLen and PDU could not be read.
 */
bool PDUparseNetGetNodeResp(uint8_t* buffer, size_t* buffLen, struct NET_GET_NODE_RESPONSE_PDU* pdu);

/* 
 * Reads from a buffer containing a NET_JOIN_PDU and copies that to a PDU struct.
 * @param buffer - The buffer.
 * @param buffLen - How much actual data is in the buffer. Could be smaller than PDU size if sent via TCP.
 * @param pdu - The PDU to read to.
 * @returns - True if PDU could be read; false if size of PDU is greater than buffLen and PDU could not be read.
 */
bool PDUparseNetJoin(uint8_t* buffer, size_t* buffLen, struct NET_JOIN_PDU* pdu);

/* 
 * Reads from a buffer containing a NET_JOIN_RESPONSE_PDU and copies that to a PDU struct.
 * @param buffer - The buffer.
 * @param buffLen - How much actual data is in the buffer. Could be smaller than PDU size if sent via TCP.
 * @param pdu - The PDU to read to.
 * @returns - True if PDU could be read; false if size of PDU is greater than buffLen and PDU could not be read.
 */
bool PDUparseNetJoinResp(uint8_t* buffer, size_t* buffLen, struct NET_JOIN_RESPONSE_PDU* pdu);

/* 
 * Reads from a buffer containing a NET_CLOSE_CONNECTION_PDU and copies that to a PDU struct.
 * @param buffer - The buffer.
 * @param buffLen - How much actual data is in the buffer. Could be smaller than PDU size if sent via TCP.
 * @param pdu - The PDU to read to.
 * @returns - True if PDU could be read; false if size of PDU is greater than buffLen and PDU could not be read.
 */
bool PDUparseNetCloseConnection(uint8_t* buffer, size_t* buffLen, struct NET_CLOSE_CONNECTION_PDU* pdu);

/* 
 * Reads from a buffer containing a NET_NEW_RANGE_PDU and copies that to a PDU struct.
 * @param buffer - The buffer.
 * @param buffLen - How much actual data is in the buffer. Could be smaller than PDU size if sent via TCP.
 * @param pdu - The PDU to read to.
 * @returns - True if PDU could be read; false if size of PDU is greater than buffLen and PDU could not be read.
 */
bool PDUparseNetNewRange(uint8_t* buffer, size_t* buffLen, struct NET_NEW_RANGE_PDU* pdu);

/* 
 * Reads from a buffer containing a NET_LEAVING_PDU and copies that to a PDU struct.
 * @param buffer - The buffer.
 * @param buffLen - How much actual data is in the buffer. Could be smaller than PDU size if sent via TCP.
 * @param pdu - The PDU to read to.
 * @returns - True if PDU could be read; false if size of PDU is greater than buffLen and PDU could not be read.
 */
bool PDUparseNetLeaving(uint8_t* buffer, size_t* buffLen, struct NET_LEAVING_PDU* pdu);

/* 
 * Reads from a buffer containing a VAL_INSERT_PDU and copies that to a PDU struct.
 * @param buffer - The buffer.
 * @param buffLen - How much actual data is in the buffer. Could be smaller than PDU size if sent via TCP.
 * @param pdu - The PDU to read to.
 * @returns - True if PDU could be read; false if size of PDU is greater than buffLen and PDU could not be read.
 */
bool PDUparseValInsert(uint8_t* buffer, size_t* buffLen, struct VAL_INSERT_PDU* pdu);

/* 
 * Reads from a buffer containing a VAL_LOOKUP_PDU and copies that to a PDU struct.
 * @param buffer - The buffer.
 * @param buffLen - How much actual data is in the buffer. Could be smaller than PDU size if sent via TCP.
 * @param pdu - The PDU to read to.
 * @returns - True if PDU could be read; false if size of PDU is greater than buffLen and PDU could not be read.
 */
bool PDUparseValLookup(uint8_t* buffer, size_t* buffLen, struct VAL_LOOKUP_PDU* pdu);

/* 
 * Reads from a buffer containing a VAL_REMOVE_PDU and copies that to a PDU struct.
 * @param buffer - The buffer.
 * @param buffLen - How much actual data is in the buffer. Could be smaller than PDU size if sent via TCP.
 * @param pdu - The PDU to read to.
 * @returns - True if PDU could be read; false if size of PDU is greater than buffLen and PDU could not be read.
 */
bool PDUparseValRemove(uint8_t* buffer, size_t* buffLen, struct VAL_REMOVE_PDU* pdu);

bool PDUparseNetFingerTable(uint8_t* buffer, size_t* buffLen, struct NET_FINGER_TABLE* pdu);

#endif