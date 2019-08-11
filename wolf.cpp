// wolf.cpp : Defines the entry point for the application.
//

#include "Win32Helper.h"
#include "framework.h"
#include <stdint.h>
#include <glm.hpp>
#include <gtx/rotate_vector.hpp>
#include "wolf.h"
#include <algorithm>
#include <string>
#include <vector>
#include <memory>
#include <math.h>
#include <Xinput.h>
#include <windowsx.h>
#include "RayCaster.h"
#include "SpriteAnimation.h"
#include "SpriteId.h"
#include "AnimationPlayer.h"
#include "LevelDataReader.h"

#include "WolfData.h"
#include "WolfRender.h"


#pragma comment(lib,"xinput.lib")

#define MAX_LOADSTRING 100
#define global_variable static

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name



struct StaticSprites
{
	Win32OffscreenBuffer Pillar;
	Win32OffscreenBuffer Barell;
	Win32OffscreenBuffer Treasure;
	Win32OffscreenBuffer Well;
};

struct WindowDimension
{
	int Width;
	int Height;
};

class Win32TextureReader : public ITextureReader
{
	std::map<UINT32, Win32OffscreenBuffer> Sprites;
public:
	Win32TextureReader()
	{
		RegisterTexture(L"soldiermap.bmp", SpriteId::Id_Map);
		RegisterTexture(L"treasure.bmp", SpriteId::Id_Treasure);
		RegisterTexture(L"well.bmp", SpriteId::Id_Well);
		RegisterTexture(L"wall.bmp", SpriteId::Id_Wall);
		RegisterTexture(L"soldiermap.bmp", SpriteId::Id_Soldier);
	}

	void RegisterTexture(std::wstring path, UINT32 id)
	{
		Win32OffscreenBuffer buffer;
		Win32Helper::LoadBufferFromImage(&buffer, path.c_str());
		Sprites[id] = buffer;
	}

	// Inherited via ITextureReader
	virtual UINT32 GetTextureWidth(int id) override
	{
		return Sprites[id].Width;
	}
	virtual UINT32 GetTextureHeight(int id) override
	{
		return Sprites[id].Height;
	}
	virtual UINT32 Pitch(int id) override
	{
		return Sprites[id].Width * Sprites[id].Bpp;
	}

	virtual void* Memory(int id) override
	{
		return Sprites[id].Memory;
	}
};


global_variable WolfRender* WolfDraw;
global_variable IAnimationPlayer* Animator;
global_variable std::shared_ptr<IMapReader> MapReader;
global_variable std::vector<std::wstring> debugString;
global_variable char* Keys;
global_variable Raycaster Caster;

global_variable Win32Renderer Renderer = Win32Renderer(new Win32TextureReader());
global_variable Win32OffscreenBuffer WallTexture;

global_variable Win32OffscreenBuffer SoldierTexture;
global_variable Win32OffscreenBuffer SoldierTexture1;
global_variable Win32OffscreenBuffer SoldierTexture2;
global_variable Win32OffscreenBuffer SoldierTexture3;
global_variable Win32OffscreenBuffer SoldierTexture4;
global_variable Win32OffscreenBuffer SoldierTexture5;
global_variable Win32OffscreenBuffer SoldierTexture6;
global_variable Win32OffscreenBuffer SoldierTexture7;

global_variable Win32OffscreenBuffer* sprites;

global_variable glm::vec2 positionVec;

//global_variable const int Spr_Barell = 0;
//global_variable const int Spr_Treasure = 1;
//global_variable const int Spr_Well = 2;
//global_variable const int Spr_Pillar = 3;
//global_variable const int Spr_Soldier = 4;
//global_variable const int Spr_Map = 5;
//global_variable const int Spr_Wall = 2;

//Mouse stuff
global_variable int LastMouseX;
global_variable bool MouseClicked;
global_variable int LastMouseY;
global_variable int MouseDistX;
global_variable bool MouseMoved;
global_variable GameObject Soldier;
global_variable GameObject Treasure;
//global_variable std::vector<Sprite> SpriteMap;
global_variable StaticSprites Sprites;

global_variable RayCaster Ray;
//global_variable SoldierAnimation MapSoldierAnimation;



// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void LoadWolfResources();
int ReadTileAt(float x, float y);
void InitializeKeys();
void DrawLevel(Win32OffscreenBuffer* buffer, LevelData level, int x, int y);
void debugPrint(std::string str);
void PrintDebugString(int x, int y);
void LoadBufferFromImage(Win32OffscreenBuffer* buffer, LPCWSTR filename);
float ReadChordRow(float x, float y);
Win32OffscreenBuffer* GetAngleSprite(int degrees, Sprite* spr);
Directions DegreestoDirection(int degrees);

void Win32DrawGameObject(Win32OffscreenBuffer* buffer, GameObject object);
void RenderWeirdBkg(Win32OffscreenBuffer* buffer, int OffsetX, int OffsetY);
void Win32UpdateKeyState(WPARAM wParam, bool isDown);
void Win32UpdateMouse(LPARAM wParam);

float MoveX(float px, float py, float dx, float dy, bool isX, IMapReader* reader)
{
	auto xmoved = reader->ReadTileAtPos(px + dx, py);
	auto ymoved = reader->ReadTileAtPos(px, py + dy);

	if (isX)
	{
		return reader->IsSolid(xmoved) ? px : px + dx;
	}


	return reader->IsSolid(ymoved) ? py : py + dy;
}



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

struct TextureBuffer
{
	int stuff;
};

LevelData GetLevelData()
{
	return WolfDraw->Level;
}

void LoadBufferFromImage(Win32OffscreenBuffer* buffer, LPCWSTR filename)
{
	HDC context = GetDC(Win32Helper::WindowHandle);
	HBITMAP hbit = (HBITMAP)LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	Win32Helper::Win32GetPixels(buffer, context, hbit);
	ReleaseDC(0, context);
}

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

	Win32Helper::Win32ResizeBuffer(&Renderer.OffscreenBuffer, 800, 600);
	InitializeKeys();
	LoadWolfResources();


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

		WolfDraw->DrawWalls(&Caster);

		HDC context = GetDC(Win32Helper::WindowHandle);
		WindowDimension clientDimension = GetWindowDimension(Win32Helper::WindowHandle);
		Win32Helper::UpdateWin32Window(&Renderer.OffscreenBuffer, context, 0, 0, clientDimension.Width, clientDimension.Height);
		ReleaseDC(0, context);

		float scalar = 0.05f;
		glm::vec2 keyDir = glm::vec2();
		keyDir.x = 0;
		keyDir.y = 0;
		if (IsKeyDown('w'))
		{
			keyDir += Caster.Direction;
			//Caster.Origin.y += Caster.Direction.y * scalar;
			//Caster.Origin.x += Caster.Direction.x * scalar;
		}
		if (IsKeyDown('s'))
		{
			keyDir -= Caster.Direction;
			//Caster.Origin.y += -Caster.Direction.y * scalar;
			//Caster.Origin.x += -Caster.Direction.x * scalar;
		}
		if (IsKeyDown('d'))
		{
			auto right = Caster.Direction;
			right.y = Caster.Direction.x;
			right.x = -Caster.Direction.y;

			keyDir = right;
		}

		if (IsKeyDown('a'))
		{

			auto right = Caster.Direction;
			right.y = -Caster.Direction.x;
			right.x = Caster.Direction.y;
			keyDir = right;
		}

		if (keyDir.x != 0 || keyDir.y != 0)
		{
			auto nextX = MoveX(Caster.Origin.x, Caster.Origin.y, keyDir.x * scalar, keyDir.y * scalar, true, MapReader.get());
			auto nextY = MoveX(Caster.Origin.x, Caster.Origin.y, keyDir.x * scalar, keyDir.y * scalar, false, MapReader.get());

			//Caster.Origin += keyDir * scalar; 
			Caster.Origin.x = nextX;
			Caster.Origin.y = nextY;
		}


		if (IsKeyDown('k') || MouseClicked)
		{
		}

		if (abs(MouseDistX) > 0)
		{
			Caster.Direction = glm::rotate(Caster.Direction, (3.14f * 5) * -(MouseDistX / GetLevelData().LevelRenderWidth));
		}

		xOffset += Caster.Direction.x * 2;
		yOffset += Caster.Direction.y * 2;
		DrawLevel(&Renderer.OffscreenBuffer, GetLevelData(), 600, 10);

		if (MouseMoved)
		{
			MouseMoved = false;
			MouseDistX = 0;
		}

		MouseClicked = false;
		PrintDebugString(600, 10);

		debugString.clear();

	}
	return (int)msg.wParam;
}

void DrawLevel(Win32OffscreenBuffer* buffer, LevelData level, int offsetx, int offsety)
{
	int sz = 16;
	int off = 1;
	for (int i = 0; i < level.Width * level.Height; i++)
	{
		int x = (i % level.Width);
		int y = (i / level.Width);

		if (level.data[i] == SOLID_TILE)
		{
			Win32Helper::Win32DrawRect(buffer, (offsetx + x * sz) - off, (offsety + y * sz) - off, sz + off * 2, sz + off * 2, 255, 255, 255);
			Win32Helper::Win32DrawRect(buffer, (offsetx + x * sz), (offsety + y * sz), sz, sz, 127, 0, 255);
		}
		else
		{
			Win32Helper::Win32DrawRect(buffer, (offsetx + x * sz) - off, (offsety + y * sz) - off, sz + off * 2, sz + off * 2, 255, 255, 255);
			Win32Helper::Win32DrawRect(buffer, (offsetx + x * sz), (offsety + y * sz), sz, sz, 0, 0, 255);
		}
	}

	int playerX = (Caster.Origin.x / (float)level.Width) * level.Width * sz;
	playerX += offsetx;
	int playerY = (Caster.Origin.y / (float)level.Height) * level.Height * sz;
	playerY += offsety;

	Win32Helper::Win32DrawRect(buffer, playerX, playerY, 3, 3, 255, 0, 0);
	Win32Helper::Win32DrawRect(buffer, playerX + Caster.Direction.x * 5, playerY + Caster.Direction.y * 5, 2, 2, 255, 0, 0);
}

void LoadWolfResources()
{
	MapReader = std::shared_ptr<IMapReader>(new LevelDataReader(LevelData()));
	Animator = new AnimationPlayer();
	WolfDraw = new WolfRender(&Renderer);
	WolfDraw->Caster = &Caster;
	WolfDraw->Level.Entitys.push_back({ 3.0f, 2.0f,0.0f, 1.0f, SpriteId::Id_Soldier,1 });


	auto sz = 2;
	for (auto i = 0; i < sz; i++)
	{
		WolfDraw->Level.Entitys.push_back({ 4.0f,(0.5f * i) + 4.0f,0.0f, 1.0f,SpriteId::Id_Well,2 + i });
	}
}


global_variable float frame = 0.0f;
global_variable float rotation;

int ReadTileAt(float x, float y)
{
	int idx = (int)y * GetLevelData().Width + (int)x;
	return GetLevelData().data[idx];
}


//float ReadChordRow(float x, float y)
//{
//	float maxx = std::min(x - (int)x, y - (int)y);
//	if (maxx < 0.01f)
//	{
//		maxx = std::max(x - (int)x, y - (int)y);
//	}
//	return  maxx;
//}
//


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

	Win32Helper::WindowHandle = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!Win32Helper::WindowHandle)
	{
		return FALSE;
	}

	ShowWindow(Win32Helper::WindowHandle, nCmdShow);
	UpdateWindow(Win32Helper::WindowHandle);

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

		auto dim = GetWindowDimension(Win32Helper::WindowHandle);
		Win32Helper::UpdateWin32Window(&Renderer.OffscreenBuffer, hdc, 0, 0, dim.Width, dim.Height);

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_MOUSEMOVE:
		Win32UpdateMouse(lParam);
		break;

	case WM_LBUTTONDOWN:
		MouseClicked = true;
		break;

	case WM_LBUTTONUP:
		MouseClicked = false;
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

UINT32 ReadPixelAt(Win32OffscreenBuffer* buffer, int x, int y)
{
	UINT8* pixel = (UINT8*)buffer->Memory + x;
	return *pixel;
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



void Win32DrawGameObject(Win32OffscreenBuffer* buffer, GameObject entity)
{
	auto pos = glm::vec2(entity.x, entity.y);
	glm::vec2 dirToObject = pos - Caster.Origin;

	auto lookingAngle = glm::atan(Caster.Direction.x, Caster.Direction.y);
	auto angleToObject = glm::atan(dirToObject.x, dirToObject.y);
	auto viewAngle = glm::degrees(angleToObject - lookingAngle);
	viewAngle = viewAngle > 0 ? viewAngle : 360 + viewAngle;
	if (viewAngle > 180)
		viewAngle -= 360;

	int angle = glm::degrees(angleToObject - glm::atan(entity.dx, entity.dy));

	float objectHeight = GetLevelData().WallHeight;
	float objectWidth = 32.0f;
	float projectedDist = glm::distance(pos, Caster.Origin);
	projectedDist *= cos(glm::radians(viewAngle));
	float projectedHeight = GetLevelData().LevelRenderHeight * (objectHeight / projectedDist);
	float projectedWidth = GetLevelData().LevelRenderHeight * (objectWidth / projectedDist);
	float startY = 0;
	float projectedY = (buffer->Height * 0.5) + ((projectedHeight * -0.5) + startY);

	debugPrint("dist: " + std::to_string(projectedDist));
	float stepSize = GetLevelData().LevelRenderWidth / Caster.Fov;
	float projectedX = (GetLevelData().LevelRenderWidth * 0.5f) + (-viewAngle * stepSize);

	float dotP = 1.0f - glm::dot(glm::normalize(Caster.Direction), dirToObject);

	//auto aaa = SoldierAnimation();
	//Animator->PlayAnimation(0, aaa.WalkingForDirection(DegreestoDirection(angle))); 
	//Frame fr = Animator->GetCurrentFrame(entity.EntityId,DegreestoDirection(angle));
	Frame fr = { 0 };

	if (abs(viewAngle) < 30)
	{
		for (int i = 0; i < projectedWidth; i++)
		{
			float xx = i + projectedX - (0.5f * projectedWidth);
			double u = i / projectedWidth;
			if (GetLevelData().ZBuffer[(int)xx] > projectedDist)
			{
				//Win32DrawTexturedLine(buffer, sprBuffer, u, projectedDist, xx, projectedY + startY, projectedHeight);
				//Win32Helper::Win32DrawTexture(buffer, &SpriteMap[Spr_Map].Buffer, xx, projectedY + startY, 1, projectedHeight, fr.x + u * fr.w, fr.y, fr.w, fr.h);
				Renderer.DrawTexture(entity.SpriteIndex, xx, projectedY + startY, 1, projectedHeight, fr.x + u * fr.w, fr.y, fr.w, fr.h);
			}
		}
	}

	//Win32Helper::Win32DrawTexture(buffer, &SpriteMap[Spr_Map].Buffer, 10, 10, 128, 128, fr.x, fr.y, fr.w, fr.h);
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


global_variable int textX = 0;
global_variable std::string text;

void printInput(WPARAM wParam)
{
	TCHAR tchar = wParam;
	HDC context = GetDC(Win32Helper::WindowHandle);
	int charWidth = 0;
	GetCharWidth32(context, (UINT)wParam, (UINT)wParam, &charWidth);
	TextOut(context, textX, 10, &tchar, 1);
	textX += charWidth;
	text += tchar;
	ReleaseDC(Win32Helper::WindowHandle, context);
}


void debugPrint(std::string str)
{
	debugString.push_back(std::wstring(str.begin(), str.end()));
}

void PrintDebugString(int x, int y)
{
	HDC context = GetDC(Win32Helper::WindowHandle);
	int charWidth = 0;

	int py = y;
	for (int i = 0; i < debugString.size(); i++)
	{
		TextOut(context, x, py, debugString[i].c_str(), debugString[i].size());
		py += 18;
	}

	ReleaseDC(Win32Helper::WindowHandle, context);
}

void InitializeKeys()
{
	if (Keys)
		delete Keys;
	Keys = new char[25];
	for (int i = 0; i < 25; i++)
		Keys[i] = false;
}

void Win32UpdateMouse(LPARAM lParam)
{
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);
	MouseDistX = LastMouseX - x;
	MouseMoved = true;
	LastMouseX = x;
	LastMouseY = y;
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

//Directions DegreestoDirection(int degrees)
//{
//	int deg = degrees;
//	{
//		if (deg < 0)
//			deg += 360;
//
//		deg /= (360 / 8);
//	}
//	return (Directions)deg;
//}

Win32OffscreenBuffer* GetAngleSprite(int degrees, Sprite* spr)
{
	int deg = degrees;
	{
		if (deg < 0)
			deg += 360;

		deg /= (360 / 8);
	}

	return spr->Frames + deg;
}

