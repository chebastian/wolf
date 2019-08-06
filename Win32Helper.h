#pragma once

#define global_variable static
#include "framework.h"
#include <stdint.h>
#include <glm.hpp>
#include <gtx/rotate_vector.hpp>
#include "wolf.h"
#include <algorithm>
#include <string>
#include <vector>
#include <windowsx.h>

struct RGBColor
{
	UINT8 r;
	UINT8 g;
	UINT8 b;
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

class Win32Helper
{
public:
	static HWND WindowHandle;

	Win32Helper()
	{

	}

	static void UpdateWin32Window(Win32OffscreenBuffer* buffer, HDC deviceContext, int x, int y, int w, int h)
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

	static void  Win32GetPixels(Win32OffscreenBuffer* buffer, HDC deviceContext, HBITMAP bitmap)
	{
		buffer->Info = { 0 };
		buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);

		auto ctx = GetDC(WindowHandle);
		int ress = GetDIBits(ctx, bitmap, 0, 0, NULL, &buffer->Info, DIB_RGB_COLORS);
		buffer->Pitch = buffer->Width * buffer->Bpp;
		buffer->Info.bmiHeader.biHeight = buffer->Info.bmiHeader.biHeight > 0 ? -buffer->Info.bmiHeader.biHeight : buffer->Info.bmiHeader.biHeight;
		buffer->Width = std::abs(buffer->Info.bmiHeader.biWidth);
		buffer->Height = std::abs(buffer->Info.bmiHeader.biHeight);

		BYTE* pixels = new BYTE[buffer->Info.bmiHeader.biSizeImage];

		int res = GetDIBits(ctx, bitmap, 0, buffer->Info.bmiHeader.biHeight, pixels, &buffer->Info, DIB_RGB_COLORS);
		buffer->Memory = pixels;
		ReleaseDC(WindowHandle, ctx);
	}

	static void Win32ResizeBuffer(Win32OffscreenBuffer* buffer, int w, int h)
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

	static void Win32ClearBuffer(Win32OffscreenBuffer* buffer)
	{
		int sz = buffer->Width * buffer->Height;
		memset(buffer->Memory, 0, sz * sizeof(UINT32));
	}

	static void Win32SetPixel(Win32OffscreenBuffer* buffer, int x, int y, UINT8 r, UINT8 g, UINT8 b)
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

	static void Win32DrawGradient(Win32OffscreenBuffer* buffer, int OffsetX, int OffsetY, int w, int h, RGBColor color)
	{
		for (int y = OffsetY; y < OffsetY + h; y++)
		{
			float tint = (float)y / buffer->Height;
			Win32Helper::Win32SetPixel(buffer, OffsetX, y, color.r * tint, color.g * tint, color.b * tint);
		}
	}

	static void Win32DrawRect(Win32OffscreenBuffer* buffer, int OffsetX, int OffsetY, int w, int h, UINT8 r, UINT8 g, UINT8 b)
	{
		UINT32* Pixel;
		UINT8* Row = (UINT8*)buffer->Memory;

		if (OffsetX < 0 || OffsetY < 0 ||
			OffsetX > buffer->Width || OffsetY > buffer->Height ||
			OffsetX + w > buffer->Width || OffsetY + h > buffer->Height)
			return;

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
	//void Win32GetPixels(Win32OffscreenBuffer* buffer, HDC deviceContext, HBITMAP bitmap);
	//void Win32ResizeBuffer(Win32OffscreenBuffer* buffer, int w, int h);
	//void RenderWeirdBkg(Win32OffscreenBuffer* buffer, int OffsetX, int OffsetY);
	//void Win32ClearBuffer(Win32OffscreenBuffer* buffer);
	//void Win32SetPixel(Win32OffscreenBuffer* buffer, int x, int y, UINT8 r, UINT8 g, UINT8 b);
	//void Win32DrawRect(Win32OffscreenBuffer* buffer, int OffsetX, int OffsetY, int w, int h, UINT8 r, UINT8 g, UINT8 b);
	//void Win32DrawTexturedLine(Win32OffscreenBuffer* buffer, Win32OffscreenBuffer* tex, double u, double dist, int OffsetX, int OffsetY, int h);
	//void Win32DrawGame(Win32OffscreenBuffer* buffer);
	//void Win32UpdateKeyState(WPARAM wParam, bool isDown);
	//void Win32UpdateMouse(LPARAM wParam);
};

HWND Win32Helper::WindowHandle = 0;
