#include "Win32Helper.h"

HWND Win32Helper::WindowHandle = 0;

Win32Renderer::Win32Renderer()
{
	Win32Helper::LoadBufferFromImage(&WallTexture, L"wall.bmp");
}

void Win32Renderer::SetPixel(int x, int y, UINT8 r, UINT8 g, UINT8 b)
{
}

void Win32Renderer::DrawGradient(int OffsetX, int OffsetY, int w, int h, RGBColor color)
{
}

void Win32Renderer::DrawRect(int OffsetX, int OffsetY, int w, int h, UINT8 r, UINT8 g, UINT8 b)
{
}

void Win32Renderer::DrawTexturedLine(int textureId, double u, double dist, int OffsetX, int OffsetY, int h)
{
	Win32Helper::Win32DrawTexturedLine(&OffscreenBuffer, &WallTexture, u, dist, OffsetX, OffsetY, h);
}

void Win32Renderer::DrawTexture(int textureId, int dx, int dy, int w, int h, int sx, int sy, int sw, int sh)
{
}
