#include <GL/glut.h>
#include <fmod.hpp>
#include <fstream>
#include <iostream>
#include <time.h>
#include "Title.h"
#include "select.h"
#include "jsoncpp\json\json.h"
#include "game.h"
#include "result.h"

using namespace FMOD;
using namespace Json;
using namespace std;



int timer = 66;
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Timerfunction(int value);
void Keyboard(unsigned char key, int x, int y);
void SpecialKeyboard(int key, int x, int y);
System* soundSystem;

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // 디스플레이 모드 설정
	glutInitWindowPosition(100, 100); // 윈도우의 위치지정
	glutInitWindowSize(800, 800); // 윈도우의 크기 지정
	glutCreateWindow("Cube Beat"); // 윈도우 생성 (윈도우 이름)
	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(timer, Timerfunction, 1);
	glutSpecialFunc(SpecialKeyboard);
	glutMainLoop();
	soundSystem->release();
	soundSystem->close();
}

Sound* title;
Sound* sounds[4];
Sound* effects[4];
Channel* channel[2];

Value root;
Value Highscore;

bool Godmode = 0;

Sound* resultbgm[2];
void initiailze()
{
	srand(time(NULL));

	ifstream notedata("../Resource/notedata.json", ifstream::binary);
	notedata >> root;

	ifstream high("../Resource/highscore.json", ifstream::binary);
	high >> Highscore;





	System_Create(&soundSystem);


	soundSystem->init(2, FMOD_INIT_NORMAL, NULL);
	

	soundSystem->createSound("../Resource/title.mp3",
		FMOD_LOOP_OFF, 0, &title);

	soundSystem->createSound("../Resource/gameover.mp3",
		FMOD_LOOP_OFF, 0, &resultbgm[0]);

	soundSystem->createSound("../Resource/result.mp3",
		FMOD_LOOP_OFF, 0, &resultbgm[1]);


	for (int i = 0; i < root["musicnum"].asInt();i++)
	{
		soundSystem->createSound(root["Music"][i]["Filepath"].asCString(),
			FMOD_LOOP_OFF, 0, &sounds[i]);
	}
	
	

	

}
bool first = 0;
int select = 0;
float volume = 0.5f;

bool gameover = 0;

bool pers = 0;

enum states {
	Title, Select, Game , Result
};
int state = 0;
GLvoid drawScene(GLvoid)
{
	
	if (first == 0)
	{
		first = 1;
		initiailze();
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	

	if (state == Title)
	{
		draw_title();
	}
	else if (state == Select)
	{
		draw_select();
	}
	else if (state == Game)
	{
		draw_game();
	}
	else if (state == Result)
	{
		draw_result(gameover);
	}
	






	glutSwapBuffers();
}
extern bool first_re;
extern bool se_first;
void Keyboard(unsigned char key, int x, int y)
{
	if (key == '+')
	{
		if (volume < 1.0f)
		{
			volume += 0.1f;
		}
		channel[0]->setVolume(volume);
	}
	else if (key == '-')
	{
		if (volume > 0.0f)
		{
			volume -= 0.1f;
		}
		channel[0]->setVolume(volume);
	}
	else if (key == ' ')
	{
		switch (state)
		{
		case Title:
			state = Select;
			channel[0]->stop();
			soundSystem->playSound(sounds[select], NULL, 0, &channel[0]);
			channel[0]->setVolume(volume);
			break;
		case Select:
			state = Game;
			se_first = 0;
			channel[0]->stop();
			initgame();
			break;
		case Result:
			Reshape(800, 800);
			state = Select;
			channel[0]->stop();
			first_re = 0;
			soundSystem->playSound(sounds[select], NULL, 0, &channel[0]);
			channel[0]->setVolume(volume);
			break;
		}
	}
	else if (key == 27 || key == 'q')
	{
		if (state == Game)
		{
			state = Select;
			channel[0]->stop();
			Reshape(800, 800);
			soundSystem->playSound(sounds[select], NULL, 0, &channel[0]);
			channel[0]->setVolume(volume);
		}
		else
		{
			exit(0);
		}
	}
	else if (key >= '1' && key <= '9')
	{
		glutTimerFunc(50, hiton, key - '1');
	}
	else if (key == 'g')
	{
		if (Godmode == 0)
			Godmode = 1;
		else
			Godmode = 0;
	}
	

	
	

	


}
void SpecialKeyboard(int key, int x, int y)
{
	if (state == Select)
	{
		if (key == GLUT_KEY_UP)
		{
			if (select > 0)
			{
				select--;
				channel[0]->stop();
				soundSystem->playSound(sounds[select], NULL, 0, &channel[0]);
				channel[0]->setVolume(volume);
			}
		}
		else if (key == GLUT_KEY_DOWN)
		{
			if (select < root["musicnum"].asInt()-1)
			{
				select++;
				channel[0]->stop();
				soundSystem->playSound(sounds[select], NULL, 0, &channel[0]);
				channel[0]->setVolume(volume);

			}
		}
	}

	
}
void Timerfunction(int value)
{
	


	glutPostRedisplay(); // 화면 재 출력
	glutTimerFunc(timer, Timerfunction, 1); // 타이머함수 재 설정
}






GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	//glOrtho(-400.0, 400.0, -300.0, 300.0, -500.0, 500.0);
	gluPerspective(60.0, w / h, 1.0, 10000.0);
	//glTranslatef(0.0, 0.0, -300);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0.0, 0.0, -800.0, 0.0, 0.0, -1.0, 0.0, 1.0, 0.0);




	

}

