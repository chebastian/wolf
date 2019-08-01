#pragma once

#include <glm.hpp>
#include <gtx/rotate_vector.hpp>

class IMapReader
{
public:
	virtual int ReadTileAtPos(const float& x, const float& y) const = 0;
	virtual bool IsSolid(const int& value) const  = 0;
	virtual int Width() const = 0;
	virtual int Height() const = 0;
};

struct RayResult
{
	float Distance;
	float TexCoord;
	float HitX;
	float HitY;
};

class RayCaster
{
public:
	RayResult RayDistanceEx(IMapReader* reader, float px, float py, float dx, float dy)
	{ 
		int mapx = px;
		int mapy = py;
 
		int stepx = dx > 0 ? 1 : -1;
		int stepy = dy > 0 ? 1 : -1;
 
		float rdx = abs(1.0f / dx);
		float dry = abs(1.0f / dy);

		float sideDistX = 0.0f;
		float sideDistY = 0.0f;

		if (dx < 0)
		{ 
			sideDistX = (px - mapx) * rdx;
		}
		else
		{ 
			sideDistX = (mapx + 1.0f - px) * rdx;
		}

		if (dy < 0)
		{
			sideDistY = (py - mapy) * dry;
		}
		else 
		{
			sideDistY = (mapy + 1.0f - py) * dry; 
		}
 

		bool hit = false;
		bool side = false;

		while (!hit)
		{
			if (sideDistX < sideDistY)
			{ 
				sideDistX += rdx;
				mapx += stepx;
				side = false;
			}
			else
			{ 
				sideDistY += dry;
				mapy += stepy;
				side = true;
			}

			int tile = reader->ReadTileAtPos(mapx, mapy);
			hit = reader->IsSolid(tile);
		}

		float dist = side ? ((float)mapy - py + (1.0f - stepy) / 2.0f) / dy :
			((float)mapx - px + (1.0f - stepx) / 2.0f) / dx;

		float distx = ((float)mapx - px + (1.0f - stepx) / 2.0f) / dx;
		float disty =  ((float)mapy - py + (1.0f - stepy) / 2.0f) / dy;

		float hitx = px + (dx * dist);
		float hity = py + (dy * dist);
		return RayResult{ dist,0.0,hitx,hity };
	}


	RayResult RayDistance(IMapReader* reader, float px, float py, float dx, float dy)
	{
		return RayDistanceEx(reader, px, py, dx, dy);
		glm::vec2 pos;
		pos.x = px;
		pos.y = py;

		glm::vec2 orig;
		orig.x = px;
		orig.y = py;

		glm::vec2 dir{ dx,dy };

		auto newpy = py + dy;
		float distanceToNextY = py - (int)py;


		float stepLength = 0.005f;
		bool hit = false;
		int lvlW = reader->Width();
		int lvlH = reader->Height();
		//if (auto rr = reader.lock())
		{ 
			while (!hit)
			{
				pos.x += dir.x * stepLength;
				pos.y += dir.y * stepLength;
				int tile = reader->ReadTileAtPos(pos.x, pos.y);
				hit = reader->IsSolid(tile) || pos.x > lvlW || pos.y > lvlH || pos.y < 0 || pos.x < 0;
			}
		}

		auto res =  RayResult();
		res.Distance = glm::distance(pos, orig);
		res.HitX = pos.x;
		res.HitY = pos.y;
		return res;
	}
};

