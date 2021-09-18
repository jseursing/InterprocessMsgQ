#include "OSInterface.h"
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

const void* OSInterface::INVALID_TASK = INVALID_HANDLE_VALUE;


/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: ProcessSpawn
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void* OSInterface::ProcessSpawn(char* cmdLine, bool waitForCreation)
{
  STARTUPINFOA info = { sizeof(STARTUPINFOA) };

  PROCESS_INFORMATION processInfo;
  if (CreateProcessA(nullptr, 
                     cmdLine, 
                     nullptr, 
                     nullptr, 
                     TRUE, 
                     NORMAL_PRIORITY_CLASS | DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP,
                     nullptr, 
                     nullptr, 
                     &info, 
                     &processInfo))
  {
    if (true == waitForCreation)
    {
      WaitForSingleObject(processInfo.hProcess, INFINITE);
    }

    // We are not interested in these handles for now...
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
  }

  return processInfo.hProcess;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: TaskCreate
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void* OSInterface::TaskCreate(void* entrypoint,
                              void* param,
                              uint32_t stackSize,
                              unsigned long& threadId,
                              int32_t priority)
{
  void* taskHandle = 
    CreateThread(nullptr, 
                 stackSize,
                 reinterpret_cast<LPTHREAD_START_ROUTINE>(entrypoint),
                 param,
                 CREATE_SUSPENDED,
                 &threadId);
  if (0 < taskHandle)
  {
    SetThreadPriority(taskHandle, priority);
  }

  return taskHandle;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: ActivateTask
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void OSInterface::ActivateTask(void* pTask)
{
  ResumeThread(pTask);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: SuspendTask
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void OSInterface::SuspendTask(void* pTask)
{
  SuspendThread(pTask);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: TaskKill
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void OSInterface::TaskKill(void* pTask)
{
  // Disable thread cleanup warning
#pragma warning(push)
#pragma warning(disable : 6258)
  TerminateThread(pTask, 0);
#pragma warning(pop)
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: GetTaskId
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
unsigned long OSInterface::GetTaskId()
{
  return GetCurrentThreadId();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: CreateSharedMem
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void* OSInterface::CreateSharedMem(const char* path, uint32_t maxLength)
{
  return CreateFileMappingA(INVALID_HANDLE_VALUE,
                            nullptr,
                            PAGE_READWRITE,
                            0,
                            maxLength,
                            path);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: GetSharedMem
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void* OSInterface::GetSharedMem(void* pSharedMem, uint32_t maxLength)
{
  return MapViewOfFile(pSharedMem, FILE_MAP_ALL_ACCESS, 0, 0, maxLength);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: CloseSharedMem
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void OSInterface::CloseSharedMem(void* pSharedMem)
{
  UnmapViewOfFile(pSharedMem);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: SignalCreate
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void* OSInterface::SignalCreate(const char* name)
{
  void* sig = CreateEventA(nullptr, TRUE, FALSE, name);
  if (ERROR_ALREADY_EXISTS == GetLastError())
  {
    sig = GetSignal(name);
  }

  return sig;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: GetSignal
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void* OSInterface::GetSignal(const char* name)
{
  return OpenEventA(EVENT_ALL_ACCESS, FALSE, name);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: SignalWait
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
StatusTypes::StatusEnum OSInterface::SignalWait(void* signal, uint32_t timeout)
{
  StatusTypes::StatusEnum status;

  unsigned long os_status = WaitForSingleObjectEx(signal, timeout, FALSE);
  switch (os_status)
  {
  case WAIT_OBJECT_0:
    status = StatusTypes::STATUS_OK;
    break;
  case WAIT_TIMEOUT:
    status = StatusTypes::STATUS_ERROR_TIMEOUT;
    break;
  default:
    status = StatusTypes::STATUS_ERROR;
  }

  SignalReset(signal);

  return status;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: SignalNotify
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void OSInterface::SignalNotify(void* signal)
{
  SetEvent(signal); 
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: SignalReset
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void OSInterface::SignalReset(void* signal)
{
  ResetEvent(signal);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: SemCreate
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void* OSInterface::SemCreate(const char* name, int32_t iVal, int32_t mVal)
{
  void* sem = CreateSemaphoreA(nullptr, iVal, mVal, name);
  if (ERROR_ALREADY_EXISTS == GetLastError())
  {
    sem = GetSem(name);
  }

  return sem;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: GetSem
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void* OSInterface::GetSem(const char* name)
{
  return OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, name);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: SemTake
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
StatusTypes::StatusEnum OSInterface::SemTake(void* sem, uint32_t timeout)
{
  StatusTypes::StatusEnum status;

  unsigned long os_status = WaitForSingleObjectEx(sem, timeout, FALSE);
  switch (os_status)
  {
  case WAIT_OBJECT_0:
    status = StatusTypes::STATUS_OK;
    break;
  case WAIT_TIMEOUT:
    status = StatusTypes::STATUS_ERROR_TIMEOUT;
    break;
  default:
    status = StatusTypes::SEMMGR_TAKE_ERROR;
  }

  return status;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: SemGive
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
StatusTypes::StatusEnum OSInterface::SemGive(void* sem)
{
  if (0 == ReleaseSemaphore(sem, 1, nullptr))
  {
    return StatusTypes::SEMMGR_GIVE_ERROR;
  }

  return StatusTypes::STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: CreateTimer
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void* OSInterface::CreateTimer(void* routine, void* param, uint32_t duration)
{
  void* timerObject = nullptr;
  if (CreateTimerQueueTimer(&timerObject, 
                            nullptr, 
                            reinterpret_cast<WAITORTIMERCALLBACK>(routine), 
                            param, 
                            duration, 
                            0, 
                            WT_EXECUTELONGFUNCTION))
  {
    return timerObject;
  }

  return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: ReleaseTimer
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void OSInterface::ReleaseTimer(void* pTimer)
{
  if (FALSE == DeleteTimerQueueTimer(nullptr, pTimer, nullptr))
  {
    // TODO: Report error..
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: UDPCreate
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
uintptr_t OSInterface::UDPCreate(StatusTypes::StatusEnum& status,
                                 uint16_t port, 
                                 bool bindPort)
{
  status = StatusTypes::STATUS_OK;

  // Initialize windows socket library (single try)
  static bool winsockInit = false;
  if (false == winsockInit)
  {
    WSADATA wsaInit;
    winsockInit = (0 == WSAStartup(MAKEWORD(2, 2), &wsaInit));
    if (false == winsockInit)
    {
      status = StatusTypes::UDP_INIT_LIB_ERROR;
      return INVALID_SOCKET;
    }
  }

  // Create socket
  uintptr_t udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (INVALID_SOCKET == udpSocket)
  {
    status = StatusTypes::UDP_SCKT_INIT_ERROR;
    return udpSocket;
  }

  // If we are binding on a port, do so now. This specifies the socket
  // is acting as a server.
  if (true == bindPort)
  {
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (SOCKET_ERROR == bind(udpSocket, 
                             reinterpret_cast<sockaddr*>(&server_addr), 
                             sizeof(sockaddr_in)))
    {
      status = StatusTypes::UDP_SCKT_BIND_ERROR;
      closesocket(udpSocket);
      udpSocket = INVALID_SOCKET;
    }
  }

  return udpSocket;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: UDPClose
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
void OSInterface::UDPClose(uintptr_t sock)
{
  closesocket(sock);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: UDPSend
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
StatusTypes::StatusEnum OSInterface::UDPSend(uintptr_t sock,
                                             char* addr,
                                             uint16_t port,
                                             char* buf,
                                             uint32_t len)
{
  if (INVALID_SOCKET == sock)
  {
    return StatusTypes::UDP_INVALID_SCKT;
  }

  sockaddr_in dest_addr;
  int addr_len = sizeof(sockaddr_in);
  memset(&dest_addr, 0, addr_len);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);

  char temp_addr[32] = {0};
  strcpy_s(temp_addr, addr);
  inet_pton(AF_INET, temp_addr, &(dest_addr.sin_addr.S_un.S_addr));

  sockaddr* pDest = reinterpret_cast<sockaddr*>(&dest_addr);
  if (SOCKET_ERROR == sendto(sock, buf, len, 0, pDest, addr_len))
  {
    return StatusTypes::UDP_SCKT_SEND_ERROR;
  }

  return StatusTypes::STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function: UDPRecv
// Notes:    None
//
/////////////////////////////////////////////////////////////////////////////////////////
StatusTypes::StatusEnum OSInterface::UDPRecv(uintptr_t sock,
                                             char* buf,
                                             uint32_t maxLen,
                                             uint32_t timeoutMs,
                                             char* addr,
                                             uint32_t addrMaxLen,
                                             uint16_t* port)
{
  StatusTypes::StatusEnum status = StatusTypes::STATUS_OK;

  // Set up the file descriptor set.
  fd_set fd;
  FD_ZERO(&fd);
  FD_SET(sock, &fd);

  // Set up the timeout.
  timeval timeout;
  uint32_t uSecsPerMs = 1000;
  uint32_t uSecs = timeoutMs * 1000;
  timeout.tv_sec = timeoutMs / 1000;
  timeout.tv_usec = uSecs % uSecsPerMs;

  // Wait until timeout or data received.
  switch (select(sock, &fd, nullptr, nullptr, &timeout))
  {
  case 0: // Timeout or nothing..
    status = StatusTypes::UDP_SCKT_NO_DATA;
    break;
  case -1: // Socket error..
    status = StatusTypes::UDP_SCKT_SELECT_ERR;
    break;
  }

  // If status is valid, we have data pending..
  if (StatusTypes::STATUS_OK == status)
  {
    sockaddr_in sender_addr;
    int addr_len = sizeof(sockaddr_in);
    sockaddr* pAddr = reinterpret_cast<sockaddr*>(&sender_addr);

    int recvLen = recvfrom(sock, buf, maxLen, 0, pAddr, &addr_len);
    if (SOCKET_ERROR == recvLen)
    {
      status = StatusTypes::UDP_SCKT_RECV_ERROR;
    }
    else
    {
      if (nullptr != addr)
      {
        inet_ntop(AF_INET, &(sender_addr.sin_addr), addr, addrMaxLen);
      }

      if (nullptr != port)
      {
        *port = ntohs(sender_addr.sin_port);
      }
    }
  }

  return status;
}