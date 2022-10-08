#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>

#define ASSERT_VULKAN(istrue, message) if(istrue) \
{ \
    printf("\n%s\n", message); \
    *(int*)0 = 0; \
}

#endif