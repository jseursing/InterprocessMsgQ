#include "IPCMsgQ.h"
#include "OSInterface.h"
#include <cassert>
#include <string>

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: SendMsg
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
StatusTypes::StatusEnum IPCMsgQ::SendMsg(void* msg, size_t len,  uint32_t timeout)
{
  // Do not continue if msg is nullptr
  if (nullptr == msg)
  {
    return StatusTypes::MSGQ_INVALID_IN_OBJ;
  }

  // Process timeout
  if (false == ProcessTimeout(timeout, true))
  {
    return StatusTypes::STATUS_ERROR_TIMEOUT;
  }

  // Lock mutex
  OSInterface::SemTake(IPCSemaphore, timeout);

  // Set the maximum length for the message
  size_t activeLength = (MaxLength < len ? MaxLength : len);

  // Set the address of the next free object to msg.
  uintptr_t bufPtr = reinterpret_cast<uintptr_t>(QueueBuffer) + 
                     (*NumObjects * MaxLength);
 
  // Write our message to the buffer, and fill the rest of the buffer with 0s
  memcpy(reinterpret_cast<void*>(bufPtr), msg, activeLength);
  memset(reinterpret_cast<void*>(bufPtr + len), 0, MaxLength - activeLength);

  // If there are 0 objects prior to now, send a signal
  if (0 == *NumObjects)
  {
    OSInterface::SignalNotify(Signal);
  }

  // Update FreeObjects
  ++(*NumObjects);

  // Unlock mutex
  OSInterface::SemGive(IPCSemaphore);

  return StatusTypes::STATUS_OK; 
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: RecvMsg
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
StatusTypes::StatusEnum IPCMsgQ::RecvMsg(void*& msg, size_t maxLen, uint32_t timeout)
{
  // If the passed in message is invalid, do not continue.
  if (nullptr == &msg)
  {
    return StatusTypes::MSGQ_INVALID_IN_OBJ;
  }

  // Process timeout
  if (false == ProcessTimeout(timeout, false))
  {
    return StatusTypes::STATUS_ERROR_TIMEOUT;
  }

  // Lock mutex
  OSInterface::SemTake(IPCSemaphore, timeout);

  // Set the maximum length for the message
  size_t activeLength = (MaxLength < maxLen ? MaxLength : maxLen);

  // Set msg to the first buffer entry, return if invalid.
  memcpy(msg, QueueBuffer, activeLength);
  if (nullptr == msg)
  {
    return StatusTypes::MSGQ_INVALID_OUT_OBJ;
  }

  // Update the queue buffer
  memcpy(QueueBuffer,
         &reinterpret_cast<char*>(QueueBuffer)[MaxLength],
         MaxLength * (MaxObjects - 1));
  memset(&reinterpret_cast<char*>(QueueBuffer)[(MaxObjects - 1) * MaxLength],
         0, MaxLength);

  // If there are max objects prior to now, send a signal
  if (MaxObjects == *NumObjects)
  {
    OSInterface::SignalNotify(Signal);
  }

  // Update NumObjects
  --(*NumObjects);

  // Unlock mutex
  OSInterface::SemGive(IPCSemaphore);

  // Update buffer block, guaranteed to succeed
  return StatusTypes::STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: IPCMsgQ
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
IPCMsgQ::IPCMsgQ(uint32_t qid, uint32_t maxObjects, uint32_t maxLen) :
  MsgQ(qid, maxObjects),
  QueueBuffer(nullptr),
  IPCSemaphore(nullptr)
{
  MaxLength = maxLen;

  // Construct Memory Mapped I/O path
  std::string mappedPath = "Global\\queue_" + std::to_string(qid);

  // Retrieve the mapped file handle and queue buffer.
  uint32_t bufSize = sizeof(uint32_t) + (maxLen * maxObjects);
  void* mappedHandle = OSInterface::CreateSharedMem(mappedPath.c_str(), bufSize);      
            
  if (nullptr != mappedHandle)
  {
    NumObjects = reinterpret_cast<uint32_t*>(
      OSInterface::GetSharedMem(mappedHandle, bufSize));

    QueueBuffer = NumObjects + 1;
    *NumObjects = 0;
  }

  // Retrieve the semaphore
  std::string queuePath = "Global\\queue_sem_" + std::to_string(qid);
  IPCSemaphore = OSInterface::SemCreate(queuePath.c_str(), 1, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: ~IPCMsgQ
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
IPCMsgQ::~IPCMsgQ()
{
  if (nullptr != QueueBuffer)
  {
    OSInterface::CloseSharedMem(QueueBuffer);
  }
}
