#pragma once
#include "WolfData.h"
#include "Interfaces.h"
#include "Win32Helper.h"
#include <map>
#include <string>

class IAnimationPlayer;

class WolfRender
{
public:
	WolfRender(IRenderer* renderer);
	void Win32DrawGameObject(GameObject entity);

	IRenderer* Renderer;
	Raycaster* Caster;
	LevelData Level;
	std::map<UINT32, Win32OffscreenBuffer> Sprites;
	Win32OffscreenBuffer WallTexture;
	IAnimationPlayer* Animator;
private:
	void RegisterTexture(std::wstring path, UINT32 id);

};

