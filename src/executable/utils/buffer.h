#ifndef BUFFER_H
#define BUFFER_H
#include <stddef.h>
#include <stdlib.h>

#define DECLARE_BUFFER(ClassName, ItemName) \
    typedef struct \
    { \
        ItemName* buffer;\
        size_t allocatedSize;\
        size_t size;\
    } ClassName; \
    void appendTo ## ClassName(ClassName* restrict buffer, const ItemName* restrict data, size_t size);\
    ClassName create ## ClassName(size_t allocatedSize);\
    void free ## ClassName(ClassName* buffer);\
    void appendEmptyTo ## ClassName(ClassName* buffer, size_t size);

#define IMPLEMENT_BUFFER_FUNCTIONS(ClassName, ItemName) \
void appendEmptyTo ## ClassName(ClassName* buffer, size_t size) \
{ \
    if (buffer->size + size > buffer->allocatedSize) \
    { \
        size_t newSize = buffer->size + size < buffer->allocatedSize * 2 ? buffer->allocatedSize * 2 : buffer->size + size; \
        buffer->buffer = realloc(buffer->buffer, newSize * sizeof(ItemName)); \
        buffer->allocatedSize = newSize; \
    } \
    buffer->size = buffer->size + size; \
} \
void appendTo ## ClassName(ClassName* restrict buffer, const ItemName* restrict data, size_t size) \
{ \
    size_t oldSize = buffer->size; \
    appendEmptyTo ## ClassName(buffer, size); \
    memcpy(buffer->buffer + oldSize, data, size * sizeof(ItemName));\
} \
ClassName create ## ClassName(size_t allocatedSize) \
{ \
    ClassName res; \
    res.buffer = malloc(allocatedSize * sizeof(ItemName)); \
    res.allocatedSize = allocatedSize; \
    res.size = 0; \
    return res; \
}\
\
void free ## ClassName(ClassName* buffer) \
{ \
    free(buffer->buffer); \
}

DECLARE_BUFFER(Buffer, char);

// typedef struct
// {
    // char* buffer;
    // size_t allocatedSize;
    // size_t size;
// } Buffer;

// void appendToBuffer(Buffer* restrict buffer, const char* restrict data, size_t size);
// Buffer createBuffer(size_t allocatedSize);

#endif