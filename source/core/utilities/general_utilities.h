#ifndef G_UTILITIY_H
#define G_UTILITIY_H

#include <windows.h>

#include "../utilities/types.h"

#define MEMRE_FALSE 0
#define MEMRE_TRUE 1

#define ARRAY_SIZE(X) sizeof(X)/sizeof(X[0])

#define ALLOCATE_MEMORY(type, count) \
(type*)VirtualAlloc(0, sizeof(type)*count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)

#define RELEASE_MEMORY(variableToFree) \
VirtualFree(variableToFree, 0, MEM_RELEASE)

#define MEMRE_ASSERT(X, err_string) \
if(X) \
{ \
OutputDebugStringA(err_string); \
exit(EXIT_FAILURE); \
}

inline int
compareTwoStrings(string_t f_string1, string_t f_string2)
{
    for(uint32_t i = 0; f_string1[i] != '\0' && f_string2[i] != '\0'; i++)
    {
        if(f_string1[i] != f_string2[i])
        {
            return(MEMRE_FALSE);
        }
    }
    return(MEMRE_TRUE);
}

typedef struct
{
    HWND* handle;
    uint32_t* width;
    uint32_t* height;
} WindowInfo;

#endif