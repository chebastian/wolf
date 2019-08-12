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

WolfRender::WolfRender(IRenderer* renderer, ITextureReader* textureReader)
{
	TextureReader = textureReader;
	Renderer = renderer;
	Animator = new AnimationPlayer();
	reader = new LevelDataReader(LevelData()); 

	Level.Entitys.push_back({ 3.0f, 2.0f,0.0f, 1.0f, SpriteId::Id_Soldier,1 }); 

	auto sz = 3;
	for (auto i = 0; i < sz; i++)
	{
		Level.Entitys.push_back({ 4.0f,(0.5f * i) + 4.0f,0.0f, 1.0f,SpriteId::Id_Treasure + i, 2 + i });
	}

	auto aaa = SoldierAnimation();
	std::vector<GameObject> soldiers;

	std::copy_if(Level.Entitys.begin(), Level.Entitys.end(),
		std::back_inserter(soldiers),
		[&](GameObject object)
		{
			return object.SpriteIndex == SpriteId::Id_Soldier;
		});

	for (auto soldier : soldiers)
	{
		Animator->PlayAnimation(soldier.EntityId, aaa.WalkingForDirection(DegreestoDirection(90)));
	}

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

	float objectHeight = Level.WallHeight;
	float objectWidth = 32.0f;

	float projectedDist = glm::distance(pos, Caster->Origin);
	projectedDist *= cos(glm::radians(viewAngle));
	float projectedHeight = Level.LevelRenderHeight * (objectHeight / projectedDist);
	float projectedWidth = Level.LevelRenderHeight * (objectWidth / projectedDist);

	float startY = 0;
	float projectedY = (Renderer->Height() * 0.5) + ((projectedHeight * -0.5) + startY); 
	float stepSize = Level.LevelRenderWidth / Caster->Fov;
	float projectedX = (Level.LevelRenderWidth * 0.5f) + (-viewAngle * stepSize);


	int angle = glm::degrees(angleToObject - glm::atan(entity.dx, entity.dy));
	Frame fr = Animator->GetCurrentFrame(entity.EntityId, DegreestoDirection(angle));

	if (fr.w == 0 || fr.h == 0)
	{
		fr.w = TextureReader->GetTextureWidth(entity.SpriteIndex);
		fr.h = TextureReader->GetTextureHeight(entity.SpriteIndex);
	}

	if (abs(viewAngle) < 30)
	{
		for (int i = 0; i < projectedWidth; i++)
		{
			float xx = i + projectedX - (0.5f * projectedWidth);
			double u = i / projectedWidth;
			if (Level.ZBuffer[(int)xx] > projectedDist)
			{
				Renderer->DrawTexture(entity.SpriteIndex, xx, projectedY + startY, 1, projectedHeight, fr.x + u * fr.w, fr.y, fr.w, fr.h);
			}
		}
	}

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
		Renderer->DrawRect(i, 0, 1, bufferHeight, 0, 0, 0);

		Renderer->DrawTexturedLine(SpriteId::Id_Wall, rayRes.TexCoord, distance, offsetX, wallStartY, actuallheight);
		Renderer->DrawGradient(i, wallStartY + actuallheight, 1, bufferHeight - (wallStartY + actuallheight), { 128,128,128 });
	}

	Animator->UpdatePlayer(0.05f);
	DrawGameObjects(caster, Level.Entitys);

}

void WolfRender::DrawGameObjects(Raycaster* caster, std::vector<GameObject> entities)
{

	std::sort(entities.begin(), entities.end(), [&](GameObject a, GameObject b) -> bool {
		return Ray.GetProjectedDistance(a.x, a.y, caster->Origin.x, caster->Origin.y, caster->Direction.x, caster->Direction.y) >
			Ray.GetProjectedDistance(b.x, b.y, caster->Origin.x, caster->Origin.y, caster->Direction.x, caster->Direction.y);
		});

	Animator->UpdatePlayer(0.05f);

	for (GameObject item : entities)
	{
		DrawGameObject(item);
	}
}
