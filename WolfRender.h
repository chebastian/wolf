#pragma once
#include "WolfData.h"
#include "Interfaces.h"
#include "Win32Helper.h"
#include <map>
#include <string>

#include "RayCaster.h"

class IAnimationPlayer;

class WolfRender
{
public:
	WolfRender(IRenderer* renderer);
	void DrawGameObject(GameObject entity);
	void DrawWalls(Raycaster* caster);
	//void DrawGameObjects(Raycaster* caster);
	void DrawGameObjects(Raycaster* caster, std::vector<GameObject> entities);

	IRenderer* Renderer;
	Raycaster* Caster;
	LevelData Level;
	std::map<UINT32, Win32OffscreenBuffer> Sprites;
	Win32OffscreenBuffer WallTexture;
	IAnimationPlayer* Animator;
private:
	void RegisterTexture(std::wstring path, UINT32 id);

	IMapReader* reader;
	const float renderWidth = 480.0f;
	const float renderHeight = 16.0f;
	const float wallHeight = 32.0f;
	RayCaster Ray;
};

