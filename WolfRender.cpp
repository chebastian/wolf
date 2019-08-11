#include "WolfRender.h"
#include "SpriteAnimation.h"
#include "AnimationPlayer.h"
#include "SpriteId.h" 
#include "LevelDataReader.h"

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
	reader = new LevelDataReader(LevelData());
}

void WolfRender::RegisterTexture(std::wstring path, UINT32 id)
{ 
	Win32OffscreenBuffer buffer;
	Win32Helper::LoadBufferFromImage(&buffer, path.c_str());
	Sprites[id] = buffer;
}

float ReadChordRow(float x, float y)
{
	float maxx = std::min(x - (int)x, y - (int)y);
	if (maxx < 0.01f)
	{
		maxx = std::max(x - (int)x, y - (int)y);
	}
	return  maxx;
}

void WolfRender::DrawGameObject(GameObject entity)
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

void WolfRender::DrawWalls(Raycaster* caster)
{	
	float res = renderWidth;
	int bufferHeight = 600;

	float fov = glm::radians<float>(caster->Fov);
	float step = fov / res;
	float wallH = wallHeight;;
	float nearPlane = 1.0f;
	float farPlaneColor = 6.0f;

	float startY = 0.0f;


	glm::vec2 dir = caster->Direction;
	dir = glm::rotate(dir, -fov * 0.5f);
	float angle = -fov * 0.5f;
	for (int i = 0; i < res; i++)
	{
		dir = glm::rotate(dir, step);
		angle += step;
		float correction = cos(angle);
		RayResult rayRes = Ray.RayDistance(reader, caster->Origin.x, caster->Origin.y, dir.x, dir.y);
		rayRes.TexCoord = ReadChordRow(rayRes.HitX, rayRes.HitY);
		float distance = rayRes.Distance * correction;
		float actuallheight = renderHeight * (wallH / distance);
		float wallStartY = startY + bufferHeight * 0.5 + (actuallheight * -0.5);
		float offsetX = i;


		Level.ZBuffer[i] = distance;
		//TODO fix, this should not be a arbitrary number
		Renderer->DrawRect(i, 0, 1, bufferHeight, 0, 0, 0);

		//Draw Wall strip
		Renderer->DrawTexturedLine(SpriteId::Id_Wall,rayRes.TexCoord, distance, offsetX, wallStartY, actuallheight);
		//Win32Helper::Win32DrawGradient(buffer, i, wallStartY + actuallheight, 1, buffer->Height - (wallStartY + actuallheight), { 128,128,128 });
		Renderer->DrawGradient(i, wallStartY + actuallheight, 1, bufferHeight - (wallStartY + actuallheight), { 128,128,128 });
	}

	std::sort(Level.Entitys.begin(), Level.Entitys.end(), [&](GameObject a, GameObject b) -> bool {
		return Ray.GetProjectedDistance(a.x, a.y, caster->Origin.x, caster->Origin.y, caster->Direction.x, caster->Direction.y) >
			Ray.GetProjectedDistance(b.x, b.y, caster->Origin.x, caster->Origin.y, caster->Direction.x, caster->Direction.y);
		});

	Animator->UpdatePlayer(0.05f);

	for (GameObject item : Level.Entitys)
	{
		DrawGameObject(item);
	}

	//Renderer->DrawRect(renderWidth * 0.5f, bufferHeight * 0.5, 2, 2, 255, 0, 0);
	//Win32Helper::Win32DrawRect(


	Animator->UpdatePlayer(0.05f);

}
