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

  static const void* INVALID_TASK;
};