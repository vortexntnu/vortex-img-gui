#ifndef platform_h
#include "platform.h"
#endif
platform_socket PlatformConnect(platform_ip IP, s32 Port) {
  platform_socket Result = {};
  return Result;
}
platform_socket PlatformBindAddressAndListen(platform_ip IP, s32 Port) {
  platform_socket Result = {};
  return Result;
}
platform_socket PlatformAccept(platform_socket *ListenSocket) {
  platform_socket Result = {};
  return Result;
}
void PlatformCloseSocket(platform_socket *Socket) {}
void PlatformInitializeNetworkLib() {}
void PlatformFreeNetworkLib() {}
s64 PlatformSend(platform_socket *Socket, u8 *Data, s64 DataLength) {
  return 0;
}
s64 PlatformRecv(platform_socket *Socket, u8 *Buffer, s64 BufferSize) {
  return 0;
}
