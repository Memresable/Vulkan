#ifndef G_UTILITIY_H
#define G_UTILITIY_H

#include "../utilities/types.h"

#define MEMRE_FALSE 0
#define MEMRE_TRUE 1

#define ARRAY_SIZE(X) sizeof(X)/sizeof(X[0])

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

#endif