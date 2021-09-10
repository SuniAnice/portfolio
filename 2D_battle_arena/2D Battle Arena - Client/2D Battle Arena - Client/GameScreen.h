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

	// ���������� new�ؼ� �ν��Ͻ� �ϳ� �����, m_Objects�� �ֱ�
	// ���ϵǴ� int�� �ش�Ǵ� �ε��� ���·� Ȱ��.
	// m_Objects���� ID�� �ε����� ������ ��.
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

