@echo off

rd /s /q build
mkdir build

xcopy /y /q /E data build

pushd build
cl /std:c++20 -Zi -FC -I ..\source\ ../source/win32_main.cpp -Fe:application.exe user32.lib C:\VulkanSDK\1.3.236.0\Lib\vulkan-1.lib
popd