#include <malloc.h>
int nil()
{
    int* p = NULL;
    *p     = 1;
    return *p;
}