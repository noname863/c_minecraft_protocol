#include <executable/connection.h>

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

// Utils functions start
void acceptConnections(ClientBufferedSockets *sockets)
{
    socketType minecraftSock = accept(sockets->listenSocket, NULL, NULL);

    if (minecraftSock == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            printf("error on accept %d\n", errno);
        }
    }
    else
    {
        puts("new connection made");
        appendEmptyToSocketBuffers(&sockets->sockets, 1);
        SocketBuffer* newSocket = sockets->sockets.buffer + sockets->sockets.size - 1;
        newSocket->inputBuffer = createBuffer(1500);
        newSocket->outputBuffer = createBuffer(1500);
        newSocket->socket = minecraftSock;
        newSocket->state = Handshake;
        appendEmptyToPollInfoBuffer(&sockets->pollBuffer, 1);
        struct pollfd* pollData = sockets->pollBuffer.buffer + sockets->pollBuffer.size - 1;
        pollData->fd = minecraftSock;
        pollData->events = POLLIN | POLLOUT;
    }
}

void closeClientSocket(ClientBufferedSockets* sockets, size_t socketIndex)
{
    SocketBuffer* socket = sockets->sockets.buffer + socketIndex;
    struct pollfd* pollData = sockets->pollBuffer.buffer + socketIndex;
    close(socket->socket);
    freeBuffer(&socket->inputBuffer);
    freeBuffer(&socket->outputBuffer);
    memmove(socket, socket + 1, sockets->sockets.size - socketIndex - 1);
    --sockets->sockets.size;
    memmove(pollData, pollData + 1, sockets->pollBuffer.size - socketIndex - 1);
    --sockets->pollBuffer.size;
}

bool attemptToRead(ClientBufferedSockets* sockets, size_t socketIndex)
{
    SocketBuffer* socket = sockets->sockets.buffer + socketIndex;
    Buffer* inputBuffer = &socket->inputBuffer;
    if (inputBuffer->size == inputBuffer->allocatedSize)
    {
        inputBuffer->allocatedSize *= 2;
        inputBuffer->buffer = realloc(inputBuffer->buffer, inputBuffer->allocatedSize);
    }
    puts("reading message");
    int result = recv(socket->socket, inputBuffer->buffer + inputBuffer->size, inputBuffer->allocatedSize - inputBuffer->size, 0);
    if (result == 0)
    {
        // POLLIN was on, but no bytes was read.
        // puts("POLLIN was on, but no bytes was read");
        return false;
    }
    else if (result == -1)
    {
        printf("error %d when receiving message\n", errno);
        // actually not sure what to return here.
        return false;
    }
    else
    {
        inputBuffer->size += result;
        return true;
    }
}

bool attemptToWrite(ClientBufferedSockets* sockets, size_t socketIndex)
{
    SocketBuffer* socket = sockets->sockets.buffer + socketIndex;
    if (socket->outputBuffer.size == 0)
    {
        return true;
    }
    Buffer* outputBuffer = &socket->outputBuffer;
    puts("sending message");
    int result = send(socket->socket, outputBuffer->buffer, outputBuffer->size, 0);
    if (result == 0)
    {
        // POLLOUT was on, but no bytes was read.
        // puts("POLLOUT was on, but no bytes sended");
        return false;
    }
    else if (result == -1)
    {
        printf("error %d when sending message\n", errno);
        return false;
    }
    else 
    {
        memmove(outputBuffer->buffer, outputBuffer->buffer + result, outputBuffer->size - result);
        outputBuffer->size -= result;
        return true;
    }
}
// Utils functions end

IMPLEMENT_BUFFER_FUNCTIONS(SocketBuffers, SocketBuffer)
IMPLEMENT_BUFFER_FUNCTIONS(PollInfoBuffer, struct pollfd)

ClientBufferedSockets openSocketListener()
{
    ClientBufferedSockets result;
    result.listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(result.listenSocket, F_SETFL, O_NONBLOCK);

    result.sockets = createSocketBuffers(16);
    result.pollBuffer = createPollInfoBuffer(16);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(25565);
    addr.sin_family = AF_INET;

    puts("binding socket");
    if (bind(result.listenSocket, (const struct sockaddr*)&addr, sizeof(addr)))
    {
        printf("bind error %d\n", errno);
        close(result.listenSocket);
        exit(-1);
    }
    return result;
}


void listenSockets(ClientBufferedSockets *sockets)
{
    puts("start listening socket");
    if (listen(sockets->listenSocket, 16))
    {
        printf("listen error %d\n", errno);
        close(sockets->listenSocket);
        exit(-2);
    }
}


void updateBuffersAndConnections(ClientBufferedSockets* sockets)
{
    acceptConnections(sockets);
    int pollResult = poll(sockets->pollBuffer.buffer, sockets->pollBuffer.size, 0);
    if (pollResult)
    {
        assert(sockets->pollBuffer.size == sockets->sockets.size);
        for (size_t socketIndex = 0; socketIndex < sockets->pollBuffer.size; ++socketIndex)
        {
            struct pollfd* pollData = sockets->pollBuffer.buffer + socketIndex;
            if (!pollData->revents)
            {
                continue;
            }
            SocketBuffer* socket = sockets->sockets.buffer + socketIndex;
            if (pollData->revents & POLLIN)
            {
                if (!attemptToRead(sockets, socketIndex))
                {
                    // false returned when connection was closed. there is now new socket on same index
                    puts("disconnecting after attempt to read");
                    closeClientSocket(sockets, socketIndex);
                    --socketIndex;
                    continue;
                }
            }
            if (pollData->revents & POLLOUT)
            {
                if (!attemptToWrite(sockets, socketIndex))
                {
                    puts("disconnecting after attempt to write");
                    closeClientSocket(sockets, socketIndex);
                    --socketIndex;
                    continue;
                }
            }
        }
    }
}
