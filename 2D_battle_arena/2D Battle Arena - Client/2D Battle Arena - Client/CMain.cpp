#include "ServerFunction.h"
#include "NGPGlobals.h"
#include "GameScreen.h"
#include "TitleScreen.h"
#include "Screen.h"
#include "ResultScreen.h"
#include <string.h>
#include <ctime>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Screen* CurrentScreen = nullptr;	// 현재 화면
int ScreenNo = 0;
NGPInputs n_inputs;

GLuint title_texture[2];
GLuint Game_texture[6];

char namebuf[10];
int Namelen = 0;
char SERVERIPinfo[15];
int Serverlen = 0;
int serverinput = 0;

PlayerCharacter* Hero = nullptr;
Bullet HeroBullets[Max_Bullet];

PlayerCharacter Enemy;
Bullet EnemyBullet[Max_Bullet];

Obstacle ObstaclesParam[MAX_Obstacles];

int prevTimeInMillisecond = 0;

SOCKET sock;
int retval;

// 데이터 통신에 사용할 변수
char buf[BUFSIZE];
int len;

void RenderScene(int temp)
{
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	int elapsedTime = currentTime - prevTimeInMillisecond;
	prevTimeInMillisecond = currentTime;
	float ElapsedTimeInSec = (float)elapsedTime / 1000.f;

	NGPInputs tempInputs;
	memcpy(&tempInputs, &n_inputs, sizeof(NGPInputs));

	if (ScreenNo == 1)
	{
		dynamic_cast<MainGame*>(CurrentScreen)->GetBullet(HeroBullets);
		send(sock, (char*)Hero, sizeof(PlayerCharacter), 0);
		send(sock, (char*)HeroBullets, sizeof(Bullet) * Max_Bullet, 0);
		recvn(sock, (char*)&Enemy, sizeof(PlayerCharacter), 0);
		recvn(sock, (char*)&EnemyBullet, sizeof(Bullet) * Max_Bullet, 0);
		dynamic_cast<MainGame*>(CurrentScreen)->SetEnemyBullet(EnemyBullet);

		if (Hero->GetHP() <= 0)
		{
			CurrentScreen = new ResultScreen(false);
			closesocket(sock);
			ScreenNo = 2;
		}
		if (Enemy.GetHP() <= 0)
		{
			CurrentScreen = new ResultScreen(true);
			closesocket(sock);
			ScreenNo = 2;
		}
	}

	if (CurrentScreen->check == 1)
	{
		char *SERVERIP = new char[Serverlen];
		for (int i = 0; i < Serverlen; i++)
		{
			SERVERIP[i] = SERVERIPinfo[i];
		}
		// connect()
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = inet_addr(SERVERIPinfo);
		serveraddr.sin_port = htons(SERVERPORT);
		retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) err_quit("connect()");

		CurrentScreen = new MainGame();

		recvn(sock, (char*)&ObstaclesParam, sizeof(Obstacle) * MAX_Obstacles, 0);
		dynamic_cast<MainGame*>(CurrentScreen)->SetObstacle(ObstaclesParam);

		Hero = dynamic_cast<MainGame*>(CurrentScreen)->GetHero();
		dynamic_cast<MainGame*>(CurrentScreen)->GetBullet(HeroBullets);

		send(sock, (char*)Hero, sizeof(PlayerCharacter), 0);
		send(sock, (char*)HeroBullets, sizeof(Bullet) * Max_Bullet, 0);

		recvn(sock, (char*)&Enemy, sizeof(PlayerCharacter), 0);
		recvn(sock, (char*)&EnemyBullet, sizeof(Bullet) * Max_Bullet, 0);

		dynamic_cast<MainGame*>(CurrentScreen)->SetEnemy(&Enemy);
		dynamic_cast<MainGame*>(CurrentScreen)->SetEnemyBullet(EnemyBullet);

		ScreenNo++;

		Hero->SetName(namebuf);
	}

	CurrentScreen->Update(ElapsedTimeInSec, &tempInputs);
	CurrentScreen->RenderScene();

	glutSwapBuffers(); // double buffering

	glutTimerFunc(16, RenderScene, 16);
}

void Idle(void)
{
}

void MouseInput(int button, int state, int x, int y)
{
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			n_inputs.MouseX = x - ScreenSizeX / 2;
			n_inputs.MouseY = y - ScreenSizeY / 2;
			n_inputs.MOUSE_LEFT = true;
		}
		else
		{
			n_inputs.MOUSE_LEFT = false;
		}
		break;
	}
}

void DragInput(int x, int y)
{
	n_inputs.MouseX = x - ScreenSizeX / 2;
	n_inputs.MouseY = y - ScreenSizeY / 2;
}

void KeyDownInput(unsigned char key, int x, int y)
{
	if (serverinput == 0)
	{
		if (ScreenNo == 0 && Serverlen > 0 && key == '\b')
		{
			SERVERIPinfo[Serverlen - 1] = '\0';
			Serverlen--;
		}
		else if (ScreenNo == 0 && key == '\r' && Serverlen > 0)
		{
			serverinput = 1;
		}
		else if (ScreenNo == 0 && Serverlen < 15)
		{
			SERVERIPinfo[Serverlen] = key;
			Serverlen++;
		}
		else if (ScreenNo == 0 && Serverlen == 15)
		{
			serverinput = 1;
		}
	}
	if (serverinput == 1)
	{
		if (ScreenNo == 0 && Namelen == 0 && key == '\b')
		{
			serverinput = 0;
		}
		else if (ScreenNo == 0 && Namelen > 0 && key == '\b')
		{
			namebuf[Namelen - 1] = '\0';
			Namelen--;
		}
		else if (ScreenNo == 0 && Namelen < 10)
		{
			namebuf[Namelen] = key;
			Namelen++;
		}
	}



	switch (key)
	{
	case 'w' | 'W':
		n_inputs.KEY_W = true;
		break;
	case 'a' | 'A':
		n_inputs.KEY_A = true;
		break;
	case 's' | 'S':
		n_inputs.KEY_S = true;
		break;
	case 'd' | 'D':
		n_inputs.KEY_D = true;
		break;

	}
}

void KeyUpInput(unsigned char key, int x, int y)
{
	
	switch (key)
	{
	case 'w' | 'W':
		n_inputs.KEY_W = false;
		break;
	case 'a' | 'A':
		n_inputs.KEY_A = false;
		break;
	case 's' | 'S':
		n_inputs.KEY_S = false;
		break;
	case 'd' | 'D':
		n_inputs.KEY_D = false;
		break;

	}
}

void SpecialKeyDownInput(int key, int x, int y)
{
	
	switch (key)
	{
	case  GLUT_KEY_UP:
		n_inputs.ARROW_UP = true;
		break;
	case  GLUT_KEY_DOWN:
		n_inputs.ARROW_DOWN = true;
		break;
	case  GLUT_KEY_LEFT:
		n_inputs.ARROW_LEFT = true;
		break;
	case  GLUT_KEY_RIGHT:
		n_inputs.ARROW_RIGHT = true;
		break;
		
	}
}

void SpecialKeyUpInput(int key, int x, int y)
{
	switch (key)
	{
	case  GLUT_KEY_UP:
		n_inputs.ARROW_UP = false;
		break;
	case  GLUT_KEY_DOWN:
		n_inputs.ARROW_DOWN = false;
		break;
	case  GLUT_KEY_LEFT:
		n_inputs.ARROW_LEFT = false;
		break;
	case  GLUT_KEY_RIGHT:
		n_inputs.ARROW_RIGHT = false;
		break;
	}
}


int main(int argc, char* argv[])
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");


	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(ScreenSizeX, ScreenSizeY);
	glutCreateWindow("2D Battle Arena");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glOrtho(-600, 600, -400, 400, -1, 1);

	memset(&n_inputs, 0, sizeof(NGPInputs));

	for (int i = 0; i < 10; i++)
	{
		buf[i] = '/0';
	}
	// 사용할 텍스쳐 로드
	{
		glGenTextures(2, title_texture);
		glBindTexture(GL_TEXTURE_2D, title_texture[0]);
		int width, height, nrChannels;
		unsigned char* data = stbi_load("../../Resource/start_btn.png", &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, nrChannels, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, title_texture[1]);
		data = stbi_load("../../Resource/exit_btn.png", &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, nrChannels, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glGenTextures(6, Game_texture);
		glBindTexture(GL_TEXTURE_2D, Game_texture[0]);
		data = stbi_load("../../Resource/heart.png", &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, nrChannels, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


		glBindTexture(GL_TEXTURE_2D, Game_texture[1]);
		data = stbi_load("../../Resource/aammo.png", &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, nrChannels, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


		glBindTexture(GL_TEXTURE_2D, Game_texture[2]);
		data = stbi_load("../../Resource/tile.png", &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, nrChannels, width, height, 0, GL_RGB,
			GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, Game_texture[3]);
		data = stbi_load("../../Resource/brick.png", &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, nrChannels, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, Game_texture[4]);
		data = stbi_load("../../Resource/Enemy.png", &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, nrChannels, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, Game_texture[5]);
		data = stbi_load("../../Resource/Player.png", &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, nrChannels, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	

	glutDisplayFunc(Idle);
	glutIdleFunc(Idle);
	glutKeyboardFunc(KeyDownInput);
	glutKeyboardUpFunc(KeyUpInput);
	glutMouseFunc(MouseInput);
	glutMotionFunc(DragInput);
	glutSpecialFunc(SpecialKeyDownInput);
	glutSpecialUpFunc(SpecialKeyUpInput);

	srand((unsigned int)time(NULL));

	CurrentScreen = new TitleScreen;
	Hero = new PlayerCharacter(0, 0, 1, 1, 1, 0, 0, 0, 0, 10);

	glutTimerFunc(16, RenderScene, 16);

	glutMainLoop();

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	
	return 0;
}