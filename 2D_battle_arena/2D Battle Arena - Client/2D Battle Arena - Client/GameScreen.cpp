#include "GameScreen.h"
#include "stdafx.h"
#include "bitmap.h"

extern GLuint Game_texture[6];

MainGame::MainGame()
{
	m_Hero = new PlayerCharacter((((float)rand() / (float)RAND_MAX) * 50.f) - 22.f,
		(((float)rand() / (float)RAND_MAX) * 45.f) - 22.f, 0, 0.5, 0.5, 1, 0, 0, 0, 15);

	for (int i = 0; i < MAX_Obstacles; i++)
	{
		if (AABBCollision(m_Hero, m_Obstacles[i]))
		{
			break;
		}
	}
	
}

MainGame::~MainGame()
{
	// Renderer delete
}

void MainGame::AdjustPosition(GameObject* a, GameObject b)
{
	float aMinX, aMaxX, aMinY, aMaxY;
	float bMinX, bMaxX, bMinY, bMaxY;
	float aVelX, aVelY;
	float aX, aY, asX, asY;
	float bX, bY, bsX, bsY;
	float temp;

	a->GetPosition(&aX, &aY, &temp);
	a->GetSize(&asX, &asY);
	a->GetVel(&aVelX, &aVelY);

	b.GetPosition(&bX, &bY, &temp);
	b.GetSize(&bsX, &bsY);

	aMinX = aX - asX / 2.f;
	aMaxX = aX + asX / 2.f;
	aMinY = aY - asY / 2.f;
	aMaxY = aY + asY / 2.f;
	bMinX = bX - bsX / 2.f;
	bMaxX = bX + bsX / 2.f;
	bMinY = bY - bsY / 2.f;
	bMaxY = bY + bsY / 2.f;

	if (aMaxY < bMaxY && aMinY > bMinY)
	{
		if (aVelX > 0)
		{
			aX = aX - (aMaxX - bMinX) - 0.01;

			a->SetPosition(aX, aY, 0.f);

			float vx, vy;
			a->GetVel(&vx, &vy);
			a->SetVel(0.f, vy);
		}
		else
		{
			aX = aX + (bMaxX - aMinX) + 0.01;

			a->SetPosition(aX, aY, 0.f);

			float vx, vy;
			a->GetVel(&vx, &vy);
			a->SetVel(0.f, vy);
		}
	}
	else if (aMaxX < bMaxX && aMinX > bMinX)
	{
		if (aVelY > 0)
		{
			aY = aY - (aMaxY - bMinY) - 0.01;

			a->SetPosition(aX, aY, 0.f);

			float vx, vy;
			a->GetVel(&vx, &vy);
			a->SetVel(vx, 0.f);
		}
		else
		{
			aY = aY + (bMaxY - aMinY) + 0.01;

			a->SetPosition(aX, aY, 0.f);

			float vx, vy;
			a->GetVel(&vx, &vy);
			a->SetVel(vx, 0.f);
		}
	}

	else if (fabs(aVelX) > fabs(aVelY))
	{
		if (aMaxX > bMaxX)
		{
			aX = aX + (bMaxX - aMinX) + 0.01;

			a->SetPosition(aX, aY, 0.f);

			float vx, vy;
			a->GetVel(&vx, &vy);
			a->SetVel(0.f, vy);
		}
		else
		{
			aX = aX - (aMaxX - bMinX) - 0.01;

			a->SetPosition(aX, aY, 0.f);

			float vx, vy;
			a->GetVel(&vx, &vy);
			a->SetVel(0.f, vy);
		}
	}
	else if (fabs(aVelX) < fabs(aVelY))
	{
		if (aMaxY > bMaxY)
		{
			aY = aY + (bMaxY - aMinY) + 0.01;

			a->SetPosition(aX, aY, 0.f);

			float vx, vy;
			a->GetVel(&vx, &vy);
			a->SetVel(vx, 0.f);
		}
		else
		{
			aY = aY - (aMaxY - bMinY) - 0.01;

			a->SetPosition(aX, aY, 0.f);

			float vx, vy;
			a->GetVel(&vx, &vy);
			a->SetVel(vx, 0.f);
		}
	}

}

bool MainGame::AABBCollision(GameObject* a, GameObject b)
{
	float aMinX, aMaxX, aMinY, aMaxY;
	float bMinX, bMaxX, bMinY, bMaxY;
	float aX, aY, asX, asY;
	float bX, bY, bsX, bsY;
	float temp;

	a->GetPosition(&aX, &aY, &temp);
	a->GetSize(&asX, &asY);
	b.GetPosition(&bX, &bY, &temp);
	b.GetSize(&bsX, &bsY);
	
	aMinX = aX - asX / 2.f;
	aMaxX = aX + asX / 2.f;
	aMinY = aY - asY / 2.f;
	aMaxY = aY + asY / 2.f;
	bMinX = bX - bsX / 2.f;
	bMaxX = bX + bsX / 2.f;
	bMinY = bY - bsY / 2.f;
	bMaxY = bY + bsY / 2.f;

	if (aMinX > bMaxX)
		return false;

	if (aMaxX < bMinX)
		return false;

	if (aMinY > bMaxY)
		return false;

	if (aMaxY < bMinY)
		return false;

	AdjustPosition(a, b);
	return true;
}

void MainGame::Update(float elapsedTimeInSec, NGPInputs* inputs)
{
	NGPUpdateParams othersParam;
	NGPUpdateParams heroParam;
	memset(&othersParam, 0, sizeof(NGPUpdateParams));
	memset(&heroParam, 0, sizeof(NGPUpdateParams));

	float forceAmount = 200.f;

	if (inputs->KEY_W)
	{
		heroParam.forceY += forceAmount;
	}
	
	if (inputs->KEY_A)
	{
		heroParam.forceX -= forceAmount;
		
	}
	if (inputs->KEY_S)
	{
		heroParam.forceY -= forceAmount;
		
	}
	if (inputs->KEY_D)
	{
		heroParam.forceX += forceAmount;
	}

	if (inputs->MOUSE_LEFT)
	{
		if (m_Hero->GetAmmo() == 0)
		{
			m_Hero->DecreaseReloadDelay(elapsedTimeInSec);
			if (m_Hero->GetReloadDelay() < 0)
			{
				m_Hero->SetAmmo(MAX_AMMO);
				m_Hero->SetReloadDelay(RELOAD_DELAY);
			}
		}
		else if (m_Hero->GetAmmo() > 0 && m_Hero->GetShotDelay() <= 0)
		{
			// 캐릭터의 좌표와 클릭한 위치의 좌표를 통해 총알의 좌표와 방향을 설정한다.
			float x, y, depth;
			float dx, dy;
			m_Hero->GetPosition(&x, &y, &depth);

			dx = (inputs->MouseX);
			dy = -(inputs->MouseY);

			float length = sqrt(dx * dx + dy * dy);

			dx = dx / length;
			dy = dy / length;

			AddObject(x, y, depth, BULLET_SIZE, BULLET_SIZE, dx * BULLET_SPEED, dy * BULLET_SPEED, 0, 0, 1);

			m_Hero->DecreaseAmmo();
			m_Hero->SetShotDelay(SHOT_DELAY);
		}
		
	}

	m_Hero->Update(elapsedTimeInSec, &heroParam);
	

	for (int i = 0; i < Max_Bullet; i++)
	{
		float dx, dy;
		if (m_Bullet[i].check == true) 
		{
			m_Bullet[i].Update(elapsedTimeInSec, &othersParam);
			m_Bullet[i].GetVel(&dx, &dy);

			if(dx == 0.f && dy == 0.f)
				m_Bullet[i].check = false;
		}

		if (Enemy_Bullet[i].check == true)
		{
			Enemy_Bullet[i].Update(elapsedTimeInSec, &othersParam);
		}
	}

	for (int i = 0; i < MAX_Obstacles; i++)
	{
		m_Obstacles[i].Update(elapsedTimeInSec, &othersParam);
	}

	for (int i = 0; i < MAX_Obstacles; i++)
	{
		if (AABBCollision(m_Hero, m_Obstacles[i]))
		{
			break;
		}
	}

	for (int i = 0; i < Max_Bullet; i++)
	{
		if (m_Bullet[i].check)
		{
			if (AABBCollision(m_Enemy, m_Bullet[i]))
			{
				m_Bullet[i].check = false;
				break;
			}
		}
	}

	for (int i = 0; i < Max_Bullet; i++)
	{
		if (Enemy_Bullet[i].check)
		{
			if (AABBCollision(m_Hero, Enemy_Bullet[i]))
			{
				m_Hero->SetHP(m_Hero->GetHP() - 10);
				break;
			}
		}
	}

	for (int i = 0; i < MAX_Obstacles; i++)
	{
		for (int j = 0; j < Max_Bullet; j++)
		{
			if (AABBCollision(&m_Bullet[j], m_Obstacles[i]))
			{
				m_Bullet[j].check = false;
				break;
			}

		}
	}
}

void MainGame::SetObstacle(Obstacle obstacles[])
{
	for (int i = 0; i < MAX_Obstacles; i++)
	{
		m_Obstacles[i] = obstacles[i];
	}
}

void MainGame::SetEnemy(PlayerCharacter* Enemy)
{
	m_Enemy = Enemy;
}

void MainGame::SetEnemyBullet(Bullet Bullets[])
{
	for (int i = 0; i < Max_Bullet; i++)
	{
		Enemy_Bullet[i] = Bullets[i];
	}
}

PlayerCharacter* MainGame::GetHero()
{
	return m_Hero;
}

void MainGame::GetBullet(Bullet Bullets[])
{
	for (int i = 0; i < Max_Bullet; i++)
	{
		Bullets[i] = m_Bullet[i];
	}
}

void MainGame::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	float hx, hy, depth;
	float x, y;
	float sx, sy;
	
	m_Hero->GetPosition(&hx, &hy, &depth);

	x = -hx * 100.f;
	y = -hy * 100.f;


	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			DrawTextureRect(x - 2250 + 500 * i, y - 2250 + 500 * j, 500, 500, Game_texture[2]);
		}
	}

	
	m_Hero->GetSize(&sx, &sy);

	hx = hx * 100.f;
	hy = hy * 100.f;
	sx = sx * 100.f;
	sy = sy * 100.f;

	// Draw My Character
	DrawTextureRect(0, 0, sx, sy, Game_texture[5]);
	//DrawRect(0, 0, sx, sy, 1, 1, 0, 1);
	glColor4f(0, 0, 0, 1);
	renderBitmapCharacterCenter(0, 40, 0, GLUT_BITMAP_HELVETICA_18, m_Hero->GetName());
	renderBitmapCharacterCenter(0, 0, 0, GLUT_BITMAP_HELVETICA_18, (to_string)(m_Hero->GetHP()).c_str());

	
	m_Enemy->GetPosition(&x, &y, &depth);

	m_Enemy->GetSize(&sx, &sy);

	x = x * 100.f - hx;
	y = y * 100.f - hy;
	sx = sx * 100.f;
	sy = sy * 100.f;
	// Draw Enemy
	DrawTextureRect(x, y, sx, sy, Game_texture[4]);
	//DrawRect(x, y, sx, sy, 1, 0, 1, 1);
	glColor4f(0, 0, 0, 1);
	renderBitmapCharacterCenter(x, y + 40, 0, GLUT_BITMAP_HELVETICA_18, m_Enemy->GetName());
	renderBitmapCharacterCenter(x, y, 0, GLUT_BITMAP_HELVETICA_18, (to_string)(m_Enemy->GetHP()).c_str());

	
	for (int i = 0; i < Max_Bullet; i++)
	{
		if (m_Bullet[i].check == true) {
			float x, y, depth;
			m_Bullet[i].GetPosition(&x, &y, &depth);

			float sx, sy;
			m_Bullet[i].GetSize(&sx, &sy);

			x = x * 100.f - hx;
			y = y * 100.f - hy;
			sx = sx * 100.f;
			sy = sy * 100.f;

			DrawTextureRect(x, y, sx, sy, Game_texture[5]);
			//DrawRect(x, y, sx, sy, 1, 1, 0, 1);
		}
	}

	for (int i = 0; i < Max_Bullet; i++)
	{
		if (Enemy_Bullet[i].check == true) {
			float x, y, depth;
			Enemy_Bullet[i].GetPosition(&x, &y, &depth);

			float sx, sy;
			Enemy_Bullet[i].GetSize(&sx, &sy);

			x = x * 100.f - hx;
			y = y * 100.f - hy;
			sx = sx * 100.f;
			sy = sy * 100.f;

			DrawTextureRect(x, y, sx, sy, Game_texture[4]);
			//DrawRect(x, y, sx, sy, 1, 0, 1, 1);
		}
	}

	for (int i = 0; i < MAX_Obstacles; i++)
	{
		float x, y, depth;
		m_Obstacles[i].GetPosition(&x, &y, &depth);

		float sx, sy;
		m_Obstacles[i].GetSize(&sx, &sy);

		x = x * 100.f - hx;
		y = y * 100.f - hy;
		sx = sx * 100.f;
		sy = sy * 100.f;

		if (i < 4)
		{
			DrawRect(x, y, sx, sy, 1, 0, 0, 1);
		}
		else
		{
			DrawTextureRect(x, y, sx, sy, Game_texture[3]);
		}
		
	}

	// 체력
	DrawTextureRect(-500, -300, 75, 75, Game_texture[0]);

	// 탄창
	DrawTextureRect(500, -300, 75, 75, Game_texture[1]);

	glColor4f(1, 1, 1, 1);
	renderBitmapCharacterCenter(-500, -300, 1, GLUT_BITMAP_HELVETICA_18, (to_string)(m_Hero->GetHP()).c_str());

	renderBitmapCharacterCenter(500, -300, 1, GLUT_BITMAP_HELVETICA_18, (to_string)(m_Hero->GetAmmo()).c_str());
	
}


int MainGame::AddObject(float x, float y, float depth,
	float sx, float sy,
	float velX, float velY,
	float accX, float accY,
	float mass)
{
	// find empty slot
	int index = -1;
	for (int i = 0; i < MaxObjects; i++)
	{
		if (m_Bullet[i].check == false)
		{
			index = i;
			m_Bullet[i].check = true;
			break;
		}
	}

	if (index < 0)
	{
		std::cout << "No empty object slot... " << std::endl;
		return -1;
	}
	//m_Objects[index] = new GameObject();
	m_Bullet[index].SetPosition(x, y, depth);
	m_Bullet[index].SetSize(sx, sy);
	m_Bullet[index].SetVel(velX, velY);
	m_Bullet[index].SetAcc(accX, accY);
	m_Bullet[index].SetMass(mass);

	return index;
}

void MainGame::DeleteObject(int index)
{
	if (m_Objects[index] != NULL)
	{
		delete m_Objects[index];
		m_Objects[index] = NULL;
	}
	else
	{
		std::cout << "Try to delete NULL object : " << index << std::endl;
	}
}

