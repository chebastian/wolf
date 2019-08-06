#pragma once


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

static class Win32Helper
{
public:
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
