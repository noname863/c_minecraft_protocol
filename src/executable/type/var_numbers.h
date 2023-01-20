#ifndef VAR_NUMBERS_H
#define VAR_NUMBERS_H

#include <executable/socket_utils.h>
#include <executable/utils/buffer.h>
#include <stdint.h>

int32_t receiveVarInt(socketType sock);
int64_t receiveVarLong(socketType sock);

void sendVarInt(socketType sock, int32_t value);
void sendVarLong(socketType sock, int64_t value);

int32_t parseVarInt(char** source);
int64_t parseVarLong(char** source);

void writeVarInt(Buffer* writeBuffer, int32_t value);
void writeVarLong(Buffer* writeBuffer, int64_t value);

#endif
