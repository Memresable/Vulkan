#ifndef G_UTILITIY_H
#define G_UTILITIY_H

#include <windows.h>

#include "../utilities/types.h"

#define MEMRE_FALSE 0
#define MEMRE_TRUE 1

#define ARRAY_SIZE(X) sizeof(X)/sizeof(X[0])
#define STRING_STRUCT_SIZE(X) sizeof(X)/8

#define MEMRE_ASSERT(X, err_string) \
if(X) \
{ \
OutputDebugStringA(err_string); \
exit(EXIT_FAILURE); \
}

int
compareTwoStrings(string_t f_string1, string_t f_string2)
{
    // TODO: this comparison doesn't make any sense, rewrite it
    for(uint32_t i = 0; f_string1[i] != '\0' && f_string2[i] != '\0'; i++)
    {
        if(f_string1[i] == f_string2[i])
        {
            if((f_string1[i+1] == '\0') && (f_string2[i+1] == '\0'))
            {
                return(MEMRE_TRUE);
            }
            continue;
        }
    }
    return(MEMRE_FALSE);
}

#endif