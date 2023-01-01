@echo off

rd /s /q build
mkdir build

xcopy /y /q /E data build

pushd build
cl /std:c17 -Zi -FC -I ..\source\ ../source/win32_main.c -Fe:application.exe user32.lib C:\VulkanSDK\1.3.236.0\Lib\vulkan-1.lib
popd