#include "vulkan.h"

int main()
{
    VulkanEngine application{};
    try
    {
        application.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return EXIT_SUCCESS;
}