#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>

#include "core/render/VulkanEngine.h"

int gloalAppIsRunning = MEMRE_TRUE;

uint32_t globalWindowWidth = 800;
uint32_t globalWindowHeight = 600;


LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
            PostQuitMessage(EXIT_FAILURE);
            gloalAppIsRunning = MEMRE_FALSE;
        }
	}
	return(DefWindowProc(hwnd, uMsg, wParam, lParam));
}

int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    const wchar_t* className = L"Main Window";
    
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
    
    RECT screenRec = {0};
    GetWindowRect(GetDesktopWindow(), &screenRec);
    SetWindowPos(hwnd,
                 NULL,
                 (screenRec.left + screenRec.right)/4,
                 (screenRec.top + screenRec.bottom)/4,
                 globalWindowWidth,
                 globalWindowHeight,
                 SWP_FRAMECHANGED);
    
    VulkanEngine renderer = {0};
    VK_initialize(&renderer, &hwnd, &globalWindowWidth, &globalWindowHeight);
    VK_run(&renderer, &gloalAppIsRunning);
    VK_cleanup(&renderer);
    
	return(0);
}