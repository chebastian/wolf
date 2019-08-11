#pragma once

#include "SpriteAnimation.h"
#include <map>

enum Directions
{
	S,
	SW,
	W,
	NW,
	N,
	NE,
	E,
	SE
};

struct SoldierAnimation
{
	std::vector<Animation> Animations;
	SoldierAnimation()
	{
		int animationWidth = 64;
		int animationheight = 64;


		Animation stand = Animation::CreateWithFrames(0, 0, animationWidth, animationheight, 1, 1, 1, 0, 8);
		Animation walkS = Animation::CreateWithFrames(0, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkSW = Animation::CreateWithFrames(1, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkW = Animation::CreateWithFrames(2, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkNW = Animation::CreateWithFrames(3, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkN = Animation::CreateWithFrames(4, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkNE = Animation::CreateWithFrames(5, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkE = Animation::CreateWithFrames(6, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		Animation walkSE = Animation::CreateWithFrames(7, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);

		Animation hit = Animation::CreateWithFrames(6, 7, animationWidth, animationheight, 1, 1, 0, 1, 1);
		Animation shoot = Animation::CreateWithFrames(0, 7, animationWidth, animationheight, 1, 1, 1, 0, 3);
		Animation death = Animation::CreateWithFrames(0, 6, animationWidth, animationheight, 1, 1, 1, 0, 3);

		Animations = {
			stand,
			walkS,
			walkSW,
			walkW,
			walkNW,
			walkN,
			walkNE,
			walkE,
			walkSE,

			hit,
			shoot,
			death

		};
	}

	Animation WalkingForDirection(Directions dir)
	{
		return Animations[1 + dir];
	}

	Animation StandingDirection(Directions dir)
	{
		Animation stion;
		auto ord = Animations[0];
		stion.AnimationStrip.push_back(ord.AnimationStrip[dir]);
		return stion;
	}

	Animation Shooting()
	{
		return Animations[10];
	}
};


class IAnimationPlayer
{
public:
	std::map<UINT32, Animation> Animations;
	virtual void PlayAnimation(int id, Animation) = 0;
	virtual Frame GetCurrentFrame(int id, Directions dir) = 0;

	virtual void UpdatePlayer(float time) = 0;
};

class AnimationPlayer : public IAnimationPlayer
{
protected:
	SoldierAnimation soldierAnim;
	std::map<UINT32, UINT32> entityFrameMap;
	float Elapsed = 0.0f;

public:
	AnimationPlayer()
	{
		entityFrameMap[0] = 0;
	}

	void PlayAnimation(int id, Animation a)
	{
		auto hasAnim = Animations.find(id) != Animations.end();
		Animations[id] = a;
	}

	Frame GetCurrentFrame(int id, Directions dir)
	{
		auto hasKey = Animations.find(id) != Animations.end();
		if (!hasKey)
			return Frame{ 0 };

		auto anim = Animations[id];
		return anim.AnimationStrip[((int)Elapsed) % anim.AnimationStrip.size()];
	}

	void UpdatePlayer(float time)
	{
		Elapsed += time;
	}
};

