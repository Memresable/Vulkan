#ifndef UNICODE
#define UNICODE
#endif 

#include <stdio.h>
#include <windows.h>

#include "core/render/vulkan_engine.h"

int appIsRunning = MEMRE_TRUE;

uint32_t globalWindowWidth = 800;
uint32_t globalWindowHeight = 600;

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW));
            EndPaint(hwnd, &ps);
            return(0);
        }
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
            PostQuitMessage(EXIT_FAILURE);
            appIsRunning = MEMRE_FALSE;
        }
	}
	return(DefWindowProc(hwnd, uMsg, wParam, lParam));
}

void mainLoop(VulkanEngine* f_engine)
{
    // this look wrong to me, will have to fix it
    MSG msg = {0};
    while(GetMessage(&msg, NULL, 0, 0) > 0 && appIsRunning)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
        VK_drawFrame(f_engine);
	}
    
    vkDeviceWaitIdle(f_engine->device);
}

int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	const wchar_t* className = L"Main application";
    
	WNDCLASS window_class = {0};
	window_class.lpfnWndProc = WindowProc;
	window_class.hInstance = hInstance;
	window_class.lpszClassName = className;
    
	RegisterClass(&window_class);
    
	HWND hwnd = CreateWindowEx(0,
                               className,
                               L"MemreVK1",
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                               NULL,
                               NULL,
                               hInstance,
                               NULL);
    
	if(hwnd == FALSE)
	{
		OutputDebugStringA("Unable to create a Window");
		return(0);
	}
    
	ShowWindow(hwnd, nCmdShow);
    
    VulkanEngine engine = {0};
    VK_initialize(&engine, &hwnd, &globalWindowWidth, &globalWindowHeight);
    
    mainLoop(&engine);
    
    VK_cleanup(&engine);
    
	return(0);
}