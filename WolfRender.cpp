#include "WolfRender.h"
#include "SpriteAnimation.h"
#include "AnimationPlayer.h"
#include "SpriteId.h" 

Directions DegreestoDirection(int degrees)
{
	int deg = degrees;
	{
		if (deg < 0)
			deg += 360;

		deg /= (360 / 8);
	}
	return (Directions)deg;
}

WolfRender::WolfRender(IRenderer* renderer)
{
	Renderer = renderer;
	RegisterTexture(L"soldiermap.bmp", SpriteId::Id_Map);
	RegisterTexture(L"treasure.bmp", SpriteId::Id_Treasure);
	RegisterTexture(L"well.bmp", SpriteId::Id_Well);
	RegisterTexture(L"soldiermap.bmp", SpriteId::Id_Soldier);
	Animator = new AnimationPlayer();
}

void WolfRender::RegisterTexture(std::wstring path, UINT32 id)
{ 
	Win32OffscreenBuffer buffer;
	Win32Helper::LoadBufferFromImage(&buffer, path.c_str());
	Sprites[id] = buffer;
}

void WolfRender::Win32DrawGameObject(GameObject entity)
{	
	auto pos = glm::vec2(entity.x, entity.y);
	glm::vec2 dirToObject = pos - Caster->Origin;

	auto lookingAngle = glm::atan(Caster->Direction.x, Caster->Direction.y);
	auto angleToObject = glm::atan(dirToObject.x, dirToObject.y);
	auto viewAngle = glm::degrees(angleToObject - lookingAngle);
	viewAngle = viewAngle > 0 ? viewAngle : 360 + viewAngle;
	if (viewAngle > 180)
		viewAngle -= 360;

	int angle = glm::degrees(angleToObject - glm::atan(entity.dx, entity.dy));

	float objectHeight = Level.WallHeight;
	float objectWidth = 32.0f;
	float projectedDist = glm::distance(pos, Caster->Origin);
	projectedDist *= cos(glm::radians(viewAngle));
	float projectedHeight = Level.LevelRenderHeight * (objectHeight / projectedDist);
	float projectedWidth = Level.LevelRenderHeight * (objectWidth / projectedDist);
	float startY = 0;
	float projectedY = (Renderer->Height() * 0.5) + ((projectedHeight * -0.5) + startY);

	//debugPrint("dist: " + std::to_string(projectedDist));
	float stepSize = Level.LevelRenderWidth / Caster->Fov;
	float projectedX = (Level.LevelRenderWidth * 0.5f) + (-viewAngle * stepSize);

	float dotP = 1.0f - glm::dot(glm::normalize(Caster->Direction), dirToObject);
 
	auto aaa = SoldierAnimation();
	Animator->PlayAnimation(entity.EntityId, aaa.WalkingForDirection(DegreestoDirection(angle))); 
	Frame fr = Animator->GetCurrentFrame(entity.EntityId,DegreestoDirection(angle));
 
	if (abs(viewAngle) < 30)
	{
		for (int i = 0; i < projectedWidth; i++)
		{
			float xx = i + projectedX - (0.5f * projectedWidth);
			double u = i / projectedWidth;
			if (Level.ZBuffer[(int)xx] > projectedDist)
			{
				//Win32DrawTexturedLine(buffer, sprBuffer, u, projectedDist, xx, projectedY + startY, projectedHeight);
				//Win32Helper::Win32DrawTexture(buffer, &SpriteMap[Spr_Map].Buffer, xx, projectedY + startY, 1, projectedHeight, fr.x + u * fr.w, fr.y, fr.w, fr.h);
				Renderer->DrawTexture(entity.SpriteIndex, xx, projectedY + startY, 1, projectedHeight, fr.x + u * fr.w, fr.y, fr.w, fr.h);
			}
		}
	}

	//Win32Helper::Win32DrawTexture(buffer, &SpriteMap[Spr_Map].Buffer, 10, 10, 128, 128, fr.x, fr.y, fr.w, fr.h);

}
