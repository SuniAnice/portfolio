#include "GameObject.h"

GameObject::GameObject()
{
	m_PositionX = -10000;
	m_PositionY = -10000;
	m_Depth = -10000;
	m_SizeX = -10000;
	m_SizeY = -10000;
	

	m_VelX = -10000;
	m_VelY = -10000;
	m_AccX = -10000;
	m_AccY = -10000;
	m_Mass = -10000;
}

GameObject::~GameObject()
{
}

void GameObject::Update(float elapsedTimeInSec, NGPUpdateParams* param)
{
	float t = elapsedTimeInSec;
	float tt = elapsedTimeInSec * elapsedTimeInSec;

	//calce temporary
	float accX = param->forceX / m_Mass;
	float accY = param->forceY / m_Mass;

	//sum with objec'acc
	accX += m_AccX;
	accY += m_AccY;

	//apply friction only m_VexX size overs zero
	if (fabs(m_VelX) > 0.f)
	{
		accX += m_VelX / fabs(m_VelX) * 0.7 * -10.0;
		//check inverse direction
		float tempVelX = m_VelX + accX * t;
		if (m_VelX * tempVelX < 0.f)
			m_VelX = 0.f;
		else
			m_VelX = tempVelX;
	}
	else
	{
		m_VelX = m_VelX + accX * t;
	}

	if (fabs(m_VelY) > 0.f)
	{
		accY += m_VelY / fabs(m_VelY) * 0.7 * -10.0;
		//check inverse direction
		float tempVelY = m_VelY + accY * t;
		if (m_VelY * tempVelY < 0.f)
			m_VelY = 0.f;
		else
			m_VelY = tempVelY;
	}
	else
	{
		m_VelY = m_VelY + accY * t;
	}

	// update position
	m_PositionX = m_PositionX + m_VelX * t + 0.5f * accX * tt;
	m_PositionY = m_PositionY + m_VelY * t + 0.5f * accY * tt;

	//update velocity
	//m_VelX = m_VelX + accX * t;
	//m_VelY = m_VelY + accY * t;
}


void GameObject::SetPosition(float x, float y, float depth)
{
	m_PositionX = x;
	m_PositionY = y;
	m_Depth = depth;
}

void GameObject::SetSize(float sx, float sy)
{
	m_SizeX = sx;
	m_SizeY = sy;
}

void GameObject::SetVel(float x, float y)
{
	m_VelX = x;
	m_VelY = y;
}

void GameObject::SetAcc(float x, float y)
{
	m_AccX = x;
	m_AccY = y;
}

void GameObject::SetMass(float x)
{
	m_Mass = x;
}

void GameObject::GetPosition(float* x, float* y, float* depth)
{
	*x = m_PositionX;
	*y = m_PositionY;
	*depth = m_Depth;
}

void GameObject::GetSize(float* sx, float* sy)
{
	*sx = m_SizeX;
	*sy = m_SizeY;
}

void GameObject::GetVel(float* x, float* y)
{
	*x = m_VelX;
	*y = m_VelY;
}

void GameObject::GetAcc(float* x, float* y)
{
	*x = m_AccX;
	*y = m_AccY;
}

void GameObject::GetMass(float* x)
{
	*x = m_Mass;
}

PlayerCharacter::PlayerCharacter()
{
	p_Color = -1;
	for (auto i : p_Name)
	{
		p_Name[i] = '\0';
	}
	p_CurrentAmmo = MAX_AMMO;
	p_ShotDelay = SHOT_DELAY;
	p_HP = PLAYER_HP;
	p_ReloadDelay = RELOAD_DELAY;

}

PlayerCharacter::PlayerCharacter(float x, float y, float depth, float sx, float sy, float velX, float velY, float accX, float accY, float mass)
{
	p_Color = -1;
	for (auto i : p_Name)
	{
		p_Name[i] = '\0';
	}
	p_CurrentAmmo = MAX_AMMO;
	p_ShotDelay = SHOT_DELAY;
	p_HP = PLAYER_HP;
	p_ReloadDelay = RELOAD_DELAY;

	this->SetPosition(x, y, depth);
	this->SetSize(sx, sy);
	this->SetVel(velX, velY);
	this->SetAcc(accX, accY);
	this->SetMass(mass);
}

PlayerCharacter::~PlayerCharacter()
{
}

void PlayerCharacter::Update(float elapsedTimeInSec, NGPUpdateParams* param)
{
	GameObject::Update(elapsedTimeInSec, param);
	p_ShotDelay = p_ShotDelay - elapsedTimeInSec;
	if (m_PositionX < -50.f || m_PositionX > 50.f)
	{
		SetPosition(0, 0, 0);
	}
	if (m_PositionY < -50.f || m_PositionY > 50.f)
	{
		SetPosition(0, 0, 0);
	}
}

void PlayerCharacter::DecreaseAmmo()
{
	p_CurrentAmmo--;
}

void PlayerCharacter::DecreaseReloadDelay(float elapsedTimeInSec)
{
	p_ReloadDelay = p_ReloadDelay - elapsedTimeInSec;
}


void PlayerCharacter::SetAmmo(int ammo)
{
	p_CurrentAmmo = ammo;
}

void PlayerCharacter::SetHP(int hp)
{
	p_HP = hp;
}

void PlayerCharacter::SetShotDelay(float delay)
{
	p_ShotDelay = delay;
}

void PlayerCharacter::SetReloadDelay(float delay)
{
	p_ReloadDelay = delay;
}

void PlayerCharacter::SetName(char name[])
{
	for (int i = 0; i < 10; i++)
	{
		p_Name[i] = name[i];
	}
}

char* PlayerCharacter::GetName()
{
	return p_Name;
}

int PlayerCharacter::GetAmmo()
{
	return p_CurrentAmmo;
}

int PlayerCharacter::GetHP()
{
	return p_HP;
}

float PlayerCharacter::GetShotDelay()
{
	return p_ShotDelay;
}

float PlayerCharacter::GetReloadDelay()
{
	return p_ReloadDelay;
}

Bullet::Bullet()
{
	b_Check = -1;
	check = false;
}

Bullet::~Bullet()
{
}

Obstacle::Obstacle()
{
	o_Sort = -1;
}

Obstacle::~Obstacle()
{
}
