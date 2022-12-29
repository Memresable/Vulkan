#ifndef G_UTILITIY_H
#define G_UTILITIY_H

#include <windows.h>
#include <stdlib.h>

#define ARRAY_SIZE(X) sizeof(X)/sizeof(X[0])
#define STRING_STRUCT_SIZE(X) sizeof(X)/8

#define MEMRE_ASSERT(X, err_string) \
if(X) \
{ \
OutputDebugStringA(err_string); \
exit(EXIT_FAILURE); \
}

#endif