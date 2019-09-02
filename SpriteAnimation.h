#pragma once

#include <vector>
#include "Win32Helper.h"
#include "Directions.h"
#include <map>


struct Frame
{
	int x;
	int y;
	int w;
	int h;
};


class Animation
{
public:
	float Speed;

	static Animation CreateWithFrames(int startx, int starty, int sw, int sh, int offsetx, int offsety, int dx, int dy, int length)
	{
		Animation anim;
		anim.AnimationStrip.clear();
		for (int i = 0; i < length; i++)
		{
			anim.AnimationStrip.push_back(anim.CreateFrameForCoord(startx + i * dx,starty + i * dy,sw,sh,offsetx,offsety));
		}

		return anim;
	}

	static Animation SingleFrame(int x, int y, int sw, int sh,int ox,int oy)
	{
		return CreateWithFrames(x, y, sw, sh, ox, oy, 1, 0, 1);
	}

	virtual Frame GetFrame(const int frame)
	{
		return AnimationStrip[frame];
	}

	virtual int Length()
	{
		return AnimationStrip.size();
	}

protected:
	std::vector<Frame> AnimationStrip;
	Frame CreateFrameForCoord(int sx, int sy, int sw, int sh, int xoffset, int yoffset)
	{
		return { sx * sw + (sx * xoffset),sy * sw + (yoffset * sy),sw,sh };
	}
};

class AnimationWithDirection : public Animation
{ 
public:
	AnimationWithDirection()
	{ 
	}

	void SetDirection(Directions dir)
	{ 
		currentDir = dir;
	}

	virtual Frame GetFrame(const int frame) override
	{ 
		return _anim[currentDir].GetFrame(frame);
	}

	virtual int Length()
	{
		return _anim[currentDir].Length();
	}

protected:

	Directions currentDir;
	std::map<Directions, Animation> _anim;
}; 
