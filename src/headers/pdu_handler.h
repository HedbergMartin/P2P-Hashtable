#ifndef __PDU_HANDLER__
#define __PDU_HANDLER__

#include "pdu.h"

bool handlePDU(struct NODE_INFO* node);
bool handleNetGetNodeResponse(struct NODE_INFO* node);
bool handleNetJoin(struct NODE_INFO* node);
bool handleNetJoinResponse(struct NODE_INFO* node);
bool handleNetCloseConnection(struct NODE_INFO* node);
bool handleNetNewRange(struct NODE_INFO* node);
bool handleNetLeaving(struct NODE_INFO* node);
bool handleValInsert(struct NODE_INFO *node);
bool handleValLookup(struct NODE_INFO* node);
bool handleValRemove(struct NODE_INFO* node);
bool handleStunResponse(struct NODE_INFO* node);

#endif