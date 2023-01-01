#ifndef G_UTILITIY_H
#define G_UTILITIY_H

#include <windows.h>
#include <stdlib.h>

#include "../utilities/types.h"

#define ARRAY_SIZE(X) sizeof(X)/sizeof(X[0])
#define STRING_STRUCT_SIZE(X) sizeof(X)/8

#define MEMRE_ASSERT(X, err_string) \
if(X) \
{ \
OutputDebugStringA(err_string); \
exit(EXIT_FAILURE); \
}

bool
compareTwoStrings(string_t f_string1, string_t f_string2)
{
    for(uint32_t k = 0; f_string1[k] != '\0' && f_string2[k] != '\0'; k++)
    {
        if(f_string1[k] == f_string2[k])
        {
            if((f_string1[k+1] == '\0') && (f_string2[k+1] == '\0'))
            {
                return(true);
            }
            continue;
        }
    }
    return(false);
}

#endif