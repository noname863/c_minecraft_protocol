#ifndef UTIL_TYPES_H
#define UTIL_TYPES_H
#include <stdint.h>

typedef struct
{
    uint64_t hi;
    uint64_t lo;
} UUID;

typedef struct
{
    int64_t x : 26;
    int64_t z : 26;
    int64_t y : 12;
} EntityPosition;

_Static_assert(sizeof(EntityPosition) == 8, "Position is greater than long");

#endif
