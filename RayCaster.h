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
	RayResult RayDistance(IMapReader* reader, float px, float py, float dx, float dy)
	{
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

