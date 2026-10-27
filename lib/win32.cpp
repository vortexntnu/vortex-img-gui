// This file is included by the platform.h file when compiling on a Windows
// platform.
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

platform_socket PlatformConnect(platform_ip IP, s32 Port) {
  platform_socket Result = {};
  Result.IsValid = 1;
  Result.SocketHandle = (s64)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (Result.SocketHandle == INVALID_SOCKET) {
    s32 ErrorCode = WSAGetLastError();
    // Failed
    Failure();
    Result.SocketHandle = 0;
    Result.IsValid = 0;
  }
  sockaddr_in server{};
  server.sin_family = AF_INET;
  server.sin_port = htons(Port);
  inet_pton(AF_INET, IP.IPAddress, &server.sin_addr);
  s32 Failed =
      connect(Result.SocketHandle, (sockaddr *)&server, sizeof(server));
  if (Failed) {
    s32 ErrorCode = WSAGetLastError();
    closesocket(Result.SocketHandle);
    Failure();
    Result.SocketHandle = 0;
    Result.IsValid = 0;
  }
  return Result;
}

platform_socket PlatformBindAddressAndListen(platform_ip IP, s32 Port) {
  platform_socket Result = {};
  Result.IsValid = 1;

  Result.SocketHandle = (s64)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (Result.SocketHandle == INVALID_SOCKET) {
    s32 ErrorCode = WSAGetLastError();
    // Failed
    Failure();
    Result.SocketHandle = 0;
    Result.IsValid = 0;
  }
  sockaddr_in Address{};
  Address.sin_family = AF_INET;
  Address.sin_port = htons(Port);
  inet_pton(AF_INET, IP.IPAddress, &Address.sin_addr);
  s32 Failed = bind(Result.SocketHandle, (sockaddr *)&Address, sizeof(Address));
  if (Failed) {
    s32 ErrorCode = WSAGetLastError();
    closesocket(Result.SocketHandle);
    Failure();
    Result.SocketHandle = 0;
    Result.IsValid = 0;
  }
  Failed = listen(Result.SocketHandle, SOMAXCONN);
  if (Failed) {
    s32 ErrorCode = WSAGetLastError();
    closesocket(Result.SocketHandle);
    Failure();
    Result.SocketHandle = 0;
    Result.IsValid = 0;
  }
  return Result;
}
platform_socket PlatformAccept(platform_socket *ListenSocket) {
  platform_socket Result = {};
  Result.IsValid = 1;
  sockaddr ConnectingAddress = {};
  s32 ConnectingAddressSize = sizeof(ConnectingAddress);
  Result.SocketHandle = accept(ListenSocket->SocketHandle, &ConnectingAddress,
                               &ConnectingAddressSize);

  if (Result.SocketHandle == INVALID_SOCKET) {
    s32 ErrorCode = WSAGetLastError();
    closesocket(Result.SocketHandle);
    Failure();
    Result.SocketHandle = 0;
    Result.IsValid = 0;
  }
  return Result;
}

void PlatformCloseSocket(platform_socket *Socket) {
  s32 Result = closesocket((SOCKET)Socket->SocketHandle);
  Socket->IsValid = 0;
  Assert(!Result);
}

void PlatformInitializeNetworkLib() {
  WSADATA wsaData = {0};
  s32 Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
  Assert(wsaData.wVersion == 0x0202);
  if (Result != 0) {
    s32 ErrorCode = WSAGetLastError();
    Failure();
  }
}

void PlatformFreeNetworkLib() { WSACleanup(); }

s64 PlatformSend(platform_socket *Socket, u8 *Data, s64 DataLength) {
  s32 Result = {};
  if (Socket->IsValid) {
    Result = send(Socket->SocketHandle, (char *)Data, DataLength, 0);
    if (Result == SOCKET_ERROR) {
      s32 ErrorCode = WSAGetLastError();
      Failure();
      PlatformCloseSocket(Socket);
      Socket->IsValid = 0;
    }
  }
  return Result;
}

s64 PlatformRecv(platform_socket *Socket, u8 *Buffer, s64 BufferSize) {
  s32 Result = {};
  if (Socket->IsValid) {
    Result = recv(Socket->SocketHandle, (char *)Buffer, BufferSize, 0);
    if (Result == SOCKET_ERROR) {
      s32 ErrorCode = WSAGetLastError();
      Failure();
      PlatformCloseSocket(Socket);
      Socket->IsValid = 0;
    }
  }
  return Result;
}

void PlatformSleepMS(s32 MilliSecounds) { Sleep(MilliSecounds); }
