#include <asm-generic/errno.h>
#ifndef platform_h
#include "platform.h"
#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

platform_socket PlatformConnect(platform_ip IP, s32 Port, b32 NonBlocking) {
  platform_socket Result = {};
  int Error = 0;
  Result.SocketHandle = socket(AF_INET, SOCK_STREAM, 0);
  if (Result.SocketHandle == -1) {
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
  if (NonBlocking) {
    PlatformSocketSetNonBlocking(&Result);
  }

  if (connect(Result.SocketHandle, (sockaddr *)&addr, sizeof(addr)) == -1) {
    int ErrorCode = errno;
    if (Result.NonBlocking && ErrorCode == EINPROGRESS) {

    } else {
      perror("connect");
      Error++;
      Failure();
    }
  }
  if (!Error) {
    Result.State = platform_socket_state_valid;
    if (NonBlocking) {
      Result.State = platform_socket_state_connecting;
    }
  } else {
    close(Result.SocketHandle);
    Result.SocketHandle = 0;
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
    Result.State = platform_socket_state_valid;
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
    ListenSocket->State = platform_socket_state_invalid;
  } else {
    Result.SocketHandle = clientfd;
    Result.State = platform_socket_state_valid;
  }
  return Result;
}
void PlatformCloseSocket(platform_socket *Socket) {
  close(Socket->SocketHandle);
  Socket->State = platform_socket_state_closed;
}
void PlatformInitializeNetworkLib() {}
void PlatformFreeNetworkLib() {}
s64 PlatformSend(platform_socket *Socket, u8 *Data, s64 DataLength) {
  s64 BytesSendt = send(Socket->SocketHandle, Data, DataLength, MSG_NOSIGNAL);
  if (BytesSendt < 0) {
    int error = errno;
    if (errno == EPIPE) {
      PlatformCloseSocket(Socket);
    } else {
      PlatformCloseSocket(Socket);
      Failure();
    }
    if (BytesSendt == 0) {
      PlatformCloseSocket(Socket);
    }
  }
  return BytesSendt;
}
s64 PlatformRecv(platform_socket *Socket, u8 *Buffer, s64 BufferSize) {
  s64 Result = 0;
  s64 BytesRecieved = recv(Socket->SocketHandle, Buffer, BufferSize, 0);
  if (BytesRecieved < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      if (!Socket->NonBlocking) {
        Failure();
        PlatformCloseSocket(Socket);
        Socket->State = platform_socket_state_error;
      } else {
      }
    }
  }
  if (BytesRecieved == 0) {
    PlatformCloseSocket(Socket);
    Socket->State = platform_socket_state_closed;
  }
  if (BytesRecieved > 0) {
    if (Socket->State == platform_socket_state_connecting) {
      Socket->State = platform_socket_state_valid;
    }
    Result = BytesRecieved;
  }
  return Result;
}

void PlatformSocketSetNonBlocking(platform_socket *Socket) {
  int flags = fcntl(Socket->SocketHandle, F_GETFL, 0);

  int Success = 1;
  if (flags == -1) {
    perror("fcntl F_GETFL");
    Failure();
    Success = 0;
  }

  if (Success &&
      fcntl(Socket->SocketHandle, F_SETFL, flags | O_NONBLOCK) == -1) {
    perror("fcntl F_SETFL");
    Failure();
    Success = 0;
  }
  int ReadFlags = 0;
  fcntl(Socket->SocketHandle, F_GETFL, ReadFlags | O_NONBLOCK);
  if (!Success) {
    PlatformCloseSocket(Socket);
    Socket->State = platform_socket_state_error;
  }
  Socket->NonBlocking = 1;
}

void PlatformSocketSetBlocking(platform_socket *Socket) {
  int flags = fcntl(Socket->SocketHandle, F_GETFL, 0);

  int Success = 1;
  if (flags == -1) {
    perror("fcntl F_GETFL");
    Failure();
    Success = 0;
  }

  if (fcntl(Socket->SocketHandle, F_SETFL, flags & ~O_NONBLOCK) == -1) {
    perror("fcntl F_SETFL");
    Failure();
    Success = 0;
  }
  if (!Success) {
    PlatformCloseSocket(Socket);
    Socket->State = platform_socket_state_error;
  }
  Socket->NonBlocking = 0;
}
