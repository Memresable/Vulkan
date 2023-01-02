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

typedef struct
{
    char* data;
    uint32_t size;
} StringData;
inline int
compareTwoStrings(string_t f_string1, string_t f_string2)
{
    StringData stringsData[2] = {0};
    
    for(uint32_t j = 0; j < 2; j++)
    {
        for(uint32_t i = 0; ((j == 1) ? f_string1[i] : f_string2[i]) != '\0'; i++)
        {
            stringsData[j].size += 1;
        }
    }
    if(stringsData[0].size != stringsData[1].size) return(MEMRE_FALSE);
    
    stringsData[0].data = (char*)calloc(1, sizeof(char)*stringsData[0].size);
    stringsData[1].data = (char*)calloc(1, sizeof(char)*stringsData[1].size);
    
    for(uint32_t j = 0; j < 2; j++)
    {
        for(uint32_t i = 0; i < stringsData[j].size; i++)
        {
            stringsData[j].data[i] = (j == 1) ? f_string1[i] : f_string2[i];
        }
    }
    
    for(uint32_t i = 0; stringsData[0].data[i] != '\0' && stringsData[1].data[i] != '\0'; i++)
    {
        if(stringsData[0].data[i] != stringsData[1].data[i])
        {
            return(MEMRE_FALSE);
        }
    }
    
    return(MEMRE_TRUE);
}

#endif