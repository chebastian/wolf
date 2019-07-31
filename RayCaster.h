#pragma once

#include <glm.hpp>
#include <gtx/rotate_vector.hpp>

class IMapReader
{
public:
	virtual int ReadTileAtPos(float x, float y) = 0;
	virtual bool IsSolid(int value) = 0;
	virtual int Width() = 0;
	virtual int Height() = 0;
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
	RayResult RayDistance(std::weak_ptr<IMapReader> reader, float px, float py, float dx, float dy)
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
		while (!hit)
		{
			pos.x += dir.x * stepLength;
			pos.y += dir.y * stepLength;
			if (auto rr = reader.lock())
			{
				int tile = rr->ReadTileAtPos(pos.x, pos.y);
				hit = rr->IsSolid(tile) || pos.x > rr->Width() || pos.y > rr->Height() || pos.y < 0 || pos.x < 0;
			}
		}

		auto res =  RayResult();
		res.Distance = glm::distance(pos, orig);
		res.HitX = pos.x;
		res.HitY = pos.y;
		return res;
	}
};

