// wolf.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include <stdint.h>
#include <glm.hpp>
#include <gtx/rotate_vector.hpp>
#include "wolf.h"
#include <algorithm>
#include <string>
#include <Xinput.h>
#pragma comment(lib,"xinput.lib")

#define MAX_LOADSTRING 100
#define global_variable static

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

struct RayResult
{
	float Distance;
	float TexCoord;
};

struct Win32OffscreenBuffer
{
	BITMAPINFO Info;
	void* Memory;
	HBITMAP Handle;
	HDC DeviceContext;
	int Width = 800;
	int Height = 600;
	int Bpp = 4;
	int Pitch = Width * Bpp;
};

struct WindowDimension
{
	int Width;
	int Height;
};


struct Raycaster {
	glm::vec2 Origin{ 5,5 };
	glm::vec2 Direction{ 0,-1 };
	int Fov;
	int Near;
	int Far;
};


struct LevelData {

	int Width = 9;
	int Height = 8;

	char data[72] = {
		1,1,1,1,1,1,1,1,1,
		1,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,1,1,1,
		1,0,0,0,0,0,1,1,1,
		1,1,1,1,1,1,1,1,1,
	};
};

global_variable char* Keys;
global_variable HWND WindowHandle;
global_variable Raycaster Caster;
global_variable LevelData Level;
global_variable Win32OffscreenBuffer OffscreenBuffer;
global_variable int SOLID_TILE = 1;
global_variable int OPEN_TILE = 0;
global_variable glm::vec2 positionVec;


bool IsKeyDown(char key)
{
	return Keys[key - 'a'];
}



WindowDimension GetWindowDimension(HWND hWnd)
{
	RECT theRect;
	GetClientRect(hWnd, &theRect);
	return WindowDimension{ theRect.right - theRect.left, theRect.bottom - theRect.top };
}

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void UpdateWin32Window(Win32OffscreenBuffer* buffer, HDC deviceContext, int x, int y, int w, int h);
void Win32ResizeBuffer(Win32OffscreenBuffer* buffer, int w, int h);
void RenderWeirdBkg(Win32OffscreenBuffer* buffer, int OffsetX, int OffsetY);
void Win32ClearBuffer(Win32OffscreenBuffer* buffer);
void Win32SetPixel(Win32OffscreenBuffer* buffer, int x, int y, UINT8 r, UINT8 g, UINT8 b);
void Win32DrawRect(Win32OffscreenBuffer* buffer, int OffsetX, int OffsetY, int w, int h, UINT8 r, UINT8 g, UINT8 b);
void Win32DrawGame(Win32OffscreenBuffer* buffer);
void Win32UpdateKeyState(WPARAM wParam, bool isDown);
int ReadTileAt(float x, float y);
void InitializeKeys();
void DrawLevel(Win32OffscreenBuffer* buffer, LevelData level, int x, int y);


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

	Win32ResizeBuffer(&OffscreenBuffer, 800, 600);
	InitializeKeys();

	int xOffset = 0;
	int yOffset = 0;
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

		//RenderWeirdBkg(&OffscreenBuffer, xOffset, yOffset);
		Win32DrawRect(&OffscreenBuffer, 20, 100, 80, 40, 255, 0, 255);
		Win32DrawGame(&OffscreenBuffer);
		HDC context = GetDC(WindowHandle);
		WindowDimension clientDimension = GetWindowDimension(WindowHandle);
		UpdateWin32Window(&OffscreenBuffer, context, 0, 0, clientDimension.Width, clientDimension.Height);
		ReleaseDC(0, context);

		if (IsKeyDown('w'))
		{
			Caster.Origin.y += Caster.Direction.y * 0.02f;
			Caster.Origin.x += Caster.Direction.x * 0.02f;
		}
		if (IsKeyDown('s'))
		{
			Caster.Origin.y += -Caster.Direction.y * 0.02f;
			Caster.Origin.x += Caster.Direction.x * 0.02f;
		}

		if (IsKeyDown('a'))
			Caster.Direction = glm::rotate(Caster.Direction, 3.14f * -0.02f);
		if (IsKeyDown('d'))
			Caster.Direction = glm::rotate(Caster.Direction, 3.14f * 0.02f);

		xOffset += Caster.Direction.x * 2;
		yOffset += Caster.Direction.y * 2;
		DrawLevel(&OffscreenBuffer, Level, 600, 10);
		//xOffset = IsKeyDown('a') ? xOffset - 1 : xOffset;
		//Win32ClearBuffer(&OffscreenBuffer);
	}
	return (int)msg.wParam;
}

void DrawLevel(Win32OffscreenBuffer* buffer, LevelData level, int offsetx, int offsety)
{
	int sz = 16;
	for (int i = 0; i < level.Width * level.Height; i++)
	{
		int x = (i % level.Width);
		int y = (i / level.Width);

		if (level.data[i] == SOLID_TILE)
			Win32DrawRect(buffer, offsetx + x * sz, offsety + y * sz, sz, sz, 127, 0, 255);
		else
			Win32DrawRect(buffer, offsetx + x * sz, offsety + y * sz, sz, sz, 0, 0, 255);
	}

	int playerX = (Caster.Origin.x / (float)level.Width) * level.Width * sz;
	playerX += offsetx;
	int playerY = (Caster.Origin.y / (float)level.Height) * level.Height * sz;
	playerY += offsety;

	Win32DrawRect(buffer, playerX, playerY, 3, 3, 255, 0, 0);
	Win32DrawRect(buffer, playerX + Caster.Direction.x * 5, playerY + Caster.Direction.y * 5, 2, 2, 255, 0, 0);
}

RayResult RayDistance(float px, float py, float dx, float dy);
void Win32DrawGame(Win32OffscreenBuffer* buffer)
{
	float res = 400.0f;
	float fov = glm::radians(60.0f);
	float step = fov / res;
	float wallH = 200;
	float farPlane = 9.0f;
	float farPlaneColor = 6.0f;

	glm::vec2 dir = Caster.Direction;
	dir = glm::rotate(dir, -fov * 0.5f);
	for (int i = 0; i < res; i++)
	{
		dir = glm::rotate(dir, step);

		RayResult res = RayDistance(Caster.Origin.x, Caster.Origin.y, dir.x, dir.y);
		float wallScale = (std::max(res.Distance, 1.0f) / farPlane);
		float colorScale = 1.0f - (std::max(res.Distance, 1.0f) / farPlaneColor);
		float wallHeightScale = (1.0f - wallScale);
		float actuallheight = wallH * wallHeightScale;

		Win32DrawRect(buffer, i, 200, 1, wallH, 0, 0, 0);
		Win32DrawRect(buffer, i, 200 + (0.5f * (wallScale * wallH)), 1, actuallheight, 255 * res.TexCoord, 255 * colorScale, 255 * colorScale);
	}

	//OutputDebugString(L"x:");
	//OutputDebugString(std::to_wstring(Caster.Origin.x).c_str());
	//OutputDebugString(L"y:");
	//OutputDebugString(std::to_wstring(Caster.Origin.y).c_str());
	//OutputDebugString(L"\n");
	//OutputDebugString(std::to_wstring(ReadTileAt(Caster.Origin.x, Caster.Origin.y)).c_str());
	//int idx = (int)Caster.Origin.y * Level.Width + (int)Caster.Origin.x;
	//OutputDebugString(L"idx:");
	//OutputDebugString(std::to_wstring(idx).c_str());
	//OutputDebugString(L"\n");
	//OutputDebugString(L"\n");

	float done = 1.0f;
}

int ReadTileAt(float x, float y)
{
	int idx = (int)y * Level.Width + (int)x;
	return Level.data[idx];
}


float ReadChordRow(float x, float y)
{
	float maxx = std::min(x - (int)x, y - (int)y);
	if (maxx < 0.01f)
	{
		maxx = std::max(x - (int)x, y - (int)y);
	}
	return maxx;
}


RayResult RayDistance(float px, float py, float dx, float dy)
{
	glm::vec2 pos;
	pos.x = px;
	pos.y = py;

	glm::vec2 orig;
	orig.x = px;
	orig.y = py;

	glm::vec2 dir{ dx,dy };

	float stepLength = 0.002f;
	bool hit = false;
	while (!hit)
	{
		pos.x += dir.x * stepLength;
		pos.y += dir.y * stepLength;
		hit = ReadTileAt(pos.x, pos.y) == SOLID_TILE || pos.x > Level.Width || pos.y > Level.Height || pos.y < 0 || pos.x < 0;
	}

	float uv = ReadChordRow(pos.x, pos.y); 
	return RayResult{ glm::distance(pos, orig), uv };
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
		Win32UpdateKeyState(wParam, true);
		break;
	case WM_KEYUP:
		Win32UpdateKeyState(wParam, false);
		break;

	case WM_SIZE:
	{
	}break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...

		auto dim = GetWindowDimension(WindowHandle);
		UpdateWin32Window(&OffscreenBuffer, hdc, 0, 0, dim.Width, dim.Height);

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


void UpdateWin32Window(Win32OffscreenBuffer* buffer, HDC deviceContext, int x, int y, int w, int h)
{
	int correctedW = 0;
	int correctedH = 0;
	if (w > h)
	{
		correctedW = h * (16.0 / 9.0);
		correctedH = h;
	}
	else {
		correctedW = h * (16.0 / 9.0);
		correctedH = h;
	}

	StretchDIBits(
		deviceContext,
		//x, y, w, h,
		//x, y, w, h,
		0, 0, correctedW, correctedH,
		0, 0, buffer->Width, buffer->Height,
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
void Win32ClearBuffer(Win32OffscreenBuffer* buffer)
{
	int sz = buffer->Width * buffer->Height;
	memset(buffer->Memory, 0, sz * sizeof(UINT32));
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
}

global_variable int textX = 0;
global_variable std::string text;

void printInput(WPARAM wParam)
{
	TCHAR tchar = wParam;
	HDC context = GetDC(WindowHandle);
	int charWidth = 0;
	GetCharWidth32(context, (UINT)wParam, (UINT)wParam, &charWidth);
	TextOut(context, textX, 10, &tchar, 1);
	textX += charWidth;
	text += tchar;
	ReleaseDC(WindowHandle, context);
}

void InitializeKeys()
{
	if (Keys)
		delete Keys;
	Keys = new char[25];
	for (int i = 0; i < 25; i++)
		Keys[i] = false;
}

void Win32UpdateKeyState(WPARAM wParam, bool isDown)
{
	TCHAR tchar = wParam;
	auto idx = tchar - 'A';

	if (idx >= 0)
	{
		Keys[idx] = isDown;
	}

	switch (tchar)
	{
	case 'W':
	{
		printInput(wParam);
	}break;
	case 'S':
	{

	}break;
	}
}

