#pragma once
#include "StatusTypes.h"
#include <stdint.h>

/*
* This class provides an interface to operating system 
* specific functions.
*/
class OSInterface
{
public:
  ////////////////////////////////////////////////////////////////////////
  //
  // Process Functions
  //
  static void* ProcessSpawn(char* cmdLine, bool waitForCreation);

  ////////////////////////////////////////////////////////////////////////
  //
  // Task Functions
  //
  static void* TaskCreate(void* entrypoint, 
                          void* param, 
                          uint32_t stackSize, 
                          unsigned long& threadId,
                          int32_t priority);
  static void ActivateTask(void* pTask);
  static void SuspendTask(void* pTask);
  static void TaskKill(void* pTask);
  static unsigned long GetTaskId();

  ////////////////////////////////////////////////////////////////////////
  //
  // Memory Functions
  //
  static void* CreateSharedMem(const char* path, uint32_t maxLength);
  static void* GetSharedMem(void* pSharedMem, uint32_t maxLength);
  static void CloseSharedMem(void* pSharedMem);

  ////////////////////////////////////////////////////////////////////////
  //
  // Synchronization Functions
  //
  static void* SignalCreate(const char* name);
  static void* GetSignal(const char* name);
  static StatusTypes::StatusEnum SignalWait(void* signal, uint32_t timeout);
  static void SignalNotify(void* signal);
  static void SignalReset(void* signal);
  static void* SemCreate(const char* name, int32_t iVal, int32_t mVal);
  static void* GetSem(const char* name);
  static StatusTypes::StatusEnum SemTake(void* sem, uint32_t timeout);
  static StatusTypes::StatusEnum SemGive(void* sem);

  ////////////////////////////////////////////////////////////////////////
  //
  // Timer Functions
  //
  static void* CreateTimer(void* routine, void* param, uint32_t duration);
  static void ReleaseTimer(void* pTimer);

  ////////////////////////////////////////////////////////////////////////
  //
  // Socket Functions
  //  
  static uintptr_t UDPCreate(StatusTypes::StatusEnum& status, 
                             uint16_t port = 0,
                             bool bindPort = false);
  static void UDPClose(uintptr_t sock);
  static StatusTypes::StatusEnum UDPSend(uintptr_t sock,
                                         char* addr, 
                                         uint16_t port, 
                                         char* buf, 
                                         uint32_t len);
  static StatusTypes::StatusEnum UDPRecv(uintptr_t sock,
                                         char* buf,
                                         uint32_t maxLen,
                                         uint32_t timeoutMs,
                                         char* addr = nullptr,
                                         uint32_t addrMaxLen = 0,
                                         uint16_t* port = nullptr);

  static const void* INVALID_TASK;
};