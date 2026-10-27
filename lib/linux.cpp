#ifndef platform_h
#include "platform.h"
#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cerrno>
#include <cstring>
#include <stdio.h>
#include <unistd.h>

platform_socket PlatformConnect(platform_ip IP, s32 Port) {
  platform_socket Result = {};
  int Error = 0;
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("socket");
    Error++;
    Failure();
  }
  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(Port);
  if (inet_pton(AF_INET, IP.IPAddress, &addr.sin_addr) != 1) {
    perror("inet_pton");
    Error++;
    Failure();
  }

  if (connect(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("connect");
    Error++;
    Failure();
  }
  if (!Error) {
    Result.SocketHandle = sockfd;
    Result.IsValid = 1;
  } else {
    close(sockfd);
  }
  return Result;
}
platform_socket PlatformBindAddressAndListen(platform_ip IP, s32 Port) {
  platform_socket Result = {};
  int Error = 0;
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("socket");
    Error++;
    Failure();
  }
  int opt = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    perror("setsockopt");
    close(sockfd);
    Error++;
    Failure();
  }

  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(Port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (inet_pton(AF_INET, IP.IPAddress, &addr.sin_addr) != 1) {
    perror("inet_pton");
    close(sockfd);
    Error++;
    Failure();
  }
  if (bind(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("bind");
    int ErrorCode = errno;
    char *ErrorStr = std::strerror(ErrorCode);
    Error++;
    Failure();
  }
  if (listen(sockfd, SOMAXCONN) == -1) {
    perror("listen");
    Error++;
    Failure();
  }

  if (!Error) {
    Result.IsValid = 1;
    Result.SocketHandle = sockfd;
  } else {
    close(sockfd);
  }

  return Result;
}
platform_socket PlatformAccept(platform_socket *ListenSocket) {
  platform_socket Result = {};
  int Error = 0;
  int clientfd = accept(ListenSocket->SocketHandle, NULL, NULL);
  if (clientfd == -1) {
    perror("accept");
    Error++;
    Failure();
  }
  if (Error) {
    close(ListenSocket->SocketHandle);
    ListenSocket->IsValid = 0;
  } else {
    Result.SocketHandle = clientfd;
    Result.IsValid = 1;
  }
  return Result;
}
void PlatformCloseSocket(platform_socket *Socket) {
  close(Socket->SocketHandle);
  Socket->IsValid = 0;
}
void PlatformInitializeNetworkLib() {}
void PlatformFreeNetworkLib() {}
s64 PlatformSend(platform_socket *Socket, u8 *Data, s64 DataLength) {
  s64 BytesSendt = send(Socket->SocketHandle, Data, DataLength, 0);
  if (BytesSendt <= 0) {
    PlatformCloseSocket(Socket);
    if (BytesSendt < 0) {
      Failure();
    }
  }
  return BytesSendt;
}
s64 PlatformRecv(platform_socket *Socket, u8 *Buffer, s64 BufferSize) {
  s64 BytesRecieved = recv(Socket->SocketHandle, Buffer, BufferSize, 0);
  if (BytesRecieved <= 0) {
    PlatformCloseSocket(Socket);
    if (BytesRecieved < 0) {
      Failure();
    }
  }
  return 0;
}
