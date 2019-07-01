// wolf.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include <stdint.h>
#include <glm.hpp>
#include "wolf.h"

#define MAX_LOADSTRING 100
#define global_variable static

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
global_variable HWND WindowHandle;

struct Win32OffscreenBuffer
{
	BITMAPINFO Info;
	void* Memory;
	HBITMAP Handle;
	HDC DeviceContext;
	int Width;
	int Height;
	int Bpp = 4;
	int Pitch = Width * Bpp;
};

global_variable Win32OffscreenBuffer OffscreenBuffer;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void UpdateWin32Window(Win32OffscreenBuffer* buffer, HDC deviceContext, RECT winRect, int x, int y, int w, int h);
void Win32ResizeBuffer(Win32OffscreenBuffer* buffer,int w, int h);
void RenderWeirdBkg(Win32OffscreenBuffer* buffer, int OffsetX, int OffsetY);
void Win32SetPixel(Win32OffscreenBuffer* buffer, int x, int y, UINT8 r, UINT8 g, UINT8 b);
void Win32DrawRect(Win32OffscreenBuffer* buffer, int OffsetX, int OffsetY, int w, int h, UINT8 r, UINT8 g, UINT8 b);
void Win32UpdateKeyState();

global_variable glm::vec2 positionVec;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WOLF, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WOLF));

	MSG msg;
	bool Running = true;

	int xOffset = 0;
	while (Running)
	{
		// Main message loop:
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				if (msg.message == WM_QUIT)
					Running = false;

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		RECT clientRect;
		GetClientRect(WindowHandle, &clientRect);
		RenderWeirdBkg(&OffscreenBuffer, xOffset, 0);
		Win32DrawRect(&OffscreenBuffer, 20, 100, 80, 40, 255, 0, 255);
		int WinH = clientRect.right - clientRect.left;
		int WinW = clientRect.bottom - clientRect.top;
		HDC context = GetDC(WindowHandle);
		UpdateWin32Window(&OffscreenBuffer, context, clientRect, 0, 0, WinW, WinH);
		ReleaseDC(0, context);
		xOffset++;
	}


	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WOLF));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WOLF);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	WindowHandle = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!WindowHandle)
	{
		return FALSE;
	}

	ShowWindow(WindowHandle, nCmdShow);
	UpdateWindow(WindowHandle);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;

	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		Win32UpdateKeyState();
	}break;

	case WM_SIZE:
	{
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		Win32ResizeBuffer(&OffscreenBuffer, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
	}break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		UpdateWin32Window(&OffscreenBuffer, hdc, clientRect, clientRect.left, clientRect.top, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
} 


void UpdateWin32Window(Win32OffscreenBuffer* buffer, HDC deviceContext, RECT winRect, int x, int y, int w, int h)
{

	int WindowWidth = winRect.right - winRect.left;
	int WindowHeight = winRect.bottom - winRect.top;
	StretchDIBits(
		deviceContext,
		//x, y, w, h,
		//x, y, w, h,
		0, 0, buffer->Width,buffer->Height,
		0, 0, WindowWidth, WindowHeight,
		buffer->Memory, &buffer->Info,
		DIB_RGB_COLORS, SRCCOPY);

}

void Win32SetPixel(Win32OffscreenBuffer* buffer, int x, int y, UINT8 r, UINT8 g, UINT8 b)
{
	UINT32* Pixel;
	UINT8* Row = (UINT8*)buffer->Memory;
	int RedOffset = 2;
	int GreenOffset = 1;
	int BlueOffset = 0;

	Row += (buffer->Pitch * y);
	Pixel = (UINT32*)Row;
	Pixel += (x);
	*Pixel = ((r << 16) | (g << 8) | b);
	Pixel++;
}

void Win32DrawRect(Win32OffscreenBuffer* buffer, int OffsetX, int OffsetY, int w, int h, UINT8 r, UINT8 g, UINT8 b)
{
	UINT32* Pixel;
	UINT8* Row = (UINT8*)buffer->Memory;;

	Row += buffer->Pitch * OffsetY;

	for (auto y = 0; y < h; y++)
	{
		Pixel = (UINT32*)Row;
		Pixel += (OffsetX);
		for (auto x = 0; x < w; x++)
		{
			*Pixel = ((r << 16) | (g << 8) | b);
			Pixel++;
		}

		Row += buffer->Pitch;
	}
}

void RenderWeirdBkg(Win32OffscreenBuffer* buffer, int OffsetX, int OffsetY)
{
	UINT8* Pixel;
	UINT8* Row = (UINT8*)buffer->Memory;

	for (int y = 0; y < buffer->Height; y++)
	{
		Pixel = (UINT8*)Row;
		for (int x = 0; x < buffer->Width; x++)
		{
			*Pixel = 0;
			Pixel++;

			*Pixel = (UINT8)(x + OffsetX);
			Pixel++;

			*Pixel = (UINT8)(y + OffsetY);
			Pixel++;

			*Pixel = 0;
			Pixel++;
		}

		Row += buffer->Pitch;
	}
}

void Win32ResizeBuffer(Win32OffscreenBuffer* buffer, int w, int h)
{
	buffer->Width = w;
	buffer->Height = h;
	buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
	buffer->Info.bmiHeader.biWidth = buffer->Width;
	buffer->Info.bmiHeader.biHeight = -buffer->Height;
	buffer->Info.bmiHeader.biPlanes = 1;
	buffer->Info.bmiHeader.biBitCount = 32;
	buffer->Info.bmiHeader.biCompression = BI_RGB;
	buffer->Pitch = buffer->Width * buffer->Bpp;

	int BitmapSizeMem = (w * h) * buffer->Bpp;

	if (buffer->Memory)
		VirtualFree(buffer->Memory, NULL, MEM_RELEASE);

	buffer->Memory = VirtualAlloc(0, BitmapSizeMem, MEM_COMMIT, PAGE_READWRITE);
	RenderWeirdBkg(&OffscreenBuffer, 0, 0);
}

void Win32UpdateKeyState()
{
	auto stateW = GetAsyncKeyState('w');
	auto stateS = GetAsyncKeyState('s');
}

