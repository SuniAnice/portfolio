#pragma once
#include "../2D Battle Arena - Client/2D Battle Arena - Client/NGPGlobals.h"
#include <math.h>

class GameObject
{
public:
	GameObject();
	~GameObject();

	void Update(float elapsedTimeInSec, NGPUpdateParams* param);

	void SetPosition(float x, float y, float depth);
	void SetSize(float sx, float sy);
	void SetVel(float x, float y);
	void SetAcc(float x, float y);
	void SetMass(float x);

	void GetPosition(float* x, float* y, float* depth);
	void GetSize(float* sx, float* sy);
	void GetVel(float* x, float* y);
	void GetAcc(float* x, float* y);
	void GetMass(float* x);

protected:

	float m_PositionX, m_PositionY;
	float m_Depth;
	float m_SizeX, m_SizeY;
	float m_VelX, m_VelY;

	float m_AccX, m_AccY;
	float m_Mass;
};

class PlayerCharacter : public GameObject
{
public:
	PlayerCharacter();
	PlayerCharacter(float x, float y, float depth,
		float sx, float sy,
		float velX, float velY,
		float accX, float accY,
		float mass);
	~PlayerCharacter();
	

	void Update(float elapsedTimeInSec, NGPUpdateParams* param);

	void DecreaseAmmo();
	void DecreaseReloadDelay(float elapsedTimeInSec);
	
	void SetAmmo(int ammo);
	void SetHP(int hp);
	void SetShotDelay(float delay);
	void SetReloadDelay(float delay);
	void SetName(char name[]);

	char* GetName();
	int GetAmmo();
	int GetHP();
	float GetShotDelay();
	float GetReloadDelay();
private:
	char p_Name[10];
	int p_Color;

	int p_HP;

	int p_CurrentAmmo;

	float p_ReloadDelay;
	float p_ShotDelay;
};


class Bullet : public GameObject
{
public:
	Bullet();
	~Bullet();

	bool check;
private:
	int b_Check; 	//	누가 쏜 총알인지.
};


class Obstacle : public GameObject
{
public:
	Obstacle();
	~Obstacle();
private:
	int o_Sort;	// 장애물의 종류
};

