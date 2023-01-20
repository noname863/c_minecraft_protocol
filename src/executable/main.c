#include <executable/socket_utils.h>
#include <executable/type/var_numbers.h>
#include <executable/utils/buffer.h>
#include <executable/connection.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

typedef struct
{
    int32_t protocolVersion;
    int32_t nextState;
} MinecraftConnectionInfo;

MinecraftConnectionInfo parseHandshake(char** buffer)
{
    MinecraftConnectionInfo connectionInfo; 
    int32_t packetLength = parseVarInt(buffer);
    int32_t packetId = parseVarInt(buffer);
    connectionInfo.protocolVersion = parseVarInt(buffer);
    int32_t serverAddressLength = parseVarInt(buffer);
    buffer += serverAddressLength;
    // serverAddress is unused currently, so skip
    // char* serverAddress = malloc(serverAddressLength + 1);
    // memcpy(serverAddress, buffer, serverAddressLength);
    // serverAddress[serverAddressLength] = '\n';
    // buffer += serverAddressLength;
    // free(serverAddress);
    uint16_t port = ntohs(*(short*)buffer);
    buffer += 2;
    connectionInfo.nextState = parseVarInt(buffer);
    return connectionInfo;
}

void writeStatus(Buffer* buffer)
{
    const char* json = 
    "{"
        "\"version\":{"
            "\"name\":\"1.12.2\","
            "\"protocol\":340"
        "},"
        "\"players\":{"
            "\"max\":20,"
            "\"online\":0,"
            "\"sample\":[]"
        "},"
        "\"description\":{"
            "\"text\":\"AAAAAAAAAAAA\""
        "},"
        "\"enforcesSecureChat\":true"
    "}";
    unsigned long sizeOfJson = strlen(json);
    writeVarInt(buffer, sizeOfJson + 3);
    writeVarInt(buffer, 0);
    writeVarInt(buffer, sizeOfJson);
    appendToBuffer(buffer, json, sizeOfJson);
}

ClientBufferedSockets sockets;

int main()
{
    sockets = openSocketListener();
    listenSockets(&sockets);
    while (true)
    {
        updateBuffersAndConnections(&sockets);

        SocketBuffer* const socketsEnd = sockets.sockets.buffer + sockets.sockets.size;
        for (SocketBuffer* socket = sockets.sockets.buffer; socket != socketsEnd; ++socket)
        {
            if (socket->inputBuffer.size == 0)
            {
                continue;
            }
            char* inputBuffer = socket->inputBuffer.buffer;
            int32_t packetLength = parseVarInt(&inputBuffer);
            size_t packetLengthLength = inputBuffer - socket->inputBuffer.buffer;
            if (packetLength > socket->inputBuffer.size)
            {
                // if whole packet isn't received, skip it
                continue;
            }
            int32_t packetId = parseVarInt(&inputBuffer);
            if (socket->state == Handshake)
            {
                if (packetId != 0)
                {
                    puts("wrong packet id while handshaking");
                    puts("skipping packet");
                    memmove(socket->inputBuffer.buffer, socket->inputBuffer.buffer + packetLength, socket->inputBuffer.size - packetLength);
                    continue;
                }
                puts("parsing handshake packet");
                MinecraftConnectionInfo info; 
                info.protocolVersion = parseVarInt(&inputBuffer);
                // TODO: convert length in symbols to length in bytes
                int32_t serverAddressLength = parseVarInt(&inputBuffer);
                inputBuffer += serverAddressLength;
                uint16_t port = ntohs(*(short*)inputBuffer);
                inputBuffer += 2;
                info.nextState = parseVarInt(&inputBuffer);
                // TODO: do something with wrong states
                if (info.nextState != 1)
                {
                    printf("tried to connect to server with next state %d!\n", info.nextState);
                    continue;
                }
                if (info.protocolVersion != 340)
                {
                    puts("wrong protocol version");
                    continue;
                }
                socket->state = info.nextState;
            }
            else if (socket->state == Status)
            {
                if (packetId > 1)
                {
                    puts("wrong packet id in status state");
                    puts("skipping packet");
                    memmove(socket->inputBuffer.buffer,
                        socket->inputBuffer.buffer + packetLength + packetLengthLength,
                        socket->inputBuffer.size - packetLength - packetLengthLength);
                    socket->inputBuffer.size -= (packetLength + packetLengthLength);
                    continue;
                }
                else if (packetId == 0)
                {
                    puts("writing status packet");
                    writeStatus(&socket->outputBuffer);
                }
                else if (packetId == 1)
                {
                    puts("writing pong packet");
                    writeVarInt(&socket->outputBuffer, 9);
                    writeVarInt(&socket->outputBuffer, 1);
                    appendToBuffer(&socket->outputBuffer, inputBuffer, 8);
                    inputBuffer += 8;
                }
            }
            else
            {
                puts("socket with unsupported state found");
            }
            if (socket->inputBuffer.buffer + packetLength + packetLengthLength != inputBuffer)
            {
                puts("packet length is different from number of bytes readed");
            }
            memmove(socket->inputBuffer.buffer,
                socket->inputBuffer.buffer + packetLength + packetLengthLength,
                socket->inputBuffer.size - packetLength - packetLengthLength);
            socket->inputBuffer.size -= (packetLength + packetLengthLength);
        }
    }
    freeSocketBuffers(&sockets.sockets);
    freePollInfoBuffer(&sockets.pollBuffer);
}
