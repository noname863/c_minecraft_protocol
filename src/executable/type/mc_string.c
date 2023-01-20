#include <executable/type/mc_string.h>
#include <executable/type/var_numbers.h>

mc_string parseString(char** buffer)
{
    int length = parseVarInt(buffer);
    mc_string result = createBuffer(length);
    // add actual utf-8 parsing
    appendToBuffer(&result, *buffer, length);
    // zero-terminate string, lol
    appendEmptyToBuffer(&result, 1);
    result.buffer[result.size - 1] = 0;

    *buffer += length;
    return result;
}

void writeString(Buffer* buffer, mc_string* string)
{
    // TODO: some day write separate class for string, which will only
    // differ from buffer that size is normal
    writeVarInt(buffer, (int)(string->size - 1));
    appendToBuffer(buffer, string->buffer, string->size);
}
