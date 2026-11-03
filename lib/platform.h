#ifndef platform_h
#define platform_h
#include "typedef.h"

struct platform_ip {
  char *IPAddress;
};

enum {
	platform_socket_state_invalid,
	platform_socket_state_connecting,
	platform_socket_state_valid,
	platform_socket_state_closed,
};

struct platform_socket {
  s32 State;
  b32 NonBlocking;
  s64 SocketHandle;
};
platform_ip PlatformIP(char *IPAddress);

platform_socket PlatformConnect(platform_ip IP, s32 Port, b32 NonBlocking);
platform_socket PlatformBindAddressAndListen(platform_ip IP, s32 Port);
platform_socket PlatformAccept(platform_socket *ListenSocket);
void PlatformCloseSocket(platform_socket *Socket);
void PlatformInitializeNetworkLib();
void PlatformFreeNetworkLib();
s64 PlatformSend(platform_socket *Socket, u8 *Data, s64 DataLength);
s64 PlatformRecv(platform_socket *Socket, u8 *Buffer, s64 BufferSize);
void PlatformSocketSetNonBlocking(platform_socket* Socket);
void PlatformSocketSetBlocking(platform_socket* Socket);

void Failure() { int a = 0; }

platform_ip PlatformIP(char *IPAddress) {
  platform_ip Result = {};
  Result.IPAddress = IPAddress;

  return Result;
}

#ifdef PLATFORM_LINUX
#include "linux.cpp"
#else
#include "win32.cpp"
#endif
#endif
