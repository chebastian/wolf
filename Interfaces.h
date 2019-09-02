#pragma once
#include "framework.h"
#include <stdint.h>
#include "framework.h"
#include "WolfData.h"


class IRenderer
{
public:
	virtual void SetPixel(int x, int y, UINT8 r, UINT8 g, UINT8 b) = 0;
	virtual void DrawGradient(int OffsetX, int OffsetY, int w, int h, RGBColor color) = 0;
	virtual void DrawRect(int OffsetX, int OffsetY, int w, int h, UINT8 r, UINT8 g, UINT8 b) = 0;
	virtual void DrawTexturedLine(int textureId, double u, double dist, int OffsetX, int OffsetY, int h) = 0;
	virtual void DrawTexture(int textureId, int dx, int dy, int w, int h, int sx, int sy, int sw, int sh) = 0;

	virtual int Height() = 0;
	virtual int Width() = 0;
};

class ITextureReader
{
public:
	virtual UINT32 GetTextureWidth(int id) = 0;
	virtual UINT32 GetTextureHeight(int id) = 0;
	virtual UINT32 Pitch(int id) = 0;
	virtual void* Memory(int id) = 0;
};

