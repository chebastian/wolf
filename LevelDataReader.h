#pragma once
#include "RayCaster.h"
#include "WolfData.h"

class LevelDataReader : public IMapReader
{
	LevelData Level;

public:
	LevelDataReader(LevelData level)
	{ 
	}

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

