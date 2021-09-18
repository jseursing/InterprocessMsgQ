#pragma once
#include "StatusTypes.h"
#include <mutex>
#include <stdint.h>

class CommMessage;

/*
* This is the base class for which all Message Queues are
* derived from.
*/
class MsgQ
{
public:
  enum DurationEnum { NO_WAIT = 0, WAIT_FOREVER = 0xFFFFFFFF };

  uint32_t GetQId() const;
  uint32_t GetMaxLength() const;
  uint32_t GetMaxObjects() const;
  uint32_t GetNumObjects() const;
  virtual StatusTypes::StatusEnum SendMsg(void* msg, size_t len, uint32_t timeout) = 0;
  virtual StatusTypes::StatusEnum RecvMsg(void*& msg, size_t maxLen, uint32_t timeout) = 0;

protected:

  bool ProcessTimeout(uint32_t timeout, bool outgoing);
  MsgQ(uint32_t qid, uint32_t maxObjects);
  ~MsgQ();

  uint32_t QId;
  uint32_t MaxLength;
  uint32_t MaxObjects;
  uint32_t* NumObjects;
  void* Signal;
};