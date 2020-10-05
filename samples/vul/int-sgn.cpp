#include <cstring>
static char data[256];
void*       store_data(char* buf, int len)
{
    if (len > 256) return (void*)-1;
    return memcpy(data, buf, len);
}
