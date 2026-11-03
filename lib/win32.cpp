// This file is included by the platform.h file when compiling on a Windows
// platform.
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

void PlatformSocketSetNonBlocking(platform_socket* Socket) {

    Socket->NonBlocking = 1;
    u_long NoBlock = 1;
    if (ioctlsocket(Socket->SocketHandle, FIONBIO, &NoBlock) == SOCKET_ERROR)
    {
        s32 ErrorCode = WSAGetLastError();
        Failure();
    }
}
void PlatformSocketSetBlocking(platform_socket* Socket) {
    Socket->NonBlocking = 0;
    u_long NoBlock = 0;
    if (ioctlsocket(Socket->SocketHandle, FIONBIO, &NoBlock) == SOCKET_ERROR)
    {
        s32 ErrorCode = WSAGetLastError();
        Failure();
    }
}

platform_socket PlatformConnect(platform_ip IP, s32 Port, b32 NonBlocking) {
  platform_socket Result = {};
  Result.State = platform_socket_state_valid;
  Result.SocketHandle = (s64)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (Result.SocketHandle == INVALID_SOCKET) {
    s32 ErrorCode = WSAGetLastError();
    // Failed
    Failure();
    Result.SocketHandle = 0;
    Result.State = platform_socket_state_invalid;
  }
  sockaddr_in server{};
  server.sin_family = AF_INET;
  server.sin_port = htons(Port);
  inet_pton(AF_INET, IP.IPAddress, &server.sin_addr);
  if (NonBlocking) {

  PlatformSocketSetNonBlocking(&Result);
  }
  s32 Failed =
      connect(Result.SocketHandle, (sockaddr *)&server, sizeof(server));
  if (Failed) {
    s32 ErrorCode = WSAGetLastError();
    if (ErrorCode == WSAEWOULDBLOCK) {
        Result.State = platform_socket_state_connecting;

    }
    else {
        closesocket(Result.SocketHandle);
        Failure();
        Result.SocketHandle = 0;
        Result.State = platform_socket_state_invalid;
    }
  }
  return Result;
}

platform_socket PlatformBindAddressAndListen(platform_ip IP, s32 Port) {
  platform_socket Result = {};
  Result.State = platform_socket_state_valid;

  Result.SocketHandle = (s64)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (Result.SocketHandle == INVALID_SOCKET) {
    s32 ErrorCode = WSAGetLastError();
    // Failed
    Failure();
    Result.SocketHandle = 0;
    Result.State = platform_socket_state_invalid;
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
    Result.State = platform_socket_state_invalid;
  }
  Failed = listen(Result.SocketHandle, SOMAXCONN);
  if (Failed) {
    s32 ErrorCode = WSAGetLastError();
    closesocket(Result.SocketHandle);
    Failure();
    Result.SocketHandle = 0;
    Result.State = platform_socket_state_invalid;
  }
  return Result;
}
platform_socket PlatformAccept(platform_socket *ListenSocket) {
  platform_socket Result = {};
  Result.State = platform_socket_state_valid;
  sockaddr ConnectingAddress = {};
  s32 ConnectingAddressSize = sizeof(ConnectingAddress);
  Result.SocketHandle = accept(ListenSocket->SocketHandle, &ConnectingAddress,
                               &ConnectingAddressSize);

  if (Result.SocketHandle == INVALID_SOCKET) {
    s32 ErrorCode = WSAGetLastError();
    closesocket(Result.SocketHandle);
    Failure();
    Result.SocketHandle = 0;
    Result.State = platform_socket_state_invalid;
  }
  return Result;
}

void PlatformCloseSocket(platform_socket *Socket) {
  s32 Result = closesocket((SOCKET)Socket->SocketHandle);
  Socket->State = platform_socket_state_closed;
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
  if (Socket->State == platform_socket_state_valid) {
    Result = send(Socket->SocketHandle, (char *)Data, DataLength, 0);
    if (Result == SOCKET_ERROR) {
      s32 ErrorCode = WSAGetLastError();
      Failure();
      PlatformCloseSocket(Socket);
    }
  }
  return Result;
}

s64 PlatformRecv(platform_socket *Socket, u8 *Buffer, s64 BufferSize) {
  s64 Result = {};
  if (Socket->State == platform_socket_state_valid || Socket->State == platform_socket_state_connecting) {
    Result = recv(Socket->SocketHandle, (char *)Buffer, BufferSize, 0);
    if (Result == SOCKET_ERROR) {
      s32 ErrorCode = WSAGetLastError();
      if (ErrorCode == WSAEWOULDBLOCK) {
          if (!Socket->NonBlocking) {
              Failure();
          }
        }
      else {
          Failure();
          PlatformCloseSocket(Socket);
          Socket->State = platform_socket_state_invalid;
      }
    }
    if (Result == 0) {
        PlatformCloseSocket(Socket);
        Socket->State = platform_socket_state_closed;
    }

    if (Result > 0 && Socket->State == platform_socket_state_connecting) {
        Socket->State = platform_socket_state_valid;
    }
  }

  return Result;
}



void PlatformSleepMS(s32 MilliSecounds) { Sleep(MilliSecounds); }
