#ifndef UNIQUE_ARRAY_H
#define UNIQUE_ARRAY_H

#include <stdlib.h>

#include "../utilities/types.h"
#include "../utilities/general_utilities.h"

// ----------------------------------------------------------------
//                          UNIQUE INTEGERS
// ----------------------------------------------------------------

typedef struct
{
    uint32_t* array;
    int* isIndexDuplicated;
    uint32_t size;
} UniqueIntegerArray;
typedef struct
{
    uint32_t number;
    int isDuplicated;
} CachedUniqueNumber;
inline void
checkForIntegerDuplicates(uint32_t* f_uniqueArray, int* f_isIndexDuplicated,
                          uint32_t* f_arrayLookup, uint32_t f_arraySize)
{
    // Initialization stage
    CachedUniqueNumber* uniqueNumberList = ALLOCATE_MEMORY(CachedUniqueNumber, f_arraySize);
    for(uint32_t i = 0; i < f_arraySize; i++)
    {
        uniqueNumberList[i].number = 0;
        uniqueNumberList[i].isDuplicated = MEMRE_FALSE;
    }
    for(uint32_t i = 0; i < f_arraySize; i++)
    {
        uniqueNumberList[i].number = f_arrayLookup[i];
    }
    
    // Checking Stage
    uint32_t reducedSize = 0;
    for(uint32_t i = 0; i < f_arraySize; i++)
    {
        if(f_arrayLookup[i] == uniqueNumberList[i+1].number)
        {
            uniqueNumberList[i+1].number = 0;
            uniqueNumberList[i+1].isDuplicated = MEMRE_TRUE;
            continue;
        }
        reducedSize++;
    }
    for(uint32_t i = 0; i < f_arraySize; i++)
    {
        if(!uniqueNumberList[i].isDuplicated)
        {
            f_uniqueArray[i] = uniqueNumberList[i].number;
            f_isIndexDuplicated[i] = MEMRE_FALSE;
        }
        else
        {
            f_isIndexDuplicated[i] = MEMRE_TRUE;
        }
    }
}
inline UniqueIntegerArray
createUniqueIntegerArray(uint32_t* f_array, uint32_t f_arraySize)
{
    UniqueIntegerArray temp = {0};
    temp.array = ALLOCATE_MEMORY(uint32_t, f_arraySize);
    temp.isIndexDuplicated = ALLOCATE_MEMORY(int, f_arraySize);
    temp.size = f_arraySize;
    checkForIntegerDuplicates(temp.array, temp.isIndexDuplicated, f_array, f_arraySize);
    
    UniqueIntegerArray result = {0};
    uint32_t totalActualArraySize = 0;
    for(uint32_t i = 0; i < temp.size; i++)
    {
        if(!temp.isIndexDuplicated[i])
        {
            totalActualArraySize++;
        }
    }
    result.size = totalActualArraySize;
    result.array = ALLOCATE_MEMORY(uint32_t, totalActualArraySize);
    for(uint32_t index = 0, realIndex = 0; index < temp.size; index++)
    {
        if(!temp.isIndexDuplicated[index])
        {
            result.array[realIndex] = temp.array[index];
            realIndex++;
        }
    }
    return(result);
}

// TODO: Test if unique strings actually behave correctly
// ----------------------------------------------------------------
//                          UNIQUE STRINGS
// ----------------------------------------------------------------

typedef struct
{
    string_t* array;
    int* isIndexDuplicated;
    uint32_t size;
} UniqueStringArray;
typedef struct
{
    string_t string;
    int isDuplicated;
} CachedUniqueString;
inline void
checkForStringDuplicates(string_t* f_uniqueArray, int* f_isIndexDuplicated,
                         string_t* f_arrayLookup, uint32_t f_arraySize)
{
    // Initialization stage
    CachedUniqueString* uniqueNumberList = ALLOCATE_MEMORY(CachedUniqueString, f_arraySize);
    for(uint32_t i = 0; i < f_arraySize; i++)
    {
        uniqueNumberList[i].string = 0;
        uniqueNumberList[i].isDuplicated = MEMRE_FALSE;
    }
    for(uint32_t i = 0; i < f_arraySize; i++)
    {
        uniqueNumberList[i].string = f_arrayLookup[i];
    }
    
    // Checking Stage
    uint32_t reducedSize = 0;
    for(uint32_t i = 0; i < f_arraySize; i++)
    {
        if(compareTwoStrings(f_arrayLookup[i], uniqueNumberList[i+1].string))
        {
            uniqueNumberList[i+1].string = 0;
            uniqueNumberList[i+1].isDuplicated = MEMRE_TRUE;
            continue;
        }
        reducedSize++;
    }
    for(uint32_t i = 0; i < f_arraySize; i++)
    {
        if(!uniqueNumberList[i].isDuplicated)
        {
            f_uniqueArray[i] = uniqueNumberList[i].string;
            f_isIndexDuplicated[i] = MEMRE_FALSE;
        }
        else
        {
            f_isIndexDuplicated[i] = MEMRE_TRUE;
        }
    }
}
inline UniqueStringArray
createUniqueStringArray(string_t* f_array, uint32_t f_arraySize)
{
    UniqueStringArray temp = {0};
    temp.array = ALLOCATE_MEMORY(string_t, f_arraySize);
    temp.isIndexDuplicated = ALLOCATE_MEMORY(int, f_arraySize);
    temp.size = f_arraySize;
    checkForStringDuplicates(temp.array, temp.isIndexDuplicated, f_array, f_arraySize);
    
    UniqueStringArray result = {0};
    uint32_t totalActualArraySize = 0;
    for(uint32_t i = 0; i < temp.size; i++)
    {
        if(!temp.isIndexDuplicated[i])
        {
            totalActualArraySize++;
        }
    }
    result.size = totalActualArraySize;
    result.array = ALLOCATE_MEMORY(string_t, totalActualArraySize);
    for(uint32_t index = 0, realIndex = 0; index < temp.size; index++)
    {
        if(!temp.isIndexDuplicated[index])
        {
            result.array[realIndex] = temp.array[index];
            realIndex++;
        }
    }
    return(result);
}

#endif