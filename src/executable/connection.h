#ifndef CONNECTION_H
#define CONNECTION_H
#include <executable/utils/buffer.h>
#include <stdint.h>
#include <string.h>
#include <poll.h>

typedef int socketType;

enum ConnectionState
{
    Handshake = 0,
    Status = 1,
    Login = 2,
    Play = 3
};

typedef struct
{
    Buffer inputBuffer;
    Buffer outputBuffer;
    socketType socket;
    enum ConnectionState state;
} SocketBuffer;

DECLARE_BUFFER(SocketBuffers, SocketBuffer)
DECLARE_BUFFER(PollInfoBuffer, struct pollfd);

typedef struct
{
    SocketBuffers sockets;
    PollInfoBuffer pollBuffer;
    socketType listenSocket;
} ClientBufferedSockets;

ClientBufferedSockets openSocketListener();
void listenSockets(ClientBufferedSockets* sockets);
void updateBuffersAndConnections(ClientBufferedSockets* sockets);

#endif
