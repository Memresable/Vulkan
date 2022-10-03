#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>

#define ASSERT_VULKAN(istrue, message) if(istrue) \
{ \
    std::cout << message << std::endl; \
    *(int*)0 = 0; \
}

#endif