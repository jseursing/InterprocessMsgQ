#pragma once
#include "MsgQ.h"
#include <mutex>

///////////////////////////////////////////////////////////////
// 
// This Message Queue class handle object transmission within
// the same/external processes. This class utilizes Memory
// mapped I/O and therefore requires administrative privileges.
//
///////////////////////////////////////////////////////////////
class IPCMsgQ : public MsgQ
{
public:

  virtual StatusTypes::StatusEnum SendMsg(void* msg,
                                          size_t len, 
                                          uint32_t timeout = NO_WAIT);
  virtual StatusTypes::StatusEnum RecvMsg(void*& msg,
                                          size_t maxLen, 
                                          uint32_t timeout = NO_WAIT);
  IPCMsgQ(uint32_t qid, uint32_t maxObjects, uint32_t maxLen);
  ~IPCMsgQ();

private:

  void* QueueBuffer;
  void* IPCSemaphore;
};