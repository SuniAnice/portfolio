#include "ServerFunction.h"
#include "../../CommonLibrary/GameObject.h"

#include <ctime>
using namespace std;

#define SERVERPORT 9000
#define BUFSIZE    10000

PlayerCharacter PlayerInfo[MAX_PLAYER];
Bullet BulletInfo[MAX_PLAYER][Max_Bullet];
Obstacle Obstacles[MAX_Obstacles];
SOCKET client_sock[MAX_PLAYER] ;

static int PlayerCount = 0;

bool AABBCollision(GameObject* a, GameObject b)
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

	return true;
}

// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	int retval;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	int len;
	
	int PlayerNo = (int)arg;
	// 클라이언트 정보 얻기

	addrlen = sizeof(clientaddr);
	getpeername(client_sock[PlayerNo], (SOCKADDR*)&clientaddr, &addrlen);

	send(client_sock[PlayerNo], (char*)&Obstacles, sizeof(Obstacle) * MAX_Obstacles, 0);
	
	float x = 0, y = 0, z = 0;
	while (1) {
		if (PlayerCount >= 2)
		{
			retval = recvn(client_sock[PlayerNo], (char*)&(PlayerInfo[PlayerNo]), sizeof(PlayerCharacter), 0);
			if (retval == SOCKET_ERROR) {
				PlayerInfo[PlayerNo].SetHP(0);
				break;
			}
			retval = recvn(client_sock[PlayerNo], (char*)&(BulletInfo[PlayerNo]), sizeof(Bullet) * Max_Bullet, 0);
			if (retval == SOCKET_ERROR) {
				PlayerInfo[PlayerNo].SetHP(0);
				break;
			}

			if (PlayerCount >= 2)
			{
				if (PlayerNo == 0)
				{
					send(client_sock[0], (char*)&PlayerInfo[1], sizeof(PlayerCharacter), 0);
					send(client_sock[0], (char*)&(BulletInfo[1]), sizeof(Bullet) * Max_Bullet, 0);
				}
				else
				{
					send(client_sock[1], (char*)&PlayerInfo[0], sizeof(PlayerCharacter), 0);
					send(client_sock[1], (char*)&(BulletInfo[0]), sizeof(Bullet) * Max_Bullet, 0);
				}
			}
		}
	}

	// closesocket()
	closesocket(client_sock[PlayerNo]);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	return 0;
}



int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKADDR_IN clientaddr;
	int addrlen;
	HANDLE hThread;

	srand((unsigned int)time(0));


	for (int i = 4; i < MAX_Obstacles; i++)
	{
		float randX = ((float)rand() / (float)RAND_MAX) * 50.f - 25.f;
		float randY = ((float)rand() / (float)RAND_MAX) * 50.f - 25.f;
		float depth = 0.f;
		float randSX = ((float)rand() / (float)RAND_MAX) * 1.f + 0.2f;
		float randSY = ((float)rand() / (float)RAND_MAX) * 1.f + 0.2f;
		float velX = 0;
		float velY = 0;
		float accX = 0;
		float accY = 0;
		float mass = 10.f;
		Obstacles[i].SetPosition(randX, randY, depth);
		Obstacles[i].SetSize(randSX, randSY);
		Obstacles[i].SetVel(velX, velY);
		Obstacles[i].SetAcc(accX, accY);
		Obstacles[i].SetMass(mass);
	}
	{
		Obstacles[0].SetPosition(0.f, 25.f, 0.f);
		Obstacles[0].SetSize(51.f, 0.5f);
		Obstacles[0].SetVel(0, 0);
		Obstacles[0].SetAcc(0, 0);
		Obstacles[0].SetMass(10.f);
	}
	{
		Obstacles[1].SetPosition(0.f, -25.f, 0.f);
		Obstacles[1].SetSize(51.f, 0.5f);
		Obstacles[1].SetVel(0, 0);
		Obstacles[1].SetAcc(0, 0);
		Obstacles[1].SetMass(10.f);
	}
	{
		Obstacles[2].SetPosition(25.f, 0.f, 0.f);
		Obstacles[2].SetSize(0.5f, 51.f);
		Obstacles[2].SetVel(0, 0);
		Obstacles[2].SetAcc(0, 0);
		Obstacles[2].SetMass(10.f);
	}
	{
		Obstacles[3].SetPosition(-25.f, 0.f, 0.f);
		Obstacles[3].SetSize(0.5f, 51.f);
		Obstacles[3].SetVel(0, 0);
		Obstacles[3].SetAcc(0, 0);
		Obstacles[3].SetMass(10.f);
	}

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock[PlayerCount] = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock[PlayerCount] == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 스레드 생성
		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)PlayerCount, 0, NULL);
		if (hThread == NULL) { closesocket(client_sock[PlayerCount]); }
		else { CloseHandle(hThread); }
		PlayerCount++;
	}

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}