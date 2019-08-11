#include "Win32Helper.h"
#include "SpriteId.h"
#include "Interfaces.h"


HWND Win32Helper::WindowHandle = 0;

Win32Renderer::Win32Renderer(ITextureReader* reader)
{
	textureReader = reader;
}

void Win32Renderer::SetPixel(int x, int y, UINT8 r, UINT8 g, UINT8 b)
{
}

void Win32Renderer::DrawGradient(int OffsetX, int OffsetY, int w, int h, RGBColor color)
{
}

void Win32Renderer::DrawRect(int OffsetX, int OffsetY, int w, int h, UINT8 r, UINT8 g, UINT8 b)
{
	UINT32* Pixel;
	UINT8* Row = (UINT8*)OffscreenBuffer.Memory;

	if (OffsetX < 0 || OffsetY < 0 ||
		OffsetX > Width() || OffsetY > Height() ||
		OffsetX + w > Width() || OffsetY + h > Height())
		return;

	Row += OffscreenBuffer.Pitch * OffsetY;

	for (auto y = 0; y < h; y++)
	{
		Pixel = (UINT32*)Row;
		Pixel += (OffsetX);
		for (auto x = 0; x < w; x++)
		{
			*Pixel = ((r << 16) | (g << 8) | b);
			Pixel++;
		}

		Row += OffscreenBuffer.Pitch;
	} 
}

void Win32Renderer::DrawTexturedLine(int textureId, double u, double dist, int OffsetX, int OffsetY, int h)
{
	auto texMemory = textureReader->Memory(textureId);
	auto texHeight = textureReader->GetTextureHeight(textureId);
	auto texWidth = textureReader->GetTextureWidth(textureId);
	auto texPitch = textureReader->Pitch(textureId);

	UINT32* Pixel;
	UINT8* Row = (UINT8*)OffscreenBuffer.Memory;

	Row += OffscreenBuffer.Pitch * OffsetY;
	Pixel = (UINT32*)Row;
	Pixel += (OffsetX);

	UINT32* TexturePixel;
	UINT32* TextureColumn = (UINT32*)texMemory;
	UINT32 startTexY = 0;
	UINT32 textureIndex = (UINT32)(u * texWidth);

	TextureColumn += textureIndex;

	for (auto y = 0; y < h; y++)
	{
		if (OffsetY + y <= 0 || OffsetY + y >= OffscreenBuffer.Height)
		{
			Row += OffscreenBuffer.Pitch;
			continue;
		}
		Pixel = (UINT32*)Row;
		Pixel += (OffsetX);

		UINT32 inTextureY = (texHeight - startTexY) * ((startTexY + y) / ((double)h));
		if (inTextureY > texHeight)
			continue;

		auto textureOffset = inTextureY * texHeight;
		TexturePixel = (UINT32*)TextureColumn + textureOffset;
		if (*TexturePixel != 0xFF00Ff)
			* Pixel = (UINT32)((*TexturePixel));
		Row += OffscreenBuffer.Pitch;
	}

}

void Win32Renderer::DrawTexture(int textureId, int dx, int dy, int w, int h, int sx, int sy, int sw, int sh)
{
	auto texMemory = textureReader->Memory(textureId);
	auto texHeight = textureReader->GetTextureHeight(textureId);
	auto texWidth = textureReader->GetTextureWidth(textureId);
	auto texPitch = textureReader->Pitch(textureId);

	UINT32* Pixel;
	UINT8* Row = (UINT8*)OffscreenBuffer.Memory;

	Row += OffscreenBuffer.Pitch * dy;
	Pixel = (UINT32*)Row;
	Pixel += (dx);

	UINT32* TexturePixel;
	UINT32* TextureRow = (UINT32*)texMemory;
	TextureRow += (UINT32)(sy * texWidth);

	for (auto y = 0; y < h; y++)
	{
		if (dy + y <= 0 || dy + y >= OffscreenBuffer.Height)
		{
			Row += OffscreenBuffer.Pitch;
			continue;
		}

		UINT32 inTextureY = (sh) * ((y) / ((double)h));
		if (inTextureY > texHeight)
			continue;

		auto texture_row_delta = inTextureY * texHeight;
		for (auto x = 0; x < w; x++)
		{
			UINT32 inTextureX = (sw) * ((x) / ((double)w));

			TexturePixel = (UINT32*)TextureRow + texture_row_delta + (inTextureX + sx);

			Pixel = (UINT32*)Row;
			Pixel += (dx + x);
			if (*TexturePixel != 0xFF00Ff)
				* Pixel = (UINT32)((*TexturePixel));
		}
		Row += OffscreenBuffer.Pitch;
	}

}

void Win32Renderer::RegisterTexture(std::wstring path, UINT32 id)
{
	Win32OffscreenBuffer buffer;
	Win32Helper::LoadBufferFromImage(&buffer, path.c_str());
	//Sprites[id] = buffer;
}
