#pragma once

#include <vector>
#include "Win32Helper.h"


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
	std::vector<Frame> AnimationStrip;

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

private:
	Frame CreateFrameForCoord(int sx, int sy, int sw, int sh, int xoffset, int yoffset)
	{
		return { sx * sw + (sx * xoffset),sy * sw + (yoffset * sy),sw,sh };
	}
};

