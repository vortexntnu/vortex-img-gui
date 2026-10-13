#ifndef platform_h
#define platform_h

#include "typedef.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

struct platform_ip {
    char* IPAddress;
};

struct platform_socket {
    b32 IsValid;
    s64 SocketHandle;
};
platform_ip PlatformIP(char* IPAddress);


platform_socket PlatformConnect(platform_ip IP, s32 Port);
platform_socket PlatformBindAddressAndListen(platform_ip IP, s32 Port);
platform_socket PlatformAccept(platform_socket* ListenSocket);
void PlatformCloseSocket(platform_socket* Socket);
void PlatformInitializeNetworkLib();
void PlatformFreeNetworkLib();
s64 PlatformSend(platform_socket* Socket, u8* Data, s64 DataLength);
s64 PlatformRecv(platform_socket* Socket, u8* Buffer, s64 BufferSize);

#include"platform.cpp"
#endif