#include "executable/socket_utils.h"
#include <executable/type/var_numbers.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>

int32_t receiveVarInt(socketType sock)
{
    uint32_t res = 0;
    uint32_t shift = 0;
    uint8_t byte;
    do
    {
        recv(sock, &byte, 1, MSG_WAITALL);
        res |= (uint32_t)(byte & 0b01111111) << shift;
        shift += 7;
    }
    while (byte & 0b10000000);
    return *(int32_t*)&res;
}

int64_t receiveVarLong(socketType sock)
{
    uint64_t res = 0;
    uint64_t shift = 0;
    uint8_t byte;
    do
    {
        recv(sock, &byte, 1, MSG_WAITALL);
        res |= (uint64_t)(byte & 0b01111111) << shift;
        shift += 7;
    }
    while (byte & 0b10000000);
    return *(int64_t*)&res;
}

void sendVarInt(socketType sock, int32_t value)
{
    if (__builtin_clz(value) >= 25)
    {
        send(sock, &value, 1, 0);
    }
    uint32_t* valuePtr = (uint32_t*)&value;
    char msg[5];
    int msgIndex = 0;
    while(*valuePtr)
    {
        msg[msgIndex++] = (*valuePtr & 0b01111111) | 0b10000000;
        *valuePtr = *valuePtr >> 7;
    }
    msg[msgIndex - 1] &= 0b01111111;
    send(sock, msg, msgIndex, 0);
}

void sendVarLong(socketType sock, int64_t value)
{
    if (__builtin_clzl(value) >= 57)
    {
        send(sock, &value, 1, 0);
    }
    uint64_t* valuePtr = (uint64_t*)&value;
    char msg[10];
    int msgIndex = 0;
    while(*valuePtr)
    {
        msg[msgIndex++] = (*valuePtr & 0b01111111) | 0b10000000;
        *valuePtr = *valuePtr >> 7;
    }
    msg[msgIndex - 1] &= 0b01111111;
    send(sock, msg, msgIndex, 0);
}

int32_t parseVarInt(char** source)
{
    uint32_t res = 0;
    int iter = 0;
    uint8_t byte;
    do
    {
        byte = *((*source)++);
        res |= (uint32_t)(byte & 0b01111111) << iter;
        iter += 7;
    }
    while (byte & 0b10000000);
    return *(int32_t*)&res;
}

int64_t parseVarLong(char** source)
{
    uint64_t res = 0;
    int iter = 0;
    uint8_t byte;
    do
    {
        byte = *((*source)++);
        res |= (uint64_t)(byte & 0b01111111) << iter;
        iter += 7;
    }
    while (byte & 0b10000000);
    return *(int64_t*)&res;
}

void writeVarInt(Buffer* writeBuffer, int32_t value)
{
    if (__builtin_clz(value) >= 25)
    {
        appendToBuffer(writeBuffer, (char*)&value, 1);
        return;
    }
    uint32_t* valuePtr = (uint32_t*)&value;
    uint8_t byte;
    while(*valuePtr)
    {
        byte = (*valuePtr & 0b01111111) | 0b10000000;
        appendToBuffer(writeBuffer, (char*)&byte, 1);
        *valuePtr = *valuePtr >> 7;
    }
    writeBuffer->buffer[writeBuffer->size - 1] &= 0b01111111;
}

void writeVarLong(Buffer* writeBuffer, int64_t value)
{
    if (__builtin_clzl(value) >= 57)
    {
        appendToBuffer(writeBuffer, (char*)&value, 1);
        return;
    }
    uint64_t* valuePtr = (uint64_t*)&value;
    uint8_t byte;
    while(*valuePtr)
    {
        byte = (*valuePtr & 0b01111111) | 0b10000000;
        appendToBuffer(writeBuffer, (char*)&byte, 1);
        *valuePtr = *valuePtr >> 7;
    }
    writeBuffer->buffer[writeBuffer->size - 1] &= 0b01111111;
}
