#pragma once

#include "SpriteAnimation.h"
#include "Directions.h"
#include "SpriteId.h"
#include <map>

enum class AnimationType
{
	NONE,
	WALK,
	STAND,
	FIRE,
	DEATH,
	DEAD
};

class AnimationMap
{
public:
	virtual Animation* GetAnimation(AnimationType type, Directions* dir)
	{
		return nullptr;
	}
};

struct SpritesMap : AnimationMap
{
	Animation still;
	SpritesMap()
	{
		still = Animation::SingleFrame(0,0,64,64,0,0);
	}
	virtual Animation* GetAnimation(AnimationType type, Directions* dir ) override
	{ 
		return &still;
	}
};

struct SoldierAnimation : AnimationMap
{
	std::vector<Animation> Animations;
	std::map<Directions, Animation> _walk;
	std::map<Directions, Animation> _stand;
	std::map<AnimationType, Animation> other;

	virtual Animation* GetAnimation(AnimationType type, Directions* dir) override
	{
		if (type == AnimationType::STAND)
		{
			return &_stand[*dir];
		}

		if (type == AnimationType::WALK)
			return &_walk[*dir];

		return &other[type];
	}

	SoldierAnimation()
	{
		int animationWidth = 64;
		int animationheight = 64;

		_stand[Directions::S] = Animation::SingleFrame(0, 0, animationWidth, animationheight, 1, 1);
		_stand[Directions::SW] = Animation::SingleFrame(1, 0, animationWidth, animationheight, 1, 1);
		_stand[Directions::W] = Animation::SingleFrame(2, 0, animationWidth, animationheight, 1, 1);
		_stand[Directions::NW] = Animation::SingleFrame(3, 0, animationWidth, animationheight, 1, 1);
		_stand[Directions::N] = Animation::SingleFrame(4, 0, animationWidth, animationheight, 1, 1);
		_stand[Directions::NE] = Animation::SingleFrame(5, 0, animationWidth, animationheight, 1, 1);
		_stand[Directions::E] = Animation::SingleFrame(6, 0, animationWidth, animationheight, 1, 1);
		_stand[Directions::SE] = Animation::SingleFrame(0, 0, animationWidth, animationheight, 1, 1);

		_walk[Directions::S] = Animation::CreateWithFrames(0, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		_walk[Directions::SW] = Animation::CreateWithFrames(1, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		_walk[Directions::W] = Animation::CreateWithFrames(2, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		_walk[Directions::NW] = Animation::CreateWithFrames(3, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		_walk[Directions::N] = Animation::CreateWithFrames(4, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		_walk[Directions::NE] = Animation::CreateWithFrames(5, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		_walk[Directions::E] = Animation::CreateWithFrames(6, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);
		_walk[Directions::SE] = Animation::CreateWithFrames(7, 1, animationWidth, animationheight, 1, 1, 0, 1, 4);

		other[AnimationType::DEATH] = Animation::CreateWithFrames(0, 5, animationWidth, animationheight, 1, 1, 1, 0, 5);
		other[AnimationType::DEAD] = Animation::CreateWithFrames(4, 5, animationWidth, animationheight, 1, 1, 1, 0, 1);
		other[AnimationType::FIRE] = Animation::CreateWithFrames(0, 6, animationWidth, animationheight, 1, 1, 1, 0, 3);
	}
};


class AnimationWrapper
{
public:
	Animation* GetAnimation(Directions* dir) {
		return anim;
	}

	Animation* anim;
};


class IAnimationPlayer
{
public:
	virtual void PlayAnimation(int id, AnimationType type) = 0;
	virtual Frame GetCurrentFrame(int id, int sprIdx, Directions dir) = 0;
	virtual void UpdatePlayer(float time) = 0;
	virtual void RegisterAnimationMap(AnimationMap* map, int entityid) = 0;
	//virtual Animation* GetAnimation(int id) = 0;

protected:
	std::map<UINT32, AnimationWrapper> Animations;
};


struct XRange {
	int start;
	int sz;
	SpriteId id;
};

class AnimationPlayer : public IAnimationPlayer
{
protected:


	SoldierAnimation soldierAnim;
	std::map<UINT32, UINT32> entityFrameMap;
	std::map<UINT32, AnimationMap*> AnimationMaps;
	std::map<UINT32, AnimationType> ActiveType;
	std::vector<XRange> SpriteMap;

	float Elapsed = 0.0f;

public:
	AnimationPlayer()
	{
		entityFrameMap[0] = 0;
	}


	void PlayAnimation(int id, AnimationType type) override
	{
		ActiveType[id] = type;
	}

	void RegisterIdRange(int start, int sz, AnimationMap* map)
	{
		//SpriteMap.push_back({ start,sz,id});
		//RegisterAnimationMap(map, id);
	} 

	void RegisterAnimationMap(AnimationMap* map, int spriteId) override
	{ 
		AnimationMaps[spriteId] = map;
	}

	Frame GetCurrentFrame(int entityId, int sprIdx, Directions dir)
	{
		auto found = AnimationMaps.find(sprIdx) != AnimationMaps.end();
		if (!found) 
			throw std::exception("No Animation map registered for spriteId" + sprIdx);

		auto theanim = AnimationMaps[sprIdx]->GetAnimation(ActiveType[entityId], &dir);
		return theanim->GetFrame(((int)Elapsed) % theanim->Length());
	}

	void UpdatePlayer(float time)
	{
		Elapsed += time;
	}

private: 
	SpriteId SpriteIdForEntity(int entity)
	{
		SpriteId sprId = SpriteId::Id_Barell;
		for (auto item : SpriteMap)
		{ 
			if (entity >= item.start && entity < item.start + item.sz)
			{
				return item.id;
			} 
		}

		throw std::exception("No spriteId registred for entity with id:  " + entity); 
	}

	AnimationMap* MapForSprite(int id)
	{
	}

};

