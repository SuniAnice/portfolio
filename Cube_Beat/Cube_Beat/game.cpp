#include "game.h"

extern System* soundSystem;
extern Sound* sounds[4];
extern Sound* effects[4];
extern Channel* channel[2]; 

extern Value root;
extern Value Highscore;

extern int select;

extern float volume;

GLuint back[1];
GLuint notetex[9];
GLubyte *game_pBytes;
BITMAPINFO *game_info;

double life = 100;



extern bool pers;

int xangle = 0, yangle = 0, zangle = 0;

bool effect[9] = { false };

enum viewstate {
	ortho,persp
};

NODE note[500];
int notenum;

int notespeed = 200;

int notey = 0;
extern int state;
int perfact = 0;
int great = 0;
int good = 0;
int miss = 0;
int score = 0;
int combo = 0;
time_t starttime;
time_t curtime;
int roorder = 0;

extern bool Godmode;
extern bool gameover;

void DrawCube(int Cubesize)
{
	glBegin(GL_QUADS);

	// 앞면 
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(Cubesize, Cubesize, Cubesize);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-Cubesize, Cubesize, Cubesize);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-Cubesize, -Cubesize, Cubesize);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(Cubesize, -Cubesize, Cubesize);


	// 오른쪽
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(Cubesize, Cubesize, Cubesize);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(Cubesize, -Cubesize, Cubesize);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(Cubesize, -Cubesize, -Cubesize);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(Cubesize, Cubesize, -Cubesize);


	// 왼쪽
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-Cubesize, Cubesize, Cubesize);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-Cubesize, Cubesize, -Cubesize);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-Cubesize, -Cubesize, -Cubesize);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-Cubesize, -Cubesize, Cubesize);


	// 뒷면
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-Cubesize, Cubesize, -Cubesize);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(Cubesize, Cubesize, -Cubesize);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(Cubesize, -Cubesize, -Cubesize);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-Cubesize, -Cubesize, -Cubesize);

	

	// 위
	glTexCoord2f(.0f, 1.0f);
	glVertex3f(Cubesize, Cubesize, Cubesize);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(Cubesize, Cubesize, -Cubesize);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-Cubesize, Cubesize, -Cubesize);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-Cubesize, Cubesize, Cubesize);

	// 아래
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-Cubesize, -Cubesize, Cubesize);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-Cubesize, -Cubesize, -Cubesize);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(Cubesize, -Cubesize, -Cubesize);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(Cubesize, -Cubesize, Cubesize);

	glEnd();
}

void dropnote(int value)
{
	notey -= 3;

	for (int i = notenum; i > 0; i--)
	{
		if (note[i].y + notey < -300 && note[i].valid == 1 && Godmode == 0)
		{
			combo = 0;
			life -= 4;
			note[i].valid = 0;
			miss++;
		}
	}

	if (life < 0)
	{
		gameover = 1;
		state = 3;
		channel[0]->stop();
	}

	curtime = clock();
	double duration = (double)(curtime - starttime) / CLOCKS_PER_SEC;

	if (duration > root["Music"][select]["Runtime"].asDouble())
	{
		state = 3;
		channel[0]->stop();
	}

	if (state == 2)
		glutTimerFunc(10, dropnote, value);
}

void rotatetimer(int value)
{
	if (roorder == 0 || roorder == 1 || roorder == 3 || roorder == 4 || roorder == 7 || roorder == 9 || roorder == 10)
		pers = 1;
	else
		pers = 0;
	switch(root["Rotateorder"][roorder].asInt())
	{
	case 0:
		glutTimerFunc(100, RotateX, 0);
		break;
	case 1:
		glutTimerFunc(100, RotateY, 0);
		break;
	case 2:
		glutTimerFunc(100, RotateZ, 0);
		break;
	}
	roorder++;
	if (roorder <12 && state == 2)
		glutTimerFunc(10000, rotatetimer, 0);
}

void initgame()
{
	 starttime = clock();
	soundSystem->playSound(sounds[select], NULL, 0, &channel[0]);
	channel[0]->setVolume(volume);

	roorder = 0;
	gameover = 0;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 500 / 500, 1.0, 10000.0);

	glMatrixMode(GL_MODELVIEW);

	glGenTextures(1, back);

	glBindTexture(GL_TEXTURE_2D, back[0]);
	game_pBytes = LoadDIBitmap(root["Music"][select]["Imagepath"].asCString(), &game_info);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 480, 360, 0, GL_BGR_EXT,
		GL_UNSIGNED_BYTE, game_pBytes);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenTextures(9, notetex);
	for (int i = 0; i < 9; i++)
	{
		glBindTexture(GL_TEXTURE_2D, notetex[i]);
		game_pBytes = LoadDIBitmap(root["NoteTex"][i].asCString(), &game_info);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 757, 190, 0, GL_BGR_EXT,
			GL_UNSIGNED_BYTE, game_pBytes);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	

	life = 100;
	score = 0;
	combo = 0;
	xangle = 0, yangle = 0, zangle = 0;
	notey = 0;

	notenum = root["Music"][select]["Notedata"].size();
	
	for (int i = 0; i < notenum; i++)
	{
		note[i].y = root["Music"][select]["Notedata"][i].asInt() * 3;
		note[i].valid = 1;
		if (i == 0)
		{
			note[i].line = rand()%9;
		}
		else
		{
			if (auto a = rand() % 10 >=6 )
			{
				if (note[i-1].line != 0)
				{
					note[i].line = note[i - 1].line - 1;
				}
				else
					note[i].line = note[i - 1].line + 1;
			}
			else if (a<=3)
			{
				if (note[i-1].line != 8)
				{
					note[i].line = note[i - 1].line + 1;
				}
				else
					note[i].line = note[i - 1].line - 1;
			}
			else if (a == 4)
			{
				if (note[i - 1].line < 5)
				{
					note[i].line = note[i - 1].line + 3;
				}
				else
					note[i].line = note[i - 1].line - 3;
			}
			else
			{
				if (note[i - 1].line > 3)
				{
					note[i].line = note[i - 1].line - 3;
				}
				else
					note[i].line = note[i - 1].line + 3;
			}
			
		}
		
	}
	int perfact = 0;
	int great = 0;
	int good = 0;
	int miss = 0;

	glutTimerFunc(10000, rotatetimer, 0);

	glutTimerFunc(50, dropnote, 1);

	


}

void setortho()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(300.0, -300.0, -300.0, 300.0, -5000.0, 5000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void setperspective()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 500 / 500, 1.0, 10000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, -600, 0, 0, +1, 0, 1, 0);
	//glTranslated(0, 0, -600);
}

void RotateX(int value)
{
	xangle += 10;

	glutPostRedisplay(); // 화면 재 출력
	if (value < 8 && state == 2)
		glutTimerFunc(50, RotateX, value+1); // 타이머함수 재 설정
}

void RotateY(int value)
{
	yangle += 10;

	glutPostRedisplay(); // 화면 재 출력
	if (value < 8 && state == 2)
		glutTimerFunc(50, RotateY, value + 1); // 타이머함수 재 설정
}

void RotateZ(int value)
{
	zangle += 10;

	glutPostRedisplay(); // 화면 재 출력
	if (value < 8 && state == 2)
		glutTimerFunc(50, RotateZ, value + 1); // 타이머함수 재 설정
}

void hiton(int value)
{
	effect[value] = 1;

	for (int i = 0; i < notenum; i++)
	{
		if (note[i].line == value && note[i].valid == 1)
		{
			if (note[i].y + notey > -250 && note[i].y + notey < -210)
			{
				note[i].valid = 0;
				score += 1000 + combo*3;
				combo++;
				if (life<100)
					life += 1;
				perfact++;
			}
			else if (note[i].y + notey > -270 && note[i].y + notey < -190)
			{
				note[i].valid = 0;
				score += 750 + combo*3;
				combo++;
				if (life<100)
					life += 0.8;
				great++;
			}
			else if (note[i].y + notey > -290 && note[i].y + notey < -170)
			{
				note[i].valid = 0;
				score += 500 + combo*3;
				combo++;
				if (life<100)
					life += 0.6;
				good++;
			}
		}
	}
	

	glutTimerFunc(250, hitoff, value);
}
void hitoff(int value)
{
	effect[value] = 0;
}

char buffer[100];

void draw_game()
{
	glColor4f(1, 1, 1, 1);
	// UI 그리기
	glViewport(500, 0, 300, 800);
	setortho();
	
	glBindTexture(GL_TEXTURE_2D, back[0]);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	{
		glPushMatrix();
		glLoadIdentity();
		glTexCoord2f(1.0f, 1.0f);
		glVertex3d(-300, 300, 1000);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3d(-300, 0, 1000);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3d(300, 0, 1000);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3d(300, 300, 1000);
		glPopMatrix();
	}
	glEnd();

	glDisable(GL_TEXTURE_2D);

	// 라이프 게이지
	glColor4f((100-life)/100, (life/100), 0, 0.8);
	glBegin(GL_QUADS);
	{
		glPushMatrix();
		glLoadIdentity();
		glVertex3d(260, 0+3*(life-100), 1000);
		glVertex3d(260, -300, 1000);
		glVertex3d(300, -300, 1000);
		glVertex3d(300, 0 + 3 * (life - 100), 1000);
		glPopMatrix();
	}
	glEnd();
	glColor4f(1, 0, 0, 0.8);
	// 스코어 출력
	renderBitmapCharacter(140,-50,0, GLUT_BITMAP_TIMES_ROMAN_24,"SCORE");
	itoa(score, buffer, 10);
	renderBitmapCharacter(60+strlen(buffer)*10, -100, 0, GLUT_BITMAP_TIMES_ROMAN_24, buffer);

	renderBitmapCharacter(140, -150, 0, GLUT_BITMAP_TIMES_ROMAN_24, "Combo");
	itoa(combo, buffer, 10);
	renderBitmapCharacter(60 + strlen(buffer) * 10, -200, 0, GLUT_BITMAP_TIMES_ROMAN_24, buffer);

	// 게임 공간 그리기
	

	glViewport(0, 0, 500, 800);
	glColor4f(1, 1, 1, 0.5);
	if (pers)
		setperspective();
	else
		setortho();

	glRotated(xangle, 1, 0, 0);
	glRotated(yangle, 0, 1, 0);
	glRotated(zangle, 0, 0, 1);

	

	// 큐브 공간
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			glPushMatrix();
			{
				glTranslated(-100 + 100 * i, 0, -100 + 100 * j);
				glScaled(1, 5, 1);
				glutWireCube(100);
			}
			glPopMatrix();
		}
	}

	// 입력 이펙트
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (effect[i+3*j] == true)
			{
				for (int a = 0; a < 9; a++)
				{
					if (i+3*j == 0 || i + 3 * j == 5 || i + 3 * j == 7)
						glColor4f(1, 0, 0, 1-0.1f*a);
					else if (i + 3 * j == 1|| i + 3 * j == 3|| i + 3 * j == 8)
						glColor4f(0, 1, 0, 1 - 0.1f*a);
					else
						glColor4f(0, 0, 1, 1 - 0.1f*a);

					glPushMatrix();
					{
						glTranslated(100 - 100 * i, -235 + 25 * a, -100 + 100 * j);
						glScaled(1, 0.25, 1);
						glutSolidCube(100);
					}
					glPopMatrix();
				}
				
			}
		}
		
	}

	// 판정선
	glColor4f(1, 1, 0, 1);
	glPushMatrix();
	{
		glTranslated(0,-235,0);
		glScaled(3, 0.2, 3);
		glutWireCube(100);
	}
	glPopMatrix();

	// 노트
	glColor4f(1, 1, 1, 1);
	
	for (int i = 0; i < notenum; i++)
	{
		if (notey + note[i].y < 300 && notey + note[i].y>-300 && note[i].valid == 1)
		{
			glPushMatrix();
			{
				glBindTexture(GL_TEXTURE_2D, notetex[note[i].line]);
				glEnable(GL_TEXTURE_2D);
				glTranslated(100 - 100 * (note[i].line % 3), notey + note[i].y, -100 + 100 * (note[i].line / 3));

				glScaled(1, 0.25, 1);

				DrawCube(50);

				glDisable(GL_TEXTURE_2D);
			}
			glPopMatrix();
		}
		
	}
		

	

	
	
	




}