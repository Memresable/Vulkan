@echo off

rd /s /q build
mkdir build

xcopy /y /q /E data build

pushd build
rem -DTH_SPEED=1 (removes asserts) TH_RELEASE
cl -Zi -FC -I ..\source\ ../source/win32_main.cpp -Fe:application.exe user32.lib C:\VulkanSDK\1.3.236.0\Lib\vulkan-1.lib
popd