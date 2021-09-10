#pragma once
#include "Renderer.h"
#include "NGPGlobals.h"
#include "../../CommonLibrary/GameObject.h"
#include "Screen.h"

class MainGame : public Screen
{
public:
	MainGame();
	~MainGame();

	void RenderScene();

	// 내부적으로 new해서 인스턴스 하나 만들고, m_Objects에 넣기
	// 리턴되는 int가 해당되는 인덱스 형태로 활용.
	// m_Objects에서 ID를 인덱스로 리턴해 줌.
	int AddObject(float x, float y, float depth,
		float sx, float sy,
		float velX, float velY,
		float accX, float accY,
		float mass);
	void DeleteObject(int index);
	void Update(float elapsedTimeInSec, NGPInputs* inputs);

	void SetObstacle(Obstacle obstacles[]);
	void SetEnemy(PlayerCharacter* Enemy);
	void SetEnemyBullet(Bullet Bullets[]);

	PlayerCharacter* GetHero();
	void GetBullet(Bullet Bullets[]);

	bool AABBCollision(GameObject* a, GameObject b);
	void AdjustPosition(GameObject* a, GameObject b);

	int ScreenNo = 1;

private:
	Renderer* m_renderer = NULL;
	GameObject* m_Objects[MaxObjects];
	Bullet m_Bullet[Max_Bullet];
	Bullet Enemy_Bullet[Max_Bullet];
	Obstacle m_Obstacles[MAX_Obstacles];
	PlayerCharacter* m_Hero;

	PlayerCharacter* m_Enemy;

	int m_HeroID = -1;
};

