#pragma once

#define Max_Bullet 50
#define MaxObjects 500
#define ScreenSizeX 1200
#define ScreenSizeY 800
#define MAX_Obstacles 200
#define MAX_PLAYER 2
#define MAX_AMMO 30

#define BULLET_SIZE 0.1
#define BULLET_SPEED 20
#define SHOT_DELAY 0.1
#define RELOAD_DELAY 2.0

#define PLAYER_HP 100

typedef struct NGPInputs
{
	bool ARROW_UP;
	bool ARROW_DOWN;
	bool ARROW_LEFT;
	bool ARROW_RIGHT;

	bool KEY_W;
	bool KEY_A;
	bool KEY_S;
	bool KEY_D;

	bool MOUSE_LEFT;

	int MouseX;
	int MouseY;
};

typedef struct NGPUpdateParams
{
	float forceX;
	float forceY;
};
