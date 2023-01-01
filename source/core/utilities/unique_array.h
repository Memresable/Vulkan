#ifndef UNIQUE_ARRAY_H
#define UNIQUE_ARRAY_H

#include <stdlib.h>

#include "../utilities/types.h"

template<typename T>
struct UniqueArray
{
    T* array;
    bool* isIndexDuplicated;
    uint32_t size;
};

// ----------------------------------------------------------------
//                          UNIQUE INTEGERS
// ----------------------------------------------------------------
struct CachedUniqueNumber
{
    uint32_t number;
    bool isDuplicated;
};
void
checkForIntegerDuplicates(uint32_t* f_uniqueArray, bool* f_isIndexDuplicated,
                          uint32_t* f_arrayLookup, uint32_t f_arraySize)
{
    // Initialization stage
    CachedUniqueNumber* uniqueNumberList = (CachedUniqueNumber*)malloc(sizeof(CachedUniqueNumber) * f_arraySize);
    for(uint32_t i = 0; i < f_arraySize; i++)
    {
        uniqueNumberList[i].number = 0;
        uniqueNumberList[i].isDuplicated = false;
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
            uniqueNumberList[i+1].isDuplicated = true;
            continue;
        }
        reducedSize++;
    }
    for(uint32_t i = 0; i < f_arraySize; i++)
    {
        if(!uniqueNumberList[i].isDuplicated)
        {
            f_uniqueArray[i] = uniqueNumberList[i].number;
            f_isIndexDuplicated[i] = false;
        }
        else
        {
            f_isIndexDuplicated[i] = true;
        }
    }
}
UniqueArray<uint32_t>
createUniqueIntegerArray(uint32_t* f_array, uint32_t f_arraySize)
{
    UniqueArray<uint32_t> temp = {};
    temp.array = (uint32_t*)calloc(1, sizeof(uint32_t)*f_arraySize);
    temp.isIndexDuplicated = (bool*)calloc(1, sizeof(bool)*f_arraySize);
    temp.size = f_arraySize;
    checkForIntegerDuplicates(temp.array, temp.isIndexDuplicated, f_array, f_arraySize);
    
    UniqueArray<uint32_t> result = {};
    uint32_t totalActualArraySize = 0;
    for(uint32_t i = 0; i < temp.size; i++)
    {
        if(!temp.isIndexDuplicated[i])
        {
            totalActualArraySize++;
        }
    }
    result.size = totalActualArraySize;
    result.array = (uint32_t*)calloc(1, sizeof(uint32_t)*totalActualArraySize);
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

// ----------------------------------------------------------------
//                          UNIQUE STRINGS
// ----------------------------------------------------------------
struct CachedUniqueString
{
    string_t string;
    bool isDuplicated;
};
void
checkForStringDuplicates(string_t* f_uniqueArray, bool* f_isIndexDuplicated,
                         string_t* f_arrayLookup, uint32_t f_arraySize)
{
    // Initialization stage
    CachedUniqueString* uniqueNumberList = (CachedUniqueString*)malloc(sizeof(CachedUniqueString) * f_arraySize);
    for(uint32_t i = 0; i < f_arraySize; i++)
    {
        uniqueNumberList[i].string = 0;
        uniqueNumberList[i].isDuplicated = false;
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
            uniqueNumberList[i+1].string = nullptr;
            uniqueNumberList[i+1].isDuplicated = true;
            continue;
        }
        reducedSize++;
    }
    for(uint32_t i = 0; i < f_arraySize; i++)
    {
        if(!uniqueNumberList[i].isDuplicated)
        {
            f_uniqueArray[i] = uniqueNumberList[i].string;
            f_isIndexDuplicated[i] = false;
        }
        else
        {
            f_isIndexDuplicated[i] = true;
        }
    }
}
UniqueArray<string_t>
createUniqueStringArray(string_t* f_array, uint32_t f_arraySize)
{
    UniqueArray<string_t> temp = {};
    temp.array = (string_t*)calloc(1, sizeof(string_t)*f_arraySize);
    temp.isIndexDuplicated = (bool*)calloc(1, sizeof(bool)*f_arraySize);
    temp.size = f_arraySize;
    checkForStringDuplicates(temp.array, temp.isIndexDuplicated, f_array, f_arraySize);
    
    UniqueArray<string_t> result = {};
    uint32_t totalActualArraySize = 0;
    for(uint32_t i = 0; i < temp.size; i++)
    {
        if(!temp.isIndexDuplicated[i])
        {
            totalActualArraySize++;
        }
    }
    result.size = totalActualArraySize;
    result.array = (string_t*)calloc(1, sizeof(string_t)*totalActualArraySize);
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
