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


#pragma comment(lib,"xinput.lib")

#define MAX_LOADSTRING 100
#define global_variable static

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


struct GameObject
{
	float x;
	float y;
	float dx;
	float dy;

	int SpriteIndex;
};


struct Sprite {
	Win32OffscreenBuffer Buffer;
	bool HasDirectionSprites;
	int DirectionCount;
	Win32OffscreenBuffer* Frames;
};

enum Directions
{
	S,
	SW,
	W,
	NW,
	N,
	NE,
	E,
	SE
};

struct SoldierAnimation
{
	std::vector<Animation> Animations;
	SoldierAnimation()
	{
		int animationWidth = 64;
		int animationheight = 64;


		Animation stand = Animation::CreateWithFrames(0, 0, animationWidth, animationheight, 1, 1, 1, 0, 8);
		Animation walkS = Animation::CreateWithFrames(0, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkSW = Animation::CreateWithFrames(1, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkW = Animation::CreateWithFrames(2, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkNW = Animation::CreateWithFrames(3, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkN = Animation::CreateWithFrames(4, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkNE = Animation::CreateWithFrames(5, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkE = Animation::CreateWithFrames(6, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkSE = Animation::CreateWithFrames(7, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);

		Animation hit = Animation::CreateWithFrames(6, 7, animationWidth, animationheight, 1, 1, 0, 1, 1);
		Animation shoot = Animation::CreateWithFrames(0, 7, animationWidth, animationheight, 1, 1, 1, 0, 3);
		Animation death = Animation::CreateWithFrames(0, 6, animationWidth, animationheight, 1, 1, 1, 0, 3);

		Animations = {
			stand,
			walkS,
			walkSW,
			walkW,
			walkNW,
			walkN,
			walkNE,
			walkE,
			walkSE,

			hit,
			shoot,
			death

		};
	}

	Animation WalkingForDirection(Directions dir)
	{
		return Animations[1 + dir];
	}

	Animation StandingDirection(Directions dir)
	{
		Animation stion;
		auto ord = Animations[0];
		stion.AnimationStrip.push_back(ord.AnimationStrip[dir]);
		return stion;
	}

	Animation Shooting()
	{
		return Animations[10];
	}
};


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


struct Raycaster {
	glm::vec2 Origin{ 5,1.5 };
	glm::vec2 Direction{ -1,0 };
	int Fov = 60;
	int Near;
	int Far;
};


struct LevelData {

	const int Width = 30;
	const int Height = 30;
	const int Sz = Width * Height;
	float LevelRenderHeight = 16.0f;
	float LevelRenderWidth = 480.0f;
	float WallHeight = 32.0f;
	float* ZBuffer = new float[LevelRenderWidth];
	char data[30 * 30] = {
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	};

	std::vector<GameObject> Entitys;
};

class LevelReader;


global_variable std::shared_ptr<IMapReader> MapReader;
global_variable std::vector<std::wstring> debugString;
global_variable char* Keys;
global_variable Raycaster Caster;
global_variable LevelData Level;
global_variable Win32OffscreenBuffer OffscreenBuffer;
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

global_variable int SEE_THROUGH_TILE = 2;
global_variable int SOLID_TILE = 1;
global_variable int OPEN_TILE = 0;
global_variable glm::vec2 positionVec;

global_variable const int Spr_Barell = 0;
global_variable const int Spr_Treasure = 1;
global_variable const int Spr_Well = 2;
global_variable const int Spr_Pillar = 3;
global_variable const int Spr_Soldier = 4;
global_variable const int Spr_Map = 5;

//Mouse stuff
global_variable int LastMouseX;
global_variable bool MouseClicked;
global_variable int LastMouseY;
global_variable int MouseDistX;
global_variable bool MouseMoved;
global_variable GameObject Soldier;
global_variable GameObject Treasure;
global_variable std::vector<Sprite> SpriteMap;
global_variable StaticSprites Sprites;

global_variable RayCaster Ray;
global_variable Animation TestAnimation;
global_variable SoldierAnimation MapSoldierAnimation;



// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void LoadWolfResources();
int ReadTileAt(float x, float y);
void InitializeKeys();
void DrawLevel(Win32OffscreenBuffer* buffer, LevelData level, int x, int y);
UINT32  PointToTextureColumn(float u, float v, int texH, float scalar);
void ResetProjectile(float x, float y, float dx, float dy);
void debugPrint(std::string str);
void PrintDebugString(int x, int y);
void LoadBufferFromImage(Win32OffscreenBuffer* buffer, LPCWSTR filename);
float ReadChordRow(float x, float y);
Win32OffscreenBuffer* GetAngleSprite(int degrees, Sprite* spr);
Directions DegreestoDirection(int degrees);

void Win32DrawGameObject(Win32OffscreenBuffer* buffer, GameObject object);

void RenderWeirdBkg(Win32OffscreenBuffer* buffer, int OffsetX, int OffsetY);
void Win32DrawGame(Win32OffscreenBuffer* buffer);
void Win32UpdateKeyState(WPARAM wParam, bool isDown);
void Win32UpdateMouse(LPARAM wParam);

float MoveX(float px, float py, float dx, float dy, bool isX)
{
	auto xmoved = MapReader->ReadTileAtPos(px + dx, py);
	auto ymoved = MapReader->ReadTileAtPos(px, py + dy);

	if (isX)
	{
		return MapReader->IsSolid(xmoved) ? px : px + dx;
	}


	return MapReader->IsSolid(ymoved) ? py : py + dy;
}

class LevelReader : public IMapReader
{
	// Inherited via IMapReader
	virtual int ReadTileAtPos(const float& x, const float& y) const override
	{
		int idx = (int)y * Level.Width + (int)x;
		return Level.data[idx];
		//return ReadTileAt(x, y);
	}
	virtual bool IsSolid(const int& value) const override
	{
		return value == SOLID_TILE;
	}
	virtual int Width() const override
	{
		return Level.Width;
	}
	virtual int Height() const override
	{
		return Level.Height;
	}
};

Sprite CreateSprite(std::wstring src)
{
	Sprite spr;
	LoadBufferFromImage(&spr.Buffer, src.c_str());
	spr.DirectionCount = 1;
	spr.HasDirectionSprites = false;
	return spr;
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

void LoadBufferFromImage(Win32OffscreenBuffer* buffer, LPCWSTR filename)
{
	HDC context = GetDC(Win32Helper::WindowHandle);
	HBITMAP hbit = (HBITMAP)LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	Win32Helper::Win32GetPixels(buffer, context, hbit);
	ReleaseDC(0, context);
}

struct Win32RGB
{
	char b;
	char g;
	char r;
};

Win32RGB PointToUvChord(float x, float y)
{
	return Win32RGB{ (char)255, (char)255, (char)25 };
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

	Win32Helper::Win32ResizeBuffer(&OffscreenBuffer, 800, 600);
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

		Win32DrawGame(&OffscreenBuffer);
		HDC context = GetDC(Win32Helper::WindowHandle);
		WindowDimension clientDimension = GetWindowDimension(Win32Helper::WindowHandle);
		Win32Helper::UpdateWin32Window(&OffscreenBuffer, context, 0, 0, clientDimension.Width, clientDimension.Height);
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
			auto nextX = MoveX(Caster.Origin.x, Caster.Origin.y, keyDir.x * scalar, keyDir.y * scalar, true);
			auto nextY = MoveX(Caster.Origin.x, Caster.Origin.y, keyDir.x * scalar, keyDir.y * scalar, false);

			//Caster.Origin += keyDir * scalar; 
			Caster.Origin.x = nextX;
			Caster.Origin.y = nextY;
		}


		if (IsKeyDown('k') || MouseClicked)
		{
			ResetProjectile(Caster.Origin.x, Caster.Origin.y, Caster.Direction.x, Caster.Direction.y);
		}

		if (abs(MouseDistX) > 0)
		{
			Caster.Direction = glm::rotate(Caster.Direction, (3.14f * 5) * -(MouseDistX / Level.LevelRenderWidth));
		}

		xOffset += Caster.Direction.x * 2;
		yOffset += Caster.Direction.y * 2;
		DrawLevel(&OffscreenBuffer, Level, 600, 10);

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

global_variable GameObject Projectile;
void ResetProjectile(float x, float y, float dx, float dy)
{
	Projectile.x = x;
	Projectile.y = y;
	Projectile.dx = dx;
	Projectile.dy = dy;
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
	MapReader = std::shared_ptr<IMapReader>(new LevelReader);

	LoadBufferFromImage(&WallTexture, L"wall.bmp");
	LoadBufferFromImage(&SoldierTexture, L"soldier_0.bmp");
	LoadBufferFromImage(&SoldierTexture1, L"soldier_1.bmp");
	LoadBufferFromImage(&SoldierTexture2, L"soldier_2.bmp");
	LoadBufferFromImage(&SoldierTexture3, L"soldier_3.bmp");
	LoadBufferFromImage(&SoldierTexture4, L"soldier_4.bmp");
	LoadBufferFromImage(&SoldierTexture5, L"soldier_5.bmp");
	LoadBufferFromImage(&SoldierTexture6, L"soldier_6.bmp");
	LoadBufferFromImage(&SoldierTexture7, L"soldier_7.bmp");


	LoadBufferFromImage(&Sprites.Barell, L"barell.bmp");
	LoadBufferFromImage(&Sprites.Well, L"well.bmp");
	LoadBufferFromImage(&Sprites.Treasure, L"treasure.bmp");
	LoadBufferFromImage(&Sprites.Pillar, L"pillar.bmp");

	sprites = new Win32OffscreenBuffer[8];
	sprites[0] = SoldierTexture;
	sprites[1] = SoldierTexture1;
	sprites[2] = SoldierTexture2;
	sprites[3] = SoldierTexture3;
	sprites[4] = SoldierTexture4;
	sprites[5] = SoldierTexture5;
	sprites[6] = SoldierTexture6;
	sprites[7] = SoldierTexture7;

	SpriteMap = std::vector<Sprite>();
	SpriteMap.push_back(Sprite{ Sprites.Barell,false,1 });
	SpriteMap.push_back(Sprite{ Sprites.Treasure,false,1 });
	SpriteMap.push_back(Sprite{ Sprites.Well,false,1 });
	SpriteMap.push_back(Sprite{ Sprites.Pillar,false,1 });
	SpriteMap.push_back(Sprite{ SoldierTexture,true,8,sprites });

	Win32OffscreenBuffer map;
	LoadBufferFromImage(&map, L"soldiermap.bmp");

	auto soldier_spr = Sprite();
	soldier_spr.Buffer = map;

	SpriteMap.push_back(Sprite{ map,false,1 });

	Soldier = { 3.0f, 2.0f,0.0f, 1.0f,Spr_Soldier };
	Treasure = { 4.0f, 2.0f,0.0f, 1.0f,Spr_Treasure };

	Level.Entitys.push_back({ 3.0f, 2.0f,0.0f, 1.0f, Spr_Soldier });
	auto sz = 0;
	for (auto i = 0; i < sz; i++)
	{
		Level.Entitys.push_back({ 4.0f,(0.5f * i) + 4.0f,0.0f, 1.0f,Spr_Treasure });
	}



	TestAnimation = Animation();
	TestAnimation.Speed = 0.05;
	TestAnimation.AnimationStrip = MapSoldierAnimation.WalkingForDirection(Directions::N).AnimationStrip;
}

UINT32  PointToTextureColumn(float u, float v, int columnHeight, float scalar)
{
	int posX = (int)(u * (float)WallTexture.Width);
	UINT32* column = (UINT32*)WallTexture.Memory + (posX * WallTexture.Bpp);

	float columnScale = (float)columnHeight / (float)WallTexture.Height;
	int pointInTex = (int)(v * WallTexture.Height);
	UINT32* columnPixel = (UINT32*)column + pointInTex;
	return *columnPixel;
}

global_variable float frame = 0.0f;
global_variable float rotation;
void Win32DrawGame(Win32OffscreenBuffer* buffer)
{
	float res = Level.LevelRenderWidth;

	float fov = glm::radians<float>(Caster.Fov);
	float step = fov / res;
	float wallH = Level.WallHeight;
	float nearPlane = 1.0f;
	float farPlaneColor = 6.0f;

	float startY = 0.0f;


	glm::vec2 dir = Caster.Direction;
	dir = glm::rotate(dir, -fov * 0.5f);
	float angle = -fov * 0.5f;
	IMapReader* reader = MapReader.get();
	for (int i = 0; i < res; i++)
	{
		dir = glm::rotate(dir, step);
		angle += step;
		float correction = cos(angle);
		RayResult rayRes = Ray.RayDistance(reader, Caster.Origin.x, Caster.Origin.y, dir.x, dir.y);
		rayRes.TexCoord = ReadChordRow(rayRes.HitX, rayRes.HitY);
		float distance = rayRes.Distance * correction;
		float actuallheight = Level.LevelRenderHeight * (wallH / distance);
		float wallStartY = startY + buffer->Height * 0.5 + (actuallheight * -0.5);
		float offsetX = i;


		Level.ZBuffer[i] = distance;
		//TODO fix, this should not be a arbitrary number
		Win32Helper::Win32DrawRect(buffer, i, 0, 1, buffer->Height, 0, 0, 0); //Clear screen

		//Draw Wall strip
		Win32Helper::Win32DrawTexturedLine(buffer, &WallTexture, rayRes.TexCoord, distance, offsetX, wallStartY, actuallheight);
		Win32Helper::Win32DrawGradient(buffer, i, wallStartY + actuallheight, 1, buffer->Height - (wallStartY + actuallheight), { 128,128,128 });
	}

	rotation += (3.14 / 60) * 0.05;
	auto rdir = glm::rotate(glm::vec2(Soldier.dx, Soldier.dy), 3.14f / 60.0f);

	std::sort(Level.Entitys.begin(), Level.Entitys.end(), [](GameObject a, GameObject b) -> bool {
		return Ray.GetProjectedDistance(a.x, a.y, Caster.Origin.x, Caster.Origin.y, Caster.Direction.x, Caster.Direction.y) >
			Ray.GetProjectedDistance(b.x, b.y, Caster.Origin.x, Caster.Origin.y, Caster.Direction.x, Caster.Direction.y);
		});

	for (GameObject item : Level.Entitys)
	{
		Win32DrawGameObject(buffer, item);
	}

	Win32Helper::Win32DrawRect(buffer, Level.LevelRenderWidth * 0.5f, buffer->Height * 0.5, 2, 2, 255, 0, 0);


	frame += TestAnimation.Speed;
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
	return  maxx;
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
		Win32Helper::UpdateWin32Window(&OffscreenBuffer, hdc, 0, 0, dim.Width, dim.Height);

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
	//debugPrint("angelIdx: " + std::to_string(angleIdx));
	//debugPrint("lookingAngle: " + std::to_string(lookingAngle));
	//debugPrint("angleToObj: " + std::to_string(angleToObject));

	float objectHeight = Level.WallHeight;
	float objectWidth = 32.0f;
	float projectedDist = glm::distance(pos, Caster.Origin);
	projectedDist *= cos(glm::radians(viewAngle));
	float projectedHeight = Level.LevelRenderHeight * (objectHeight / projectedDist);
	float projectedWidth = Level.LevelRenderHeight * (objectWidth / projectedDist);
	float startY = 0;
	float projectedY = (buffer->Height * 0.5) + ((projectedHeight * -0.5) + startY);

	debugPrint("dist: " + std::to_string(projectedDist));
	float stepSize = Level.LevelRenderWidth / Caster.Fov;
	float projectedX = (Level.LevelRenderWidth * 0.5f) + (-viewAngle * stepSize);

	float dotP = 1.0f - glm::dot(glm::normalize(Caster.Direction), dirToObject);

	Sprite spr = SpriteMap[entity.SpriteIndex];
	Win32OffscreenBuffer* sprBuffer = &SpriteMap[entity.SpriteIndex].Buffer;

	if (SpriteMap[entity.SpriteIndex].HasDirectionSprites)
	{
		sprBuffer = GetAngleSprite(angle, &spr);
	}

	auto framess = MapSoldierAnimation.StandingDirection(DegreestoDirection(angle));
	int fx = ((int)frame) % framess.AnimationStrip.size();
	Frame fr = framess.AnimationStrip[fx];

	if (abs(viewAngle) < 30)
	{
		for (int i = 0; i < projectedWidth; i++)
		{
			float xx = i + projectedX - (0.5f * projectedWidth);
			double u = i / projectedWidth;
			if (Level.ZBuffer[(int)xx] > projectedDist)
			{
				//Win32DrawTexturedLine(buffer, sprBuffer, u, projectedDist, xx, projectedY + startY, projectedHeight);
				Win32Helper::Win32DrawTexture(buffer, &SpriteMap[Spr_Map].Buffer, xx, projectedY + startY, 1, projectedHeight, fr.x + u * fr.w, fr.y, fr.w, fr.h);
			}
		}
	}

	Win32Helper::Win32DrawTexture(buffer, &SpriteMap[Spr_Map].Buffer, 10, 10, 128, 128, fr.x, fr.y, fr.w, fr.h);
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

Directions DegreestoDirection(int degrees)
{
	int deg = degrees;
	{
		if (deg < 0)
			deg += 360;

		deg /= (360 / 8);
	}
	return (Directions)deg;
}

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

