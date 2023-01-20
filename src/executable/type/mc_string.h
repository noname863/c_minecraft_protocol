#ifndef MC_STRING_H
#define MC_STRING_H
#include <executable/utils/buffer.h>

typedef Buffer mc_string;

mc_string parseString(char** buffer);
void writeString(Buffer* buffer, mc_string* string);

#endif
